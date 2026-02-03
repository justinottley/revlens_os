
#ifndef CORE_PY_QVARIANT_H
#define CORE_PY_QVARIANT_H

#include "RlpCore/PY/Global.h"

class PyQVariantMap;

class CORE_PY_API PyTypeConverter {

public:
    PyTypeConverter(QString typeName);

    virtual py::object toPy(QVariant val);
    virtual QVariant fromPy(py::handle objIn);


private:
    QString _typeName;

};

class CORE_PY_API PyQVariant  {

public:
    RLP_DEFINE_LOGGER

    PyQVariant();
    PyQVariant(int);
    PyQVariant(float);
    PyQVariant(double);
    PyQVariant(bool);
    PyQVariant(const char *);
    PyQVariant(PyQVariantMap* map);
    PyQVariant(QVariant val);
    PyQVariant(QString val);
    PyQVariant(py::dict pdict);
    PyQVariant(py::list plist);

    int toInt();
    const char* toString();
    double toDouble();
    qlonglong toLongLong();
    bool toBool();
    QVariantMap toMap();
    QVariantList toList();
    QStringList toStringList();

    py::dict toDict();
    py::list toPyList();

    QVariant val() { return _val; }
    QString typeName() { return _val.typeName(); }

    py::object pyval();

    static QString getPyClassName(py::handle objIn);
    static QVariant fromPyHandle(py::handle objIn);

    static void registerConverter(QString typeName, PyTypeConverter* converter);

    static QHash<QString, PyTypeConverter*> CONVERTERS;

private:

    QVariant _val;
    //static const char* _logName;

};

#endif