//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include <iostream>

#include "RlpCore/LOG/Logging.h"

#include <QtCore/QtGlobal>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QVariantMap>
#include <QtCore/QMutexLocker>

#include <QtCore/qmath.h>
#include <QtCore/qendian.h>

#include <QtCore/QIODevice>
#include <QtCore/QRunnable>
#include <QtCore/QTimer>
#include <QtCore/QPair>

#include <QtGui/QPainter>

#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioDevice>
#include <QtMultimedia/QAudioSink>
#include <QtMultimedia/QMediaDevices>

#ifndef REVLENS_AUDIO_LIB
#define REVLENS_AUDIO_API Q_DECL_EXPORT
#else
#define REVLENS_AUDIO_API Q_DECL_IMPORT
#endif
