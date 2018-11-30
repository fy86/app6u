#-------------------------------------------------
#
# Project created by QtCreator 2018-10-09T15:58:18
#
#-------------------------------------------------

QT       += core
QT       += network

QT       -= gui

TARGET = app6u
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    udpserver.cpp \
    threadudpr.cpp \
    frameparser.cpp \
    buffer16.cpp \
    dataupload.cpp \
    crc32.cpp \
    threaduart.cpp \
    threaduartsend.cpp \
    mylib.cpp

HEADERS += \
    udpserver.h \
    threadudpr.h \
    mydef.h \
    frameparser.h \
    buffer16.h \
    dataupload.h \
    crc32.h \
    threaduart.h \
    threaduartsend.h \
    mylib.h
