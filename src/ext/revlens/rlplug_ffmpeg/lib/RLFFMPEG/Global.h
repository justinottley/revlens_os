//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpCore/LOG/Logging.h"

#include "RlpCore/UTIL/ThreadPool.h" // wasm compat

#include <QtCore/QRunnable>
#include <QtCore/QtGlobal>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QTimer>

#ifndef EXTREVLENS_RLFFMPEG_LIB
#define EXTREVLENS_RLFFMPEG_API Q_DECL_EXPORT
#else
#define EXTREVLENS_RLFFMPEG_API Q_DECL_IMPORT
#endif
