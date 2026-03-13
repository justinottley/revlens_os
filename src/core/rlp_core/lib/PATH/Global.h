//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#include <memory>

// required before Qt if pybind is running in same compiled .so
#include <pybind11/pybind11.h>

#include <QtCore/QtGlobal>
#include <QtCore/QDebug>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUuid>
#include <QtCore/QVariantMap>
#include <QtCore/QFile>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>

#include <QtGui/QFont>
#include <QtGui/QFontMetrics>

#include "RlpCore/LOG/Logging.h"

#ifndef CORE_PATH_GLOBAL_H
#define CORE_PATH_GLOBAL_H

const QString POSIX_SEP = "/";
const QString WIN_SEP = "\\";

#endif

#ifndef CORE_PATH_LIB
#define CORE_PATH_API Q_DECL_EXPORT
#else
#define CORE_PATH_API Q_DECL_IMPORT
#endif
