#include "threadudpr.h"

ThreadUdpR::ThreadUdpR(QObject *parent) :
    QThread(parent)
{
}

void ThreadUdpR::run()
{
    char ch;
    QByteArray ba,baST;
    for(int n=0;;){
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
            }

            //testNewFrame();
        }
    }

}
void ThreadUdpR::doRemote()
{

}
void ThreadUdpR::doShort5()
{
    switch(0x0ff & m_stFrame.buf[1]){
    case 0x01:// run python app
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
        m_dataUpload.setup(m_stFrame);
        break;
    case 0x99:// just for test
        qDebug(" reply..upload.start   des:%02x    src:%02x",m_stFrame.des,m_stFrame.src);
        break;
    default:
        break;
    }

}
void ThreadUdpR::do1st()
{
    m_dataUpload.setData1(m_stFrame);

}
void ThreadUdpR::do2nd()
{
    m_dataUpload.setData2(m_stFrame);

}
void ThreadUdpR::doTimeSync()
{

}

void ThreadUdpR::parseID32type()
{
    switch(m_stFrame.type){
    case 0:// restore frame    do nothing                 feng....can.c=>parse_can_packet()
        break;
    case 1:// cmd.single frame
        doSingle();
        break;
    case 2:// cmd.complex frame 1st
        do1st();
        break;
    case 3:// cmd.complex frame continue
        do2nd();
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
    pStFrame->id32 = ((0x0ff&ba.at(2))<<24)
            | ((0x0ff & ba.at(3))<<16)
            | ((0x0ff & ba.at(4))<<8)
            | (0x0ff & ba.at(5));
    pStFrame->len = 8;
    memcpy(pStFrame->buf,ba.data()+6,8);

    pStFrame->id=0x0ff & pStFrame->id32;
    pStFrame->info = 1 & (pStFrame->id32 >>8 );
    pStFrame->type = 0x0f & (pStFrame->id32 >>9 );
    pStFrame->des = 0x0ff & (pStFrame->id32 >>13 );
    pStFrame->src = 0x0ff & (pStFrame->id32 >>21 );

    qDebug("id32:%08x id:%02x  info:%02x type:%02x  des:%02x src:%02x",
           pStFrame->id32,pStFrame->id,
           pStFrame->info,pStFrame->type,
           pStFrame->des,pStFrame->src);

}
// feng.check_packet()
// 55 aa id32 data8 sum 88   2+4+8+1+1
bool ThreadUdpR::hasValid16()
{
    bool b=false;
    while(m_ba16.size()>=16){
        if((char)0x55!=m_ba16.at(0)){
            m_ba16.remove(0,1);
            continue;
        }
        if((char)0xaa!=m_ba16.at(1)){
            m_ba16.remove(0,2);
            continue;
        }
        if((char)0x88!=m_ba16.at(15)){
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
    qDebug(" str. valid frame16: %s",str.toLatin1().data());
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
    m_q.enqueue(ch);


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
