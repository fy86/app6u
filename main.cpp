#include <QCoreApplication>

#include <QDebug>

#include "threadudpr.h"
#include "udpserver.h"
#include "frameparser.h"
#include "threaduart.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //qRegisterMetaType<st_frame>();

    ThreadUdpR thudpr;
    UdpServer us;
    frameparser parser;

    threaduart thuart;

    QObject::connect(&thudpr,SIGNAL(finished()),&a,SLOT(quit()));
    QObject::connect(&us,SIGNAL(newChar(char)),&thudpr,SLOT(newChar(char)));
    QObject::connect(&thudpr,SIGNAL(sigF16(QByteArray)),&parser,SLOT(slotTest(QByteArray)));

    thuart.start();
    thudpr.start();

    qDebug("app6u start");

    // !!!!!!! will block udp recv
    //thudpr.wait();
    return a.exec();
}
