#include <QDebug>

#include "udpserver.h"

UdpServer::UdpServer(QObject *parent) :
    QObject(parent)
{
    //init();
}

void UdpServer::init()
{
    m_pus = new QUdpSocket(this);

    //m_pus->bind(QHostAddress::LocalHost, 7755);

    connect(m_pus, SIGNAL(readyRead()),this, SLOT(readData()));
    m_pus->bind(QHostAddress::LocalHost, 7755);
}

void UdpServer::readData()
{
    while (m_pus->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_pus->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_pus->readDatagram(datagram.data(), datagram.size(),&sender, &senderPort);

        for(int i=0;i<datagram.size();i++){
            qDebug(" udp server : %02x",0x0ff & datagram.at(i));
            emit newChar(datagram.at(i));
        }
        //processTheDatagram(datagram);
    }



}
