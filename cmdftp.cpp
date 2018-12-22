#include "cmdftp.h"

cmdftp::cmdftp(QObject *parent) : dataUpload(parent)
{

}

void cmdftp::setData1(st_frame st)
{
    char bB0=0xb0;// ftp
    char b01 = 0x01;
    if(bB0!=st.buf[2])return;
    if(b01!=st.buf[3])return;
    m_nCmd1st = 0x0ff & st.buf[2];

    m_nLen24 = ((0x0ff & st.buf[0])<<8) + (0x0ff & st.buf[1]);
    m_nLen4 = m_nLen24-2;

    m_nDataType = 0x0ff & st.buf[2];

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

    syslog(LOG_INFO,"ftp.1st.frame len24:%d buf2:%02x -- ",m_nLen24,0x0ff & st.buf[2]);


}
void cmdftp::mkUart0(QByteArray ba8)
{

}

void cmdftp::mkUart(int n, QByteArray ba8,char id8)
{
    //int ret=0;
    QByteArray bb8;
    int i;
    int id32;
    QByteArray ba16;
    char b8,bsum;
    bb8=ba8.mid(0,8);
    b8=0;
    while(bb8.size()<8)bb8.append(b8);
    switch(n){
    case 0:// 1st frame
        bsum=0;
        ba16.clear();

        b8=0xaa;
        bsum+=b8;
        ba16.append(b8);

        b8=0x55;
        ba16.append(b8);
        bsum+=b8;

        id32=getID32(MY_CAN_ID,0,2,0,id8);
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

        for(i=0;i<8;i++){
            b8=bb8.at(i);
            ba16.append(b8);
            bsum+=b8;
        }
        ba16.append(bsum);

        b8=0x88;
        ba16.append(b8);

        //sigUart(ba16);
        emit sigFtp(ba16);

        break;
    default:
        bsum=0;
        ba16.clear();

        b8=0xaa;
        bsum+=b8;
        ba16.append(b8);

        b8=0x55;
        ba16.append(b8);
        bsum+=b8;

        id32=getID32(MY_CAN_ID,0,3,0,id8);
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

        for(i=0;i<8;i++){
            b8=bb8.at(i);
            ba16.append(b8);
            bsum+=b8;
        }
        ba16.append(bsum);

        b8=0x88;
        ba16.append(b8);

        //sigUart(ba16);
        emit sigFtp(ba16);

        break;
    }

    //return ret;

}
// start sn=0
void cmdftp::mkFtpFrame(int sn, int nPkg, QByteArray ba512)
{
    int id8;
    int i;
    char b8=0;
    QByteArray ba42;// table4.2
    ba42=m_baFileName.mid(0,32);
    while(ba42.size()<32)ba42.append(b8);

    short s,ns;
    char *pns;
    pns=(char*)&ns;

    ////////////// set sn
    s=sn+1;
    ns=htons(s);

    b8=pns[0];
    ba42.append(b8);
    b8=pns[1];
    ba42.append(b8);

    //////////////// set nPkg
    s=nPkg;
    ns=htons(s);

    b8=pns[0];
    ba42.append(b8);
    b8=pns[1];
    ba42.append(b8);

    int h32,n32;
    char *p32;
    p32=(char*)&n32;

    h32=sn<<9;
    n32=htonl(h32);
    for(i=0;i<4;i++)ba42.append(p32[i]);
    h32=ba512.size();
    n32=htonl(h32);
    for(i=0;i<4;i++)ba42.append(p32[i]);

    ba42.append(ba512);

    unsigned int crc=m_crc32.csp_crc32_memory((unsigned char*)ba512.data(),ba512.size());
    int icrc = crc;
    syslog(LOG_INFO," ===  ftp download crc32: %08x   signed : %d",crc,icrc);

    h32 = crc;
    n32 = htonl(h32);
    for(i=0;i<4;i++) ba42.append(p32[i]);

    syslog(LOG_INFO,"   ftp.ba42: ");
    printBA(ba42);



    int len=ba512.size();
    int len24=2+32+2+2+4+4+len+4;
    int len24e=(((len24+3+7)>>3)<<3)-3;
    int nEx0=len24e-len24;
    short s24=len24e;
    short ns24 = htons(s24);
    char *p=(char*)&ns24;
    QByteArray ba;
    char bsum=0;

    b8=p[0];
    ba.append(b8);
    b8=p[1];
    ba.append(b8);

    b8=0xb1;////////////// ftp
    ba.append(b8);
    b8=0x02;
    ba.append(b8);

    ba.append(ba42);

    b8=0;
    for(i=0;i<nEx0;i++)ba.append(b8);

    len=ba.size();
    for(i=0;i<len;i++)bsum+=ba.at(i);
    ba.append(bsum);

    int n=(ba.size()+7)>>3;
    emit sigInt(n);/////////////////////////////////////////
    id8=myobject::ID8send;
    myobject::ID8send++;
    for(i=0;i<n;i++){
        QByteArray ba8;
        ba8=ba.mid(i<<3,8);
        mkUart(i,ba8,id8);
    }
}

void cmdftp::doDownload()
{
    int flen;
    QString strFn;
    strFn = m_myfiles.findFile(QString(m_baFileName.data()));
    if(strFn.length()<1){
        syslog(LOG_INFO," ftp.file not found %s",strFn.toLatin1().data());
        return;

    }
    QFile file(strFn);
    if(!file.exists()){
        syslog(LOG_INFO," ftp.file not found");
        return;
    }
    flen=file.size();
    if(flen<1){
        syslog(LOG_INFO," ftp.file.length==0");
        return;
    }

    if(!file.open(QIODevice::ReadOnly)){
        syslog(LOG_INFO," ftp download , open file error");
        return;
    }
    QByteArray baFile=file.readAll();
    if(baFile.length()<1){
        syslog(LOG_INFO," readall ftp.file.length==0");
        return;
    }

    int len=baFile.length();
    int nBlock=(len+511)>>9;
    for(int i=0;i<nBlock;i++){
        QByteArray ba512=baFile.mid(i<<9,512);
        mkFtpFrame(i,nBlock,ba512);
    }
    emit sigStartThreadFtp();

}

void cmdftp::doFtp()
{
    char b2=0x02;// download

    if(b2!=m_baData.at(2)){
        syslog(LOG_INFO," errerr ftp.cmd unknown %02x",0x0ff & m_baData.at(2));
        return;
    }
    echoFtp(true);

    m_baFileName = m_baData.mid(4,m_nLen24-8);
    char b0=0x0;
    if(m_baFileName.size()>32){
        syslog(LOG_INFO," errerr ftp.filename.length>32");
        return;
    }
    while(m_baFileName.size()<32) m_baFileName.append(b0);
    syslog(LOG_INFO," ftp.download.file.name: %s",m_baFileName.data());

    doDownload();

}
void cmdftp::echoFtp(bool bOK)
{
    int id32;
    char b8,bsum,byteOK;
    char sum3;
    QByteArray ba16;
    if(bOK){
        byteOK = 0x0ff;
    }
    else byteOK = 0x00;

        bsum=0;
        sum3 = 0;
        ba16.clear();

        b8=0xaa;
        bsum+=b8;
        ba16.append(b8);

        b8=0x55;
        ba16.append(b8);
        bsum+=b8;

        id32=getID32(MY_CAN_ID,0,1,0,0);
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
        b8=0xc0;//////////////////////////////////
        ba16.append(b8);
        bsum+=b8;
        sum3+=b8;

        b8=0x01;
        ba16.append(b8);
        bsum+=b8;
        sum3+=b8;

        b8=byteOK;//////////////////////// byteOK
        ba16.append(b8);
        bsum+=b8;
        sum3+=b8;

        b8=sum3;////////////////////////////////////// fixme
        ba16.append(b8);
        bsum+=b8;

        b8=0x00;//
        ba16.append(b8);
        bsum+=b8;

        b8=0x00;
        ba16.append(b8);
        bsum+=b8;

        b8=0x00;
        ba16.append(b8);
        bsum+=b8;

        b8=0x00;////////////////////////////// 99
        ba16.append(b8);
        bsum+=b8;

        ba16.append(bsum);

        b8=0x88;
        ba16.append(b8);

        //////////////
        emit sigUart(ba16);


}
