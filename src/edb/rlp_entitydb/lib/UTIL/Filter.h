
#ifndef EDB_UTIL_FILTER_H
#define EDB_UTIL_FILTER_H

#include "RlpEdb/UTIL/Global.h"

class EDB_UTIL_API EdbUtil_Filter {

public:
    RLP_DEFINE_LOGGER

    static QVariant makeFilter(QVariantList filterIn);


};


#endif