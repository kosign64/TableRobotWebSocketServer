#include "comport.h"
#include <QDebug>
#ifdef __linux__
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#endif

ComPort::ComPort() : opened_(false)
{
#if defined(_WIN32) || defined(WIN32)
    dcb_=(DCB*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DCB));
    dcb_->DCBlength = sizeof(DCB);
    BuildCommDCB(TEXT("parity=N data=8 stop=1"), dcb_);
    ct_.ReadIntervalTimeout = 0;
    ct_.ReadTotalTimeoutMultiplier = 0;
    ct_.ReadTotalTimeoutConstant = 100;
    ct_.WriteTotalTimeoutMultiplier = ct_.WriteTotalTimeoutConstant = 0;
    dcb_->BaudRate = CBR_9600;
#endif
}

ComPort::~ComPort()
{
    closePort();
}

ComStatus ComPort::openPort(int number, ComSpeed speed)
{
#if defined(_WIN32) || defined(WIN32)
    string portName = "\\\\.\\COM";
    portName += to_string(number);
    return openPort(portName, speed);
#endif

#ifdef __linux__
    string portName = "/dev/ttyS";
    portName += to_string(number);
    return openPort(portName, speed);
#endif
}

ComStatus ComPort::openPort(string name, ComSpeed speed)
{
    closePort();
#if defined(_WIN32) || defined(WIN32)
    switch(speed)
    {
    case COM4800:
        dcb_->BaudRate = CBR_4800;
        break;
    case COM9600:
        dcb_->BaudRate = CBR_9600;
        break;
    case COM115200:
        dcb_->BaudRate = CBR_115200;
        break;
    default:
        dcb_->BaudRate = CBR_115200;
    }

    string portName = "\\\\.\\";
    portName += name;

    port_ = CreateFileA(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                       NULL, OPEN_EXISTING, 0, NULL);

    if(port_ == INVALID_HANDLE_VALUE)
    {
        return PORT_OPEN_ERROR;
    }

    SetCommState(port_, dcb_);
    SetCommTimeouts(port_, &ct_);
    HeapFree(GetProcessHeap(), 0, dcb_);
    PurgeComm(port_, PURGE_TXCLEAR | PURGE_RXCLEAR);
    opened_ = true;

    return PORT_OPEN_OK;
#endif

#ifdef __linux__
    port_ = open(name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

    if(port_ == -1)
    {
        return PORT_OPEN_ERROR;
    }

    tcgetattr(port_, &portOptions_);

    cfmakeraw(&portOptions_);

    speed_t s;
    switch(speed)
    {
    case COM4800:
        s = B4800;
        break;
    case COM9600:
        s = B9600;
        break;
    case COM115200:
        s = B115200;
        break;
    default:
        s = B115200;
    }
    cfsetispeed(&portOptions_, s);
    cfsetospeed(&portOptions_, s);

    portOptions_.c_cflag &= ~PARENB;
    portOptions_.c_cflag &= ~CSTOPB;
    portOptions_.c_cflag &= ~CSIZE;
    portOptions_.c_cflag |= CS8;

    portOptions_.c_cflag |= (CLOCAL | CREAD);

    portOptions_.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                             INLCR | PARMRK | INPCK | ISTRIP | IXON);

    portOptions_.c_oflag = 0;

    portOptions_.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    portOptions_.c_cc[VMIN] = 0;
    portOptions_.c_cc[VTIME] = 1;

    tcsetattr(port_, TCSANOW, &portOptions_);
    fcntl(port_, F_SETFL, 0);
    opened_ = true;

    return PORT_OPEN_OK;
#endif
}

void ComPort::closePort()
{
    if(opened_)
    {
#if defined(_WIN32) || defined(WIN32)
        CloseHandle(port_);
#endif

#ifdef __linux__
        close(port_);
#endif
        opened_ = false;
    }
}

unsigned char ComPort::readByte(ComStatus &status)
{
#if defined(_WIN32) || defined(WIN32)
    unsigned char byte;
    if(opened_)
    {
        ReadFile(port_, &byte, 1, &bc_, NULL);
    }
    else
    {
        status = PORT_CLOSED;
        return byte;
    }
    if(!bc_)
    {
        status = BYTE_READ_TIMEOUT;
    }
    else
    {
        status = BYTE_READ_OK;
    }

    return byte;
#endif

#ifdef __linux__
    unsigned char byte;
    int count = 0;
    if(opened_)
    {
        count = read(port_, &byte, 1);
    }
    else
    {
        status = PORT_CLOSED;
        return byte;
    }
    if(count == 0)
    {
        status = BYTE_READ_TIMEOUT;
    }
    else if(count == -1)
    {
        status = PORT_OPEN_ERROR;
    }
    else
    {
        status = BYTE_READ_OK;
    }

    return byte;
#endif
}

void ComPort::sendByte(unsigned char byte)
{
#if defined(_WIN32) || defined(WIN32)
    WriteFile(port_, &byte, 1, &bc_, NULL);
#endif

#ifdef __linux__
    write(port_, &byte, 1);
#endif
}

ComPort &ComPort::operator <<(unsigned char byte)
{
#if defined(_WIN32) || defined(WIN32)
    WriteFile(port_, &byte, 1, &bc_, NULL);
    return *this;
#endif

#ifdef __linux__
    write(port_, &byte, 1);
    return *this;
#endif
}

ComPort &ComPort::operator <<(const char *string)
{
#if defined(_WIN32) || defined(WIN32)
    while(*string)
    {
        WriteFile(port_, string, 1, &bc_, NULL);
        ++string;
    }
    return *this;
#endif

#ifdef __linux__
    while(*string)
    {
        write(port_, string, 1);
        ++string;
    }
    return *this;
#endif
}

ComPort &ComPort::operator >> (unsigned char &byte)
{
    if(opened_)
    {
        ComStatus status = BYTE_READ_TIMEOUT;
        while(status != BYTE_READ_OK)
        {
            byte = readByte(status);
        }
    }

    return *this;
}

vector<string> ComPort::getAvailablePorts()
{
    vector<string> ports;

#if defined(_WIN32) || defined(WIN32)

    for(int i = 1; i < 16; ++i)
    {
        string portName = "\\\\.\\COM";
        portName += to_string(i);
        HANDLE p = CreateFileA(portName.c_str(), GENERIC_READ | GENERIC_WRITE,
                               0, NULL, OPEN_EXISTING, 0, NULL);
        if(p != INVALID_HANDLE_VALUE)
        {
            ports.push_back(portName.substr(4, portName.length() - 4));
            CloseHandle(p);
        }
    }

#endif

#ifdef __linux__

    for(int i = 0; i < 10; ++i)
    {
        int p;
        string portName = "/dev/ttyS";
        string portNameUsb = "/dev/ttyUSB";
        portName += to_string(i);
        portNameUsb += to_string(i);
        p = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if(p != -1)
        {
            ports.push_back(portName);
            close(p);
        }
        p = open(portNameUsb.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if(p != -1)
        {
            ports.push_back(portName);
            close(p);
        }
    }

#endif

    return ports;
}
