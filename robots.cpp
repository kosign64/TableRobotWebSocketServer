#include "robots.h"
#include "comport.h"
#include <QDebug>

Robots::Robots(QObject *parent) : QObject(parent),
    stop_(false)
{
    if(port_.openPort("/dev/ttyUSB0") > 0)
    {
        qDebug() << "Port opened";
    }
    else
    {
        qDebug() << "Error: Can't open port";
    }
}

Robots::~Robots()
{

}

void Robots::process()
{
    unsigned char byte;
    unsigned char dotNumber;
    ComStatus status;
    points_.clear();
    qDebug() << "Thread started";
    while(!stop_)
    {
        byte = port_.readByte(status);
        if(status < 0) continue;
        if(byte == 255)
        {
            byte = port_.readByte(status);
            if(status < 0) continue;
            dotNumber = byte;
            points_.resize(dotNumber);
            for(int i = 0; i < points_.size(); ++i)
            {
                byte = port_.readByte(status);
                if(status < 0) continue;
                int x = (int)byte * 100;
                byte = port_.readByte(status);
                x += byte;
                byte = port_.readByte(status);
                int y = (int)byte * 100;
                byte = port_.readByte(status);
                y += byte;
                Point2D point;
                point.x = x;
                point.y = y;
                points_[i] = point;
            }
            emit sendPoints(points_);
        }
    }
}

void Robots::sendWheels(QVector <RobotData> data)
{
    qSort(data.begin(), data.end(), [](const RobotData &d1,
          const RobotData &d2) {return d1.number < d2.number;});
    qDebug() << "0xFF" << data.size();
    port_ << 0xFF;
    int j = 0;
    qDebug() << data.back().number;
    for(int i = 0; i < (data.back().number); ++i)
    {
        if(data[j].number == (i + 1))
        {
            qDebug() << data[j].cByte;
            port_ << data[j].cByte;
            j++;
        }
        else
        {
            qDebug() << "N";
            port_ << (unsigned char)0;
        }
    }
}

void Robots::sendWheels(QMap <uint8_t, uint8_t> data)
{
    port_ << 0xFF;
    for(int i = 0; i < data.lastKey(); ++i)
    {
        if(data.contains(i + 1))
        {
            qDebug() << i + 1 << data[i + 1];
            port_ << data[i + 1];
        }
        else
        {
            port_ << (unsigned char)0;
            qDebug() << i + 1 << "No";
        }
    }
    port_ << 0xFE;
}
