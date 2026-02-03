
#include "RlpCore/LOG/Logging.h"

#include <QtCore/QtGlobal>
#include <QtCore/QList>
#include <QtCore/QVariantMap>
#include <QtCore/QThreadPool>
#include <QtCore/QRunnable>
#include <QtCore/QFileInfo>

// #include <QtGui/QImage>
// #include <QtGui/QImageReader>
// #include <QtCore/QDir>

// #include <QtGui/QImage>
// #include <QtGui/QImageReader>
#include <QtWidgets/QApplication>


#ifndef EXTREVLENS_RLEXRMEDIA_LIB
#define EXTREVLENS_RLEXRMEDIA_API Q_DECL_EXPORT
#else
#define EXTREVLENS_RLEXRMEDIA_API Q_DECL_IMPORT
#endif
