//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_UTIL_APPGLOBALS_H
#define REVLENS_UTIL_APPGLOBALS_H

#include "RlpCore/UTIL/Global.h"
#include "RlpCore/PY/PyQVariantMap.h"


class CORE_UTIL_API UTIL_AppGlobals : public QObject {
    Q_OBJECT

signals:
    void changed(QVariantMap globals);
    void updated(QVariantMap entries);

public:
    RLP_DEFINE_LOGGER

    static QString KEY;

    // This is for passing data to/from python and C++
    //
    static UTIL_AppGlobals* instance() { return UTIL_AppGlobals::_instance; }
    void setInstance(UTIL_AppGlobals* instance) { UTIL_AppGlobals::_instance = instance; }
    static UTIL_AppGlobals* _instance;

public slots:

    QVariantMap globals() { return _globals; }
    QVariantMap* globalsPtr() { return &_globals; }
    void setGlobals(QVariantMap globals);

    void update(QVariantMap newEntries);
    bool contains(QString val) { return _globals.contains(val); }
    QVariant value(QString key) { return _globals.value(key); }

    QDateTime buildTime();
    QString buildVersion();


private:
    QVariantMap _globals;
};


#endif