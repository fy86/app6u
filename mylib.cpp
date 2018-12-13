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
    syslog(LOG_INFO," len:%d  :: %s",len,str.toLatin1().data());

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
    syslog(LOG_INFO," len:%d  :: %s",len,str.toLatin1().data());

}
void myLib::printBA16(char *p,QByteArray ba)
{
    QString str,str1;

    int len=ba.size();

    str.append(QString(p));
    str.append(" ");
    if(len<16)return;// not frame16
    for(int i=0;i<4;i++){
        str1.sprintf(" %02x",0x0ff & ba.at(i+2));
        str.append(str1);
    }
    str1.sprintf(" -- ");
    str.append(str1);

    for(int i=0;i<8;i++){
        str1.sprintf(" %02x",0x0ff & ba.at(i+6));
        str.append(str1);
    }
    syslog(LOG_INFO," %s",str.toLatin1().data());

}


