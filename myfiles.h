#ifndef MYFILES_H
#define MYFILES_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDir>

#include "myobject.h"

class myfiles : public myobject
{
    Q_OBJECT
public:
    explicit myfiles(QObject *parent = 0);

    QString m_root;

signals:

public slots:

};

#endif // MYFILES_H
