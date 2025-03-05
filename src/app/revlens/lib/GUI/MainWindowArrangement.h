
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_MAIN_ARRANGEMENT_H
#define REVLENS_GUI_MAIN_ARRANGEMENT_H

#include "RlpRevlens/GUI/Global.h"

#include "RlpGui/PANE/View.h"
#include "RlpGui/MENU/MenuBar.h"

#include "RlpRevlens/GUI/ArrangementBase.h"
#include "RlpRevlens/GUI/ViewerPane.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/PluginManager.h"

#include "RlpRevlens/DISP/GLView.h"


class REVLENS_GUI_API GUI_MainWindowArrangement : public QObject {
    Q_OBJECT

signals:
    void arrangementLoaded();

    void directoryChosen(QString callId, QString dir);


public:
    RLP_DEFINE_LOGGER

    GUI_MainWindowArrangement();

    void init(GUI_View* view, CNTRL_MainController* cntrl);

public slots:

    GUI_MenuButton* addMenu(QString menuName);

    void close() {}
    GUI_ArrangementBase* arrangement() { return _arr; }
    GUI_View* mainView() { return _mainView; }
    GUI_MenuBar* mainMenuBar() { return _globalMenuBar; }
    GUI_Pane* mainPane() { return _mainView->rootPane()->panePtr(1); }

    GUI_View* createNewWindow();

    void initPresentationWindow(bool showOverlayTimeline=true);
    GUI_View* presentationWindow();
    GUI_ViewerPane* presentationViewer() { return _presentationViewer; }
    bool hasPresentationViewer() { return _presentationViewer != nullptr; }
    void enablePresentationMode();
    void disablePresentationMode();
    void setPresentationWindowSize(int width, int height);

    // This rigmarole is to support calling the QWidget based QFileDialog
    // without messy python thread / GIL issues
    void _chooseDirectoryInternal(QString callId);
    void chooseDirectory(QString callId);

private:

    GUI_MenuBar* _globalMenuBar;
    GUI_ArrangementBase* _arr;
    GUI_View* _mainView;

    GUI_View* _presentationWindow;
    GUI_ViewerPane* _presentationViewer;
};

#endif
