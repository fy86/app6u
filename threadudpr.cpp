#include "threadudpr.h"

ThreadUdpR::ThreadUdpR(QObject *parent) :
    QThread(parent)
{
    m_nCmd1st = 0;
    m_only25 = true;
    m_isArm=QFile::exists(QString("/dev/ttymxc2"));

    m_dtSet.setTime_t(QDateTime::currentDateTime().toTime_t());
    connect(&m_dataUpload,SIGNAL(sigUart(QByteArray)),this,SIGNAL(sigUart(QByteArray)));
    connect(&m_ftp,SIGNAL(sigUart(QByteArray)),this,SIGNAL(sigUart(QByteArray)));
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

    syslog(LOG_INFO," getID32 : %08x  src:%02x des:%02x type:%01x info:%01x id:%02x",id32,src2821,des2013,type1209,info8,id71);

    return id32;
}

int ThreadUdpR::getBAStatFile()
{
    char b8;
    m_baStatFile.clear();
    for(int i=0;i<0x40;i++){
        b8 = i + 0x10;
        m_baStatFile.append(b8);
    }
    return 20;
}
int ThreadUdpR::mkBAStatN(int n)
{
    int ret=0;
    char b8,bsum;
    int i;
    int len;

    //qDebug(" mkba.stat.n : %d",n);

    switch(n){
    case 0:// 1st frame
        m_baStat.clear();
        m_baStat.append((char)0x0);
        m_baStat.append((char)0x0);//len

        m_baStat.append((char)0x35);// fix
        m_baStat.append((char)0x01);// fix

        // byte 5 base1
        m_baStat.append((char)0x55);// req number

        // byte 6 can.ok.number
        m_baStat.append((char)0xaa);// ok.number
        m_baStat.append((char)0x0);// err.number
        // byte 8 base1
        m_baStat.append((char)0x5a);// lastest cmd.code

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
            qDebug(" m_baStat.len :%d     size: %d",len, m_baStat.size());
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
            qDebug(" m_baStat.len :%d     size: %d",len, m_baStat.size());
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
    getBAStatFile();

    m_sumStat=0;
    mkBAStatN(0);
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
}
int ThreadUdpR::mkBAStatUartN(int n)
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

        id32=getID32(MY_CAN_ID,0,2,0,0);
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

        id32=getID32(MY_CAN_ID,0,3,0,0);
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

    len = m_baStat.size();
    n=len>>3;

    for(i=0;i<n;i++)mkBAStatUartN(i);

    return ret;
}

// sttatus report
void ThreadUdpR::doRemote()
{
    // make frame.status
    mkBAStat();
    mkBAStatUart();


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
    default:
        break;
    }

}

void ThreadUdpR::doShort5()
{
    switch(0x0ff & m_stFrame.buf[1]){
    case 0x01:// user define cmd
        do51();
        break;
    case 0x02:// kill python app
        break;
    case 0x03:// delete python app
        break;
    default:
        break;
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
    case 0x0c6:// upload file start     user.define
        //m_dataUpload.setup(m_stFrame);
        break;
    default:
        break;
    }

}
void ThreadUdpR::do1st()
{
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
    default:
        break;
    }
}

#define SECS_5 5
#define SECS_120 30
//#define SECS_120 120
void ThreadUdpR::doTimeSync()
{
    QDateTime dt,dtNow;
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
            || dt<dtNow.addSecs(-SECS_5)
            || dt>m_dtSet.addSecs(SECS_120)
            || dt<m_dtSet.addSecs(-SECS_120)){
        tv.tv_sec=th;
        tv.tv_usec = 0;
        settimeofday(&tv,NULL);

        m_dtSet = dt;

        syslog(LOG_INFO,"doTimeSync ntohl  tn:%08x th:%08x  %d-%d-%d %d:%d:%d",tn,th,
               dt.date().year(),dt.date().month(),dt.date().day(),
               dt.time().hour(),dt.time().minute(),dt.time().second());
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
    for(int i=0;i<16;i++){
        str1.sprintf(" %02x",0x0ff & m_ba16.at(i));
        str+=str1;
    }
    syslog(LOG_INFO," --%s-- validFrame16: %s",Q_FUNC_INFO,str.toLatin1().data());
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
    qDebug("valid frame16: %s",buf);
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
