//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include <iostream>
#include <math.h>
#include <time.h>
#include <cmath>

#include "RlpCore/LOG/Logging.h"

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QEvent>
#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QVariantList>
#include <QtCore/QVariantMap>

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>


#include <QtCore/QTimer>

#include <QtWidgets/QApplication>


#ifndef REVLENS_VIDEO_LIB
#define REVLENS_VIDEO_API Q_DECL_EXPORT
#else
#define REVLENS_VIDEO_API Q_DECL_IMPORT
#endif
