#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QList>
#include "robots.h"

class QWebSocketServer;
class QWebSocket;
class QThread;

/*
 * PROTOCOL
 *
 * Server sends:
 * JSON array of points' coordinates [x1, y1, x2, y2, ...]
 *
 * Server receives:
 * String containg robot numbers and control bytes: "number controlByte number controlByte..."
 * Example: "1 64 2 110"
 */

typedef QVector<Point2D> PointVector;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    ~Server();
    bool startServer();

protected:
    void timerEvent(QTimerEvent*);

private:
    QWebSocketServer *server_;
    QList <QWebSocket *> sockets_;
    Robots *robots_;
    QThread *comThread_;
    bool started_;
    PointVector points_;
    QMap <uint8_t, uint8_t> data_;

    static const int MAX_CONNECTIONS = 10;

    void sendPoints(QWebSocket *socket) const;

private slots:
    void newConnection();
    void readyRead(QString message);
    void disconnected();
    void getPoints(PointVector points) {points_ = points;}

signals:

public slots:
};

#endif // SERVER_H
