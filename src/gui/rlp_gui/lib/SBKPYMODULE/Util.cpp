

#include "RlpGui/SBKPYMODULE/Util.h"

#ifndef SCAFFOLD_WASM
#include <shiboken.h>
#include <sbkpython.h>
#include <sbkconverter.h>
#include <pyside6_qtwidgets_python.h>
#endif

// PyTypeObject **SbkPySide6_QtWidgetsTypes;


QWidget*
PYSBK_Util::getQWidget(PyObject* o)
{
    #ifdef SCAFFOLD_WASM
    return nullptr;
    #else

    PyObject* objType = PyObject_Type(o);
    if (objType == NULL) {
        return nullptr;
    }

    PyObject* typeName = PyObject_GetAttrString(objType, "__name__");
    if (typeName == NULL) {
        Py_DECREF(objType);
        return nullptr;
    }

    const char* className = PyUnicode_AsUTF8(typeName);
    QString typeresult = QString(strdup(className));

    Py_DECREF(typeName);
    Py_DECREF(objType);

    if (typeresult != "QWidget")
    {
        qInfo() << "ERROR: ONLY QWidget allowed";
        return nullptr;
    }


    PyTypeObject* pt = Shiboken::Conversions::getPythonTypeObject("QWidget");

    if (pt == nullptr)
    {
        qInfo() << "ERROR: UNABLE TO GET PyTypeObject for QWidget";
        return nullptr;
    }

    QWidget* result = 0;
    Shiboken::Conversions::pythonToCppPointer(pt, o, &result);

    return result;
    #endif
}
