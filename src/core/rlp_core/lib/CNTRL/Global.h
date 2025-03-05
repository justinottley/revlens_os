//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//


#include <pybind11/pybind11.h>


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
#include <QtCore/QFile>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QUrlQuery>

#include <QtNetwork/QSslSocket>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>


#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsObject>

#include "RlpCore/LOG/Logging.h"

#ifndef CORE_CNTRL_LIB
#define CORE_CNTRL_API Q_DECL_EXPORT
#else
#define CORE_CNTRL_API Q_DECL_IMPORT
#endif
