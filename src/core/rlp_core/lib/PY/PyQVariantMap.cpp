

#include "RlpCore/PY/PyQVariantMap.h"
#include "RlpCore/PY/PyQVariant.h"

PyQVariantMap::PyQVariantMap():
    _val()
{
}

PyQVariantMap::PyQVariantMap(QVariantMap val):
    _val(val)
{
}


PyQVariantMap::PyQVariantMap(py::dict dval)
{
    for (auto item : dval)
    {
        QString key = std::string(py::str(item.first)).c_str();
        QVariant val = PyQVariant::fromPyHandle(item.second);

        _val.insert(key, val);
    }
}


void
PyQVariantMap::insertPy(const char* key, PyQVariant val)
{
    _val.insert(QString(key), val.val());
}


void
PyQVariantMap::insertPy(const char* key, const char* val)
{
    _val.insert(QString(key), QString(val));
}


void
PyQVariantMap::insertPy(const char* key, float val)
{
    _val.insert(QString(key), val);
}


void
PyQVariantMap::insertPy(const char* key, double val)
{
    _val.insert(QString(key), val);
}


void
PyQVariantMap::insertPy(const char* key, bool val)
{
    _val.insert(QString(key), val);
}


py::dict
PyQVariantMap::toDict()
{
    py::dict result;

    for (auto it : _val.keys())
    {
        result[it.toLocal8Bit().constData()] = PyQVariant(_val.value(it)).pyval();
    }

    return result;
}