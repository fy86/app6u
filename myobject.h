#ifndef MYOBJECT_H
#define MYOBJECT_H

#include <QObject>
#include <QFile>
#include <QDir>

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

class myobject : public QObject
{
    Q_OBJECT
public:
    explicit myobject(QObject *parent = 0);

    static int snLog;

    bool m_isArm;

signals:

public slots:

};

#endif // MYOBJECT_H
