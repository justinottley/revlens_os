

#include "RlpCore/LOG/Logger.h"

LOG_Logger::LOG_Logger(QString nameIn, int loglevelIn, LOG_Logger* parent):
    _name(nameIn),
    _loglevel(loglevelIn),
    _parent(parent)
{
    _lock = new QMutex();
}


bool
LOG_Logger::isActive(int loglevelIn)
{
    if ((_loglevel == -1) && (_parent != nullptr)) {
        return _parent->isActive(loglevelIn);
    }

    // qInfo() << _name << " isActive(): " << loglevelIn << " mine: " << _loglevel;

    return loglevelIn >= _loglevel;
}


QString
LOG_Logger::loglevelName(int loglevel)
{
    QString result;

    if (loglevel <= 5) {
        result = "VERBOSE";
    } else if (loglevel <= 19) {
        result = "DEBUG";
    } else if (loglevel <= 29) {
        result = "INFO";
    } else if (loglevel <= 39) {
        result = "WARNING";
    } else if (loglevel <= 49) {
        result = "ERROR";
    } else if (loglevel <= 59) {
        result = "CRITICAL";
    } else {
        result = "FATAL";
    }

    return result;
}


LOG_Logger*
LOG_Logger::getLogger(QStringList logNameList)
{
    QMutexLocker l(_lock);

    // qInfo() << "getLogger(): " << logNameList;

    if (logNameList.size() == 0) {
        qInfo() << "ERROR: SHOULD NOT REACH HERE";
        Q_ASSERT(false);
    }


    if (!_loggers.contains(logNameList[0])) {
        LOG_Logger* nl = new LOG_Logger(logNameList[0], -1, this);
        _loggers.insert(logNameList[0], nl);
    }


    if (logNameList.size() == 1) {
        return _loggers.value(logNameList[0]);

    } else {
        return _loggers.value(logNameList[0])->getLogger(LOG_Logger::topslice(logNameList));

    }
}

QStringList
LOG_Logger::topslice(QStringList logNameList)
{
    // Take a slice [1:] of the name (is there no Qt builtin way to do this?)
    QStringList logNameRest;
    for (int i=1; i != logNameList.size(); ++i) {
        logNameRest.append(logNameList.at(i));
    }

    return logNameRest;
}

