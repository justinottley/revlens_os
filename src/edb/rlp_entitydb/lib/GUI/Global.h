//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#include "RlpCore/LOG/Logging.h"

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


#include <QtGui/QResizeEvent>

#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QBrush>
#include <QtGui/QPen>
#include <QtGui/QFontMetrics>


#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>




#ifndef EDB_GUI_LIB
#define EDB_GUI_API Q_DECL_EXPORT
#else
#define EDB_GUI_API Q_DECL_IMPORT
#endif
