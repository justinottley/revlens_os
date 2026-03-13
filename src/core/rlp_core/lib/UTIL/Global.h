//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#include <memory>
#include <iostream>

// required before Qt if pybind is running in same compiled .so
#include <pybind11/pybind11.h>

namespace py = pybind11;

#include "RlpCore/LOG/Logging.h"


#include <QtCore/QtGlobal>
#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUuid>
#include <QtCore/QVariantMap>
#include <QtCore/QBuffer>
#include <QtCore/QList>
#include <QtCore/QHash>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>

#include <QtGui/QFont>
#include <QtGui/QFontMetrics>

#include <QtGui/QPixmap>

#include <QtWidgets/QApplication>


#ifndef CORE_UTIL_LIB
#define CORE_UTIL_API Q_DECL_EXPORT
#else
#define CORE_UTIL_API Q_DECL_IMPORT
#endif
