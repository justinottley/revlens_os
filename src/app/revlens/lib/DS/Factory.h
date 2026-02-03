//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_FACTORY_H
#define REVLENS_DS_FACTORY_H


#include "RlpRevlens/DS/Global.h"
#include "RlpRevlens/DS/Node.h"

class REVLENS_DS_API DS_Factory : public QObject {
    Q_OBJECT

public:

    DS_Factory()
    {
    }

    virtual DS_NodePtr createMedia(QVariantMap mediaIn, bool avOnly=false)
    {
        return nullptr;
    }

    virtual QVariantMap identifyMedia(QVariantMap mediaInput)
    {
        return QVariantMap();
    }
};

#endif