// nothing
#ifndef CORE_PY_QVARIANTMAP_H
#define CORE_PY_QVARIANTMAP_H

#include "RlpCore/PY/Global.h"
#include "RlpCore/PY/PyQVariant.h"
//class PyQVariant;

class CORE_PY_API PyQVariantMap {

public:
    PyQVariantMap();
    PyQVariantMap(QVariantMap val);
    PyQVariantMap(py::dict dval);

    py::dict toDict();
    QVariantMap val() { return _val; }

    QVariant value(QString key) { return _val.value(key); }

    
    void insert(QString key, QVariant vval) { _val.insert(key, vval); }

    void insertPy(const char* key, PyQVariant vval);
    void insertPy(const char* key, const char* val);
    void insertPy(const char* key, float val);
    void insertPy(const char* key, double val);
    void insertPy(const char* key, bool val);


private:
    QVariantMap _val;
};

#endif