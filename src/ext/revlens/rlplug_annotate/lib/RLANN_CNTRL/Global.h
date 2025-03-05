//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpCore/LOG/Logging.h"

#include "RlpCore/UTIL/ThreadPool.h" // wasm compat

#include <QtCore/QtGlobal>
#include <QtCore/QTimer>

#include <QtGui/QImage>
#include <QtGui/QPainter>


#ifndef EXTREVLENS_RLANN_CNTRL_LIB
#define EXTREVLENS_RLANN_CNTRL_API Q_DECL_EXPORT
#else
#define EXTREVLENS_RLANN_CNTRL_API Q_DECL_IMPORT
#endif
