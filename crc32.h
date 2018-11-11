#ifndef CRC32_H
#define CRC32_H

#include <QObject>

class crc32 : public QObject
{
    Q_OBJECT
public:
    explicit crc32(QObject *parent = 0);

    void check();

    unsigned int crc_tab[256];

    unsigned int csp_crc32_memory(const unsigned char * data, unsigned int length);

signals:

public slots:

};

#endif // CRC32_H
