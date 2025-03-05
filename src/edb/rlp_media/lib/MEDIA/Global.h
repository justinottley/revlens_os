

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
#include <QtCore/QEvent>

#include <QtCore/QDir>
#include <QtCore/QByteArray>


#ifndef EDB_MEDIA_LIB

#define EDB_MEDIA_API Q_DECL_EXPORT
#else
#define EDB_MEDIA_API Q_DECL_IMPORT
#endif
