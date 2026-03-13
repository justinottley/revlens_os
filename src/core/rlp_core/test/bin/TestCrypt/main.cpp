

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

#include <string.h>
#include <cstring>
#include <stdlib.h>

#include <iostream>

#include "RlpCore/CRYPT/RSA.h"
#include "RlpCore/CRYPT/AES.h"


const char* RSA_KEY = R"(

)";


void handleErrors()
{
    ERR_print_errors_fp(stderr);
}


void test_rsa()
{

    QByteArray ba(RSA_KEY);
    CRYPT_RSA ce;
    ce.initPrivateKey(ba);

    QString inputStr = "Hello, World!";
    QString result = ce.encryptToString(inputStr);

    qInfo() << "result:";
    qInfo() << result;

    QString dresult = ce.decryptToString(result);

    qInfo() << "decripted result";
    qInfo() << dresult;
}


void
test_aes()
{

    const char* key = "ECF6CE5CBF48D9F98BCEEDCDDB7FAC91";
    const char* iv = "32BF8B56FAF8B4F446D8351ED0DDE7EC";

    QByteArray bakey(key);
    QByteArray baiv(iv);

    CRYPT_AES caes;
    caes.initKey(bakey, baiv);

    QString msg = "hello, world!";

    QByteArray bmsg(msg.toLocal8Bit());
    qInfo() << bmsg;

    QByteArray enc = caes.encryptData(bmsg);
    QByteArray encresult = caes.decryptData(enc);

    qInfo() << encresult;

    // qInfo() << "plaintext length:" << msg.size();
    // QString encresult = caes.encryptToString(msg);

    // qInfo() << encresult;

    // QString result = caes.decryptToString(encresult);
    // qInfo() << "decrypted result:" << result;
}


int main(int argc, char** argv)
{
    // Initialize OpenSSL library
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();

    test_aes();
}