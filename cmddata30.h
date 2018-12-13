#ifndef CMDDATA30_H
#define CMDDATA30_H

#include <QObject>

#include "dataupload.h"

class cmdData30 : public dataUpload
{
    Q_OBJECT
public:
    explicit cmdData30(QObject *parent = 0);

    virtual void setData1(struct st_frame st);

    virtual void doData30();

signals:

public slots:
};

#endif // CMDDATA30_H
