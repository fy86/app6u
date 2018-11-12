#ifndef THREADUDPR_H
#define THREADUDPR_H

#include <QObject>
#include <QThread>
#include <QQueue>

#include "mydef.h"
#include "frameparser.h"
#include "buffer16.h"
#include "dataupload.h"

class ThreadUdpR : public QThread
{
    Q_OBJECT
public:
    explicit ThreadUdpR(QObject *parent = 0);

    dataUpload m_dataUpload;

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

protected:
    void run();

signals:
    void sigF16(QByteArray);
    void sigSTDframe(QByteArray);
    void sigTest();

public slots:
    void newChar(char ch);

};

#endif // THREADUDPR_H
