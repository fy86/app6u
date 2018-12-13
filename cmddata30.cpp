#include "cmddata30.h"

cmdData30::cmdData30(QObject *parent) : dataUpload(parent)
{

}

void cmdData30::setData1(st_frame st)
{
    m_nCmd1st = 0x0ff & st.buf[2];

    m_nLen24 = ((0x0ff & st.buf[0])<<8) + (0x0ff & st.buf[1]);
    m_nLen4 = m_nLen24-2;

    m_baData.clear();
    m_bSum=false;//////////////////////
    m_nSumCal = 0x0;
    for(int i=0;i<2;i++){
        m_nSumCal += 0x0ff & st.buf[i];
        m_nSumCal &= 0x0ff;
    }
    for(int i=0;i<6;i++){
        addChar(st.buf[2+i]);
    }

    syslog(LOG_INFO,"  data30.1st.frame len24:%d buf2:%02x -- ",m_nLen24,0x0ff & st.buf[2]);

}

void cmdData30::doData30()
{
    syslog(LOG_INFO,"  doData30 , save data");
    QFile file("/dev/shm/data30.bin");
    if(file.open(QIODevice::ReadWrite)){
        file.write(m_baData.data()+2,m_nLen4);
    }
}
