
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_GUI_EDBGRID_H
#define EDB_GUI_EDBGRID_H

#include "RlpGui/GRID/View.h"

#include "RlpEdb/GUI/Global.h"
#include "RlpEdb/CNTRL/Query.h"

class EDB_GUI_API EdbGui_GridView : public GUI_GRID_View {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbGui_GridView(EdbCntrl_Query* qcntrl, GUI_ItemBase* parent);

    void initFromEntityType(QVariantMap entityInfo);


private:

    EdbCntrl_Query* _qcntrl;

};

#endif