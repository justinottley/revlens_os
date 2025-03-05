
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_ARRANGEMENT_BASE_H
#define REVLENS_GUI_ARRANGEMENT_BASE_H

#include "RlpRevlens/GUI/Global.h"

#include "RlpCore/CNTRL/ControllerBase.h"

#include "RlpGui/BASE/Scene.h"
#include "RlpGui/PANE/View.h"
#include "RlpGui/PANE/Pane.h"

class REVLENS_GUI_API GUI_ArrangementBase : public QObject {
    Q_OBJECT

signals:
    void arrangementLoaded();


public:
    RLP_DEFINE_LOGGER

    virtual void init(GUI_PanePtr parent, GUI_View* view, CoreCntrl_ControllerBase* cntrl);

public slots:

    bool _loadArrangement(GUI_Pane* rp, QVariantMap arrangementIn);
    bool loadArrangement(GUI_Pane* rp, QVariantMap arrangementIn);

};

#endif