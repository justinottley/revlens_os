//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include <iostream>

#ifndef SCAFFOLD_WASM
#ifndef _WIN32
#include <signal.h>
#include <execinfo.h>
#include <cxxabi.h>
#endif
#endif

#include "RlpCore/UTIL/StacktraceHandler.h"



RLP_SETUP_LOGGER(core, UTIL, StacktraceHandler)

#ifndef SCAFFOLD_WASM
#ifndef _WIN32

void
UTIL_StacktraceHandler::printStacktrace(int sig, siginfo_t *info, void *secret)
{
    

    /*
    stacktrace::call_stack st;
    std::cout << "Received Signal " << sig << std::endl;
    std::cout << "Stacktrace:" << std::endl;
    std::cout << st.to_string() << std::endl;
    */

    void *trace[16];
    char **messages = (char **)NULL;
    int i, trace_size = 0;
    ucontext_t *uc = (ucontext_t *)secret;

    /* Do something useful with siginfo_t */
    if (sig == SIGSEGV) {
        // printf("Got signal %d, faulty address is %p, from %p\n", sig, info->si_addr, uc->uc_mcontext->gregs[REG_EIP]);
        printf("\n\nUTIL_StacktraceHandler::printStacktrace(): [ CRASH ]\nGot signal %d, faulty address is %p\n", sig, info->si_addr);
    }

    else {
        printf("\n\nUTIL_StacktraceHandler::printStacktrace(): [ CRASH ]\nGot signal %d\n", sig);
    }


    trace_size = backtrace(trace, 16);

    /* overwrite sigaction with caller's address */
    // trace[1] = (void *) uc->uc_mcontext->gregs[REG_EIP];

    messages = backtrace_symbols(trace, trace_size);


    // get the length of the longest named library first, then justify
    //
    int libJustifyLength = 0;
    for (i=1; i<trace_size; ++i)
    {
        QString msg(messages[i]);
        QList<QString> msgParts = msg.split(" ", Qt::SkipEmptyParts);
        int libStrLen = msgParts[1].size();
        if (libStrLen > libJustifyLength) {
            libJustifyLength = libStrLen;
        }
    }
    libJustifyLength += 2;

    // Print
    for (i=1; i<trace_size; ++i)
    {
        
        
        QString msg(messages[i]);
        // std::cout << messages[i] << std::endl;

        QList<QString> msgParts = msg.split(" ", Qt::SkipEmptyParts);
        
        std::string finalMsg;
        int status = -1;
        
        const char* mangledName;
        

        if (msgParts.size() > 2) {
            
            // macOS style
            //

            mangledName = msgParts[3].toStdString().c_str();
            char* demangledName = abi::__cxa_demangle(mangledName, NULL, NULL, &status );

            finalMsg += msgParts[0].leftJustified(3).toStdString();
            finalMsg += " ";
            finalMsg += msgParts[1].leftJustified(libJustifyLength).toStdString();
            finalMsg += " ";

            if ( status == 0 ) {
                finalMsg += demangledName;

            } else {
                // std::cout << msgParts[3].toStdString().c_str()  << std::endl;
                finalMsg += msgParts[3].toStdString();
            }

            free( demangledName );

        } else {

            // Linux style
            //

            QList<QString> mparts = msgParts[0].split("(", Qt::SkipEmptyParts);
            QList<QString> soPathParts = mparts[0].split("/");
            QString soName = soPathParts[soPathParts.size() - 1];
            QString mangled = mparts[1].split("+", Qt::SkipEmptyParts)[0];

            char* demangledName = abi::__cxa_demangle(mangled.toStdString().c_str(), NULL, NULL, &status );
            
            finalMsg += soName.leftJustified(34).toStdString();
            finalMsg += " ";
            
            if (status == 0) {
                finalMsg += demangledName;
                
            } else {
                finalMsg += mangled.toStdString().c_str();
            }
            
        }
                
        

        // Print the stackframe
        //
        std::cout << finalMsg.c_str() << std::endl;
        // RLP_LOG_(finalMsg.c_str());

        
    }
    exit(sig);

}

#endif
#endif

void
UTIL_StacktraceHandler::installSignalHandler()
{
    #ifndef SCAFFOLD_WASM
    #ifndef _WIN32


    /* Install our signal handler */
    struct sigaction sa;

    sa.sa_sigaction = &UTIL_StacktraceHandler::printStacktrace;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);

    #endif
    #endif

}

