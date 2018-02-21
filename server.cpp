#include "server.h"
#include "robots.h"
#include <QWebSocket>
#include <QWebSocketServer>
#include <QAbstractSocket>
#include <QThread>
#include <stdint-gcc.h>

Server::Server(QObject *parent) : QObject(parent),
    started_(false)
{
    qRegisterMetaType<PointVector>("PointVector");
    server_ = new QWebSocketServer(QStringLiteral("Robot Server"),
                                   QWebSocketServer::NonSecureMode,
                                   this);
    comThread_ = new QThread;
    robots_ = new Robots;
    robots_->moveToThread(comThread_);
    connect(server_, &QWebSocketServer::newConnection, this,
            &Server::newConnection);
    connect(robots_, &Robots::sendPoints, this, &Server::getPoints);
    connect(comThread_, &QThread::started, robots_,
            &Robots::process);
    comThread_->start();
    startTimer(1000 / 100);
}

Server::~Server()
{
    robots_->stopProcess();
    comThread_->quit();
    comThread_->wait(1000);
}

void Server::newConnection()
{
    if(sockets_.size() < MAX_CONNECTIONS)
    {
        QWebSocket *socket = server_->nextPendingConnection();
        connect(socket, &QWebSocket::textMessageReceived, this, &Server::readyRead);
        connect(socket, &QWebSocket::disconnected, this,
                &Server::disconnected);
        sockets_ << socket;

        sendPoints(socket);
    }
    else
    {
        QWebSocket *unknownSocket = server_->nextPendingConnection();
        unknownSocket->close();
        delete unknownSocket;
        qDebug() << "Something else trying to connect";
    }
}

bool Server::startServer()
{
    if(!started_)
    {
        if(server_->listen(QHostAddress::Any, 3337))
        {
            started_ = true;
            qDebug() << "Server started";
            return true;
        }
        else
        {
            qDebug() << "Error: can't start server";
            return false;
        }
    }
    return true;
}

void Server::disconnected()
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Socket disconnected" << socket->peerAddress().toString();
    if(socket)
    {
        sockets_.removeAll(socket);
        socket->deleteLater();
    }
}

void Server::readyRead(QString message)
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    QStringList controlsList = message.split(" ");
    for(int i = 0; i < controlsList.size(); i += 2)
    {
        int robotNumber = controlsList[i].toInt();
        uint8_t controlByte = controlsList[i + 1].toInt();
        data_[robotNumber] = controlByte;
    }

    sendPoints(socket);
}

void Server::timerEvent(QTimerEvent *)
{
    if(!data_.isEmpty())
    {
        robots_->sendWheels(data_);
        data_.clear();
    }
}

void Server::sendPoints(QWebSocket *socket) const
{
    QString pointsJson = "[";
    for(auto &point : points_)
    {
        pointsJson += QString::number(point.x) + QString(",");
        pointsJson += QString::number(point.y) + QString(",");
    }
    if(!points_.empty())
    {
        pointsJson = pointsJson.left(pointsJson.length() - 1);
    }
    pointsJson += "]";
    socket->sendTextMessage(pointsJson);
}
