#include "dataupload.h"

dataUpload::dataUpload(QObject *parent) :
    QObject(parent)
{
    m_bInit = false;
    m_nLen24 = 0;
}
// single.frame upload.start(init)
void dataUpload::setup(st_frame st)
{
    m_bInit = false;
    //m_nLen24 = 0;

    if(0x0c6 != (0x0ff & st.buf[0]))return;// not upload start cmd frame

    if(0x051 !=(0x0ff & st.buf[1])) return;// not fs system
    m_fileID = 0x0ff & st.buf[3];
    m_nVersion = 0x0ff & st.buf[2];// == 0x02

    m_numPkt = ((0x0ff & st.buf[4])<<16) | ((0x0ff & st.buf[5])<<8) | (0x0ff & st.buf[6]); // big endian  24b
    m_nSum7 = 0x0ff & st.buf[7];

    m_bInit = true;
}
void dataUpload::init()
{
    int sum=0;
    int len1=m_baData.size()-1;

    m_bInit = false;
    if(0x51 != m_nDataSn) return;// not fs system
    m_nVersion = 0x0ff & m_baData.at(2);
    m_fileID = 0x0ff & m_baData.at(3);
    m_numPkt = ((0x0ff & m_baData.at(len1-3))<<16)
            | ((0x0ff & m_baData.at(len1-2))<<8)
            | (0x0ff & m_baData.at(len1-1));// big endian 24b

    for(int i=0;i<len1;i++){
        sum+=0x0ff & m_baData.at(i);
    }
    sum &= 0x0ff;
    m_bInit=true;

    qDebug("upload.start(init) sum.cal: 0x%02x   sum: 0x%02x",sum,0x0ff & m_baData.at(len1));
    qDebug("  numPkt:%d   version:0x%02x  fileID: 0x%02x",m_numPkt,m_nVersion,m_fileID);
    qDebug(" file.name: %s",m_baData.data()+4);
}
/// parse frame.complex  ??????
void dataUpload::parseBA()
{
    qDebug("parse complex.frame");
    switch(m_nDataType){
    case CMD_UPLOAD_INIT:// 0xc6
        init();
        break;
    case CMD_UPLOAD_DATA:// 0xc7
        qDebug("     ****** file data frame.complex ");
        doC7();
        break;
    default:
        break;
    }
}
void dataUpload::doC7()
{
    int len=m_baData.size();
    int i;
    QByteArray ba;

    if(len>4096){
        ba=m_baData.mid(len-4096-4,4096);
    }
    else if(len>1024){
        ba=m_baData.mid(len-1024-4,1024);
    }
    else return;
    qDebug(" file.txt : %s",ba.data());

    unsigned int crc32cal=m_crc32.csp_crc32_memory((unsigned char*)(m_baData.data()),len-4);
    unsigned int crc32 = ((0x0ff&m_baData[len-4])<<24)
            | ((0x0ff & m_baData[len-3])<<16)
            | ((0x0ff & m_baData[len-2])<<8)
            | (0x0ff & m_baData[len-1]);
    qDebug("crc32 : %08x     cal: %08x",crc32, crc32cal);


}

void dataUpload::printBA(QByteArray ba)
{
    QString str,str1;
    int len=ba.size();
    for(int i=0;i<len;i++){
        str1.sprintf(" %02x",0x0ff & ba.at(i));
        str.append(str1);
    }
    qDebug(" len:%d  :: %s",len,str.toLatin1().data());

}

void dataUpload::addChar(char ch)
{
    int sum;
    if(m_nLen24==m_baData.size()){
        if(m_bSum)return;
        m_bSum=true;
        sum = 0x0ff & ch;
        qDebug("sum.cal: %02x    sum:%02x",m_nSumCal,sum);
        printBA(m_baData);
        parseBA();
    }
    else{
        m_baData.append(ch);
        m_nSumCal += 0x0ff & ch;
        m_nSumCal &= 0x0ff;

    }
}

void dataUpload::setData1(st_frame st)
{
    m_nLen24 = ((0x0ff & st.buf[0])<<8) + (0x0ff & st.buf[1]);
    m_nLen4 = m_nLen24-2;
    m_nDataType = 0x0ff & st.buf[2];
    m_nDataSn = 0x0ff & st.buf[3];

    m_baData.clear();
    m_bSum=false;
    m_nSumCal = 0x0;
    for(int i=0;i<2;i++){
        m_nSumCal += 0x0ff & st.buf[i];
        m_nSumCal &= 0x0ff;
    }
    for(int i=0;i<6;i++){
        addChar(st.buf[2+i]);
    }

    qDebug("1st.frame len24:%d  type:%02x sn:%02x",m_nLen24,m_nDataType,m_nDataSn);

}

void dataUpload::setData2(st_frame st)
{
    for(int i=0;i<8;i++){
        addChar(st.buf[i]);
    }

}
