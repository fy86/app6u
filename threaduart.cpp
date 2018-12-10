#include "threaduart.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>

#define BAUDRATE B460800
#define MODEMDEVICE "/dev/ttymxc2"

threaduart::threaduart(QObject *parent) :
    QThread(parent)
{
    m_isArm = false;


    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
    if (fd <0) return;
    qDebug(" open can.uart ok");

    tcgetattr(fd,&oldtio); /* save current port settings */

    bzero(&newtio, sizeof(newtio));
    //newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;///////////////  rm rts.cts
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* 1<==5 blocking read until 5 chars received */

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

    m_isArm = true;

#if 0
    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf,255);   /* returns after 5 chars have been input */
      buf[res]=0;               /* so we can printf... */
      printf(":%s:%d\n", buf, res);
      if (buf[0]=='z') STOP=TRUE;
    }
    tcsetattr(fd,TCSANOW,&oldtio);
#endif
}

void threaduart::init()
{
    m_pusSend = new QUdpSocket(this);

    m_pusRcv = new QUdpSocket(this);
    //QHostAddress gAddr=QHostAddress("239.255.43.21");

    m_pusRcv->bind(QHostAddress::LocalHost,7756,QUdpSocket::ShareAddress);
    //m_pusRcv->joinMulticastGroup(QHostAddress("239.255.43.21"));

    connect(m_pusRcv, SIGNAL(readyRead()),this, SLOT(readData()));

}
void threaduart::readData()
{
    //qDebug(" udp read data");
    while (m_pusRcv->hasPendingDatagrams()) {
        //qDebug(" udp read data while");
        QByteArray datagram;
        datagram.resize(m_pusRcv->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_pusRcv->readDatagram(datagram.data(), datagram.size(),&sender, &senderPort);

        for(int i=0;i<datagram.size();i++){
            qDebug(" thread.uart udp rcv : %02x",0x0ff & datagram.at(i));
            //emit newChar(datagram.at(i));
        }
    }

}
int threaduart::sumChar(char *p, int len)
{
    int ret=0;
    int i;
    for(i=0;i<len;i++){
        ret += 0x0ff & p[i];
    }
    ret &= 0x0ff;

    return ret;
}

void threaduart::slotSendTest()
{
    QByteArray ba;
    char sum;

    if(!m_isArm)return;

    ba.clear();
    ba.append((char)FRAME_1ST_CHAR);ba.append((char)FRAME_2ND_CHAR);

    ba.append((char)0x02);ba.append((char)0x20);ba.append((char)0x02);ba.append((char)0x00);

    ba.append((char)0x99);ba.append((char)0xff);ba.append((char)0x02);ba.append((char)0x01);
    ba.append((char)0xaa),ba.append((char)0xaa);ba.append((char)0xaa);ba.append((char)0x99);

    sum = sumChar(ba.data(),14);
    sum=0;
    ba.append(sum);
    ba.append((char)FRAME_ENDING_CHAR);

    write(fd,ba.data(),16);
    qDebug("  uart.slot.send.test   sum:%02x",sum);
}
void threaduart::slotEnQ(QByteArray ba)
{
    m_q.enqueue(ba);
    emit sigWakeSend();
}

void threaduart::slotSend(QByteArray ba)
{
    int i;
    for(i=0;i<ba.size();i++){
        qDebug(" slot.uart.send : %02X ",ba.at(i));
    }
    qDebug(" -- slot.uart.send --");

    if(!m_isArm)return;
    if(ba.size()<1)return;
    write(fd,ba.data(),ba.size());

}
void threaduart::slotSendQ()
{
    QByteArray ba;

    if(m_q.isEmpty()) return;
    ba=m_q.dequeue();

    m_lib.printBA("baToSend",ba);

    if(!m_isArm)return;
    if(ba.size()<1)return;
    write(fd,ba.data(),ba.size());
}

void threaduart::run()
{
    int i;
    int len;
    int sum=0;
    if(!m_isArm)return;
    for(;;){
        len = read(fd,m_buf,100);
        if(len>0){
            if(len>14){
                sum=sumChar(m_buf,14);
            }
            qDebug(" 2 !!!!!!! can.uart.read %d   sum:0x%02x",len,sum);
            for(i=0;i<len;i++){
                emit newChar(m_buf[i]);
            }

            //QTimer::singleShot(1000,this,SLOT(slotSendTest()));

            //m_pusSend->writeDatagram(buf,len,QHostAddress::LocalHost,7755);
            //m_pusSend->writeDatagram(buf,len,QHostAddress::LocalHost,7756);
            //m_pusSend->writeDatagram(buf,len,QHostAddress("239.255.43.21"),7755);
            //m_pusSend->writeDatagram(buf,len,QHostAddress("239.255.43.21"),7756);
        }
    }

}


#if 0
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

main()
{
  int fd,c, res;
  struct termios oldtio,newtio;
  char buf[255];

  fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
  if (fd <0) {perror(MODEMDEVICE); exit(-1); }

  tcgetattr(fd,&oldtio); /* save current port settings */

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */

  tcflush(fd, TCIFLUSH);
  tcsetattr(fd,TCSANOW,&newtio);


  while (STOP==FALSE) {       /* loop for input */
    res = read(fd,buf,255);   /* returns after 5 chars have been input */
    buf[res]=0;               /* so we can printf... */
    printf(":%s:%d\n", buf, res);
    if (buf[0]=='z') STOP=TRUE;
  }
  tcsetattr(fd,TCSANOW,&oldtio);
}

#endif
