//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include <iostream>

#include <QtCore/QFile>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QAction>

#include "RlpCore/NET/PyWebChannelHelper.h"

#include "RlpRevlens/GUI/WebEngineView.h"

RLP_SETUP_LOGGER(revlens, GUI, WebEngineView)

GUI_WebEngineView::GUI_WebEngineView(GUI_ItemBase* parent) :
    GUI_WebEngineViewItem(parent),
    _script(nullptr),
    _channel(nullptr)
{
    
    
    // _webView->setUrl(QUrl(QStringLiteral("http://www.qt.io")));
    /*
    QFile qWebChannelJsFile(":/qtwebchannel/qwebchannel.js");
    const char webChannelAdditionalScript[] = "new QWebChannel(qt.webChannelTransport, function(channel) {"
                                               "  window.lens = channel.objects.lens;"
                                               "});";
    if (qWebChannelJsFile.open(QIODevice::ReadOnly)) {
        
        RLP_LOG_DEBUG("GUI_WebEngineView::GUI_WebEngineView(): ADDING WEB CHANNEL JAVASCRIPT" << std::endl;
        
        QByteArray qWebChannelJs = qWebChannelJsFile.readAll();
        
        qWebChannelJs.append(webChannelAdditionalScript);
        
        QWebEngineScript wcscript;
        wcscript.setSourceCode(qWebChannelJs);
        wcscript.setName("qwebchannel.js");
        wcscript.setWorldId(QWebEngineScript::MainWorld);
        wcscript.setInjectionPoint(QWebEngineScript::DocumentCreation);
        
        QWebEngineProfile::defaultProfile()->scripts()->insert(wcscript);
    } else {
        RLP_LOG_DEBUG("COULD NOT ADD WEB CHANNEL JAVASCRIPT" << std::endl;
    }
    */
    
    // _webView = new QWebEngineView(parent);
    _page = new QWebEnginePage(_webView);
    _webView->setPage(_page);
    
    connect(
        _webView,
        SIGNAL(loadFinished(bool)),
        this,
        SLOT(onLoadFinished(bool)));
    
    // view.resize(1024, 750);
    // view.show();
    
    // QVBoxLayout* layout = new QVBoxLayout();
    // layout->addWidget(_webView);
    // setLayout(layout);
}


QQuickItem*
GUI_WebEngineView::create(GUI_ItemBase* parent, QVariantMap toolInfo)
{
    return new GUI_WebEngineView(parent);
}


void
GUI_WebEngineView::addScript(QString& name, QString& sourceCode, int injectionPoint)
{
    RLP_LOG_DEBUG(name);
    
    QWebEngineScript wscript;
    wscript.setName(name);
    wscript.setSourceCode(sourceCode);
    wscript.setWorldId(QWebEngineScript::MainWorld);
    wscript.setInjectionPoint((QWebEngineScript::InjectionPoint)injectionPoint);
    
    QWebEngineProfile::defaultProfile()->scripts()->insert(wscript);
}


void
GUI_WebEngineView::onLoadFinished(bool ok)
{
    RLP_LOG_DEBUG(ok );
    
    if (ok) {
        if (_script != nullptr) {
            RLP_LOG_DEBUG("Loading " << _script->name());
            _webView->page()->runJavaScript(_script->sourceCode());
        }
        
        if (_channel != nullptr) {
            
            // RLP_LOG_DEBUG("GUI_WebEngineView::onLoadFinished(): setting web channel" << std::endl;
            // _webView->page()->setWebChannel(_channel);
        }
    }
}


void
GUI_WebEngineView::setUrl(QString& url)
{
    
    // _webView->setPage(page);
    
    _page->load(url);
    
    // _webView->setUrl(url);
}


void
GUI_WebEngineView::setPostPageLoadScript(QString& name, QString& scriptCode)
{
    _script = new QWebEngineScript();
    _script->setName(name);
    _script->setSourceCode(scriptCode);
    
    _script->setInjectionPoint(QWebEngineScript::Deferred);
    _script->setWorldId(0);
    
    RLP_LOG_DEBUG("adding " << name);
    // QWebEngineProfile::defaultProfile()->scripts()->insert(s);
}


void
GUI_WebEngineView::setWebChannelPyHandler(QString& name, QString& pyModule, QString& pyObj)
{
    RLP_LOG_DEBUG(name)

    NET_WebChannelPyHelper* pyHandler = new NET_WebChannelPyHelper(pyModule, pyObj);
    setWebChannelHandler(name, pyHandler);
}


void
GUI_WebEngineView::setWebChannelHandler(QString name, QObject* handler)
{
    RLP_LOG_DEBUG(name);

    QFile qWebChannelJsFile(":/qtwebchannel/qwebchannel.js");
    
    /*
     * NOTE: client web pages should have this javascript
     * to expose the handler in javascript:
     * In the example, the "name" parameter from this function (setWebChannelHandler())
     * is "revlens"
     * 
     *  new QWebChannel(qt.webChannelTransport, function(channel) {
     * 
     *       console.log('setting up Qt WebChannel for revlens ShotgunHandler');
     *       window.revlens = channel.objects.revlens;
     *  });
     */    
    
    if (qWebChannelJsFile.open(QIODevice::ReadOnly)) {
        
        RLP_LOG_DEBUG("adding web channel javascript");
        
        QByteArray qWebChannelJs = qWebChannelJsFile.readAll();

        QWebEngineScript wcscript;
        wcscript.setSourceCode(qWebChannelJs);
        wcscript.setName("qwebchannel.js");
        wcscript.setWorldId(QWebEngineScript::MainWorld);
        wcscript.setInjectionPoint(QWebEngineScript::DocumentCreation);
        
        // QWebEngineProfile::defaultProfile()->scripts()->insert(wcscript);
        _page->scripts().insert(wcscript);
        
    } else {
        RLP_LOG_ERROR("COULD NOT ADD WEB CHANNEL JAVASCRIPT" );
    }
    
    _channel = new QWebChannel(_page); // view->page());
    _page->setWebChannel(_channel);
    _channel->registerObject(name, handler);
    
}