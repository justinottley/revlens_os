//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#include <memory>

// required before Qt if pybind is running in same compiled .so
#include <pybind11/pybind11.h>

#include "RlpCore/LOG/Logging.h"

#include <QtCore/QtGlobal>
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUuid>
#include <QtCore/QVariant>
#include <QtCore/QVariantMap>
#include <QtCore/QBuffer>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QRectF>
#include <QtCore/QDateTime>
#include <QtCore/QMetaMethod>

#include <QtCore/QProcess>

namespace py = pybind11;

#ifndef CORE_PROC_LIB
#define CORE_PROC_API Q_DECL_EXPORT
#else
#define CORE_PROC_API Q_DECL_IMPORT
#endif
