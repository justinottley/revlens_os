//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpCore/LOG/Logging.h"

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QEvent>
#include <QtCore/QUuid>
#include <QtCore/QSet>
#include <QtCore/QBuffer>
#include <QtCore/QJsonObject>
#include <QtCore/QRunnable>
#include <QtCore/QMap>
#include <QtCore/QSet>


#include <QtGui/QImage>
#include <QtGui/QColor>
#include <QtGui/QPainter>

#include <iostream>

#ifndef EXTREVLENS_RLANN_DS_LIB
#define EXTREVLENS_RLANN_DS_API Q_DECL_EXPORT
#else
#define EXTREVLENS_RLANN_DS_API Q_DECL_IMPORT
#endif
