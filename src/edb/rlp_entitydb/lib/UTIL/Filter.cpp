
#include "RlpEdb/UTIL/Global.h"
#include "RlpEdb/UTIL/Filter.h"

RLP_SETUP_LOGGER(edb, EdbUtil, Filter)


QVariant
EdbUtil_Filter::makeFilter(QVariantList filterIn)
{
    RLP_LOG_WARN("DEPRECATED")

    QVariantMap fWrapper;
    
    fWrapper.insert("field", filterIn.at(0));
    fWrapper.insert("op", filterIn.at(1));
    fWrapper.insert("val", filterIn.at(2));

    return fWrapper;
}