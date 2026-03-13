
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_MAIN_ARRANGEMENT_H
#define REVLENS_GUI_MAIN_ARRANGEMENT_H

#include "RlpRevlens/GUI/Global.h"

#include "RlpGui/PANE/View.h"
#include "RlpGui/MENU/MenuBar.h"

#include "RlpRevlens/GUI/ArrangementLoader.h"
#include "RlpRevlens/GUI_VIEW/Pane.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/PluginManager.h"

#include "RlpRevlens/DISP/GLView.h"


class REVLENS_GUI_API GUI_MainWindowArrangement : public QObject {
    Q_OBJECT

signals:
    void arrangementLoaded();
    void pyGuiReady();
    void registerGUIToolRequested(QVariantMap toolInfo);

    void directoryChosen(QString callId, QString dir);


public:
    RLP_DEFINE_LOGGER

    GUI_MainWindowArrangement();

    void init(GUI_View* view=nullptr);

    static GUI_MainWindowArrangement* instance() { return _INSTANCE; }
    static void setInstance(GUI_MainWindowArrangement* mwa) { _INSTANCE = mwa; }


public slots:

    GUI_MenuButton* addMenu(QString menuName);

    void close() {}
    GUI_ArrangementLoader* arrangementLoader() { return _arr; }
    GUI_View* mainView() { return _mainView; }
    GUI_MenuBar* mainMenuBar() { return _globalMenuBar; }
    GUI_Pane* mainPane() { return _mainView->rootPane()->panePtr(1); }

    void hideMenuPanes();

    GUI_View* createNewWindow();

    void initPresentationWindow(bool showTimeline=true);
    GUI_View* presentationWindow();
    GUI_VIEW_Pane* presentationViewer() { return _presentationViewer; }
    bool hasPresentationViewer() { return _presentationViewer != nullptr; }
    void enablePresentationMode();
    void disablePresentationMode();
    void setPresentationWindowSize(int width, int height);

    // This rigmarole is to support calling the QWidget based QFileDialog
    // without messy python thread / GIL issues
    void _chooseDirectoryInternal(QString callId);
    void chooseDirectory(QString callId);

    void registerToolGUI(QVariantMap toolInfoIn);

    void loadArrangement(QVariantMap arrangementData, bool initView=false);

private:

    GUI_MenuBar* _globalMenuBar;
    GUI_ArrangementLoader* _arr;
    GUI_View* _mainView;

    GUI_View::ToolInfoMap* _tools;

    GUI_View* _presentationWindow;
    GUI_VIEW_Pane* _presentationViewer;

    static GUI_MainWindowArrangement* _INSTANCE;
};

#endif
