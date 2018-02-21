#-------------------------------------------------
#
# Project created by QtCreator 2015-07-10T15:28:51
#
#-------------------------------------------------

QT       += core websockets

QT       -= gui

TARGET = Server
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += c++11

TEMPLATE = app


SOURCES += main.cpp \
    comport.cpp \
    robots.cpp \
    server.cpp

HEADERS += \
    comport.h \
    robots.h \
    server.h
