#ifndef THREADUARTSEND_H
#define THREADUARTSEND_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>

class threadUartSend : public QThread
{
    Q_OBJECT
public:
    explicit threadUartSend(QObject *parent = 0);

    QMutex m_mux;
    QWaitCondition m_wait;
    QSemaphore m_sem;

protected:
    void run();

signals:
    void sigQSend();

public slots:
    void slotWakeSend();
};

#endif // THREADUARTSEND_H
