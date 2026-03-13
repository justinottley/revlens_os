//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include <iostream>

#include "RlpCore/NET/WebChannelPyHelper.h"
#include "RlpCore/PY/Interp.h"

RLP_SETUP_LOGGER(core, NET, WebChannelPyHelper)

NET_WebChannelPyHelper::NET_WebChannelPyHelper()
{
}


QVariant
NET_WebChannelPyHelper::call(const QString& method, const QVariantList& argList)
{
    RLP_LOG_DEBUG(method << argList)

    QVariant result = PY_Interp::call(method, argList);
    return result;
}
