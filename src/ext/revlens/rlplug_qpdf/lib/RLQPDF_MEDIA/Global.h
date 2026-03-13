
#include "RlpCore/LOG/Logging.h"

#include <QtCore/QFileInfo>
#include <QtGui/QImage>
#include <QtGui/QImageReader>
#include <QtCore/QDir>

#include <QtPdf/QPdfDocument>
#include <QtPrintSupport/QPrinter>

#ifndef EXTREVLENS_RLQPDF_MEDIA_LIB
#define EXTREVLENS_RLQPDF_MEDIA_API Q_DECL_EXPORT
#else
#define EXTREVLENS_RLQPDF_MEDIA_API Q_DECL_IMPORT
#endif
