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
    threadudpr.cpp

HEADERS += \
    udpserver.h \
    threadudpr.h
