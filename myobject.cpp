#include "myobject.h"

myobject::myobject(QObject *parent) :
    QObject(parent)
{
    m_isArm=QFile::exists(QString("/dev/ttymxc2"));
}

int myobject::snLog=0;
