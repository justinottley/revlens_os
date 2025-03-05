//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#include <memory>

// required before Qt if pybind is running in same compiled .so
#include <pybind11/pybind11.h>

#include <QtCore/QtGlobal>
#include <QtCore/QDebug>

#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QColor>
#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtGui/QPainterPath>

#include <QtGui/QHoverEvent>

#include <QtQuick/QQuickPaintedItem>

namespace py = pybind11;

#ifndef GUI_QTGUI_LIB
#define GUI_QTGUI_API Q_DECL_EXPORT
#else
#define GUI_QTGUI_API Q_DECL_IMPORT
#endif
