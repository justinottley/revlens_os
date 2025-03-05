
#ifndef CORE_LOG_STDOUTHANDLER_H
#define CORE_LOG_STDOUTHANDLER_H

#include "RlpCore/LOG/HandlerBase.h"


class CORE_LOG_API LOG_StdoutHandler : public LOG_HandlerBase {

public:
    LOG_StdoutHandler();

    void handleLog(QString msg);

};

#endif