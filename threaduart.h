#ifndef THREADUART_H
#define THREADUART_H

#include <QObject>
#include <QThread>
#include <QUdpSocket>
#include <QTimer>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#include "mydef.h"


class threaduart : public QThread
{
    Q_OBJECT
public:
    explicit threaduart(QObject *parent = 0);

    void init();
    int sumChar(char *p,int len);

    bool m_isArm;//
    QUdpSocket *m_pusSend;
    QUdpSocket *m_pusRcv;


    // serial howto
    int fd,c, res;
    struct termios oldtio,newtio;
    char m_buf[255];




protected:
    void run();

signals:
    void newChar(char ch);

public slots:
    void readData();
    void slotSendTest();
    void slotSend(QByteArray ba);

};

#endif // THREADUART_H
