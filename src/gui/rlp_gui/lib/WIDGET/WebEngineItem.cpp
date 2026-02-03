
#include "RlpGui/WIDGET/WebEngineItem.h"

#include "RlpGui/BASE/Scene.h"
#include "RlpGui/PANE/View.h"

#include "RlpCore/UTIL/AppGlobals.h"
#include "RlpCore/NET/WebChannelPyHelper.h"

#include <QtQml/QtQml>

#include <QtWebEngineQuick/QtWebEngineQuick>
#include <QtWebChannelQuick/QQmlWebChannel>


RLP_SETUP_LOGGER(gui, GUI, WebEngineItem)

GUI_WebEngineItem::GUI_WebEngineItem(GUI_ItemBase* parent, QString url):
    GUI_ItemBase(parent),
    _yoffset(0),
    _profile(nullptr),
    _postLoadScript(nullptr)
{
    setAcceptHoverEvents(true);

    QString storagePath = UTIL_AppGlobals::instance()->globalsPtr()->value("app.config_dir").toString();
    storagePath += "/webengine_storage";

    _profile = new QQuickWebEngineProfile(this);
    _profile->setPersistentCookiesPolicy(QQuickWebEngineProfile::ForcePersistentCookies);
    _profile->setStorageName(QStringLiteral("Default"));
    _profile->setOffTheRecord(false);
    _profile->setPersistentStoragePath(storagePath);
    _profile->setCachePath(storagePath);

    initQWebChannel();

    _view = new QQuickView(parent->scene()->view());
    _view->setResizeMode(QQuickView::SizeRootObjectToView);
    _view->setSource(QUrl::fromLocalFile(":misc/WebView.qml"));

    QQuickItem* ro = _view->rootObject();

    // IMPORTANT!! 
    ro->setParentItem(this); 
    ro->setWidth(parent->width()- 5);
    ro->setHeight(parent->height() - 25);

    _webview = ro->children()[0];

    QVariant vv;
    vv.setValue(_profile);

    _webview->setProperty("profile", vv);

    if (url != "")
    {
        setUrl(url);
    }

    connect(
        _webview,
        SIGNAL(loadingChanged(const QWebEngineLoadingInfo&)),
        this,
        SLOT(onInternalLoadingChanged(const QWebEngineLoadingInfo&))
    );

    addJSObjectHandler("py", new NET_WebChannelPyHelper());
    addJSObjectHandler("app_globals", UTIL_AppGlobals::instance());
}


GUI_WebEngineItem::~GUI_WebEngineItem()
{
    RLP_LOG_DEBUG("")
    delete _profile;
}


void
GUI_WebEngineItem::initQWebChannel()
{
    RLP_LOG_DEBUG("")

    QFile qWebChannelJsFile(":/qtwebchannel/qwebchannel.js");
    if (qWebChannelJsFile.open(QIODevice::ReadOnly))
    {
        QByteArray qWebChannelJs = qWebChannelJsFile.readAll();
        addScript("qwebchannel.js", QString(qWebChannelJs), QWebEngineScript::DocumentCreation);
    }

    /*
     * EXAMPLE USE TO SETUP PYTHON CALL FROM JAVASCRIPT
     *
     * new QWebChannel(qt.webChannelTransport, function(channel) {
     *
     *     console.log('setting up Qt WebChannel for python handler');
     *     window.py = channel.objects.py;
     * });
     * 
     * window.py.call('revlens.cmds.toggle_play_pause', []);
     * 
     */
}


void
GUI_WebEngineItem::setUrl(QString url)
{
    RLP_LOG_DEBUG(url)

    QUrl u(url);
    _webview->setProperty("url", url);
}


void
GUI_WebEngineItem::reload()
{
    RLP_LOG_DEBUG("")
    QMetaObject::invokeMethod(_webview, "reload");
}


void
GUI_WebEngineItem::addScript(QString name, QString sourceCode, int injectionPoint)
{
    RLP_LOG_DEBUG(name);
    // return;

    QWebEngineScript wscript;
    wscript.setName(name);
    wscript.setSourceCode(sourceCode);
    wscript.setWorldId(QWebEngineScript::MainWorld);
    wscript.setInjectionPoint((QWebEngineScript::InjectionPoint)injectionPoint);

    // Headers are not available for QQuickWebEngineScriptCollection (???)
    // work around this via serious magic
    //
    // _profile->scripts()->insert(wscript);
    // _profile->userScripts()->insert(wscript);
    //
    QObject* us = reinterpret_cast<QObject*>(_profile->userScripts());
    QMetaObject::invokeMethod(us, "insert", Q_ARG(QWebEngineScript, wscript));

    QWebEngineProfile::defaultProfile()->scripts()->insert(wscript);
}


void
GUI_WebEngineItem::setPostLoadScript(QString name, QString scriptCode)
{
    RLP_LOG_DEBUG("adding " << name);

    _postLoadScript = new QWebEngineScript();
    _postLoadScript->setName(name);
    _postLoadScript->setSourceCode(scriptCode);
    
    _postLoadScript->setInjectionPoint(QWebEngineScript::Deferred);
    _postLoadScript->setWorldId(0);
}


void
GUI_WebEngineItem::addJSObjectHandler(QString name, QObject* handler)
{
    RLP_LOG_DEBUG(name << handler)

    QVariant v = _webview->property("webChannel");
    QQmlWebChannel* ch = v.value<QQmlWebChannel*>();

    QVariant hw;
    hw.setValue(handler);

    QVariantMap hp;
    hp.insert(name, hw);
    ch->registerObjects(hp);
}


void
GUI_WebEngineItem::printProperty(QString name)
{
    QVariant p = _webview->property(name.toUtf8().constData());
    RLP_LOG_DEBUG(p)
}


void
GUI_WebEngineItem::forceResetProfile()
{
    //
    // wont work with persistent profile
    //

    RLP_LOG_WARN("BYPASSED") 
    return;

    // // Used to work around a bug with the size of the web viewport not refreshing when resizing the window
    // QVariant v = _webview->property("profile");

    // QQuickWebEngineProfile* np = new QQuickWebEngineProfile(_webview);

    // QVariant vv;
    // vv.setValue(np);

    // // Force (internal private object) adapter to reset
    // _webview->setProperty("profile", vv);

    // // Go back to previous profile
    // _webview->setProperty("profile", v);
}


void
GUI_WebEngineItem::onInternalLoadingChanged(const QWebEngineLoadingInfo& info)
{
    // RLP_LOG_DEBUG("")
    // RLP_LOG_DEBUG(info)

    if ((info.status() == QWebEngineLoadingInfo::LoadSucceededStatus) &&
        (_postLoadScript != nullptr))
    {
        // RLP_LOG_DEBUG("Running post load javascript")
        QMetaObject::invokeMethod(
            _webview,
            "runJavaScript",
            Q_ARG(QString, _postLoadScript->sourceCode())
        );

    }

    emit loadingChanged((int)info.status());
}


void
GUI_WebEngineItem::hoverLeaveEvent(QHoverEvent* event)
{
    // Reset keyboard focus to the app
    //
    setFocus(true);
    setFocus(false);
}


void
GUI_WebEngineItem::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(width << height)

    setWidth(width);
    setHeight(height);

    _view->rootObject()->setWidth(width - 5);
    _view->rootObject()->setHeight(height - (35 + _yoffset));
}