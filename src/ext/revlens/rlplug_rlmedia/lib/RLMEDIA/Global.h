//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpCore/LOG/Logging.h"

#include "RlpCore/UTIL/ThreadPool.h" // wasm compat

#include <QtCore/QtGlobal>
#include <QtCore/QList>
#include <QtCore/QVariantMap>
#include <QtCore/QRunnable>
#include <QtCore/QThreadStorage>
#include <QtCore/QCache>

#include <QtCore/QFileInfo>
#include <QtGui/QImage>
#include <QtGui/QImageReader>
#include <QtCore/QDir>


#ifndef EXTREVLENS_RLMEDIA_LIB
#define EXTREVLENS_RLMEDIA_API Q_DECL_EXPORT
#else
#define EXTREVLENS_RLMEDIA_API Q_DECL_IMPORT
#endif
