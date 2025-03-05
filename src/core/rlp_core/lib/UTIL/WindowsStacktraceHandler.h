//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef CORE_UTIL_STACKTRACEHANDLER_H
#define CORE_UTIL_STACKTRACEHANDLER_H

#include "RlpCore/LOG/Logging.h"
#include "RlpCore/UTIL/Global.h"

class CORE_UTIL_API UTIL_StacktraceHandler {

public:
    RLP_DEFINE_LOGGER
    
    
    // static void printStacktrace(int sig, siginfo_t *info, void *secret);
    static void installSignalHandler() {}
};

#endif
