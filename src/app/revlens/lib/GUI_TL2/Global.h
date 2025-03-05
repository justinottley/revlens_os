//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include <iostream>
#include <memory>

#include "RlpCore/LOG/Logging.h"

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/Scene.h"

#include <QtCore/QtGlobal>
#include <QtCore/QBuffer>

#include <QtCore/QTimer>

#include <QtGui/QPainter>
#include <QtGui/QBrush>
#include <QtGui/QPen>
#include <QtGui/QCursor>

#include <QtGui/QResizeEvent>
#include <QtGui/QMouseEvent>

#include <QtCore/QUuid>


#include <QtWidgets/QMenu>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMessageBox>


#ifndef REVLENS_GUI_TL2_LIB
#define REVLENS_GUI_TL2_API Q_DECL_EXPORT
#else
#define REVLENS_GUI_TL2_API Q_DECL_IMPORT
#endif
