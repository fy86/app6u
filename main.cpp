#include <QCoreApplication>

#include <QDebug>

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "threadudpr.h"
#include "udpserver.h"
#include "frameparser.h"
#include "threaduart.h"
#include "threaduartsend.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //qRegisterMetaType<st_frame>();

    ThreadUdpR thudpr;
    UdpServer us;
    frameparser parser;

    threaduart thuart;
    threadUartSend thuartSend;

    QObject::connect(&thudpr,SIGNAL(finished()),&a,SLOT(quit()));
    QObject::connect(&us,SIGNAL(newChar(char)),&thudpr,SLOT(newChar(char)));
    QObject::connect(&thudpr,SIGNAL(sigF16(QByteArray)),&parser,SLOT(slotTest(QByteArray)));
    QObject::connect(&thuart,SIGNAL(newChar(char)),&thudpr,SLOT(newChar(char)));

    //QObject::connect(&thudpr,SIGNAL(sigTest()),&thuart,SLOT(slotSendTest()));

    // enqueue
    QObject::connect(&thudpr,SIGNAL(sigUart(QByteArray)),&thuart,SLOT(slotEnQ(QByteArray)));

    QObject::connect(&thuartSend,SIGNAL(sigQSend()),&thuart,SLOT(slotSendQ()));
    QObject::connect(&thuart,SIGNAL(sigWakeSend()),&thuartSend,SLOT(slotWakeSend()));

    thuart.start();
    thudpr.start();
    thuartSend.start();

    //qDebug("app6u start");
    syslog(LOG_INFO," ------ app6u start ------");

    // !!!!!!! will block udp recv
    //thudpr.wait();
    return a.exec();
}
