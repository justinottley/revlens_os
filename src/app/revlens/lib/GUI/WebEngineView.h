//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_WEBENGINEVIEW_H
#define REVLENS_GUI_WEBENGINEVIEW_H

#include <QtWidgets/QDialog>
#include <QtWebChannel/QWebChannel>

#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebEngineWidgets/QWebEngineScript>
#include <QtWebEngineWidgets/QWebEngineProfile>
#include <QtWebEngineWidgets/QWebEngineScriptCollection>

#include "RlpGui/BASE/VLayout.h"

#include "RlpGui/WIDGET/WebEngineViewItem.h"

#include "RlpCore/LOG/Logging.h"
#include "RlpRevlens/GUI/Global.h"


class REVLENS_GUI_API GUI_WebEngineView : public GUI_WebEngineViewItem {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER
    
    GUI_WebEngineView(GUI_ItemBase* parent);

    static QQuickItem* create(GUI_ItemBase* parent, QVariantMap toolInfo);


private slots:
    
    void onLoadFinished(bool ok);
    
    
public slots:
    
    void setUrl(QString& url);
    void setPostPageLoadScript(QString& name, QString& scriptCode);
    void setWebChannelPyHandler(QString& name, QString& pyModule, QString& pyObj);
    void setWebChannelHandler(QString name, QObject* object);
    void addScript(QString& name, QString& souceCode, int injectionPoint=2);
    
protected:

    // QWebEngineView* _view;

    QWebEnginePage* _page;
    QWebChannel* _channel;
    
    // TODO FIXME UNUSED?
    //
    QWebEngineScript* _script;
    
    
};


class REVLENS_GUI_API GUI_WebEngineViewDecorator : public QObject {
    Q_OBJECT
    
public slots:
    
    GUI_WebEngineView* new_GUI_WebEngineView(GUI_ItemBase* parent) {
        return new GUI_WebEngineView(parent);
    }
};

#endif
