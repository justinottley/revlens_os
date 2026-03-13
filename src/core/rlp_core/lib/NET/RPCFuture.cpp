
#include "RlpCore/NET/RPCFuture.h"


void
CoreNet_RPCFuture::onResultReady(QVariantMap msgObj)
{
    emit resultReady(msgObj);
}