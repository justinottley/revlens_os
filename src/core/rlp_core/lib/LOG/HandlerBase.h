
#ifndef CORE_LOG_HANDLERBASE_H
#define CORE_LOG_HANDLERBASE_H

#include "RlpCore/LOG/Global.h"

class CORE_LOG_API LOG_HandlerBase : public QObject {

public:
    LOG_HandlerBase() {}

    virtual void handleLog(QString msg) {}

};

#endif