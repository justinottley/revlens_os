
#ifndef PROD_LAUNCH_CONTROLLER_H
#define PROD_LAUNCH_CONTROLLER_H

#include "RlpProd/LAUNCH/Global.h"

#include "RlpCore/CNTRL/ControllerBase.h"
#include "RlpCore/CNTRL/Auth.h"

#include "RlpGui/PANE/View.h"
#include "RlpGui/MENU/MenuBar.h"


class PROD_LAUNCH_API LAUNCH_Controller : public CoreCntrl_ControllerBase {
    Q_OBJECT

signals:
    // void ioModeChanged(int mode); // for RFS, not available

public:
    RLP_DEFINE_LOGGER

    LAUNCH_Controller(GUI_View* view);


public slots:

    CoreCntrl_Auth* loginCntrl() { return _auth; }

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

private:
    CoreCntrl_Auth* _auth;

    QThread* _serverThread;

    GUI_View* _view;
    GUI_MenuBar* _menuBar;

    QString _startupPyScript;
};

#endif

