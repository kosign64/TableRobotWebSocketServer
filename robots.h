#ifndef ROBOTS_H
#define ROBOTS_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <comport.h>
#include <stdint-gcc.h>

/*
 * PROTOCOL
 *
 * --Server sends:--
 * 0xFF
 * robot1 control byte
 * robot2 control byte
 * ...
 * 0xFE
 *
 * --MCU sends:--
 * 0xFF
 * number of found points (uint8_t)
 * point1 x / 100 (uint8_t)
 * point1 x % 100 (uint8_t)
 * point1 y / 100 (uint8_t)
 * point1 y % 100 (uint8_t)
 * ...
 */

#define IGNORE_NUMBER 255

struct Point2D
{
    uint16_t x;
    uint16_t y;
};

struct RobotData
{
    uint8_t number;
    uint8_t cByte;
};

typedef QVector<Point2D> PointVector;

class Robots : public QObject
{
    Q_OBJECT
public:
    explicit Robots(QObject *parent = 0);
    ~Robots();
    void sendWheels(QVector<RobotData> data);
    void sendWheels(QMap <uint8_t, uint8_t> data);
    void stopProcess() {stop_ = true;}

private:
    ComPort port_;
    bool stop_;
    PointVector points_;

signals:
    void sendPoints(PointVector p);

public slots:
    void process();
};

#endif // ROBOTS_H
