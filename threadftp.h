#ifndef THREADFTP_H
#define THREADFTP_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QSemaphore>

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

class threadftp : public QThread
{
    Q_OBJECT
public:
    explicit threadftp(QObject *parent = 0);

    void rmData();

    QSemaphore m_sema;

    QQueue<int> m_qn;
    QQueue<QByteArray> m_qba;

    bool m_bTimeout;
    bool m_bEcho;

protected:
    virtual void run();

signals:
    void sigUart(QByteArray);

public slots:
    void slotAddBA(QByteArray ba);
    void slotAddn(int n);
    void slotStart();
    void slotRelease();
};

#endif // THREADFTP_H
