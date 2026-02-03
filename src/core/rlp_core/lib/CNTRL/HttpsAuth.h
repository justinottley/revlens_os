
#ifndef CORE_CNTRL_LOGIN_H
#define CORE_CNTRL_LOGIN_H

#include "RlpCore/CNTRL/Global.h"

#include "RlpCore/CRYPT/SimpleCrypt.h"

class CORE_CNTRL_API CoreCntrl_HttpsAuth : public QObject {
    Q_OBJECT

signals:
    void loginFailed();
    void loginSuccessful();
    void initReady();

public:
    RLP_DEFINE_LOGGER

    CoreCntrl_Auth();

    void tryLogin();

public slots:

    bool loginCheck(QString siteUrl, QString username, QString password);
    void loginInitPy();
    void loginInit();

    void csrfReplyFinished();
    void loginReplyFinished();
    
    QString siteName() { return _siteName; }
    QString authKey() { return _authKey; }

protected:

    bool _auth;
    QNetworkAccessManager* _netMgr;

    QNetworkReply* _csrfReply;
    QNetworkReply* _loginReply;

    QString _csrfToken;

    QString _siteName;
    QString _authKey;
};

#endif