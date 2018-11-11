#include "frameparser.h"

frameparser::frameparser(QObject *parent) :
    QObject(parent)
{
}


void frameparser::slotParse(st_frame stFrame)
{
    formatFrame(&stFrame);

    //cpFrame(&stFrame);

    switch(0x0f & stFrame.type){
    case 0x00:// restore
        break;
    case 0x01:// cmd single
        break;
    case 0x02:// cmd frames 1st
        break;
    case 0x03:// cmd frames 2nd.....
        break;
    case 0x04:// time sync
        doTimeSync(stFrame);
        break;
    default://
        break;
    }
}

void frameparser::cpFrame(st_frame *pFrame)
{
    int len=sizeof(struct st_frame);
    memcpy(&m_frameIn,pFrame,len);
}
void frameparser::doTimeSync(st_frame stFrame)
{
    time_t tn,thost;
    struct tm *ptm;

    if(stFrame.src!=0)return;
    if(stFrame.des!=0x0fd)return;
    if(stFrame.buf[0]==0x50 && stFrame.buf[1]==0x05){
        memcpy(&tn,stFrame.buf+2,4);
        thost=ntohl(tn);
        ptm=localtime(&thost);
        qDebug("y:%d m:%d d:%d hh:%d mm:%d ss:%d",
               ptm->tm_year,ptm->tm_mon,ptm->tm_mday,
               ptm->tm_hour,ptm->tm_min,ptm->tm_sec);

    }
}

void frameparser::formatFrame(st_frame *pFrame)
{
    pFrame->id=0x0ff & pFrame->id32;
    pFrame->info = 1 & (pFrame->id32 >>8 );
    pFrame->type = 0x0f & (pFrame->id32 >>9 );
    pFrame->des = 0x0ff & (pFrame->id32 >>13 );
    pFrame->src = 0x0ff & (pFrame->id32 >>21 );

}
void frameparser::slotTest(QByteArray ba)
{
    QString str,str1;
    int len=ba.size();
    for(int i=0;i<len;i++){
        str1.sprintf(" %02x",0x0ff & ba.at(i));
        str+=str1;
    }
    qDebug("  slot test: frame: %s",str.toLatin1().data());
}
void frameparser::slotParseBA(QByteArray ba)
{

}
