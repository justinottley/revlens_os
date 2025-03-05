
#ifndef CORE_UTIL_CONVERT_H
#define CORE_UTIL_CONVERT_H

#include "RlpCore/UTIL/Global.h"
#include "RlpCore/LOG/Logging.h"


class CORE_UTIL_API UTIL_ConverterBase {

public:

    virtual QString name() = 0;
    virtual QJsonObject toJsonObject(QVariant valIn) = 0;
};


class CORE_UTIL_API UTIL_Convert : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    UTIL_Convert();

    static QJsonParseError LAST_ERROR;

    static void toJsonArray(QVariantList listIn, QJsonArray* result);
    static void toJsonObject(QVariantMap mapIn, QJsonObject* result);

    static bool registerConverter(QString typeName, UTIL_ConverterBase* conv);

    static QString serializeToJson(QVariantMap mapIn);
    static QString serializeToJson(QJsonObject jobjIn);

    static QVariantMap fromJsonString(QString jsonStr);

public slots:

    static UTIL_Convert* new_UTIL_Convert()
    {
        return new UTIL_Convert();
    }

    QVariantMap jsonifyObj(QVariantMap mapIn);
    QVariantList jsonifyList(QVariantList listIn);


private:
    static QMap<QString, UTIL_ConverterBase*> _converterMap;


};



#endif