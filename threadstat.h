#ifndef THREADSTAT_H
#define THREADSTAT_H

#include <QObject>
#include <QThread>

#include <QFile>
#include <QDir>
#include <QString>
#include <QProcess>
#include <QTextStream>

#include "myfiles.h"

class threadStat : public QThread
{
    Q_OBJECT
public:
    explicit threadStat(QObject *parent = 0);

    void restoreRunning();

    bool m_isArm;

    void checkPSstr(QString str);

    int m_sn;
    QProcess m_p0;

    char m_files8;
    char m_running8;
    //char m_running8old;

    myfiles m_myfiles;
    QString m_fnRunning;

    int char2int(char b8);

    void getRunning();

protected:
    virtual void run();
signals:
    void sigFile8(char);
    void sigRunning8(char);

public slots:
};

#endif // THREADSTAT_H
