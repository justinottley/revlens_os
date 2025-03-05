#ifndef GUI_WEBENGINE_ITEM_H
#define GUI_WEBENGINE_ITEM_H

#include "RlpGui/WIDGET/Global.h"
#include "RlpGui/BASE/ItemBase.h"

#include <QtQuick/QQuickView>

#include <QtQml/QQmlEngine>
#include <QtQml/QQmlComponent>

#include <QtWebChannel/QWebChannel>
#include <QtWebEngineCore/QWebEngineScript>
#include <QtWebEngineCore/QWebEngineProfile>
#include <QtWebEngineCore/QWebEngineScriptCollection>
#include <QtWebEngineCore/QWebEngineLoadingInfo>
#include <QtWebEngineWidgets/QWebEngineView>

class GUI_WebEngineItem : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_WebEngineItem(GUI_ItemBase* parent, QString url="");

    void initQWebChannel();

    void onParentSizeChanged(qreal width, qreal height);

signals:
    void loadingChanged(int state);

public slots:
    static GUI_WebEngineItem* new_GUI_WebEngineItem(GUI_ItemBase* parent, QString url)
    {
        return new GUI_WebEngineItem(parent, url);
    }

    void setUrl(QString url);
    void reload();

    void addScript(QString name, QString souceCode, int injectionPoint=2);
    void addJSObjectHandler(QString name, QObject* handler);
    void setPostLoadScript(QString name, QString scriptCode);
    

    void printProperty(QString name);

    // Workaround for bug where web viewport stops refreshing after reloading a URL
    void forceResetProfile();

    void setYOffset(int yoffset) { _yoffset = yoffset; }

private slots:
    void onInternalLoadingChanged(const QWebEngineLoadingInfo& info);

private:
    QQuickView* _view;
    QObject* _webview;
    int _yoffset;

    QWebEngineScript* _postLoadScript;

};

#endif