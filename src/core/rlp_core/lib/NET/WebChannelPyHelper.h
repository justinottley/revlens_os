//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef CORE_NET_PYWEBCHANNELHELPER_H
#define CORE_NET_PYWEBCHANNELHELPER_H

#include "RlpCore/NET/Global.h"

#include "RlpCore/LOG/Logging.h"


class CORE_NET_API NET_WebChannelPyHelper : public QObject {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER

    NET_WebChannelPyHelper();


public slots:
    QVariant call(const QString& method, const QVariantList& argList);


private:

};

#endif
