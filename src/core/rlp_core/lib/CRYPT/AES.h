
#ifndef CORE_CRYPT_AES_H
#define CORE_CRYPT_AES_H

#include "RlpCore/CRYPT/Global.h"

class CORE_CRYPT_API CRYPT_AES {

public:
    RLP_DEFINE_LOGGER

    CRYPT_AES();

    bool initialized() { return _initialized; }

    void initKey(QByteArray initKey, QByteArray initialVector);

    QString decryptToString(QString& msg);
    QByteArray decryptData(QByteArray& inData);

    QString encryptToString(QString& msg);
    QByteArray encryptData(QByteArray& inData);

private:
    QByteArray _key;
    QByteArray _initialVector;

    bool _initialized;

};

#endif
