//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#include "RlpCore/LOG/Logging.h"
#include "RlpGui/BASE/ItemBase.h"

#include <QtCore/QtGlobal>
#include <QtCore/QDebug>
#include <QtCore/QMetaMethod>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUuid>
#include <QtCore/QVariantMap>
#include <QtCore/QJsonObject>
#include <QtCore/QDataStream>
#include <QtCore/QMimeData>

#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QBrush>
#include <QtGui/QPen>
#include <QtGui/QDrag>
#include <QtGui/QClipboard>
#include <QtGui/QMouseEvent>

#include <QtWidgets/QWidget>


#ifndef GUI_GRID_GLOBAL_H
#define GUI_GRID_GLOBAL_H

enum GridSelectionMode {
    GRID_SEL_REPLACE,
    GRID_SEL_APPEND,
    GRID_SEL_REMOVE
};

#endif


#ifndef GUI_GRID_LIB
#define GUI_GRID_API Q_DECL_EXPORT
#else
#define GUI_GRID_API Q_DECL_IMPORT
#endif
