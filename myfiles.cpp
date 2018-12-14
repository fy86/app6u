#include "myfiles.h"

char myfiles::running8=0;

myfiles::myfiles(QObject *parent) :
    myobject(parent)
{
    if(!m_isArm){// linux
        QDir dirRoot("/home/c/tmp/qt");
        QDir dirFiles("/home/c/tmp/qt/files");
        if(dirRoot.exists()){
            if(!dirFiles.exists()) dirRoot.mkdir("files");
        }
        m_root=QString("/home/c/tmp/qt/files/");

    }
    else{// arm
        QDir dirRoot("/home/root/qt");
        QDir dirFiles("/home/root/qt/files");
        if(dirRoot.exists()){
            if(!dirFiles.exists()) dirRoot.mkdir("files");
        }
        m_root=QString("/home/root/qt/files/");

    }
    m_fnRunning = getFullName(QString("running.txt"));

    //QDir d("/home/home/file.txt");
    //syslog(LOG_INFO,"dir path: %s      filename : %s",d.path().toLatin1().data(),d.dirName().toLatin1().data());
}
QString myfiles::getFullName(QString path_filename)
{
    QDir dir(path_filename);
    QString ret;
    ret=m_root + dir.dirName();
    return ret;
}
QString myfiles::findFile(QString path_filename)
{
    QDir dir(path_filename);
    QString ret;
    QString shm("/dev/shm/");

    ret=shm+dir.dirName();
    if(QFile(ret).exists())return ret;
    ret = m_root + dir.dirName();
    if(QFile(ret).exists()) return ret;
    ret="";
    return ret;
}
bool myfiles::isBash(QString filenameFull)
{
    bool ret=false;

    QFile f(filenameFull);
    if(!f.exists())return ret;
    if(f.open(QIODevice::ReadOnly)){
        QByteArray ba=f.readAll();
        ba.resize(10);
        QString str = QString(ba.data());
        if(str.contains(QString("#!/bin/sh"),Qt::CaseSensitive)) ret=true;
    }


    return ret;
}
bool myfiles::isPython(QString filenameFull)
{
    return false;
}

void myfiles::slotRunFile(QString filenameFull)
{
    QDir d(filenameFull);
    QString fn=getFullName(d.dirName());
    QString strBA;
    QStringList sl;
    if(!QFile(fn).exists())return;

    if(isBash(fn)){
        m_pProcess = new QProcess();
        strBA=QString("/bin/sh ")+fn+QString(" &");
        syslog(LOG_INFO," run bash :   %s",strBA.toLatin1().data());
        m_pProcess->start(strBA);
    }

}
bool myfiles::findPythonFile(QString fn)
{
    QDir dir(fn);
    QString ret;
    ret = m_root + dir.dirName();
    //syslog(LOG_INFO,"  find python %s ",ret.toLatin1().data());
    if(QFile(ret).exists()) return true;
    return false;

}
int myfiles::char2int(char b8)
{
    int sn=-1;
    int i;
    int mask;
    for(i=0;i<8;i++){
        mask = 1<<i;
        if(mask & b8){
            sn = i;
            break;
        }
    }
    return sn;

}

void myfiles::restoreRunning()
{
    //m_fnRunning = getFullName(QString("running.txt"));

    int sn;
    QString fn,fnFull;
    QFile f(m_fnRunning);
    //syslog(LOG_INFO,"   restore running %s",m_fnRunning.toLatin1().data());
    if(!f.exists()){
        //syslog(LOG_INFO,"   not.found  restore running %s",m_fnRunning.toLatin1().data());
        return;
    }
    if(f.open(QIODevice::ReadOnly)){
        QByteArray ba=f.readAll();
        f.close();
        running8 = ba.at(0);
        sn=char2int(ba.at(0));
        //syslog(LOG_INFO,"  restore sn:%d",sn);
        if(sn<0) return;
        fn=QString("file");
        fn+=QString::number(sn);
        fn+=QString(".py");
        //syslog(LOG_INFO,"  py name: %s",fn.toLatin1().data());
        if(!findPythonFile(fn)){
            syslog(LOG_INFO,"  not found py file %s",fn.toLatin1().data());
            return;
        }
        fnFull = getFullName(fn);
        //syslog(LOG_INFO,"  py full name : %s",fnFull.toLatin1().data());
        QString cmd0=QString("/usr/bin/killall python");
        m_p0.start(cmd0);
        m_p0.waitForFinished();
        QString cmd=QString("/usr/bin/python ")+fnFull+QString(" &");
        syslog(LOG_INFO,"   restore running cmd:%s",cmd.toLatin1().data());
        m_p0.start(cmd);
    }

}
void myfiles::saveRunning8()
{
    QByteArray ba;
    ba.append(running8);
    QFile f(m_fnRunning);
    if(!f.open(QIODevice::ReadWrite)){
        //syslog(LOG_INFO," save running8 open error -----%s",m_fnRunning);
        return;
    }
    f.write(ba);
    f.close();
}


