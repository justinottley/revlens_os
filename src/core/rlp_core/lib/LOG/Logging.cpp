//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//


#include "RlpCore/LOG/Logging.h"


#include "RlpCore/LOG/StdoutHandler.h"
#include "RlpCore/LOG/FileHandler.h"

QMap<QString, LOG_Logger*> LOG_Logging::_nameMap;
LOG_HandlerBase* LOG_Logging::_handler;

const int LOG_Logging::VERBOSE = 5;
const int LOG_Logging::DEBUG = 10;
const int LOG_Logging::INFO = 20;
const int LOG_Logging::WARN = 30;
const int LOG_Logging::ERR = 40;
const int LOG_Logging::CRITICAL = 50;
const int LOG_Logging::FATAL = 60;

// QMutex LOG_Logging::_LOGMUTEX;

bool LOG_Logging::initialized = false;

void rlpMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if ((msg.indexOf("Converting image") != -1) ||
        (msg.indexOf("Creating suitable QNanoBackend") != -1) ||
        (msg.indexOf("Using backend:") != -1) ||
        (msg.indexOf("Empty image:") != -1))
    {
        return;
    }

    QStringList msgParts = msg.split("|||");

    if (msgParts.size() == 4) {

        QString logName = msgParts[0].trimmed();
        int loglevel = msgParts[1].trimmed().toInt();
        QString funcName = msgParts[2].trimmed();
        funcName += "()";
        QString msg = msgParts[3].trimmed();

        LOG_Logger* logger = LOG_Logging::getLogger(logName);

        if (!logger->isActive(loglevel)) {
            return;
        }


        QString fmsg = logName.leftJustified(30);
        fmsg += " [ ";
        fmsg += logger->loglevelName(loglevel).leftJustified(8);
        fmsg += " ] ";
        fmsg += funcName.leftJustified(8);
        fmsg += " - ";
        fmsg += msg;

        if (logger != nullptr)
        {
            LOG_Logging::handler()->handleLog(fmsg);

            #ifdef SCAFFOLD_WASM
            qInfo() << fmsg;
            #endif
            // fprintf(stderr, "%s\n", fmsg.toLocal8Bit().constData());
        }
        
    } else
    {
        // fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
        LOG_Logging::handler()->handleLog(msg);

        #ifdef SCAFFOLD_WASM
        qInfo() << msg;
        #endif
    }
    
}



void
LOG_Logging::init()
{
    if (initialized)
        return;

    #ifdef SCAFFOLD_IOS
    _handler = new LOG_StdoutHandler();
    #else
    _handler = new LOG_StdoutHandler(); // LOG_FileHandler();
    #endif

    // QMutex _LOGMUTEX;
    qInstallMessageHandler(rlpMessageOutput);

    initialized = true;
}


void
LOG_Logging::pprint(QVariantMap mapIn, int indent)
{
    if (indent == 0)
    {
        qInfo() << "";
    }
    QStringList keys = mapIn.keys();
    for (int i =0; i != keys.size(); ++i)
    {
        QString key = keys.at(i);
        QVariant val = mapIn.value(key);
        QString tn = val.typeName();

        QString pad = "  ";

        for (int ii=0; ii != indent; ++ii)
        {
            pad += "  ";
        }

        QString result = pad + key + " : ";

        if (tn == "QVariantMap")
        {
            int nindent = indent + 1;
            QString msg = pad + key + " : {}";
            qInfo() << msg;
            pprint(val.toMap(), nindent);
        }

        else if (tn == "QString")
        {
            result += val.toString();
            qInfo() << result;
        }

        else if (tn == "int")
        {
            result += QString("%1").arg(val.toInt());
            qInfo() << result;
        }

        else if ((tn == "float") || (tn == "double"))
        {
            result += QString("%1").arg(val.toDouble());
            qInfo() << result;
        }

        else if (tn == "QVariantList")
        {
            // Q_ASSERT(false);
            result += "[]";

            QVariantList vals = val.toList();

            qInfo() << result;
            for (int v = 0; v != vals.size(); ++v)
            {
                qInfo() << pad << vals.at(v);
            }
            
            // qInfo() << pad << val;
        }

        else if (tn == "qlonglong")
        {
            result += QString("%1").arg(val.toLongLong());
            qInfo() << result;
        }

        else if (tn == "std::nullptr_t")
        {
            result += "NULL";
            qInfo() << result;
        }
        
        else if (tn == "bool")
        {
            result += QString("%1").arg(val.toBool());
            qInfo() << result;
        }

        else
        {   
            qInfo() << result << ": UNKNOWN TYPE:" << tn;
        }
    }

    if (indent == 0)
    {
        qInfo() << "";
    }
}

/*

bool
LOG_Logging::basicConfig(const char* logname, bool stdout)
{
    // TODO: logging handlers, logging to disk - not implemented yet

    return true;
}
*/

LOG_Logger*
LOG_Logging::getLogger(QString loggerName)
{
    // QMutexLocker(&LOG_Logging::_LOGMUTEX);
    // QMutexLocker(&_LOGMUTEX);

    QStringList logNameList = loggerName.split(".");

    if (!_nameMap.contains(logNameList[0])) {
        LOG_Logger* nl = new LOG_Logger(logNameList[0]);
        _nameMap.insert(logNameList[0], nl);
    }

    if (logNameList.size() == 1) {
        return _nameMap.value(logNameList[0]);

    } else {
        return _nameMap.value(logNameList[0])->getLogger(LOG_Logger::topslice(logNameList));

    }
    
}


void
LOG_Logging::handleLog(const char* msg)
{
    QString smsg(msg);
    _handler->handleLog(msg);
}