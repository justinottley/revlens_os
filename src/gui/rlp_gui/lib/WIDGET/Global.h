//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#include <memory>

#include <pybind11/pybind11.h>

#include "RlpCore/LOG/Logging.h"

#include <QtCore/QtGlobal>
#include <QtCore/QDebug>
#include <QtCore/QMetaMethod>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUuid>
#include <QtCore/QVariantMap>
#include <QtCore/QJsonObject>
#include <QtCore/QDataStream>
#include <QtCore/QTimer>

#include <QtGui/QResizeEvent>

#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QBrush>
#include <QtGui/QPen>
#include <QtGui/QFontMetrics>


#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>

#include <QtGui/QMouseEvent>
#include <QtGui/QHoverEvent>

#include <QtQuick/QQuickItem>
#include <QtQuickWidgets/QQuickWidget>

namespace py = pybind11;

#ifndef GUI_WIDGET_LIB
#define GUI_WIDGET_API Q_DECL_EXPORT
#else
#define GUI_WIDGET_API Q_DECL_IMPORT
#endif


