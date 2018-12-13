#include "buffer16.h"

buffer16::buffer16(QObject *parent) :
    QObject(parent)
{
    mnHead=0;
    mnTail=0;
    mnLen=0;
}

void buffer16::add(char ch)
{
    add1(ch);
    check16();

}
void buffer16::add1(char ch)
{
    mbuf[mnTail]=ch;
    mnTail = LEN16_1 & (mnTail+1);
    mnLen++;
}
void buffer16::check16()
{
    int n,n15,n16,n2;
    while(mnLen>=16){
        if(mbuf[mnHead]!=(char)0x55){
            mnHead = LEN16_1 & (mnHead+1);
            mnLen--;
            continue;
        }
        n=LEN16_1 & (mnHead+1);
        if(mbuf[n]!=(char)0xaa){
            mnHead = n;
            mnLen--;
            continue;
        }
        n15=LEN16_1 & (mnHead+15);
        n2=LEN16_1 & (mnHead+2);
        if(mbuf[n15]!=(char)0x88){
            mnHead = n2;
            mnLen-=2;
            continue;
        }
        //qDebug("frame16 found");
        n16=LEN16_1 & (mnHead+16);
        mnHead=n16;
        mnLen-=16;
    }

}
bool buffer16::isValid16()
{
    if(mnLen<16)return false;
    return true;
}
bool buffer16::getValid(char *pbuf16)
{
    int n,n2,n15,n16;
    if(mnLen<16)return false;
    while(mnLen>=16){
        if(mbuf[mnHead]!=(char)0x55){
            mnHead = LEN16_1 & (mnHead+1);
            mnLen--;
            continue;
        }
        n=LEN16_1 & (mnHead+1);
        if(mbuf[n]!=(char)0xaa){
            mnHead = n;
            mnLen--;
            continue;
        }
        n15=LEN16_1 & (mnHead+15);
        n2=LEN16_1 & (mnHead+2);
        if(mbuf[n15]!=(char)0x88){
            mnHead = n2;
            mnLen-=2;
            continue;
        }
        // valid frame16 found
        for(int i=0;i<16;i++){
            n=LEN16_1 & (mnHead +i);
            pbuf16[i]=mbuf[n];
        }
        n16=LEN16_1 & (mnHead+16);
        mnHead=n16;
        mnLen-=16;
        return true;
    }

    return false;
}

