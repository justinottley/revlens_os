
#ifndef CORE_CRYPT_RSA_H
#define CORE_CRYPT_RSA_H

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "RlpCore/CRYPT/Global.h"

class CORE_CRYPT_API CRYPT_RSA {

public:
    RLP_DEFINE_LOGGER

    CRYPT_RSA();

    void initPrivateKey(QByteArray keyData);

    QString decryptToString(QString& msg);
    QString encryptToString(QString& msg);


private:
    RSA* _privkey;
    RSA* _pubkey;

};

#endif
