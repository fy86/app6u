#include <QDebug>

#include "udpserver.h"

UdpServer::UdpServer(QObject *parent) :
    QObject(parent)
{
    init();
    //test();
}
void UdpServer::test()
{
    QByteArray ba;
    ba.append('a');
    qDebug(" bytearray len(1): %d",ba.size());
    ba.append('b');
    qDebug(" bytearray len(2): %d",ba.size());
    ba.append((char)0x00);
    qDebug(" bytearray len(3): %d",ba.size());
    ba.append('c');
    qDebug(" bytearray len(4): %d %s",ba.size(),ba.data());
    ba.prepend('c');
    qDebug(" bytearray len(4): %d %s",ba.size(),ba.data());
}

void UdpServer::init()
{
    m_pus = new QUdpSocket(this);
    //QHostAddress gAddr=QHostAddress("239.255.43.21");

    m_pus->bind(QHostAddress::LocalHost,7755,QUdpSocket::ShareAddress);
    m_pus->joinMulticastGroup(QHostAddress("239.255.43.21"));

    connect(m_pus, SIGNAL(readyRead()),this, SLOT(readData()));
}

void UdpServer::readData()
{
    //qDebug(" udp read data");
    while (m_pus->hasPendingDatagrams()) {
        //qDebug(" udp read data while");
        QByteArray datagram;
        datagram.resize(m_pus->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_pus->readDatagram(datagram.data(), datagram.size(),&sender, &senderPort);

        for(int i=0;i<datagram.size();i++){
            //qDebug(" udp server : %02x",0x0ff & datagram.at(i));
            emit newChar(datagram.at(i));
        }
        //processTheDatagram(datagram);
    }

}
