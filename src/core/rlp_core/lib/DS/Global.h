//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#include "RlpCore/LOG/Logging.h"

#include <QtCore/QtGlobal>
#include <QtCore/QDebug>
#include <QtCore/QMetaMethod>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUuid>
#include <QtCore/QVariantMap>
#include <QtCore/QJsonObject>
#include <QtCore/QDataStream>
#include <QtCore/QMimeData>
#include <QtCore/QDir>

#ifndef CORE_DS_LIB
#define CORE_DS_API Q_DECL_EXPORT
#else
#define CORE_DS_API Q_DECL_IMPORT
#endif
