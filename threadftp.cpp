#include "threadftp.h"

threadftp::threadftp(QObject *parent) : QThread(parent)
{
    m_bEcho=false;
    m_bTimeout = false;

}
void threadftp::rmData()
{
    while(!m_qn.isEmpty()){
        m_qn.dequeue();
    }
    while(!m_qba.isEmpty()){
        m_qba.dequeue();
    }
}

void threadftp::run()
{
    int sn=0;
    if(m_qn.isEmpty()){
        syslog(LOG_INFO," thread.ftp.queue.int.empty");
        return;// error
    }
    int n=m_qn.dequeue();
    for(int i=0;i<n;i++){
        if(m_qba.isEmpty())break;
        QByteArray ba=m_qba.dequeue();
        emit sigUart(ba);
    }
    sn++;
    for(;;){
        if(m_qn.isEmpty())break;
        if(!m_sema.tryAcquire(1,6000)){
            syslog(LOG_INFO,"   !!!! thread.ftp sema timeout ");
            rmData();
            break;
        }
        if(m_qn.isEmpty())break;
        syslog(LOG_INFO,"   thread.ftp sending: %d .... block512 ",++sn);
        n = m_qn.dequeue();
        for(int i=0;i<n;i++){
            if(m_qba.isEmpty())break;
            QByteArray ba = m_qba.dequeue();
            emit sigUart(ba);
        }
    }
    syslog(LOG_INFO,"   thread.ftp end ");
}
void threadftp::slotRelease()
{
    if(m_sema.available()<1){
        m_sema.release();
    }
}

void threadftp::slotAddn(int n)
{
    m_qn.enqueue(n);
    syslog(LOG_INFO,"  thread.ftp enqueue int");
}
void threadftp::slotAddBA(QByteArray ba)
{
    m_qba.enqueue(ba);
    syslog(LOG_INFO,"   thread.ftp   enqueue QByteArray ");
}

void threadftp::slotStart()
{
    if(isRunning())return;
    start();
    syslog(LOG_INFO,"  thread.ftp.slotStart ---- ");

}

