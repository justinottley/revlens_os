

#include "RlpCore/DS/Flags.h"

RLP_SETUP_LOGGER(core, CoreDs, Flags)

int CoreDs_Flags::_ENUMS = 0;
QMutex* CoreDs_Flags::_allLock = new QMutex();
QHash<QString, int> CoreDs_Flags::_flagNameToEnumMap;
QHash<int, QString> CoreDs_Flags::_flagEnumToNameMap;

CoreDs_Flags::CoreDs_Flags()
{
    _lock = new QMutex();
}


bool
CoreDs_Flags::registerFlag(QString flagName)
{
    QMutexLocker l(CoreDs_Flags::_allLock);

    if (CoreDs_Flags::_flagNameToEnumMap.contains(flagName))
    {
        RLP_LOG_ERROR("Flag name already registered:" << flagName)
        return false;
    }

    CoreDs_Flags::_ENUMS += 1;
    int flagEnum = CoreDs_Flags::_ENUMS;
    RLP_LOG_DEBUG(flagName << flagEnum)

    if (CoreDs_Flags::_flagEnumToNameMap.contains(flagEnum))
    {
        RLP_LOG_ERROR("Flag enum already regisrtered:" << flagEnum)
        return false;
    }


    CoreDs_Flags::_flagNameToEnumMap.insert(flagName, flagEnum);
    CoreDs_Flags::_flagEnumToNameMap.insert(flagEnum, flagName);

    return true;
}


bool
CoreDs_Flags::checkFlagByName(QString name)
{
    QMutexLocker l(_lock);

    return _flagNameValMap.value(name);
}


bool
CoreDs_Flags::checkFlagByEnum(int num)
{
    QMutexLocker l(_lock);

    return _flagEnumValMap.value(num);
}


bool
CoreDs_Flags::setFlagByName(QString flagName, bool val)
{
    int flagEnum = getFlagEnum(flagName);
    if (flagEnum < 0)
    {
        return false;
    }


    QMutexLocker l(_lock);

    _flagNameValMap.insert(flagName, val);
    _flagEnumValMap.insert(flagEnum, val);

    emit flagStateChanged(flagName, val);

    return true;
}


bool
CoreDs_Flags::setFlagByEnum(int flagEnum, bool val)
{
    QString flagName = getFlagName(flagEnum);
    if (flagName.size() == 0)
    {
        return false;
    }

    QMutexLocker l(_lock);

    _flagEnumValMap.insert(flagEnum, val);
    _flagNameValMap.insert(flagName, val);

    emit flagStateChanged(flagName, val);

    return true;
}


QString
CoreDs_Flags::getFlagName(int flagEnum)
{
    QMutexLocker l(_allLock);

    if (CoreDs_Flags::_flagEnumToNameMap.contains(flagEnum))
    {
        return CoreDs_Flags::_flagEnumToNameMap.value(flagEnum);
    }

    RLP_LOG_WARN("Unknown flag enum:" << flagEnum)

    return QString();
}


int
CoreDs_Flags::getFlagEnum(QString flagName)
{
    QMutexLocker l(_allLock);

    if (CoreDs_Flags::_flagNameToEnumMap.contains(flagName))
    {
        return CoreDs_Flags::_flagNameToEnumMap.value(flagName);
    }

    RLP_LOG_WARN("Unknown flag name:" << flagName)

    return -1;
}


QVariantList
CoreDs_Flags::flagInfo()
{
    QMutexLocker l(_lock);

    QVariantList result;

    QHash<QString, int>::iterator it;
    for (it = CoreDs_Flags::_flagNameToEnumMap.begin(); it != CoreDs_Flags::_flagNameToEnumMap.end(); ++it)
    {
        QString flagName = it.key();
        int flagEnum = it.value();
        bool val = CoreDs_Flags::_flagNameValMap.value(flagName);

        QVariantMap valInfo;
        valInfo.insert("name", flagName);
        valInfo.insert("num", flagEnum);
        valInfo.insert("value", val);

        result.append(valInfo);
    }

    return result;
}
