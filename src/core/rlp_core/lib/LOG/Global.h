
// required before Qt if pybind is running in same compiled .so
#include <pybind11/pybind11.h>


#include <QtCore/QtGlobal>
#include <QtCore/QCoreApplication>
#include <QtCore/QObject>
#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QVariant>
#include <QtCore/QVariantMap>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QProcess>

#ifndef CORE_LOG_LIB
#define CORE_LOG_API Q_DECL_EXPORT
#else
#define CORE_LOG_API Q_DECL_IMPORT
#endif


// prevents compiler warning between QT and native OpenGL header version setting
#define QNANO_QT_GL_INCLUDE