#include "threadudpr.h"

ThreadUdpR::ThreadUdpR(QObject *parent) :
    QThread(parent)
{
    int i;
    for(i=0;i<256;i++)m_aCmd1stID8[i]=0;
    for(i=0;i<16;i++)m_stat[i]=0;
    m_stat[0]=1;
    m_stat[1]=2;// arm status    app6u.version

    m_md5s = 0x55;
    m_md5e = 0x55;

    m_stat[4]=m_md5s;
    m_stat[5]=m_md5e;

    m_n30=0;
    m_stat[6]=m_n30;

    m_nRemote = 0;
    m_nRemoteOK=0;
    m_nRemoteErr=0;
    m_cmdD3 = 1;

    m_nCmd1st = 0;
    m_only25 = true;
    m_isArm=QFile::exists(QString("/dev/ttymxc2"));

    m_dtSet.setTime_t(QDateTime::currentDateTime().toTime_t());
    connect(&m_dataUpload,SIGNAL(sigUart(QByteArray)),this,SIGNAL(sigUart(QByteArray)));
    //connect(&m_ftp,SIGNAL(sigUart(QByteArray)),this,SIGNAL(sigUart(QByteArray)));
    connect(&m_ftp,SIGNAL(sigFtp(QByteArray)),this,SIGNAL(sigFtp(QByteArray)));
    connect(&m_ftp,SIGNAL(sigInt(int)),this,SIGNAL(sigInt(int)));
    connect(&m_ftp,SIGNAL(sigStartThreadFtp()),this,SIGNAL(sigStartThreadFtp()));

    connect(&m_dataUpload,SIGNAL(sigMD5(QByteArray)),this,SLOT(slotMD5(QByteArray)));
    connect(&m_data30,SIGNAL(sig30()),this,SLOT(slot30()));



}
void ThreadUdpR::slot30()
{
    m_n30++;
    m_stat[6]=m_n30;
}

void ThreadUdpR::slotMD5(QByteArray ba)
{
    m_md5s = ba.at(0);
    m_md5e = ba.at(ba.size()-1);

    m_stat[4]=m_md5s;
    m_stat[5]=m_md5e;
}

void ThreadUdpR::setOnly25(bool b)
{
    m_only25 = b;
    emit sigOnly25(b);
    if(b) syslog(LOG_INFO," ---- !!! only.can.id.25.25.25.25.valid ---");
    else syslog(LOG_INFO,"  --- !!! all.can.id.valid ---");
}

void ThreadUdpR::run()
{
    char ch;
    QByteArray ba,baST;
    for(int n=0;;){
        if(m_q.isEmpty()){
            m_mutex.lock();
            m_wait.wait(&m_mutex);
            m_mutex.unlock();
        }
        if(!m_q.isEmpty()){
            ch = m_q.dequeue();
            //qDebug("%02x",ch & 0x0ff);
            n++;

            m_ba16.append(ch);
            if(hasValid16()){  //  m_ba16 has valid16
                ba=m_ba16.mid(0,16);
                print16ba();
                m_ba16.remove(0,16);

                toStFrame(ba,&m_stFrame);
                //emit sigF16(ba);
                parseID32type();

                baST.clear();
                baST.append((char*)(&m_stFrame),sizeof(struct st_frame));
                emit sigSTDframe(baST);

                //emit sigTest();
            }

            //testNewFrame();
        }
    }

}
int ThreadUdpR::getID32(int src2821, int des2013, int type1209, int info8, int id71)
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

int ThreadUdpR::getBAStatFile()
{
    int i,len;
    char b8;
    char *p;
    int n;
    m_baStatFile.clear();
    for(i=0;i<7;i++) m_baStatFile.append(m_stat[i]);/////////////// add md5 2bytes  num30

    QFile file12(QString("/dev/shm/data12.bin"));
    QFile file95(QString("/dev/shm/data95.bin"));
    n=9;//    = 12-2-1   add md5.0...e   num30
    if(file12.exists()){
        if(file12.open(QIODevice::ReadOnly)){
            QByteArray ba=file12.readAll();
            len = ba.size();
            ba.resize(n);
            b8=0;
            p=ba.data();
            for(i=len;i<n;i++)p[i]=b8;
            m_baStatFile.append(ba);
        }
    }
    else {
        b8 = 0xaa;
        for(i=0;i<n;i++) m_baStatFile.append(b8);
    }
    n=95;
    if(file95.exists()){
        if(file95.open(QIODevice::ReadOnly)){
            QByteArray ba=file95.readAll();
            len = ba.size();
            ba.resize(n);
            b8=0;
            p=ba.data();
            for(i=len;i<n;i++)p[i]=b8;
            m_baStatFile.append(ba);
        }
    }
    else {
        b8 = 0x55;
        for(i=0;i<n;i++) m_baStatFile.append(b8);
    }

    return 0;
}
int ThreadUdpR::mkBAStatN(int n)
{
    int ret=0;
    char b8,bsum;
    int i;
    int len;
    short len16,nlen16;
    char *p;
    p=(char*)&nlen16;

    //qDebug(" mkba.stat.n : %d",n);

    switch(n){
    case 0:// 1st frame
        m_baStat.clear();
        len=m_baStatFile.size()+6;
        len16 = len;
        nlen16 = htons(len16);
        b8 = p[0];
        m_baStat.append(b8);
        b8=p[1];
        m_baStat.append(b8);//len

        m_baStat.append((char)0x35);// fix
        m_baStat.append((char)0x01);// fix

        // byte 5 base1
        b8 = m_nRemote;
        m_baStat.append(b8);// req number

        // byte 6 can.ok.number
        b8 = m_nRemoteOK;
        m_baStat.append(b8);// ok.number
        b8 = m_nRemoteErr;
        m_baStat.append(b8);// err.number
        // byte 8 base1
        b8 = m_cmdD3;
        m_baStat.append(b8);// lastest cmd.code

        break;
    case 1:
        // byte 1 base1
        b8 = 0x0;// stat.32
        m_baStat.append(b8);//

        // byte 2
        b8 = 0x0; // stat.arm
        m_baStat.append(b8);

        // byte 3
        b8 = 0x0;// stat.app
        m_baStat.append(b8);

        m_lenStatAdd=0;

        for(i=0;i<4;i++){
            if(m_baStatFile.size()>m_lenStatAdd){
                b8 = m_baStatFile.at(m_lenStatAdd);
                m_baStat.append(b8);
                m_lenStatAdd++;
            }
            else{

                b8 = 0x20;
                m_baStat.append(b8);
            }
        }
        if(m_baStatFile.size()>m_lenStatAdd){
            b8 = m_baStatFile.at(m_lenStatAdd);
            m_baStat.append(b8);
            m_lenStatAdd++;
        }
        else{
            len = m_baStat.size()-2;
            //qDebug(" m_baStat.len :%d     size: %d",len, m_baStat.size());
            bsum = 0;
            for(i=0;i<len;i++) bsum+=m_baStat.at(2+i);
            m_baStat.append(bsum);
            b8 = len>>8;
            m_baStat.replace(0,1,&b8,1);
            b8 = len;
            m_baStat.replace(1,1,&b8,1);
        }
        ret = m_lenStatAdd;
        break;
    default:
        for(i=0;i<7;i++){
            if(m_baStatFile.size()>m_lenStatAdd){
                b8 = m_baStatFile.at(m_lenStatAdd);
                m_baStat.append(b8);
                m_lenStatAdd++;
            }
            else {
                b8 = 0x20;
                m_baStat.append(b8);
            }
        }
        if(m_baStatFile.size()>m_lenStatAdd){
            b8 = m_baStatFile.at(m_lenStatAdd);
            m_baStat.append(b8);
            m_lenStatAdd++;
        }
        else{
            len = m_baStat.size()-2;
            //qDebug(" m_baStat.len :%d     size: %d",len, m_baStat.size());
            bsum = 0;
            for(i=0;i<len;i++) bsum+=m_baStat.at(2+i);
            m_baStat.append(bsum);
            b8 = len>>8;
            m_baStat.replace(0,1,&b8,1);
            b8 = len;
            m_baStat.replace(1,1,&b8,1);
        }
        ret = m_lenStatAdd;
        break;
    }


    //qDebug("  mkBAStat m_baStat.size %d", m_baStat.size());
    return ret;
}

// set m_baStat
int ThreadUdpR::mkBAStat()
{
    int ret=0;
    int n;
    char bSum=0;
    int len;
    int i;
    getBAStatFile();

    m_sumStat=0;
    mkBAStatN(0);
    m_baStat.append(m_baStatFile);
    bSum = 0;
    len = m_baStat.size()-2;
    for(i=0;i<len;i++)bSum+=m_baStat.at(i+2);
    m_baStat.append(bSum);

#if 0
    mkBAStatN(1);
    //mkBAStatN(2);
    if(m_baStatFile.size() <= m_lenStatAdd){
        return 1;// finish
    }
    for(n=2;;n++){
        mkBAStatN(n);
        if(m_lenStatAdd>=m_baStatFile.size()){
            ret = 1;
            break;
        }

    }
    return ret;
#endif
    return 1;
}
int ThreadUdpR::mkBAStatUartN(int n,char id8)
{
    int ret=0;
    int i;
    int id32;
    QByteArray ba16;
    char b8,bsum;
#if 0
    for(i=0;i<8;i++){
        qDebug(" %02x ",m_baStat.at((n<<3)+i));
    }
    qDebug(" -- ");
#endif
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
            b8=m_baStat.at(i);
            ba16.append(b8);
            bsum+=b8;
        }
        ba16.append(bsum);

        b8=0x88;
        ba16.append(b8);

        sigUart(ba16);

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
            b8=m_baStat.at((n<<3)+i);
            ba16.append(b8);
            bsum+=b8;
        }
        ba16.append(bsum);

        b8=0x88;
        ba16.append(b8);

        sigUart(ba16);

        break;
    }

    return ret;
}

int ThreadUdpR::mkBAStatUart()
{
    int ret=0;
    int len;
    int i,n;
    char id8;

    len = m_baStat.size();
    n=len>>3;

    id8=myobject::ID8send;
    myobject::ID8send++;

    for(i=0;i<n;i++)mkBAStatUartN(i,id8);

    return ret;
}
void ThreadUdpR::slotFile8(char b8)
{
    m_stat[3]=b8;
}
void ThreadUdpR::slotRunning8(char b8)
{
    m_stat[2]=b8;
}

// sttatus report
void ThreadUdpR::doRemote()
{
    m_cmdD3 = m_stFrame.buf[1];
    m_nRemote++;
    m_nRemoteOK++;

    // make frame.status
    mkBAStat();
    mkBAStatUart();


}
// remote query
void ThreadUdpR::do514001()
{
    QByteArray ba16;
    char b8;

    b8=0xaa;ba16.append(b8);
    b8=0x55;ba16.append(b8);

    int id32=getID32(0,0x25,1,0,0);
    b8=id32;
    ba16.append(b8);
    b8=id32>>8;
    ba16.append(b8);
    b8=id32>>16;
    ba16.append(b8);
    b8=id32>>24;
    ba16.append(b8);

    b8=0x00;ba16.append(b8);
    b8=0x01;ba16.append(b8);
    b8=0x55;ba16.append(b8);
    b8=0x55;ba16.append(b8);
    b8=0x55;ba16.append(b8);
    b8=0x55;ba16.append(b8);
    b8=0x55;ba16.append(b8);
    b8=0x55;ba16.append(b8);

    b8=0x00;ba16.append(b8);
    b8=0x88;ba16.append(b8);


    sig7755(ba16);

}
void ThreadUdpR::do514002()
{
    sigRun(QString(QString("testbash.sh")));

}

void ThreadUdpR::do5140()
{
    switch(0x0ff & m_stFrame.buf[3]){
    case 0x01:
        do514001();
        break;
    case 0x02:
        do514002();// run bash testbash.sh
        break;
    default:
        break;
    }

}
void ThreadUdpR::doC8(bool bOK,char idv,char id8)
{
    int i;
    int id32;
    QByteArray ba16;
    char b8,bsum,byteOK;

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
    b8=0x97;///////////////// 97 97
    ba16.append(b8);
    bsum+=b8;

    b8=byteOK;////////////////////// byteOK
    ba16.append(b8);
    bsum+=b8;

    b8=idv;
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

    sigUart(ba16);



}

void ThreadUdpR::do51()
{
    switch(0x0ff & m_stFrame.buf[2]){
    case 0x25:// only addr 0x25 valid
        setOnly25(true);
        break;
    case 0x52:// all address valid
        setOnly25(false);
        break;
    case 0x40:
        do5140();
        break;
    default:
        break;
    }

}
// shutdown
void ThreadUdpR::do52()
{
    sigRumCmd(QString("/sbin/shutdown -h now"));

}
// reboot
void ThreadUdpR::do53()
{
    sigRumCmd(QString("/sbin/reboot"));

}

void ThreadUdpR::doShort5()
{
    switch(0x0ff & m_stFrame.buf[1]){
    case 0x01:// user define cmd
        do51();
        break;
    case 0x02:// shutdown
        do52();
        break;
    case 0x03:// restore , do nothing
        do53();
        break;
    default:
        break;
    }

}
// c1.02.(ff,00)
void ThreadUdpR::doFtpAck()
{
    char b82=2;
    char b80=0;
    char b8ff=0x0ff;

    if(b82!=m_stFrame.buf[1])return;
    if(b8ff==m_stFrame.buf[2]){
        sigReleaseFtp();
    }
}

void ThreadUdpR::doSingle()
{
    switch(0x0ff & m_stFrame.buf[0]){
    case 0:// remote req
        doRemote();
        break;
    case 0x05:// short cmd   user.define
        doShort5();
        break;
    case 0x0c8:// upload file end     user.define
        //
        doC8(true,m_stFrame.buf[3],m_stFrame.buf[4]);
        break;
    case 0x0c1:
        doFtpAck();
        break;
    default:
        break;
    }

}
void ThreadUdpR::do1st()
{
    int sn;

    sn=0x0ff & m_stFrame.id;

    switch(0x0ff & m_stFrame.buf[2]){
    case 0x0c6:
        m_nCmd1st = 0x0c6;

        m_dataUpload.setData1(m_stFrame);
        break;
    case 0x0c7:
        m_nCmd1st = 0x0c7;
        m_dataUpload.setData1(m_stFrame);
        break;
    case 0x0b0:
        m_nCmd1st = 0x0b0;
        m_ftp.setData1(m_stFrame);
        break;
    case 0x0f:
        m_nCmd1st = 0x0f;
        if(1==(0x0ff & m_stFrame.buf[3])) m_data30.setData1(m_stFrame);
        break;
    default:
        break;
    }

}
void ThreadUdpR::do2nd()
{
    switch(m_nCmd1st){
    case 0x0c6:
    case 0x0c7:
        m_dataUpload.setData2(m_stFrame);
        break;
    case 0x0b0:
        m_ftp.setData2(m_stFrame);
        break;
    case 0x0f:
        m_data30.setData2(m_stFrame);
        break;
    default:
        break;
    }
}

#define SECS_5 5
#define SECS_120 300
//#define SECS_120 120
void ThreadUdpR::doTimeSync()
{
    QDateTime dt,dtNow,dt1;
    struct timeval tv;

    //syslog(LOG_INFO," func doTimeSync ");

    //char buf[100];
    char b80=0x50;
    char b81=0x05;
    unsigned int tn,th;
    char *p8;
    int i;

    if(b80!=m_stFrame.buf[0] || b81 != m_stFrame.buf[1]) return;
    p8=(char*)&tn;
    for(i=0;i<4;i++)p8[i]=m_stFrame.buf[2+i];
    th=ntohl(tn);
    dt.setTime_t(th);
    //::snprintf(buf,90," ntohl  tn:%08x th:%08x",tn,th);
    //syslog(LOG_INFO,"%s",buf);

    if(!m_isArm){
        syslog(LOG_INFO," ntohl  tn:%08x th:%08x  %d-%d-%d %d:%d:%d",tn,th,
               dt.date().year(),dt.date().month(),dt.date().day(),
               dt.time().hour(),dt.time().minute(),dt.time().second());
        return;
    }
    dtNow = QDateTime::currentDateTime();
    if(dt>dtNow.addSecs(SECS_5)
            || dt<dtNow.addSecs(-SECS_5) ){
//            || dt>m_dtSet.addSecs(SECS_120)
//            || dt<m_dtSet.addSecs(-SECS_120)){
        tv.tv_sec=th;
        tv.tv_usec = 0;
        settimeofday(&tv,NULL);

        dt1 = m_dtSet;
        m_dtSet = dt;

        syslog(LOG_INFO,"doTimeSync %ds new: %d-%d-%d %d:%d:%d   now: %d-%d-%d %d:%d:%d",SECS_5,
               dt.date().year(),dt.date().month(),dt.date().day(),
               dt.time().hour(),dt.time().minute(),dt.time().second(),
               dtNow.date().year(),dtNow.date().month(),dtNow.date().day(),
               dtNow.time().hour(),dtNow.time().minute(),dtNow.time().second()
               );
    }
    else if(
            //dt>dtNow.addSecs(SECS_5)
            //|| dt<dtNow.addSecs(-SECS_5) ){
            dt>m_dtSet.addSecs(SECS_120)
            || dt<m_dtSet.addSecs(-SECS_120)){
        tv.tv_sec=th;
        tv.tv_usec = 0;
        settimeofday(&tv,NULL);

        dt1 = m_dtSet;
        m_dtSet = dt;

        syslog(LOG_INFO,"doTimeSync %ds  new: %d-%d-%d %d:%d:%d  set.old: %d-%d-%d %d:%d:%d",SECS_120,
               dt.date().year(),dt.date().month(),dt.date().day(),
               dt.time().hour(),dt.time().minute(),dt.time().second(),
               dt1.date().year(),dt1.date().month(),dt1.date().day(),
               dt1.time().hour(),dt1.time().minute(),dt1.time().second()
               );
    }

}
bool ThreadUdpR::isID25(int IDdes)
{
    if(!m_only25)return true;
    if(IDdes==MY_CAN_ID)return true;
    else return false;
}

// req.stat b28-21 src   b20-b13.des   b12-b9 type     b8 info   b7-b0 ID
//      000     0000.0000      0010.0101     0001            0          0000
//                                        single
//      0x00          0x04           a2                                 0x00
void ThreadUdpR::parseID32type()
{
    switch(m_stFrame.type){
    case 0:// restore frame    do nothing                 feng....can.c=>parse_can_packet()
        break;
    case 1:// cmd.single frame
        if(isID25(m_stFrame.des)) doSingle();
        break;
    case 2:// cmd.complex frame 1st
        if(isID25(m_stFrame.des)) do1st();
        break;
    case 3:// cmd.complex frame continue
        if(isID25(m_stFrame.des)) do2nd();
        break;
    case 4:// time sync frame
        doTimeSync();
        break;
    default:
        break;
    }
}

void ThreadUdpR::toStFrame(QByteArray ba, st_frame *pStFrame)
{
#if false
    // normail id32
    pStFrame->id32 = ((0x0ff&ba.at(2))<<24)
            | ((0x0ff & ba.at(3))<<16)
            | ((0x0ff & ba.at(4))<<8)
            | (0x0ff & ba.at(5));
#else
    // uart id32 ,
    pStFrame->id32 = ((0x0ff&ba.at(5))<<24)
            | ((0x0ff & ba.at(4))<<16)
            | ((0x0ff & ba.at(3))<<8)
            | (0x0ff & ba.at(2));
#endif
    pStFrame->len = 8;
    memcpy(pStFrame->buf,ba.data()+6,8);

    pStFrame->id=0x0ff & pStFrame->id32;
    pStFrame->info = 1 & (pStFrame->id32 >>8 );
    pStFrame->type = 0x0f & (pStFrame->id32 >>9 );
    pStFrame->des = 0x0ff & (pStFrame->id32 >>13 );
    pStFrame->src = 0x0ff & (pStFrame->id32 >>21 );

    syslog(LOG_INFO,"  id32:%08x src8:%02x des8:%02x type4:%01x info1:%01x id8:%02x",
           pStFrame->id32,
           pStFrame->src,
           pStFrame->des,
           pStFrame->type,
           pStFrame->info,
           pStFrame->id
           );

}
// feng.check_packet()
// 55 aa id32 data8 sum 88   2+4+8+1+1

bool ThreadUdpR::hasValid16()
{
    bool b=false;
    while(m_ba16.size()>=16){
        if((char)FRAME_1ST_CHAR != m_ba16.at(0)){
            m_ba16.remove(0,1);
            continue;
        }
        if((char)FRAME_2ND_CHAR != m_ba16.at(1)){
            m_ba16.remove(0,2);
            continue;
        }
        if((char)FRAME_ENDING_CHAR != m_ba16.at(15)){
            m_ba16.remove(0,2);
            continue;
        }
        b=true;
        break;
    }
    return b;
}
void ThreadUdpR::printBA(QByteArray ba)
{
    QString str,str1;
    int len=ba.size();
    for(int i=0;i<len;i++){
        str1.sprintf(" %02x",0x0ff & ba.at(i));
        str.append(str1);
    }
    qDebug(" len:%d  :: %s",len,str.toLatin1().data());

}

void ThreadUdpR::print16ba()
{
    QString str,str1;
    for(int i=0;i<2;i++){
        str1.sprintf(" %02x",0x0ff & m_ba16.at(i+0));
        str+=str1;
    }
    str1.sprintf(" -- ");
    str+=str1;
    for(int i=0;i<4;i++){
        str1.sprintf(" %02x",0x0ff & m_ba16.at(i+2));
        str+=str1;
    }
    str1.sprintf(" - ");
    str+=str1;
    for(int i=0;i<8;i++){
        str1.sprintf(" %02x",0x0ff & m_ba16.at(i+6));
        str+=str1;
    }
    str1.sprintf("  -- ");
    str+=str1;
    for(int i=0;i<2;i++){
        str1.sprintf(" %02x",0x0ff & m_ba16.at(i+14));
        str+=str1;
    }

    syslog(LOG_INFO," (%d) validFrame16: %s",myobject::snLog++,
           //Q_FUNC_INFO,
           str.toLatin1().data());
}

void ThreadUdpR::print16()
{
    int i;
    char buf[200];
    char buf1[20];
    buf[0]=0;
    for(i=0;i<16;i++){
        sprintf(buf1,"%02x ",0x0ff & m_pbuf16[i]);
        strcat(buf,buf1);
    }
    syslog(LOG_INFO,"valid frame16: %s",buf);
}

void ThreadUdpR::newChar(char ch)
{
    //qDebug(" newchar 1byte");
    m_q.enqueue(ch);
    //emit sigTest();

    m_wait.wakeOne();

}
void ThreadUdpR::testNewFrame()
{
    st_frame f;
    f.id32=0x001fa800;
    f.len=8;
    f.buf[0]=0x50;
    f.buf[1]=0x05;
    f.buf[2]=0x5b;
    f.buf[3]=0xd6;
    f.buf[4]=0x57;
    f.buf[5]=0xf2;
    f.buf[6]=0x0;
    f.buf[7]=0x0;

    m_parser.slotParse(f);
}
