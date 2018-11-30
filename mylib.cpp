#include "mylib.h"

myLib::myLib(QObject *parent) : QObject(parent)
{

}

void myLib::printBA(QByteArray ba)
{
    QString str,str1;
    int len=ba.size();
    for(int i=0;i<len;i++){
        str1.sprintf(" %02x",0x0ff & ba.at(i));
        str.append(str1);
    }
    qDebug(" len:%d  :: %s",len,str.toLatin1().data());

}

void myLib::printBA(char *p,QByteArray ba)
{
    QString str,str1;

    int len=ba.size();

    str.append(QString(p));
    str.append(" ");
    for(int i=0;i<len;i++){
        str1.sprintf(" %02x",0x0ff & ba.at(i));
        str.append(str1);
    }
    qDebug(" len:%d  :: %s",len,str.toLatin1().data());

}


