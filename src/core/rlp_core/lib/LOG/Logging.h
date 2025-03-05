//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef CORE_LOG_LOGGING_H
#define CORE_LOG_LOGGING_H


#include "RlpCore/LOG/Global.h"
#include "RlpCore/LOG/Logger.h"
#include "RlpCore/LOG/HandlerBase.h"

class CORE_LOG_API LOG_Logging : public QObject {
    Q_OBJECT

public:
    static const int VERBOSE;
    static const int DEBUG;
    static const int INFO;
    static const int WARN;
    static const int ERR;
    static const int CRITICAL;
    static const int FATAL;

public:
    LOG_Logging() {}

    static void init();
    static LOG_Logger* getLogger(QString loggerName);
    static LOG_HandlerBase* handler() { return _handler; }

public slots:

    // static bool basicConfig(const char* logName, bool sout=true);

    static void pprint(QVariantMap mapIn, int indent=0);
    static void handleLog(const char* msg);

private:

    static QMap<QString, LOG_Logger*> _nameMap;
    static LOG_HandlerBase* _handler;

    // static QMutex _LOGMUTEX;

    static bool initialized;

};




#define RLP_LOG_FATAL(INPUT) qFatal() << _logName << "|||" << 50 << "|||" << __func__ << "|||" << INPUT;
#define RLP_LOG_ERROR(INPUT) qCritical() << _logName << "|||" << 40 << "|||" << __func__ << "|||" << INPUT;
#define RLP_LOG_INFO(INPUT) qInfo() << _logName << "|||" << 20 << "|||" << __func__ << "|||" << INPUT;
#define RLP_LOG_WARN(INPUT) qWarning() << _logName << "|||" << 30 << "|||" << __func__ << "|||" << INPUT;
#define RLP_LOG_DEBUG(INPUT) qDebug() << _logName << "|||" << 10 << "|||" << __func__ << "|||" << INPUT;
#define RLP_LOG_VERBOSE(INPUT) qDebug() << _logName << "|||" << 5 << "|||" << __func__ << "|||" << INPUT;

#define RLP_LOG_INFO_MODULE(GROUP, NS, INPUT)  qDebug() << GROUP ## _ ## NS ## _logName << "|||" << 10 << "|||" << __func__ << "|||" << INPUT;
#define RLP_LOG_DEBUG_MODULE(GROUP, NS, INPUT)  qDebug() << GROUP ## _ ## NS ## _logName << "|||" << 10 << "|||" << __func__ << "|||" << INPUT;

#define RLP_DEFINE_LOGGER \
    static const char* _logName; \
    static LOG_Logger* logger() { \
        return LOG_Logging::getLogger(_logName); \
    }


#define RLP_SETUP_APP_LOGGER(NS, LOGNAME) \
    const char* NS ## _ ## LOGNAME::_logName = "app."#NS"."#LOGNAME;

#define RLP_SETUP_CORE_LOGGER(NS, LOGNAME) \
    const char* NS ## _ ## LOGNAME::_logName = "core."#NS"."#LOGNAME;

#define RLP_SETUP_EXT_LOGGER(NS, LOGNAME) \
    const char* NS ## _ ## LOGNAME::_logName = "ext."#NS"."#LOGNAME;

#define RLP_SETUP_LOGGER(GROUP, NS, LOGNAME) \
    const char* NS ## _ ## LOGNAME::_logName = #GROUP"."#NS"."#LOGNAME;

#define RLP_SETUP_3P_LOGGER(GROUP, LOGNAME) \
    const char* LOGNAME::_logName = #GROUP"."#LOGNAME;

#define RLP_SETUP_MODULE_LOGGER(GROUP, NS) \
    const char* GROUP ## _ ## NS ## _logName = #GROUP"."#NS;


#ifdef SCAFFOLD_PYBIND_STATIC
#define SCAFFOLD_PYMODULE PYBIND11_EMBEDDED_MODULE
#else
#define SCAFFOLD_PYMODULE PYBIND11_MODULE
#endif


#endif