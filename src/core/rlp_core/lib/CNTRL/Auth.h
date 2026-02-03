
#ifndef CORE_CNTRL_LOGIN_H
#define CORE_CNTRL_LOGIN_H

#include "RlpCore/CNTRL/Global.h"
#include "RlpCore/NET/WebSocketClient.h"

#include "RlpCore/CRYPT/VinCrypt.h"

class CORE_CNTRL_API CoreCntrl_Auth : public QObject {
    Q_OBJECT

signals:
    void loginFailed();
    void loginSuccessful();
    void initReady();

public:
    RLP_DEFINE_LOGGER

    static const QString USER_NONE;

    CoreCntrl_Auth();

    void tryLogin();

public slots:

    void loginCheck(QString siteUrl, QString username, QString password, QString callback=QString(""));
    void loginInit(QString callback=QString(""));

    bool loginDone(QVariantMap rloginResult);
    
    void emitLoginFailed();

    QString siteName() { return _siteName; }
    QString authKey() { return _authKey; }
    QString userId() { return _userId; }
    QString userName() { return _userName; }

    QString encrypt(QString rawIn);
    QString decrypt(QString encryptedIn);

    QString getCreds(QString credName);

    void empty();

protected:

    QNetworkAccessManager* _netMgr;

    QNetworkReply* _csrfReply;
    QNetworkReply* _loginReply;

    QString _csrfToken;

    QString _siteName;
    QString _authKey;
    QString _userId;
    QString _userName;

    CRYPT_VinCrypt* _crypt;
};

#endif