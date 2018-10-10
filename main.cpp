#include <QCoreApplication>

#include <QDebug>

#include "threadudpr.h"
#include "udpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ThreadUdpR thudpr;
    UdpServer *m_pus=new UdpServer();

    m_pus->init();

    QObject::connect(&thudpr,SIGNAL(finished()),&a,SLOT(quit()));
    QObject::connect(m_pus,SIGNAL(newChar(char)),&thudpr,SLOT(newChar(char)));

    thudpr.start();

    qDebug("hello");

    thudpr.wait();
    return a.exec();
}
