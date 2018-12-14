#include "threadstat.h"

threadStat::threadStat(QObject *parent) : QThread(parent)
{
    m_files8 = 0;
    m_running8 = 0;
    m_sn=0;
    m_isArm=QFile::exists(QString("/dev/ttymxc2"));
    m_fnRunning = m_myfiles.getFullName(QString("running.txt"));

}
void threadStat::run()
{
    int i;
    QString fn;
    //restoreRunning();
    for(;;){
        for(i=0;i<8;i++){
            fn=QString("file");
            fn+=QString::number(i);
            fn+=QString(".py");
            if(m_myfiles.findPythonFile(fn)){
                //syslog(LOG_INFO," found %s",fn.toLatin1().data());
                m_files8 |= 0x01<<i;
            }
            else{
                //syslog(LOG_INFO," not found %s",fn.toLatin1().data());

                m_files8 &=~(0x01<<i);
            }
        }
        emit sigFile8(m_files8);

        getRunning();
        emit sigRunning8(m_running8);
        //syslog(LOG_INFO,"  ------ old:%02x  %02x",m_myfiles.running8,m_running8);
        if(m_running8!=m_myfiles.running8){
            //syslog(LOG_INFO," old:%02x  %02x",m_myfiles.running8,m_running8);
            m_myfiles.running8 = m_running8;
            m_myfiles.saveRunning8();
        }

        sleep(20);

    }

}
void threadStat::getRunning()
{
    QProcess p;
    QString cmd;
    QStringList sl;
    sl<<"-Af";

    if(m_isArm)cmd = QString("/bin/ps");
    else cmd=QString("/bin/ps -Af");

    //syslog(LOG_INFO,"   start get running stat");
    p.setStandardOutputFile(QString("/dev/shm/psout.txt"));
    p.start(cmd);
    p.waitForFinished();

    //QByteArray ba=p.readAllStandardOutput();
    //QByteArray ba = p.readAll();
    //ba.append((char)0);
    //QByteArray ba;
    QString str;
    QFile f("/dev/shm/psout.txt");
    m_running8 = 0;
    if(f.open(QIODevice::ReadOnly)){
        QTextStream in(&f);
        while(!in.atEnd()){
            str=in.readLine();
            //syslog(LOG_INFO," readline %s",str.toLatin1().data());
            checkPSstr(str);
        }
        f.close();
        //ba=f.readAll();
    }

    syslog(LOG_INFO," (%d) get running stat , file8:%02x   running:%02x  ",m_sn++,0x0ff & m_files8,0x0ff & m_running8);

}
void threadStat::checkPSstr(QString str)
{
    QString fn;
    char b8;
    for(int i=0;i<8;i++){
        fn=QString("file");
        fn+=QString::number(i);
        fn+=QString(".py");

        //syslog(LOG_INFO,"  str: %s   %s",str.toLatin1().data(),fn.toLatin1().data());
        if(str.contains(fn,Qt::CaseSensitive)){
            //syslog(LOG_INFO,"    ---------------  match file1.py  %02x", 0x0ff & m_running8);
            b8 = 1<<i;
            m_running8 |= b8;
            //syslog(LOG_INFO,"    match file1.py b8:%02x  %02x",0x0ff & b8, 0x0ff & m_running8);
        }

    }

}
int threadStat::char2int(char b8)
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

void threadStat::restoreRunning()
{
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
        sn=char2int(ba.at(0));
        syslog(LOG_INFO,"  restore sn:%d",sn);
        if(sn<0) return;
        fn=QString("file");
        fn+=QString::number(sn);
        fn+=QString(".py");
        syslog(LOG_INFO,"  py name: %s",fn.toLatin1().data());
        if(!m_myfiles.findPythonFile(fn)){
            syslog(LOG_INFO,"  not found py file %s",fn.toLatin1().data());
            return;
        }
        fnFull = m_myfiles.getFullName(fn);
        syslog(LOG_INFO,"  py full name : %s",fnFull.toLatin1().data());
        QString cmd0=QString("/usr/bin/killall python");
        m_p0.start(cmd0);
        m_p0.waitForFinished();
        QString cmd=QString("/usr/bin/python ")+fnFull+QString(" &");
        syslog(LOG_INFO,"   restore running cmd:%s",cmd.toLatin1().data());
        m_p0.start(cmd);
    }

}
