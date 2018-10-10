#include "threadudpr.h"

ThreadUdpR::ThreadUdpR(QObject *parent) :
    QThread(parent)
{
}

void ThreadUdpR::run()
{
    char ch;
    for(int n=0;n<4;){
        if(!m_q.isEmpty()){
            ch = m_q.dequeue();
            qDebug("%02x",ch & 0x0ff);
            n++;
        }
    }

}
void ThreadUdpR::newChar(char ch)
{
    m_q.enqueue(ch);
}
