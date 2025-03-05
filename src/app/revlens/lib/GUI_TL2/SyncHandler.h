
//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_TL2_SYNCHANDLER_H
#define REVLENS_GUI_TL2_SYNCHANDLER_H

#include "RlpRevlens/GUI_TL2/Global.h"

#include "RlpRevlens/CNTRL/MainController.h"


class GUI_TL2_View;

class REVLENS_GUI_TL2_API GUI_TL2_SyncHandler : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_SyncHandler(GUI_TL2_View* view);

public slots:

    void onScrubStart(qlonglong frame);
    void onScrubEnd(qlonglong frame);
    void onUpdateToFrame(qlonglong frame);

private:
    GUI_TL2_View* _view;
    CNTRL_MainController* _controller;

};

#endif