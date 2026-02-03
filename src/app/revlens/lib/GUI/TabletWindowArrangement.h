
#ifndef REVLENS_GUI_TABLET_ARRANGEMENT_H
#define REVLENS_GUI_TABLET_ARRANGEMENT_H

#include "RlpRevlens/GUI/Global.h"

#include "RlpRevlens/GUI/ArrangementLoader.h"


class GUI_View;
class CNTRL_MainController;

class REVLENS_GUI_API GUI_TabletWindowArrangement : public QObject {
    Q_OBJECT

signals:
    void arrangementLoaded();
    void pyGuiReady();
    void registerGUIToolRequested(QVariantMap toolInfo);

public:
    RLP_DEFINE_LOGGER

    GUI_TabletWindowArrangement();

    static GUI_TabletWindowArrangement* instance() { return _INSTANCE; }
    static void setInstance(GUI_TabletWindowArrangement* twa) { _INSTANCE = twa; }


    void init();

public slots:
    GUI_View* mainView() { return _mainView; }
    GUI_ArrangementLoader* arrangementLoader() { return _arr; }

    void hideMenuPanes();

    bool mainMenuBar() { return false; }
    GUI_Pane* mainPane() { return _mainView->rootPane()->panePtr(1); }

    void registerToolGUI(QVariantMap toolInfoIn);

    void loadArrangement(QVariantMap arrangementData, bool initView=false);


private:
    GUI_ArrangementLoader* _arr;
    GUI_View* _mainView;

    GUI_View::ToolInfoMap* _tools;

    static GUI_TabletWindowArrangement* _INSTANCE;
};

#endif
