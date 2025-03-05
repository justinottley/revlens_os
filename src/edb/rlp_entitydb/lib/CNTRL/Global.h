//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#include <QtCore/QtGlobal>
#include <QtCore/QDebug>
#include <QtCore/QMetaMethod>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUuid>
#include <QtCore/QVariantMap>
#include <QtCore/QJsonObject>
#include <QtCore/QDataStream>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>

#ifndef EDB_CNTRL_LIB
#define EDB_CNTRL_API Q_DECL_EXPORT
#else
#define EDB_CNTRL_API Q_DECL_IMPORT
#endif
