#include <QCoreApplication>

#include <QDebug>

#include "threadudpr.h"
#include "udpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ThreadUdpR thudpr;
    UdpServer us;

    QObject::connect(&thudpr,SIGNAL(finished()),&a,SLOT(quit()));
    QObject::connect(&us,SIGNAL(newChar(char)),&thudpr,SLOT(newChar(char)));

    thudpr.start();

    qDebug("app6u start");

    // !!!!!!! will block udp recv
    //thudpr.wait();
    return a.exec();
}
