

#include "RlpCore/NET/NullRequestHandler.h"

RLP_SETUP_LOGGER(core, NET, NullRequestHandler)

void
NET_NullRequestHandler::handle(QVariantMap payload)
{
    RLP_LOG_DEBUG(payload)
}