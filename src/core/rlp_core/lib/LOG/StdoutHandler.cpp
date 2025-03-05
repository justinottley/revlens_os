
#include "RlpCore/LOG/StdoutHandler.h"

LOG_StdoutHandler::LOG_StdoutHandler()
{
    qInfo() << "LOG_StdoutHandler::LOG_StdoutHandler()";
}


void
LOG_StdoutHandler::handleLog(QString msg)
{
    fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
}