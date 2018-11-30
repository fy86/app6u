#include "threaduartsend.h"

threadUartSend::threadUartSend(QObject *parent) : QThread(parent)
{

}


void threadUartSend::run()
{
    for(;;){
        //m_mux.lock();
        //m_wait.wait(&m_mux);
        //m_mux.unlock();
        m_sem.acquire();
        emit sigQSend();
        msleep(20);
        //usleep(1000);
    }

}
void threadUartSend::slotWakeSend()
{
    //m_wait.wakeOne();
    m_sem.release();

}
