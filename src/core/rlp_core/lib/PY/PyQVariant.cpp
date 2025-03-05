
#include "RlpCore/PY/PyQVariant.h"
#include "RlpCore/PY/PyQVariantMap.h"


const char* PyQVariant::_logName = "core.UTIL.PyQVariant";

PyTypeConverter::PyTypeConverter(QString typeName):
    _typeName(typeName)
{
}


py::object
PyTypeConverter::toPy(QVariant val)
{
    return py::none();
}


QVariant
PyTypeConverter::fromPy(py::handle objIn)
{
    QVariant result;
    return result;
}


// ---------------

QHash<QString, PyTypeConverter*> PyQVariant::CONVERTERS;

PyQVariant::PyQVariant():
    _val()
{
}


void
PyQVariant::registerConverter(QString typeName, PyTypeConverter* converter)
{
    // RLP_LOG_VERBOSE(typeName)

    if (CONVERTERS.contains(typeName))
    {
        RLP_LOG_WARN("already registered:" << typeName)
        return;
    }

    CONVERTERS.insert(typeName, converter);
}


PyQVariant::PyQVariant(int val):
    _val(val)
{
}

PyQVariant::PyQVariant(float val):
    _val(val)
{    
}

PyQVariant::PyQVariant(double val):
    _val(val)
{
}

PyQVariant::PyQVariant(bool val):
    _val(val)
{
}

PyQVariant::PyQVariant(const char* val):
    _val(val)
{
}

PyQVariant::PyQVariant(QVariant val):
    _val(val)
{    
}

PyQVariant::PyQVariant(PyQVariantMap* map):
    _val(map->val())
{
}

PyQVariant::PyQVariant(QString val):
    _val(val)
{
}

PyQVariant::PyQVariant(py::list plist)
{
    QVariantList rval;

    for (auto item : plist)
    {
        rval.push_back(fromPyHandle(item));
    }

    _val.setValue(rval);
}

PyQVariant::PyQVariant(py::dict pdict)
{
    QVariantMap mval;
    for (auto item : pdict)
    {
        QString key = std::string(py::str(item.first)).c_str();
        QVariant val = PyQVariant::fromPyHandle(item.second);

        mval.insert(key, val);
    }

    _val.setValue(mval);
}


int
PyQVariant::toInt()
{
    return _val.toInt();
}


const char*
PyQVariant::toString()
{
    // WARNING: DOES NOT WORK FOR BASE64 ENCODED DATA IN A QSTRING
    return _val.toString().toLocal8Bit().constData();
}


double
PyQVariant::toDouble()
{
    return _val.toDouble();
}


bool
PyQVariant::toBool()
{
    return _val.toBool();
}


qlonglong
PyQVariant::toLongLong()
{
    return _val.toLongLong();
}


QVariantList
PyQVariant::toList()
{
    return _val.toList();
}


QStringList
PyQVariant::toStringList()
{
    QVariantList l = toList();
    QStringList result;
    for (int i=0; i != l.size(); ++i)
    {
        result.push_back(l.at(i).toString());
    }

    return result;
}


QVariantMap
PyQVariant::toMap()
{
    return _val.toMap();
}


py::dict
PyQVariant::toDict()
{
    PyQVariantMap m(_val.toMap());
    return m.toDict();
}


py::list
PyQVariant::toPyList()
{
    py::list result;

    QVariantList l = _val.toList();
    for (auto v : l)
    {
        result.append(PyQVariant(v).pyval());
    }

    return result;
}


py::object
PyQVariant::pyval()
{
    py::object pval = py::none();

    QString valTypeName = _val.typeName();

    if (valTypeName == "QString")
    {
        #ifdef _WIN32

            QString strval = _val.toString();
            char* csval = new char[strval.size() + 1];
            strcpy(csval, strval.toUtf8().constData());
            pval = py::cast(csval);
            delete csval;

        #else

            pval = py::cast(_val.toString().toStdString());

        #endif


    } else if ((valTypeName == "int") || (valTypeName == "uint"))
    {
        pval = py::cast(toInt());

    } else if ((valTypeName == "float") || (valTypeName == "double"))
    {
        pval = py::cast(toDouble());

    } else if (valTypeName == "bool")
    {
        pval = py::cast(toBool());

    } else if (valTypeName == "qlonglong")
    {
        pval = py::cast(toLongLong());

    } else if (valTypeName == "QVariantMap")
    {
        py::dict dval;
        QVariantMap mval = _val.toMap();
        for (auto key : mval.keys())
        {
            PyQVariant p(mval.value(key));
            dval[key.toLocal8Bit().constData()] = p.pyval();
        }

        return dval;
    }
    else if (valTypeName == "QVariantList")
    {
        py::list lval;
        QVariantList rlval = _val.toList();
        for (auto llval : rlval)
        {
            PyQVariant p(llval);
            lval.append(p.pyval());
        }

        return lval;
    }
    else if (valTypeName == "QUuid")
    {
        pval = py::cast(_val.value<QUuid>());
    }
    else if (valTypeName == "QPointF")
    {
        pval = py::cast(_val.value<QPointF>());
    }
    else if (valTypeName == "QImage")
    {
        pval = py::cast(_val.value<QImage>());
    }
    else if (valTypeName == "std::nullptr_t")
    {
        // pval is already py::none()
    }
    else if (CONVERTERS.contains(valTypeName))
    {
        // qInfo() <<"pyval(): FOUND REGISTERED CONVERTER for" << valTypeName;

        pval = CONVERTERS.value(valTypeName)->toPy(_val);

    } else
    {
        qInfo() << "PyQVariant::pyval(): ERROR: NO CAST FOR" << valTypeName;
    }

    return pval;
}


QString
PyQVariant::getPyClassName(py::handle objIn)
{
    QString notype("NULL");

    PyObject* objType = PyObject_Type(objIn.ptr());
    if (objType == NULL) {
        return notype;
    }

    PyObject* typeName = PyObject_GetAttrString(objType, "__name__");
    if (typeName == NULL) {
        Py_DECREF(objType);
        return notype;
    }

    const char* className = PyUnicode_AsUTF8(typeName);
    QString result = QString(strdup(className));

    Py_DECREF(typeName);
    Py_DECREF(objType);

    return result;
}


QVariant
PyQVariant::fromPyHandle(py::handle objIn)
{
    QVariant val;

    if (py::isinstance<py::bool_>(objIn))
    {
        val.setValue<bool>(objIn.cast<bool>());
    }
    else if (py::isinstance<py::int_>(objIn))
    {
        val.setValue<int>(objIn.cast<int>());

    } else if (py::isinstance<py::float_>(objIn))
    {
        val.setValue<float>(objIn.cast<float>());

    } else if (py::isinstance<py::str>(objIn))
    {
        val.setValue<QString>(std::string(objIn.cast<py::str>()).c_str());

    } else if (py::isinstance<py::list>(objIn))
    {
        QVariantList rlval;

        for (auto lval : objIn)
        {
            rlval.push_back(fromPyHandle(lval));
        }

        val.setValue(rlval);

    } else if (py::isinstance<py::dict>(objIn))
    {
        py::dict dobjIn = objIn.cast<py::dict>();

        QVariantMap rmval;
        for (auto item : dobjIn)
        {
            QString key = std::string(py::str(item.first)).c_str();
            QVariant val = fromPyHandle(item.second);

            rmval.insert(key, val);
        }
        val.setValue(rmval);
    }
    else
    {
        QString pyClass = getPyClassName(objIn);

        if (pyClass == "QImage")
        {
            val.setValue(objIn.cast<QImage>());
        } else if (pyClass == "NoneType")
        {
            // val is already "null"
        } else
        {
            QString shPtrPyClass = QString("std::shared_ptr<%1>").arg(pyClass);
            QString pyClassPtr = QString("%1*").arg(pyClass);

            if (CONVERTERS.contains(pyClassPtr))
            {
                val = CONVERTERS.value(pyClassPtr)->fromPy(objIn);

            } else if (CONVERTERS.contains(shPtrPyClass))
            {
                val = CONVERTERS.value(shPtrPyClass)->fromPy(objIn);

            } else
            {
                qInfo() << "ERROR: Invalid value, cannot cast:" << pyClass;
            }

        }
    }

    return val;
}
