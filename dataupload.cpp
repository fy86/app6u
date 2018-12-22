#include "dataupload.h"

dataUpload::dataUpload(QObject *parent) :
    QObject(parent)
{
    m_nCmd1st = 0;
    m_bInit = false;
    m_nLen24 = 0;
    m_bStartOK=false;

    m_isArm=QFile::exists(QString("/dev/ttymxc2"));
}
// single.frame upload.start(init)
/// removed
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
int dataUpload::getID32(int src2821, int des2013, int type1209, int info8, int id71)
{
    int id32=0;

    id32 |= (0x0ff & src2821)<<21;
    id32 |= (0x0ff & des2013)<<13;
    id32 |= (0x0f & type1209)<<9;
    id32 |= (0x01 & info8)<<8;
    id32 |= 0x0ff & id71;

    if(id32!=myobject::id32old){
        myobject::id32old = id32;
        syslog(LOG_INFO," getID32 : %08x  src:%02x des:%02x type:%01x info:%01x id:%02x",id32,src2821,des2013,type1209,info8,id71);
    }

    return id32;

}

void dataUpload::echoStartOK(bool bOK,char id8)
{
    int id32;
    char b8,bsum,byteOK;
    QByteArray ba16;
    if(bOK){
        byteOK = 0x0ff;
    }
    else byteOK = 0x55;

        bsum=0;
        ba16.clear();

        b8=0xaa;
        bsum+=b8;
        ba16.append(b8);

        b8=0x55;
        ba16.append(b8);
        bsum+=b8;

        id32=getID32(0x25,0,1,0,0);
        b8= id32;
        ba16.append(b8);
        bsum+=b8;

        b8= id32>>8;
        ba16.append(b8);
        bsum+=b8;

        b8=id32>>16;
        ba16.append(b8);
        bsum+=b8;

        b8=id32>>24;
        ba16.append(b8);
        bsum+=b8;

        // byte 1 , base 1
        b8=0x99;//////////////////////////////////
        ba16.append(b8);
        bsum+=b8;

        b8=byteOK;////////////////////// byteOK
        ba16.append(b8);
        bsum+=b8;

        b8=0x02;
        ba16.append(b8);
        bsum+=b8;

        b8=id8;////////////////////////////////////// fixme
        ba16.append(b8);
        bsum+=b8;

        b8=0xaa;//
        ba16.append(b8);
        bsum+=b8;

        b8=0xaa;
        ba16.append(b8);
        bsum+=b8;

        b8=0xaa;
        ba16.append(b8);
        bsum+=b8;

        b8=0x99;////////////////////////////// 99
        ba16.append(b8);
        bsum+=b8;

        ba16.append(bsum);

        b8=0x88;
        ba16.append(b8);

        emit sigUart(ba16);



}
void dataUpload::echoC7(bool bOK,char id8)
{
    int id32;
    char b8,bsum,byteOK;
    QByteArray ba16;
    if(bOK){
        byteOK = 0x0ff;
    }
    else byteOK = 0x55;

        bsum=0;
        ba16.clear();

        b8=0xaa;
        bsum+=b8;
        ba16.append(b8);

        b8=0x55;
        ba16.append(b8);
        bsum+=b8;

        id32=getID32(0x25,0,1,0,0);
        b8= id32;
        ba16.append(b8);
        bsum+=b8;

        b8= id32>>8;
        ba16.append(b8);
        bsum+=b8;

        b8=id32>>16;
        ba16.append(b8);
        bsum+=b8;

        b8=id32>>24;
        ba16.append(b8);
        bsum+=b8;

        // byte 1 , base 1
        b8=0x98;///////////////// 98 98 98 98
        ba16.append(b8);
        bsum+=b8;

        b8=byteOK;////////////////////// byteOK
        ba16.append(b8);
        bsum+=b8;

        b8=0x02;
        ba16.append(b8);
        bsum+=b8;

        b8=id8;////////////////////////////////////// fixme
        ba16.append(b8);
        bsum+=b8;

        b8=0xaa;//
        ba16.append(b8);
        bsum+=b8;

        b8=0xaa;
        ba16.append(b8);
        bsum+=b8;

        b8=0xaa;
        ba16.append(b8);
        bsum+=b8;

        b8=0x97;/////        97 97 97
        ba16.append(b8);
        bsum+=b8;

        ba16.append(bsum);

        b8=0x88;
        ba16.append(b8);

        emit sigUart(ba16);



}


// handle upload.start 1st frame.complex
void dataUpload::init()
{
    int sum=0;
    int len1=m_baData.size()-1;
    char id8;

    m_bInit = false;
    if(0x51 != m_nOStype){
        syslog(LOG_INFO," filesystem type error(!=0x51) : %02x",m_nDataSn);
        return;// not fs system
    }
    m_nVersion = 0x0ff & m_baData.at(2);
    m_fileID = 0x0ff & m_baData.at(3);
    id8=m_baData.at(3);
    m_numPkt = ((0x0ff & m_baData.at(len1-3))<<16)
            | ((0x0ff & m_baData.at(len1-2))<<8)
            | (0x0ff & m_baData.at(len1-1));// big endian 24b
    m_C7pkt = -1;

    for(int i=0;i<len1;i++){
        sum+=0x0ff & m_baData.at(i);
    }
    sum &= 0x0ff;
    m_bInit=true;

    syslog(LOG_INFO,"upload.start(init) sum.cal: 0x%02x   sum: 0x%02x",sum,0x0ff & m_baData.at(len1));
    if( (0x0ff & sum)!=(0x0ff & m_baData.at(len1))){
        m_bStartOK=false;
        echoStartOK(m_bStartOK,id8);

        return;
    }
    m_bStartOK=true;
    echoStartOK(m_bStartOK,id8);
    syslog(LOG_INFO,"  numPkt:%d   version:0x%02x  fileID: 0x%02x",m_numPkt,m_nVersion,m_fileID);
    syslog(LOG_INFO," file.name: %s",m_baData.data()+4);

    QDir d(QString(m_baData.data()+4));
    syslog(LOG_INFO," dataupload   filename.full: %s   filename: %s",
           d.path().toLatin1().data(),
           d.dirName().toLatin1().data());

    //if(m_isArm) m_strFN=QString(m_baData.data()+4);
    //else m_strFN = QString("/home/c/tmp/save6u.bin");

    m_strFN = m_myfiles.getFullName(QString(m_baData.data()+4));
    syslog(LOG_INFO,"  save.filename: %s" , m_strFN.toLatin1().data());
    QFile::remove(m_strFN);
}
/// parse frame.complex  ??????
void dataUpload::parseBA()
{
    syslog(LOG_INFO,"parse complex.frame  DataType:%02x",m_nDataType);
    switch(m_nDataType){
    case CMD_UPLOAD_INIT:// 0xc6
        init();
        break;
    case CMD_UPLOAD_DATA:// 0xc7
        //qDebug("     ****** file data frame.complex ");
        doC7();
        break;
    case CMD_FTP:
        doFtp();
        break;
    case CMD_DATA30:
        doData30();
        break;
    default:
        break;
    }
}
void dataUpload::doData30()
{

}

void dataUpload::doFtp()
{

}

void dataUpload::doC7()
{
    int len=m_baData.size();
    int lenWrite;
    int i;
    int start,end;
    QByteArray ba;
    char id8;

    syslog(LOG_INFO," doC7 baData.len: %d   m_baData.at0:%02x at1:%02x at2:%02x",
                     len,
                     0x0ff & m_baData.at(0),
                     0x0ff & m_baData.at(1),
                     0x0ff & m_baData.at(2));
    if(len>4096){
        ba=m_baData.mid(len-4096-4,4096);
    }
    else if(len>1024){
        ba=m_baData.mid(len-1024-4,1024);
    }
    else return;
    //syslog(LOG_INFO," file.txt : %s",ba.data());

    id8=m_baData.at(2);//  at3.4.5 pkt.No.
    m_C7pkt = ((0x0ff & m_baData.at(3))<<16)
            | ((0x0ff & m_baData.at(4))<<8)
            | (0x0ff & m_baData.at(5));// big endian 24b

    start = (0x0ff & m_baData.at(6))<<24;
    start |= (0x0ff & m_baData.at(7))<<16;
    start |= (0x0ff & m_baData.at(8))<<8;
    start |= 0x0ff & m_baData.at(9);

    end = (0x0ff & m_baData.at(10))<<24;
    end |= (0x0ff & m_baData.at(11))<<16;
    end |= (0x0ff & m_baData.at(12))<<8;
    end |= 0x0ff & m_baData.at(13);

    lenWrite=end-start;
    syslog(LOG_INFO,"  doC7 offfset.start: %08x   end:%08x  len: %d",start,end,lenWrite);

    unsigned int crc32cal=m_crc32.csp_crc32_memory((unsigned char*)(m_baData.data()),len-4);
    unsigned int crc32 = ((0x0ff&m_baData[len-4])<<24)
            | ((0x0ff & m_baData[len-3])<<16)
            | ((0x0ff & m_baData[len-2])<<8)
            | (0x0ff & m_baData[len-1]);
    int icrc32=crc32cal;
    syslog(LOG_INFO,"crc32 : %08x     cal: %08x   int.crc32:%d",crc32, crc32cal,icrc32);

    if(crc32cal!=crc32){
        echoC7(false,id8);
        return;
    }
    echoC7(true,id8);


    QFile file(m_strFN);
    if(file.exists()){
        if(file.open(QIODevice::Append)){
            file.write(ba.data(),lenWrite);
            file.close();
        }
    }
    else{
        if(file.open(QIODevice::ReadWrite)){
            file.write(ba.data(),lenWrite);
            file.close();

        }

    }

    //fileMD5(m_strFN);
    syslog(LOG_INFO," savefile   numPkt:%d   C7pkt:%d",m_numPkt,m_C7pkt);
    if(m_numPkt==m_C7pkt){
        fileMD5(m_strFN);
        m_myfiles.slotRunFile(m_strFN);
        syslog(LOG_INFO," upload finish , try run file : %s",m_strFN.toLatin1().data());
    }

}
void dataUpload::fileMD5(QString strFN)
{
    QFile f(strFN);
    QCryptographicHash hash(QCryptographicHash::Md5);
    QByteArray ba,r;

    if(f.open(QIODevice::ReadOnly)){
        hash.reset();
        ba=f.readAll();
        f.close();
        hash.addData(ba);
        r=hash.result();
        syslog(LOG_INFO,"%s   md5:%s     %02x....%02x",strFN.toLatin1().data(),r.toHex().data(),0x0ff & r.at(0),0x0ff & r.at(r.size()-1));
        emit sigMD5(r);
    }

}

void dataUpload::printBA(QByteArray ba)
{
    QString str,str1;
    int len=ba.size();
    syslog(LOG_INFO," printBA len24:%d  len.ba:%d",m_nLen24,len);
    for(int i=0;i<len;i++){
        str1.sprintf(" %02x",0x0ff & ba.at(i));
        str.append(str1);
        if(str.length()>64){
            syslog(LOG_INFO," printBA %s",str.toLatin1().data());
            str.resize(0);
        }
    }
    if(str.length()>0)
        syslog(LOG_INFO," printBA - %s",str.toLatin1().data());

}

void dataUpload::addChar(char ch)
{
    int sum;
    if(m_nLen24==m_baData.size()){
        if(m_bSum)return;
        m_bSum=true;
        sum = 0x0ff & ch;
        syslog(LOG_INFO," complex.frame sum.cal: %02x    sum:%02x",m_nSumCal,sum);
        printBA(m_baData);
        if(m_nSumCal==sum) parseBA();
        else{
            syslog(LOG_INFO," complex.frame checksum error !!!!!!!!!!!!!!!!!!!!!! ");
        }
    }
    else{
        m_baData.append(ch);
        m_nSumCal += 0x0ff & ch;
        m_nSumCal &= 0x0ff;

    }
}

/// upload cmd
void dataUpload::setData1(st_frame st)
{
    char bC6=0xc6;
    char bC7=0xc7;
    m_nLen24 = ((0x0ff & st.buf[0])<<8) + (0x0ff & st.buf[1]);
    m_nLen4 = m_nLen24-2;
    // buf[2] : 0xc6 , 0xc7
    m_nDataType= 0x0ff & st.buf[2];// c6 c7
    if(bC6==st.buf[2]){
        m_nOStype = 0x0ff & st.buf[3];// 0x51
        // buf[4] : 0x02 // version No.
        m_nDataSn = 0x0ff & st.buf[5];
    }
    else if( bC7 == st.buf[2]){
        m_nOStype = 0x51;// no this byte
        m_nDataSn = 0x0ff & st.buf[4];
    }
    else{
        syslog(LOG_INFO," errerr : 1st frame not C6.C7");
    }

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

    syslog(LOG_INFO,"1st.frame len24:%d buf2:%02x -- OStype:%02x sn:%02x",m_nLen24,0x0ff & st.buf[2],m_nOStype,m_nDataSn);

}

void dataUpload::setData2(st_frame st)
{
    for(int i=0;i<8;i++){
        addChar(st.buf[i]);
    }

}
