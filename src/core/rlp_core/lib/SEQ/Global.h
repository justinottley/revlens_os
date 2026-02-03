
#include <memory>
#include <iostream>

// required before Qt if pybind is running in same compiled .so
#include <pybind11/pybind11.h>

#include "RlpCore/LOG/Logging.h"

#include <QtCore/QtGlobal>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QRegularExpression>
#include <QtCore/QVariantMap>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>


#ifndef CORE_SEQ_LIB
#define CORE_SEQ_API Q_DECL_EXPORT
#else
#define CORE_SEQ_API Q_DECL_IMPORT
#endif
