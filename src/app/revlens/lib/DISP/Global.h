//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include <math.h>

#include "RlpCore/LOG/Logging.h"

#include <QtCore/QtGlobal>

#include <QtGui/QPainter>
#include <QtGui/QOpenGLFunctions>

#include <QtOpenGL/QOpenGLShaderProgram>
#include <QtOpenGL/QOpenGLTexture>

class DISP_CompositePlugin;
typedef QMap<QString, DISP_CompositePlugin*> CompositePluginMap;

#ifndef REVLENS_DISP_LIB
#define REVLENS_DISP_API Q_DECL_EXPORT

#else
#define REVLENS_DISP_API Q_DECL_IMPORT
#endif
