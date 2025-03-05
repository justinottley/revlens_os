

#include <QtNetwork/QSslSocket>

#include "RlpCore/UTIL/AppGlobals.h"
#include "RlpCore/PY/Interp.h"

#include "RlpCore/CNTRL/Auth.h"


RLP_SETUP_LOGGER(core, CoreCntrl, HttpsAuth)


CoreCntrl_HttpsAuth::CoreCntrl_HttpsAuth()
{
    _netMgr = new QNetworkAccessManager(this);

    _crypt = new SimpleCrypt();
    _crypt->setKey(CRYPT_KEY);

    /*
    connect(
        _netMgr,
        SIGNAL(finished(QNetworkReply*)),
        this,
        SLOT(replyFinished(QNetworkReply*))
    );
    */


    /*
    connect(
        _netMgr,
        SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
        this,
        SLOT(provideAuthenication(QNetworkReply*,QAuthenticator*))
    );
    */
}



void
CoreCntrl_HttpsAuth::csrfReplyFinished()
{
    RLP_LOG_DEBUG("")

    QByteArray result = _csrfReply->readAll();
    QString sr(result);

    // RLP_LOG_DEBUG(sr)

    QXmlStreamReader sreader(sr);
    while(sreader.readNextStartElement())
    {
        if (sreader.name() == "input")
        {
            QXmlStreamAttributes attribs = sreader.attributes();

            if (attribs.hasAttribute("value"))
            {
                _csrfToken = attribs.value("value").toString();
                RLP_LOG_DEBUG("CSRF OK")

                tryLogin();
            }
        }
    }
    
}


void
CoreCntrl_HttpsAuth::tryLogin()
{
    RLP_LOG_DEBUG("")

    QUrlQuery loginData;
    loginData.addQueryItem("username", "");
    loginData.addQueryItem("password", "");
    loginData.addQueryItem("remember_me", "0");
    loginData.addQueryItem("submit", "1");
    loginData.addQueryItem("csrf_token", _csrfToken);

    QNetworkRequest req(QUrl("https://localhost:8005/rlpsite/login_embedded"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    _loginReply = _netMgr->post(req, loginData.toString(QUrl::FullyEncoded).toUtf8());
    connect(
        _loginReply,
        &QNetworkReply::finished,
        this,
        &CoreCntrl_HttpsAuth::loginReplyFinished
    );
}


void
CoreCntrl_HttpsAuth::loginReplyFinished()
{
    RLP_LOG_DEBUG("")

    QByteArray result = _loginReply->readAll();
    QString rawMsg(result);

    QJsonParseError _error;
    QJsonDocument msg = QJsonDocument::fromJson(rawMsg.toUtf8(), &_error);
    if (msg.isNull())
    {
        RLP_LOG_ERROR("Error parsing login reply")
        return;
    }


    QVariantMap msgObj = msg.object().toVariantMap();

    if (msgObj.contains("auth_key") && msgObj.contains("site_name"))
    {
        _authKey = msgObj.value("auth_key").toString();
        _siteName = msgObj.value("site_name").toString();

        RLP_LOG_DEBUG("Login Successfull")
    }
}


void
CoreCntrl_HttpsAuth::loginInit()
{
    RLP_LOG_DEBUG("")

    QFile f(":localhost.crt");
    f.open(QIODevice::ReadOnly);

    QByteArray r = f.readAll();

    // RLP_LOG_DEBUG(f.exists());

    QList<QSslCertificate> cList = QSslCertificate::fromData(r);
    // RLP_LOG_DEBUG(cList.size())

    QSslCertificate c = cList.at(0);

    QSslSocket::addDefaultCaCertificate(c);

    RLP_LOG_DEBUG("Loaded certificate:" << c.issuerDisplayName())

    _csrfReply = _netMgr->get(QNetworkRequest(QUrl("https://localhost:8005/rlpsite/login_embedded")));
    connect(
        _csrfReply,
        &QNetworkReply::finished,
        this,
        &CoreCntrl_HttpsAuth::csrfReplyFinished
    );
}



void
CoreCntrl_HttpsAuth::loginInitPy()
{

    RLP_LOG_DEBUG("")

    PythonQtObjectPtr mainModule = PythonQt::self()->getMainModule();
    mainModule.evalScript("import revlens_prod.site;");
    
    QVariantList argList;
    argList.append(UTIL_AppGlobals::KEY);
    argList.append(QString("auth"));

    QVariant result = mainModule.call("rlp.util.read_login_info", argList);

    RLP_LOG_DEBUG(result);


    if (result.canConvert<QVariantMap>())
    {
        QVariantMap authInfo = result.toMap();

        if (authInfo.size() == 4)
        {
            QString username = authInfo.value("username").toString();
            QString password = authInfo.value("password").toString();
            QString siteUrl = authInfo.value("site_url").toString();
            QString siteName = authInfo.value("site_name").toString();


            if (loginCheck(siteUrl, username, password))
            {
                UTIL_AppGlobals* ag = UTIL_AppGlobals::instance();
                ag->globalsPtr()->insert("site_name", siteName);

                RLP_LOG_INFO("Site Name:" << siteName);

                // NOTE: early return here
                return;
            }
        }
    }

    emit loginFailed();
}


bool
CoreCntrl_HttpsAuth::loginCheck(QString siteUrl, QString username, QString password)
{
    RLP_LOG_DEBUG(siteUrl << " " << username)

    bool returnResult = false;


    QVariantList argList;
    argList.append(siteUrl);
    argList.append(username);
    argList.append(password);

    QVariant rloginResult = PY_Interp::call("revlens_prod.site.login", argList);

    // RLP_LOG_DEBUG(rloginResult)

    QVariantMap loginResult = rloginResult.toMap();
    bool lresult = loginResult.value("result").toBool();
    if (lresult)
    {
        _auth = true;

        QString siteName = loginResult.value("site_name").toString();

        RLP_LOG_DEBUG("Auth OK:" << siteName);

        QVariantMap loginData;
        loginData.insert("site_name", siteName);
        loginData.insert("site_url", siteUrl);
        loginData.insert("username", username);
        loginData.insert("password", password);

        // Save site to AppGlobals so other plugins have acess to authenticated site url

        UTIL_AppGlobals* ag = UTIL_AppGlobals::instance();
        ag->globalsPtr()->insert("site_name", siteName);
        ag->globalsPtr()->insert("site_url", siteUrl);

        QString siteSlug = siteUrl.replace("https://", "").replace(":", "_");
        ag->globalsPtr()->insert("site_slug", siteSlug);

        QVariantList wArgList;
        wArgList.append(UTIL_AppGlobals::KEY);
        wArgList.append(QString("auth"));
        wArgList.append(loginData);

        QVariant authw = PY_Interp::call("rlp.util.write_login_info", wArgList);

        returnResult = true;

        emit loginSuccessful();


    }

    return returnResult;
}


