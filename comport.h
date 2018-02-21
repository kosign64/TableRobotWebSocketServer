#ifndef COMPORT_H
#define COMPORT_H

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#endif

#ifdef __linux__
#include <termios.h>
#endif

#include <vector>
#include <string>

using namespace std;

enum ComStatus
{
    PORT_OPEN_OK = 1,
    PORT_OPEN_ERROR = -1,
    BYTE_READ_OK = 2,
    BYTE_READ_TIMEOUT = -2,
    PORT_CLOSED = -3
};

enum ComSpeed
{
    COM4800,
    COM9600,
    COM115200
};

class ComPort
{
public:
    ComPort();
    ~ComPort();
    ComStatus openPort(int number, ComSpeed speed = COM115200);
    ComStatus openPort(string name, ComSpeed speed = COM115200);
    void closePort();
    bool isOpened() {return opened_;}
    void sendByte(unsigned char byte);
    unsigned char readByte(ComStatus &status);
    ComPort &operator << (unsigned char byte);
    ComPort &operator << (const char *string);
    ComPort &operator >> (unsigned char &byte);
    static vector<string> getAvailablePorts();

private:
    bool opened_;
#if defined(_WIN32) || defined(WIN32)
    HANDLE port_;
    COMMTIMEOUTS ct_;
    DCB *dcb_;
    COMMCONFIG *cf_;
    DWORD sz_;
    DWORD bc_;
#endif

#ifdef __linux__
    termios portOptions_;
    int port_;
#endif
};

#endif // COMPORT_H
