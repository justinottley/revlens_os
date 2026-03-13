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
#include <QtCore/QProcessEnvironment>
#include <QtCore/QMetaObject>
#include <QtCore/QTextStream>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>

#include <QtWebSockets/QtWebSockets>

#include <QtGui/QFont>
#include <QtGui/QFontMetrics>

#include <QtWidgets/QApplication>

#include "RlpCore/LOG/Logging.h"

#ifndef CORE_NET_LIB
#define CORE_NET_API Q_DECL_EXPORT
#else
#define CORE_NET_API Q_DECL_IMPORT
#endif
