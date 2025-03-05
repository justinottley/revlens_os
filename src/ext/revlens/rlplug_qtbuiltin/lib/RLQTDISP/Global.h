//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpCore/LOG/Logging.h"
#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/Painter.h"

#include <QtCore/QtGlobal>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <QtWidgets/QApplication>

#include <QtCore/QFileInfo>
#include <QtGui/QImage>
#include <QtGui/QImageReader>
#include <QtCore/QDir>
#include <QtCore/QVariantList>
#include <QtCore/QVariantMap>


#include <QtGui/QColor>
#include <QtCore/QVariantMap>

// #include <QtCore/QAbstractAnimation>
// #include <QtCore/QPropertyAnimation>

#include <QtGui/QPainter>

#ifndef EXTREVLENS_RLQTDISP_LIB
#define EXTREVLENS_RLQTDISP_API Q_DECL_EXPORT
#else
#define EXTREVLENS_RLQTDISP_API Q_DECL_IMPORT
#endif
