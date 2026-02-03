#ifndef CORE_CRYPT_VINCRYPT_H
#define CORE_CRYPT_VINCRYPT_H

#include <string>

#include "RlpCore/CRYPT/Global.h"


class CORE_CRYPT_API CRYPT_VinCrypt {

public:

    CRYPT_VinCrypt();
    void setKey(const char* key) { _key = key; }

    QString decryptToString(QString& msg);
    QString encryptToString(QString& msg);


private:
    std::string _key;
};

#endif