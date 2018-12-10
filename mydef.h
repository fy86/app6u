#ifndef MYDEF_H
#define MYDEF_H

#define MY_CAN_ID 0x25

struct st_frame{
    int id32;
    int len;
    char buf[8];
    int des;
    int src;
    int type;
    int info;
    int id;
    int sum;

};
struct st_c7{
    int len;

};

#define CMD_REMOTE_REQ 0x00

#define CMD_UPLOAD_INIT 0x0c6
#define CMD_UPLOAD_INIT_REPLY 0x099
#define CMD_UPLOAD_SEND_REPLY 0x098
#define CMD_UPLOAD_RUN 0x0c8
#define CMD_UPLOAD_RUN_REPLY 0x097

#define CMD_UPLOAD_DATA 0x0c7

#define FRAME_1ST_CHAR 0xaa
#define FRAME_2ND_CHAR 0x55
#define FRAME_ENDING_CHAR 0x88


#endif // MYDEF_H
