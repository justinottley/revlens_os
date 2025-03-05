//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpCore/UTIL/AppGlobals.h"

RLP_SETUP_LOGGER(core, UTIL, AppGlobals)

// REVLENS_UTIL_API QVariantMap UTIL_AppGlobals::_globals;
CORE_UTIL_API UTIL_AppGlobals* UTIL_AppGlobals::_instance = nullptr;

QString UTIL_AppGlobals::KEY = "5f56e0dc-0920-4dc9-802e-58c3ffd676e6";


void
UTIL_AppGlobals::setGlobals(QVariantMap globals)
{
    RLP_LOG_DEBUG("");
    
    _globals = globals;
    emit changed(globals);
}


void
UTIL_AppGlobals::update(QVariantMap newEntries)
{
    _globals.insert(newEntries);

    emit updated(newEntries);
}


QDateTime
UTIL_AppGlobals::buildTime()
{
    return QDateTime::fromSecsSinceEpoch(SCAFFOLD_BUILD_TIME);
}


QString
UTIL_AppGlobals::buildVersion()
{
    QDate bd = buildTime().date();
    QString by = QString("%1").arg(bd.year()).sliced(2);

    QString bm = QString("%1").arg(bd.month());
    if (bm.size() == 1)
    {
        bm = bm.prepend("0");
    }

    QString sbd = QString("%1").arg(bd.day());
    if (sbd.size() == 1)
    {
        sbd = sbd.prepend("0");
    }


    QString result = QString("%1.%2.%3").arg(by).arg(bm).arg(sbd);
    return result;
}
