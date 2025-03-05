//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpCore/LOG/Logging.h"

#include <QtCore/QtGlobal>

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QProcessEnvironment>

#include <QtWidgets/QApplication>


#ifndef EXTREVLENS_RLSYNC_LIB
#define EXTREVLENS_RLSYNC_API Q_DECL_EXPORT
#else
#define EXTREVLENS_RLSYNC_API Q_DECL_IMPORT
#endif
