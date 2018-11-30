#ifndef THREADUART_H
#define THREADUART_H

#include <QObject>
#include <QThread>
#include <QUdpSocket>
#include <QTimer>
#include <QQueue>


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#include "mydef.h"
#include "mylib.h"


class threaduart : public QThread
{
    Q_OBJECT
public:
    explicit threaduart(QObject *parent = 0);

    myLib m_lib;

    void init();
    int sumChar(char *p,int len);

    bool m_isArm;//
    QUdpSocket *m_pusSend;
    QUdpSocket *m_pusRcv;


    // serial howto
    int fd,c, res;
    struct termios oldtio,newtio;
    char m_buf[255];

    QQueue<QByteArray> m_q;


protected:
    void run();

signals:
    void newChar(char ch);
    void sigWakeSend();

public slots:
    void readData();
    void slotSendTest();
    void slotSend(QByteArray ba);
    void slotEnQ(QByteArray ba);
    void slotSendQ();

};

#endif // THREADUART_H
