#ifndef THREADUDPR_H
#define THREADUDPR_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QDateTime>
#include <QUdpSocket>

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "mydef.h"
#include "frameparser.h"
#include "buffer16.h"
#include "dataupload.h"
#include "cmdftp.h"
#include "cmddata30.h"
#include "myobject.h"
#include "myfiles.h"

class ThreadUdpR : public QThread
{
    Q_OBJECT
public:
    explicit ThreadUdpR(QObject *parent = 0);

    char m_md5s,m_md5e;
    char m_n30;

    char m_stat[16];
    myfiles m_myfiles;

    int m_nRemote;
    int m_nRemoteOK;
    int m_nRemoteErr;
    char m_cmdD3;

    int m_aCmd1stID8[256];
    int m_nCmd1st;
    bool m_only25;
    void setOnly25(bool b);
    bool isID25(int IDdes);
    bool m_isArm;
    QDateTime m_dtSet;

    int getBAStatFile();
    int mkBAStat();
    int mkBAStatUart();
    int mkBAStatUartN(int n,char id8);
    int mkBAStatN(int n);
    int m_sumStat;
    int m_lenStatAdd;

    QByteArray m_baStat;// no id32 header
    QByteArray m_baStatUart;
    QByteArray m_baStatFile;

    QWaitCondition m_wait;
    QMutex m_mutex;

    int getID32(int src2821,int des2013,int type1209, int info8 ,int id71);

    dataUpload m_dataUpload;
    cmdftp m_ftp;
    cmdData30 m_data30;


    QQueue<char> m_q;
    void testNewFrame();

    frameparser m_parser;

    buffer16 m_buf16;// replace with ba16
    QByteArray m_ba16;//uart buffer

    st_frame m_stFrame;
    QByteArray m_baUart16;// valid frame

    QByteArray m_baFrameSingle;
    QByteArray m_baFrameComplex;
    char m_pbuf16[16];
    void print16();
    void print16ba();
    void printBA(QByteArray ba);

    bool hasValid16();  // check m_ba16
    void toStFrame(QByteArray ba,struct st_frame *pStFrame);

    void parseID32type();// parse m_stFrame
    void doSingle();// cmd single frame
    void do1st();// cmd.complex 1st frame
    void do2nd();// cmd.complex 2nd.3.4... frame
    void doTimeSync();

    void doRemote();
    void doShort5();

    void doC8(bool bOK,char idv,char id8);
    void do51();
    void do52();// shutdown
    void do53();// reboot;
    void do5140();
    void do514001();
    void do514002();// run testbash.sh
    void doFtpAck();

protected:
    void run();

signals:
    void sigF16(QByteArray);
    void sigSTDframe(QByteArray);
    void sigTest();
    void sigUart(QByteArray);
    void sigOnly25(bool);
    void sig7755(QByteArray);
    void sigFtp(QByteArray);
    void sigInt(int);
    void sigStartThreadFtp();
    void sigReleaseFtp();

    void sigRun(QString);
    void sigRumCmd(QString);

public slots:
    void newChar(char ch);
    void slotFile8(char b8);
    void slotRunning8(char b8);
    void slotMD5(QByteArray ba);

    void slot30();

};

#endif // THREADUDPR_H
