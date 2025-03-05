
#include "RlpCore/UTIL/Temp.h"

#include <QtCore/QTemporaryDir>
#include <QtCore/QDir>

RLP_SETUP_LOGGER(core, UTIL, Temp)

QString
UTIL_Temp::mkdtemp(QString prefix)
{
    QTemporaryDir d; // prefix);
    d.setAutoRemove(false);

    RLP_LOG_DEBUG(d.path())

    return d.path();
}


QString
UTIL_Temp::gettempdir()
{
    return QDir::tempPath();
}