

#include "RlpCore/UTIL/Convert.h"


RLP_SETUP_LOGGER(core, UTIL, Convert)

QMap<QString, UTIL_ConverterBase*> UTIL_Convert::_converterMap;

QJsonParseError UTIL_Convert::LAST_ERROR;

UTIL_Convert::UTIL_Convert()
{
}


bool
UTIL_Convert::registerConverter(QString typeName, UTIL_ConverterBase* conv)
{
    RLP_LOG_DEBUG(typeName)

    if (_converterMap.contains(typeName))
    {
        RLP_LOG_ERROR(typeName << "already registered, skipping")
        return false;
    }

    _converterMap.insert(typeName, conv);

    return true;
}


void
UTIL_Convert::toJsonArray(QVariantList listIn, QJsonArray* result)
{
    RLP_LOG_VERBOSE(listIn)
    
    QVariantList::iterator lit;

    for (lit = listIn.begin(); lit != listIn.end(); ++lit) {
        QVariant val = *lit;
        QString typeName = val.typeName();
        
        if (typeName == "int") {
            result->append(val.toInt());

        } else if (typeName == "QString") {
            result->append(val.toString());

        } else if (typeName == "double") {
            result->append(val.toDouble());
        
        } else if (typeName == "float") {
            result->append(val.toFloat());

        } else if (typeName == "bool") {
            result->append(val.toBool());

        } else if (typeName == "QVariantList") {

            QJsonArray listResult;
            QVariantList rlist = val.toList();

            toJsonArray(rlist, &listResult);

            result->append(listResult);

        } else if (typeName == "QVariantMap") {

            QJsonObject jresult;
            QVariantMap rmap = val.toMap();

            toJsonObject(rmap, &jresult);

            result->append(jresult);

        }else if (_converterMap.contains(typeName)) {
            
            UTIL_ConverterBase* conv = _converterMap.value(typeName);

            RLP_LOG_VERBOSE(typeName << " : using converter " << conv->name())

            QJsonObject mapResult = conv->toJsonObject(val);
            
            result->append(mapResult);

        } else {

            RLP_LOG_ERROR("Unknown type: " << typeName)
        }
    }
}


void
UTIL_Convert::toJsonObject(QVariantMap mapIn, QJsonObject* result)
{
    // RLP_LOG_DEBUG(mapIn)
    
    QVariantMap::iterator mit;

    for (mit = mapIn.begin(); mit != mapIn.end(); ++mit) {

        QString key = mit.key();
        QVariant value = mit.value();
        QString typeName = value.typeName();

        // RLP_LOG_VERBOSE(key << " " << value.typeName())

        if (typeName == "int") {
            result->insert(key, value.toInt());

        } else if (typeName == "QString") {
            result->insert(key, value.toString());

        } else if (typeName == "double") {
            result->insert(key, value.toDouble());

        } else if (typeName == "float") {
            result->insert(key, value.toFloat());

        } else if (typeName == "bool") {
            result->insert(key, value.toBool());

        } else if (typeName == "qlonglong") {
            result->insert(key, value.toLongLong());

        } else if (typeName == "QVariantList") {

            QJsonArray lresult;
            QVariantList lval = value.toList();

            toJsonArray(lval, &lresult);

            result->insert(key, lresult);

        } else if (typeName == "QVariantMap") {

            QJsonObject mresult;
            QVariantMap mval = value.toMap();

            toJsonObject(mval, &mresult);

            result->insert(key, mresult);

        } else if (typeName == "QJsonObject") {
            
            QJsonObject jv = value.toJsonObject();
            result->swap(jv);

        } else if (_converterMap.contains(typeName)) {
            
            UTIL_ConverterBase* conv = _converterMap.value(typeName);

            RLP_LOG_VERBOSE(typeName << " : using converter " << conv->name())

            QJsonObject mapResult = conv->toJsonObject(value);
            
            result->insert(key, mapResult);

        } else {

            // Attempt to see if the object has a toJson slot
            //
            RLP_LOG_ERROR("Unknown type: " << typeName)
        }
    }
}


QString
UTIL_Convert::serializeToJson(QVariantMap mapIn)
{
    return QJsonDocument(QJsonObject::fromVariantMap(mapIn)).toJson(QJsonDocument::Compact);
}


QString
UTIL_Convert::serializeToJson(QJsonObject jobjIn)
{
    return QJsonDocument(jobjIn).toJson(QJsonDocument::Compact);
}


QVariant _JsonValToQVariantHelper(QJsonValue vIn)
{
    qInfo() << "_JsonValToQVariantHelper()" << vIn;

    if (vIn.isString())
    {
        return vIn.toString();
    }
    else if (vIn.isBool())
    {
        return vIn.toBool();
    }
    else if (vIn.isDouble())
    {
        return vIn.toDouble();
    }
    else if (vIn.isNull())
    {
        return QVariant();
    }
    else if (vIn.isArray())
    {
        QVariantList vlist;
        QJsonArray varr = vIn.toArray();
        for (int i = 0; i != varr.size(); ++i)
        {
            QVariant n = _JsonValToQVariantHelper(varr.at(i));
            vlist.push_back(n);
        }
        return vlist;
    } else if (vIn.isObject())
    {
        QVariantMap vmap;
        QJsonObject vo = vIn.toObject();
        QStringList keyList = vo.keys();
        for (int i = 0; i != keyList.size(); ++i)
        {
            QString keyName = keyList.at(i);
            QVariant n = _JsonValToQVariantHelper(vo.value(keyName));
            vmap.insert(keyName, n);
        }

        return vmap;
    }

    return QVariant();
}



QVariantMap
UTIL_Convert::fromJsonString(QString jsonStr)
{
    QVariantMap result;

    QJsonDocument msg = QJsonDocument::fromJson(jsonStr.toUtf8(), &LAST_ERROR);
    
    if (msg.isNull()) {
        RLP_LOG_ERROR("error decoding message as JSON: " << LAST_ERROR.errorString());
        
        return result;
    }
    
    // result = _JsonValToQVariantHelper(QJsonValue(msg.object())).toMap();
    result = msg.object().toVariantMap();

    return result;
}


QVariantMap
UTIL_Convert::jsonifyObj(QVariantMap mapIn)
{
    QJsonObject result;
    UTIL_Convert::toJsonObject(mapIn, &result);

    return result.toVariantMap();
}


QVariantList
UTIL_Convert::jsonifyList(QVariantList listIn)
{
    QJsonArray result;
    UTIL_Convert::toJsonArray(listIn, &result);

    return result.toVariantList();
}