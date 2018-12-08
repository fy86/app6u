#include "myfiles.h"

myfiles::myfiles(QObject *parent) :
    myobject(parent)
{
    if(!m_isArm){// linux
        QDir dirRoot("/home/c/tmp/qt");
        QDir dirUpload("/home/c/tmp/qt/fileupload");
        QDir dirPub("/home/c/tmp/qt/filepub");
        if(dirRoot.exists()){
            if(!dirUpload.exists()) dirRoot.mkdir("fileupload");
            if(!dirPub.exists()) dirRoot.mkdir("filepub");
        }

    }
    else{// arm
        QDir dirRoot("/home/root/qt");
        QDir dirUpload("/home/root/qt/fileupload");
        QDir dirPub("/home/root/qt/filepub");
        if(dirRoot.exists()){
            if(!dirUpload.exists()) dirRoot.mkdir("fileupload");
            if(!dirPub.exists()) dirRoot.mkdir("filepub");
        }

    }

    //QDir d("/home/home/file.txt");
    //syslog(LOG_INFO,"dir path: %s      filename : %s",d.path().toLatin1().data(),d.dirName().toLatin1().data());
}
