#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>

class UdpServer : public QObject
{
    Q_OBJECT
public:
    explicit UdpServer(QObject *parent = 0);

    QUdpSocket *m_pus;

    void init();
    void test();
signals:
    void newChar(char ch);

public slots:
    void readData();
    void slot7755(QByteArray ba);

};

#endif // UDPSERVER_H
