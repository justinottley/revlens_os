
#include "RlpCore/CRYPT/RSA.h"

RLP_SETUP_LOGGER(core, CRYPT, RSA)


void CRYPT_RSA_print_errors()
{
    ERR_print_errors_fp(stderr);
}



RSA*
CRYPT_RSA_extract_public_key(RSA *private_key)
{
    RSA *public_key = RSAPublicKey_dup(private_key);
    if (!public_key)
    {
        CRYPT_RSA_print_errors();
    }

    return public_key;
}


int
CRYPT_RSA_encrypt_with_public_key(RSA *public_key, unsigned char *input, unsigned char *output)
{
    int result = RSA_public_encrypt(strlen((const char *)input) + 1, input, output, public_key, RSA_PKCS1_OAEP_PADDING);
    if (result == -1)
    {
        CRYPT_RSA_print_errors();
    }

    return result;
}



// Function to decrypt data using the private key
int
CRYPT_RSA_decrypt_with_private_key(RSA *private_key, unsigned char *encrypted_input, unsigned char *output)
{
    int result = RSA_private_decrypt(RSA_size(private_key), encrypted_input, output, private_key, RSA_PKCS1_OAEP_PADDING);
    if (result == -1)
    {
        CRYPT_RSA_print_errors();
    }

    return result;
}


// --------------


CRYPT_RSA::CRYPT_RSA():
    _privkey(nullptr),
    _pubkey(nullptr)
{
}


void
CRYPT_RSA::initPrivateKey(QByteArray pkeyData)
{
    if (_privkey != nullptr)
    {
        RLP_LOG_WARN("key already initialized, skipping")
        return;
    }
  
    FILE* fp = fmemopen(pkeyData.data(), pkeyData.size(), "r");
    if (!fp)
    {
        RLP_LOG_ERROR("Error opening private key")
        return;
    }

    RSA* pkey = PEM_read_RSAPrivateKey(fp, nullptr, nullptr, nullptr);
    fclose(fp);

    if (!pkey)
    {
        RLP_LOG_ERROR("Error reading private key")

        CRYPT_RSA_print_errors();
    }

    _privkey = pkey;
    _pubkey = CRYPT_RSA_extract_public_key(pkey);
}



QString
CRYPT_RSA::encryptToString(QString& msg)
{
    unsigned char encryptedData[4096];

    int elength = CRYPT_RSA_encrypt_with_public_key(
        _pubkey,
        reinterpret_cast<unsigned char*>(msg.toUtf8().data()),
        encryptedData
    );

    // Print the encrypted data in hexadecimal
    // printf("Encrypted data (hex):\n");
    // for (int i = 0; i < elength; i++) {
    //     printf("%02x", encrypted_data[i]);
    // }
    // printf("\n");


    QByteArray bresult((const char*)encryptedData, elength);
    return QString(bresult.toBase64());
}


QString
CRYPT_RSA::decryptToString(QString& msg)
{
    QByteArray ba = QByteArray::fromBase64(QByteArray(msg.toLatin1()));

    unsigned char* encryptedData = reinterpret_cast<unsigned char*>(ba.data());

    unsigned char decryptedData[4096];
    int decryptedLength = CRYPT_RSA_decrypt_with_private_key(
        _privkey,
        encryptedData,
        decryptedData
    );

    RLP_LOG_DEBUG(decryptedLength)

    QByteArray baresult((const char*)decryptedData, decryptedLength - 1);

    QString result(baresult);
    return result;
}

