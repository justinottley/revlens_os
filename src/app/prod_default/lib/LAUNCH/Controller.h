
#ifndef PROD_LAUNCH_CONTROLLER_H
#define PROD_LAUNCH_CONTROLLER_H

#include "RlpProd/LAUNCH/Global.h"

#include "RlpCore/CNTRL/ControllerBase.h"
#include "RlpCore/CNTRL/Auth.h"
#include "RlpCore/NET/WebSocketServer.h"

#include "RlpGui/PANE/View.h"
#include "RlpGui/MENU/MenuBar.h"

#include "RlpRevlens/MEDIA/Locator.h"
#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/DS/EventPlugin.h"


class PROD_LAUNCH_API LAUNCH_Controller : public CoreCntrl_ControllerBase {
    Q_OBJECT

signals:
    // void ioModeChanged(int mode); // for RFS, not available

    void startupReady();
    void toolCreated(QVariantMap toolInfo); // interface compatibility, unused

public:
    RLP_DEFINE_LOGGER

    LAUNCH_Controller(GUI_View* view);


public slots:

    CoreCntrl_Auth* loginCntrl() { return _auth; }

    CoreNet_WebSocketServer* getServerManager() { return _wsServer; }
    LAUNCH_Controller* getPluginManager() { return this; }

    void initPython();
    void initArrangement();
    void initLayout();
    // void initRlpfs(); // For RFS, not available
    void initPyScript();

    void startup();

    GUI_MenuBar* menuBar() { return _menuBar; }

    void showWindow(bool /* unused */);
    void quit(bool /* unused */);

    void setStartupPyScript(QString pyPath) { _startupPyScript = pyPath; }
    void openTool(QString toolName);

    void onWsServerReady(int port);
    void handleRemoteCall(QVariantMap msgObj);


    void registerMediaPlugin(MEDIA_Plugin* plugin);
    void registerMediaLocator(MEDIA_Locator* locator);
    bool registerEventPlugin(DS_EventPlugin* plugin) { return false; }


private:
    CoreCntrl_Auth* _auth;

    // QThread* _serverThread;
    CoreNet_WebSocketServer* _wsServer;

    GUI_View* _view;
    GUI_MenuBar* _menuBar;

    QString _startupPyScript;

    QHash<QString, QString> _runIdIdentMap;
};

#endif

