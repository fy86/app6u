#include <QCoreApplication>

#include <QDebug>

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "myobject.h"
#include "threadudpr.h"
#include "udpserver.h"
#include "frameparser.h"
#include "threaduart.h"
#include "threaduartsend.h"
#include "threadftp.h"
#include "threadstat.h"

#include "myfiles.h"

//int myobject::snLog=0;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //qRegisterMetaType<st_frame>();

    ThreadUdpR thudpr;
    UdpServer us;
    frameparser parser;

    threaduart thuart;
    threadUartSend thuartSend;
    threadftp thFtp;
    threadStat thStat;
    myfiles files;

    QObject::connect(&thudpr,SIGNAL(finished()),&a,SLOT(quit()));
    QObject::connect(&us,SIGNAL(newChar(char)),&thudpr,SLOT(newChar(char)));
    QObject::connect(&thudpr,SIGNAL(sigF16(QByteArray)),&parser,SLOT(slotTest(QByteArray)));
    QObject::connect(&thuart,SIGNAL(newChar(char)),&thudpr,SLOT(newChar(char)));

    //QObject::connect(&thudpr,SIGNAL(sigTest()),&thuart,SLOT(slotSendTest()));

    // enqueue
    QObject::connect(&thudpr,SIGNAL(sigUart(QByteArray)),&thuart,SLOT(slotEnQ(QByteArray)));

    QObject::connect(&thuartSend,SIGNAL(sigQSend()),&thuart,SLOT(slotSendQ()));
    QObject::connect(&thuart,SIGNAL(sigWakeSend()),&thuartSend,SLOT(slotWakeSend()));

    QObject::connect(&thudpr,SIGNAL(sigFtp(QByteArray)),&thFtp,SLOT(slotAddBA(QByteArray)));
    QObject::connect(&thudpr,SIGNAL(sigInt(int)),&thFtp,SLOT(slotAddn(int)));
    QObject::connect(&thudpr,SIGNAL(sigStartThreadFtp()),&thFtp,SLOT(slotStart()));
    QObject::connect(&thudpr,SIGNAL(sigReleaseFtp()),&thFtp,SLOT(slotRelease()));

    QObject::connect(&thFtp,SIGNAL(sigUart(QByteArray)),&thuart,SLOT(slotEnQ(QByteArray)));

    QObject::connect(&thStat,SIGNAL(sigFile8(char)),&thudpr,SLOT(slotFile8(char)));
    QObject::connect(&thStat,SIGNAL(sigRunning8(char)),&thudpr,SLOT(slotRunning8(char)));
    QObject::connect(&thudpr,SIGNAL(sigRun(QString)),&files,SLOT(slotRunFile(QString)));
    QObject::connect(&thudpr,SIGNAL(sigRumCmd(QString)),&files,SLOT(slotRunCmd(QString)));

    //QObject::connect(&thudpr,SIGNAL(sig7755(QByteArray)),&thuart,SLOT(slot7755(QByteArray)));

    files.restoreRunning();

    thuart.start();
    thudpr.start();
    thuartSend.start();
    thStat.start();

    //qDebug("app6u start");
    syslog(LOG_INFO," ------ app6u start ------ %s",QDir::currentPath().toLatin1().data());

    // !!!!!!! will block udp recv
    //thudpr.wait();
    return a.exec();
}
