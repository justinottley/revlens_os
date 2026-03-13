//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpCore/LOG/Logging.h"

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QFile>

#include <QtGui/QImage>
#include <QtGui/QColor>
#include <QtGui/QPainter>

#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsItem>

#include <iostream>

#ifndef EXTREVLENS_RLANN_MP_LIB
#define EXTREVLENS_RLANN_MP_API Q_DECL_EXPORT
#else
#define EXTREVLENS_RLANN_MP_API Q_DECL_IMPORT
#endif
