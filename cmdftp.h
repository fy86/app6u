#ifndef CMDFTP_H
#define CMDFTP_H

#include <QObject>

#include "dataupload.h"

class cmdftp : public dataUpload
{
    Q_OBJECT
public:
    explicit cmdftp(QObject *parent = 0);

    QByteArray m_baFileName;

    virtual void setData1(struct st_frame st);
    virtual void doFtp();


    void echoFtp(bool bOK);

    void doDownload();
    void mkFtpFrame(int sn,int nPkg,QByteArray ba512);
    void mkUart(int n,QByteArray ba8);
    void mkUart0(QByteArray ba8);
signals:

public slots:
};

#endif // CMDFTP_H
