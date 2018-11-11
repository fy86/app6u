#ifndef THREADUART_H
#define THREADUART_H

#include <QObject>
#include <QThread>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>


class threaduart : public QThread
{
    Q_OBJECT
public:
    explicit threaduart(QObject *parent = 0);

    bool m_bArm;//

    // serial howto
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];




protected:
    void run();

signals:

public slots:

};

#endif // THREADUART_H
