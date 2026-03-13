
#ifndef CORE_UTIL_LOGUTIL_H
#define CORE_UTIL_LOGUTIL_H

#include "RlpCore/UTIL/Global.h"

/* This wrapper is needed for a compile dependency order issue on Windows
 * It doesn't quite work to bind the logging module (on Windows) since
 * The binding system requires core.PY, which is set to compiile after core.LOG
 * since core.PY uses core.LOG logging; so we have this wrapper, which compiles
 * after both core.LOG and core.PY, making Windows happy
 */
class CORE_UTIL_API UTIL_LogUtil : public QObject {
    Q_OBJECT

public slots:
    static void handleLog(const char* msg);

};

#endif
