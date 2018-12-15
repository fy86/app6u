#ifndef MYFILES_H
#define MYFILES_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDir>
#include <QProcess>

#include "myobject.h"

class myfiles : public myobject
{
    Q_OBJECT
public:
    explicit myfiles(QObject *parent = 0);

    void saveRunning8();

    static char running8;

    QString m_fnRunning;

    void restoreRunning();
    int char2int(char b8);

    QProcess *m_pProcess;
    QProcess m_p0;

    QString m_root;
    QString getFullName(QString path_filename);
    // find file by name
    QString findFile(QString path_filename);

    //void runFile(QString filenameFull);

    bool isBash(QString filenameFull);
    bool isPython(QString filenameFull);

    bool findPythonFile(QString fn);

signals:

public slots:
    void slotRunFile(QString filenameFull);
    void slotRunCmd(QString cmd);

};

#endif // MYFILES_H
