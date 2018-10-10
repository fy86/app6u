#ifndef THREADUDPR_H
#define THREADUDPR_H

#include <QObject>
#include <QThread>
#include <QQueue>

class ThreadUdpR : public QThread
{
    Q_OBJECT
public:
    explicit ThreadUdpR(QObject *parent = 0);

    QQueue<char> m_q;

protected:
    void run();

signals:

public slots:
    void newChar(char ch);

};

#endif // THREADUDPR_H
