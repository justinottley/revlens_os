

#include "RlpCore/CNTRL/Auth.h"
#include "RlpCore/CRYPT/Global.h"

#include "RlpCore/UTIL/AppGlobals.h"
#include "RlpCore/PY/Interp.h"



RLP_SETUP_LOGGER(core, CoreCntrl, Auth)

const QString CoreCntrl_Auth::USER_NONE("{00000000-0000-0000-00000000000000000}");

CoreCntrl_Auth::CoreCntrl_Auth():
    _authKey(""),
    _userId(CoreCntrl_Auth::USER_NONE)
{
    _netMgr = new QNetworkAccessManager(this);

    _crypt = new CRYPT_VinCrypt();

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
CoreCntrl_Auth::tryLogin()
{
}


void
CoreCntrl_Auth::loginInit(QString callback)
{
    RLP_LOG_DEBUG("Callback:" << callback)

    QVariant result = PY_Interp::call("rlp.util.read_login_info");

    // RLP_LOG_DEBUG(result);

    if (result.canConvert<QVariantMap>())
    {
        QVariantMap authInfo = result.toMap();

        if (authInfo.size() == 3)
        {
            QString username = authInfo.value("username").toString();
            QString password = authInfo.value("password").toString();
            QString siteUrl = authInfo.value("site_url").toString();

            loginCheck(siteUrl, username, password, callback);
            return;
        }
    }

    emit loginFailed();
}


void
CoreCntrl_Auth::loginCheck(QString siteUrl, QString username, QString password, QString callback)
{
    RLP_LOG_DEBUG(siteUrl << " " << username << "cb:" << callback)

    bool returnResult = false;

    UTIL_AppGlobals* ag = UTIL_AppGlobals::instance();
    QVariantMap* glptr = ag->globalsPtr();

    glptr->insert("edb.site_url", siteUrl);

    QVariantMap loginData;
    loginData.insert("site_url", siteUrl);
    loginData.insert("username", username);
    loginData.insert("password", password);

    QVariantList wArgList;
    wArgList.append(loginData);

    PY_Interp::call("rlp.util.write_login_info", wArgList);

    QVariantList argList;
    argList.append(siteUrl);
    argList.append(username);
    argList.append(password);
    argList.append(callback);

    PY_Interp::call("revlens_prod.site.login", argList);
}


bool
CoreCntrl_Auth::loginDone(QVariantMap loginInfo)
{
    RLP_LOG_DEBUG(loginInfo)

    bool returnResult = false;

    if (!loginInfo.value("result").toBool())
    {
        RLP_LOG_ERROR("Login failed")

        emit loginFailed();
        return false;
    }


    QString siteName = loginInfo.value("site_name").toString();
    QString authKey = loginInfo.value("auth_key").toString();
    QString username = loginInfo.value("username").toString();
    QString userId = loginInfo.value("user_id").toString();
    QString firstName = loginInfo.value("first_name").toString();
    QString lastName = loginInfo.value("last_name").toString();

    RLP_LOG_DEBUG("Auth OK - Site:" << siteName);


    // Save site to AppGlobals so other plugins have acess to authenticated site url

    UTIL_AppGlobals* ag = UTIL_AppGlobals::instance();
    QVariantMap* glptr = ag->globalsPtr();
    glptr->insert("edb.site_name", siteName);
    glptr->insert("edb.username", username);
    glptr->insert("edb.auth_key", authKey);
    glptr->insert("edb.user_id", userId);
    glptr->insert("edb.first_name", firstName);
    glptr->insert("edb.last_name", lastName);

    if (loginInfo.contains("site_url"))
    {
        glptr->insert("edb.site_url", loginInfo.value("site_url").toString());
    }

    returnResult = true;

    _userId = userId;
    _userName = username;

    emit loginSuccessful();


    return returnResult;
}


void
CoreCntrl_Auth::emitLoginFailed()
{
    emit loginFailed();
}


QString
CoreCntrl_Auth::encrypt(QString rawIn)
{
    // RLP_LOG_DEBUG(rawIn)

    return _crypt->encryptToString(rawIn);
}


QString
CoreCntrl_Auth::decrypt(QString encryptedIn)
{
    // RLP_LOG_DEBUG(encryptedIn)

    QString result = _crypt->decryptToString(encryptedIn);

    // RLP_LOG_DEBUG(result)
    return result;
}


void
CoreCntrl_Auth::empty()
{
    RLP_LOG_DEBUG("")
}


QString
CoreCntrl_Auth::getCreds(QString credName)
{
    QString fname = ":";
    fname += credName;
    fname += ".json";

    RLP_LOG_DEBUG(fname)

    QFile f(fname);
    f.open(QIODevice::ReadOnly);

    QString contents = f.readAll();
    return _crypt->encryptToString(contents);
    

}