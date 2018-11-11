#ifndef BUFFER16_H
#define BUFFER16_H

#include <QObject>


#define LEN16 64
#define LEN16_1 (LEN16-1)

class buffer16 : public QObject
{
    Q_OBJECT
public:
    explicit buffer16(QObject *parent = 0);

    char mbuf[LEN16];
    int mnHead,mnTail,mnLen;

    void add(char ch);
    void add1(char ch);

    void search55();
    void check16();
    bool isValid16();
    bool getValid(char *pbuf16);
signals:

public slots:

};

#endif // BUFFER16_H
