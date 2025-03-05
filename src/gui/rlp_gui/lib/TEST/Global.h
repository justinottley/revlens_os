//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#include <memory>

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

#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickPaintedItem>

#include "RlpCore/LOG/Logging.h"


#ifndef GUI_TEST_LIB
#define GUI_TEST_API Q_DECL_EXPORT
#else
#define GUI_TEST_API Q_DECL_IMPORT
#endif


