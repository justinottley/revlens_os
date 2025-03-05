//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef CORE_UTIL_STACKTRACEHANDLER_H
#define CORE_UTIL_STACKTRACEHANDLER_H

#ifndef SCAFFOLD_WASM
#ifndef _WIN32
// NOTE: something starting at PythonQt import is causing the siginfo_t struct to go missing..?
// These includes are used in the StacktraceHandler but are included here early
// otherwise siginfo_t is not found.. not sure...
//
#include <signal.h>
#include <execinfo.h>
#include <cxxabi.h>
#endif
#endif

#include "RlpCore/UTIL/Global.h"

class CORE_UTIL_API UTIL_StacktraceHandler {


public:
    RLP_DEFINE_LOGGER
    
    
    #ifndef SCAFFOLD_WASM
    #ifndef _WIN32
    static void printStacktrace(int sig, siginfo_t *info, void *secret);
    #endif
    #endif
    
    static void installSignalHandler();
    
};


#endif
