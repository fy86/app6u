#ifndef MYLIB_H
#define MYLIB_H

#include <QObject>

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

class myLib : public QObject
{
    Q_OBJECT
public:
    explicit myLib(QObject *parent = 0);

    static int snSend;

    void printBA(QByteArray ba);
    void printBA(char *p,QByteArray ba);
    void printBA16(char *p,QByteArray ba);

signals:

public slots:
};

#endif // MYLIB_H
