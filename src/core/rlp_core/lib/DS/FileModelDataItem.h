
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_DS_FILEMODELDATAITEM_H
#define CORE_DS_FILEMODELDATAITEM_H

#include "RlpCore/DS/Global.h"

#include "RlpCore/LOG/Logging.h"
#include "RlpCore/DS/ModelDataItem.h"




class CORE_DS_API CoreDs_FileModelDataItem : public CoreDs_ModelDataItem {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    CoreDs_FileModelDataItem();

    static CoreDs_ModelDataItemPtr makeItem();

    void populate();


};

#endif