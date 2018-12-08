#ifndef DATAUPLOAD_H
#define DATAUPLOAD_H

#include <QObject>
#include <QByteArray>
#include <QFile>
#include <QIODevice>

#include "mydef.h"
#include "crc32.h"
#include "myfiles.h"

class dataUpload : public QObject
{
    Q_OBJECT
public:
    explicit dataUpload(QObject *parent = 0);

    myfiles m_myfiles;

    void echoStartOK(bool bOK);
    int getID32(int src2821,int des2013,int type1209, int info8 ,int id71);

    bool m_bStartOK;
    bool m_isArm;
    QString m_strFN;

    crc32 m_crc32;

    /////////////////////////////
    bool m_bInit;
    int m_nVersion;
    int m_fileID;
    int m_numPkt;
    int m_nSum7;

    /// 1st frame
    int m_nLen24;
    int m_nLen4;// m_nLen24-2
    //int m_nLen241;// m_nlen24+1
    int m_nDataType;
    int m_nDataSn;
    QByteArray m_baData;
    QByteArray m_baFile;
    int m_nRcv;
    bool m_bSum;
    int m_nSumCal;

    void setup(struct st_frame st);
    void setData1(struct st_frame st);
    void setData2(struct st_frame st);
    void addChar(char ch);

    void parseBA();
    void init();
    void doC7();// file data


    void printBA(QByteArray ba);

signals:
    void sigUart(QByteArray);

public slots:

};

#endif // DATAUPLOAD_H
