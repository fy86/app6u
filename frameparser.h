#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include <QObject>

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <time.h>
#include <syslog.h>

#include "mydef.h"

class frameparser : public QObject
{
    Q_OBJECT
public:
    explicit frameparser(QObject *parent = 0);

    struct st_frame m_frameIn;

    void doFrameSingle(st_frame stFrame);
    void cpFrame(st_frame *pFrame);
    void doTimeSync(st_frame stFrame);
    void formatFrame(st_frame *pFrame);

    //void slotParse(st_frame stFrame);

signals:

public slots:
    void slotParse(st_frame stFrame);
    void slotTest(QByteArray ba);
    void slotParseBA(QByteArray ba);

};

#endif // FRAMEPARSER_H
