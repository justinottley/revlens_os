

#include "RlpCore/CRYPT/VinCrypt.h"

#include "RlpCore/CRYPT/vigenere.h"

CRYPT_VinCrypt::CRYPT_VinCrypt():
    _key("0xf03f2061eecb9b98")
{
}


QString
CRYPT_VinCrypt::decryptToString(QString& msg)
{
    std::string eMsg = msg.toStdString();
    std::string result = VIG_decrypt(eMsg, _key);

    QString qstr(result.c_str());

    return qstr;
}


QString
CRYPT_VinCrypt::encryptToString(QString& msg)
{
    std::string eMsg = msg.toStdString();
    std::string result = VIG_encrypt(eMsg, _key);

    QString qstr(result.c_str());
    return qstr;
}
