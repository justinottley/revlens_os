//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpCore/LOG/Logging.h"

#include <QtCore/QtGlobal>
#include <QtCore/QtDebug>

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>

#include <QtWidgets/QFileDialog>

#ifndef REVLENS_GUI_LIB
#define REVLENS_GUI_API Q_DECL_EXPORT
#else
#define REVLENS_GUI_API Q_DECL_IMPORT
#endif
