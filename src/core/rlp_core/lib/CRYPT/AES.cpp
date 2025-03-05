
#include "RlpCore/CRYPT/AES.h"

#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

RLP_SETUP_LOGGER(core, CRYPT, AES)

void CRYPT_AES_handleErrors()
{
    ERR_print_errors_fp(stderr);
}


int
CRYPT_AES_encrypt(
    unsigned char *plaintext, 
    int plaintext_len, 
    unsigned char *key,
    unsigned char *iv, 
    unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        CRYPT_AES_handleErrors();

    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
        CRYPT_AES_handleErrors();


    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        CRYPT_AES_handleErrors();
    ciphertext_len = len;

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        CRYPT_AES_handleErrors();
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}


int
CRYPT_AES_decrypt(
    unsigned char *ciphertext,
    int ciphertext_len, 
    unsigned char *key,
    unsigned char *iv, 
    unsigned char *plaintext)
{

    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        CRYPT_AES_handleErrors();

    /*
     * Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
        CRYPT_AES_handleErrors();

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary.
     */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        CRYPT_AES_handleErrors();
    plaintext_len = len;

    /*
     * Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        CRYPT_AES_handleErrors();


    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

// --------------------

CRYPT_AES::CRYPT_AES():
    _initialized(false)
{
}


void
CRYPT_AES::initKey(QByteArray key, QByteArray initialVector)
{
    RLP_LOG_DEBUG("")

    _key = key;
    _initialVector = initialVector;
    _initialized = true;
}


QString
CRYPT_AES::encryptToString(QString& msg)
{
    if (!_initialized)
    {
        RLP_LOG_ERROR("Keys not initialized");
        return QString();
    }

    std::string text = msg.toStdString();
    size_t textLength = text.size();

    char* ciphertext = (char*) malloc ((sizeof(char) * textLength) + AES_BLOCK_SIZE);

    size_t cipherTextLength = CRYPT_AES_encrypt(
        (unsigned char*)text.c_str(),
        textLength,
        (unsigned char*)_key.data(),
        (unsigned char*)_initialVector.data(),
        (unsigned char*)ciphertext
    );

    QByteArray cba(ciphertext, cipherTextLength);
    delete ciphertext;

    return QString(cba.toBase64());
}


QByteArray
CRYPT_AES::encryptData(QByteArray& inData)
{
    if (!_initialized)
    {
        RLP_LOG_ERROR("Keys not initialized");
        return QByteArray();
    }

    // RLP_LOG_DEBUG("")

    char* ciphertext = (char*) malloc ((sizeof(char) * inData.size()) + AES_BLOCK_SIZE);

    size_t cipherTextLength = CRYPT_AES_encrypt(
        (unsigned char*)inData.data(),
        inData.size(),
        (unsigned char*)_key.data(),
        (unsigned char*)_initialVector.data(),
        (unsigned char*)ciphertext
    );

    QByteArray cba(ciphertext, cipherTextLength);
    delete ciphertext;

    return cba;
}


QString
CRYPT_AES::decryptToString(QString& msg)
{
    if (!_initialized)
    {
        RLP_LOG_ERROR("Keys not initialized");
        return QString();
    }

    QByteArray decstr = QByteArray::fromBase64(msg.toLocal8Bit());

    char* plaintext = (char*) malloc((sizeof(char) * decstr.size()) + 1);

    size_t decLength = CRYPT_AES_decrypt(
        (unsigned char*)decstr.data(),
        decstr.size(),
        (unsigned char*)_key.data(),
        (unsigned char*)_initialVector.data(),
        (unsigned char*)plaintext
    );

    QString result = QString(QByteArray(plaintext, decLength));
    delete plaintext;

    return result;
}


QByteArray
CRYPT_AES::decryptData(QByteArray& inData)
{
    if (!_initialized)
    {
        RLP_LOG_ERROR("Keys not initialized");
        return QByteArray();
    }

    // RLP_LOG_DEBUG("")

    char* plaintext = (char*) malloc((sizeof(char) * inData.size()) + 1);

    size_t decLength = CRYPT_AES_decrypt(
        (unsigned char*)inData.data(),
        inData.size(),
        (unsigned char*)_key.data(),
        (unsigned char*)_initialVector.data(),
        (unsigned char*)plaintext
    );

    QByteArray result = QByteArray(plaintext, decLength);
    delete plaintext;

    return result;
}