
#include "RlpCore/LOG/Logging.h"

#include <QtCore/QtGlobal>
#include <QtGui/QOpenGLFunctions>


#ifndef EXTREVLENS_RLEXRDS_LIB
#define EXTREVLENS_RLEXRDS_API Q_DECL_EXPORT
#ifdef _WIN32
#define IMATH_DLL
#endif
#else
#define EXTREVLENS_RLEXRDS_API Q_DECL_IMPORT
#endif
