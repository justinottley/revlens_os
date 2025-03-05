//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_CNTRL_PLUGIN_MANAGER_H
#define REVLENS_CNTRL_PLUGIN_MANAGER_H

#include "RlpRevlens/CNTRL/Global.h"
#include "RlpRevlens/DISP/GLPaintEngine2D.h"
#include "RlpRevlens/MEDIA/Locator.h"

class CNTRL_MainController;

class DS_SyncProbe;
class DS_EventPlugin;
class DS_NodeFactoryPlugin;
class MEDIA_Plugin;

/*
 * Just a helper class to centralize all the plugin registration
 */
class REVLENS_CNTRL_API CNTRL_PluginManager : public QObject {
    Q_OBJECT

signals:
    void registerToolRequested(QVariantMap toolInfo);

public:
    RLP_DEFINE_LOGGER

    CNTRL_PluginManager(CNTRL_MainController* controller);

    bool registerPaintEngine(QString name, PaintEngineCreateFunc func);

public slots:

    bool registerEventPlugin(DS_EventPlugin* plugin);
    bool registerMediaPlugin(MEDIA_Plugin* plugin);
    bool registerMediaLocator(MEDIA_Locator* plugin);
    bool registerNodeFactoryPlugin(DS_NodeFactoryPlugin* plugin);
    bool registerVideoProbe(DS_SyncProbe* probe);
    bool registerToolGUI(QVariantMap toolInfo);

    QVariantList getMediaPlugins();
    QVariant getMediaPlugin(QString name);


private:

    CNTRL_MainController* _controller;

};

#endif

