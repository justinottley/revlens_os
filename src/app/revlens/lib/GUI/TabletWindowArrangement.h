
#ifndef REVLENS_GUI_TABLET_ARRANGEMENT_H
#define REVLENS_GUI_TABLET_ARRANGEMENT_H

#include "RlpRevlens/GUI/Global.h"

#include "RlpRevlens/GUI/ArrangementBase.h"


class GUI_View;
class CNTRL_MainController;

class REVLENS_GUI_API GUI_TabletWindowArrangement : public QObject {
    Q_OBJECT

signals:
    void arrangementLoaded();

public:
    RLP_DEFINE_LOGGER

    GUI_TabletWindowArrangement();

    void init(GUI_View* view, CNTRL_MainController* cntrl);

public slots:
    GUI_View* mainView() { return _mainView; }
    GUI_ArrangementBase* arrangement() { return _arr; }

    bool mainMenuBar() { return false; }
    GUI_Pane* mainPane() { return _mainView->rootPane(); }

private:
    GUI_ArrangementBase* _arr;
    GUI_View* _mainView;

};

#endif
