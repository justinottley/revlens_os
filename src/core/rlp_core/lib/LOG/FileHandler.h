
#ifndef CORE_LOG_FILEHANDLER_H
#define CORE_LOG_FILEHANDLER_H

#include "RlpCore/LOG/Global.h"
#include "RlpCore/LOG/HandlerBase.h"

class CORE_LOG_API LOG_FileHandler : public LOG_HandlerBase {

public:
    LOG_FileHandler(QString filePath="");

    void handleLog(QString msg);


private:
    QFile _logfile;
    bool _ok;
    QMutex* _lock;

};

#endif