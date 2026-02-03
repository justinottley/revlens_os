
#ifndef CORE_UTIL_TEMP_H
#define CORE_UTIL_TEMP_H

#include "RlpCore/UTIL/Global.h"

/*
 * Useful in cases where python's tempfile may not be available (ios / wasm)
 */
class CORE_UTIL_API UTIL_Temp : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    UTIL_Temp() {}

public slots:

    static UTIL_Temp* new_UTIL_Temp()
    {
        return new UTIL_Temp();
    }

    QString mkdtemp(QString prefix="");
    QString gettempdir();

};

#endif
