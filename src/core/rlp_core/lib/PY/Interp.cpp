//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#include "RlpCore/PY/Interp.h"
#include "RlpCore/PY/PyQVariant.h"
#include "RlpCore/PY/PyQVariantMap.h"

RLP_SETUP_LOGGER(core, PY, Interp)


PY_Interp::State PY_Interp::state = PY_Interp::PythonUninitialized;
PyInterpreterState* PY_Interp::istate = NULL;
QHash<QString, PyThreadState*> PY_Interp::_pytstate;

bool PY_Interp::_enabled = true;

py::object PY_Interp::_PY_CALL_FUNC;


void
PY_Interp::initPyCall()
{
    RLP_LOG_DEBUG("")

    py::object modobj = py::module_::import("rlp.core.util");
    _PY_CALL_FUNC = modobj.attr("py_call");

    RLP_LOG_DEBUG("py_call OK")
}


void
PY_Interp::init(int flags)
{
    LOG_Logging::init();

    Py_InitializeEx(0);
    initPyCall();

    qAddPostRoutine(cleanup);
    
    state = PY_Interp::PythonInitialized;

    istate = PyInterpreterState_Get();

    // initiliaze thread state for main thread
    _pytstate.insert("main", PyGILState_GetThisThreadState());

    QString renderLoopMode = getenv("QSG_RENDER_LOOP");
    if (renderLoopMode == "basic")
    {
        RLP_LOG_INFO("QSG_RENDER_LOOP=basic, setting single thread mode")
        _enabled = false;
    }

    RLP_LOG_DEBUG("OK")
    return;
}


void
PY_Interp::cleanup()
{
    acquireGIL();

    if (state == PY_Interp::PythonInitialized)
    {
        Py_Finalize();
        state = PY_Interp::PythonUninitialized;
    }
}


QVariant
PY_Interp::call(QString method, QVariantList args, QVariantMap kwargs)
{
    RLP_LOG_DEBUG(method << args)

    acquireGIL();

    QStringList methodParts = method.split(".");
    QString func = methodParts.last();
    methodParts.removeLast();
    QString modName = methodParts.join(".");

    QVariant rresult;

    try
    {
        py::object modobj = py::module_::import(modName.toLocal8Bit().constData());
        py::object pyFunc = modobj.attr(func.toLocal8Bit().constData());

        py::object pyArgs = PyQVariant(args).toPyList();
        py::object pyKwargs = PyQVariantMap(kwargs).toDict();

        py::object pyResult = _PY_CALL_FUNC(pyFunc, pyArgs, pyKwargs);

        rresult = PyQVariant::fromPyHandle(pyResult);

    } catch (py::error_already_set &e)
    {

        RLP_LOG_ERROR("")
        RLP_LOG_ERROR("Error running" << method)
        RLP_LOG_ERROR("")
        RLP_LOG_ERROR(e.what())
    }

    return rresult;
}


void
PY_Interp::evalFile(QString pyFile)
{
    RLP_LOG_DEBUG(pyFile)

    QFile f(pyFile);
    f.open(QIODevice::ReadOnly);

    QString r = f.readAll();

    f.close();

    eval(r);
    // PythonQtObjectPtr mainModule = PythonQt::self()->getMainModule();
    // mainModule.evalFile(pyFile);
    
}


bool
PY_Interp::eval(QString content)
{
    RLP_LOG_DEBUG(content)
    
    acquireGIL();

    // Executing the whole script as one line
    bool result = true;
    const QByteArray line = content.toUtf8();
    if (PyRun_SimpleString(line.constData()) == -1) {
        if (PyErr_Occurred())
            PyErr_Print();
        result = false;
    }

    return result;
}


bool
PY_Interp::bindAppObject(const QString &moduleName,
                         const QString &name,
                         QObject *o)
{
    // RLP_LOG_VERBOSE(moduleName << name << o)

    py::object m = py::module_::import(moduleName.toLocal8Bit().constData());
    py::object pyobj = py::cast(o);
    py::setattr(m, name.toLocal8Bit().constData(), pyobj);

    return true;
}


bool
PY_Interp::bindModule(const char* modsrc, const char* name, PyObject* addmod)
{
    // RLP_LOG_DEBUG(modsrc << ":" << name)

    PyObject *module = PyImport_AddModule(modsrc);
    if (!module) {
        // Py_DECREF(po);
        if (PyErr_Occurred())
            PyErr_Print();

        RLP_LOG_WARN("Failed to locate module" << modsrc)

        return false;
    }


    if (PyModule_AddObject(module, name, addmod) < 0) {
        if (PyErr_Occurred())
            PyErr_Print();

        RLP_LOG_WARN("Failed add object" << name << "to" << modsrc)

        return false;
    }

    RLP_LOG_DEBUG(modsrc << ":" << name << "OK")

    return true;
}


bool
PY_Interp::initThreadState(QString tstateId)
{
    #ifdef SCAFFOLD_WASM
    return true;
    #endif

    RLP_LOG_DEBUG(tstateId)

    PyThreadState* ts = PyThreadState_New(istate);
    _pytstate.insert(tstateId, ts);

    return true;
}


void
PY_Interp::acquireGIL(QString tstateId)
{
    // RLP_LOG_DEBUG(tstateId)

    #ifdef SCAFFOLD_WASM
    return;
    #endif

    #ifdef __MACH__
    return;
    #endif

    if (!_enabled)
    {
        //RLP_LOG_DEBUG("skipping")
        return;
    }


    if (!PyGILState_Check())
    {
        PyThreadState* ts = _pytstate.value(tstateId);
        PyEval_RestoreThread(ts);

    } else
    {
        // RLP_LOG_WARN("Cannot acquire, GIL already held!")
    }
}


void
PY_Interp::releaseGIL()
{
    #ifdef SCAFFOLD_WASM
    return;
    #endif

    #ifdef __MACH__
    return;
    #endif

    if (!_enabled)
    {
        //RLP_LOG_DEBUG("skipping")
        return;
    }

    if (PyGILState_Check())
    {
        PyEval_SaveThread();
        // RLP_LOG_DEBUG("OK")
    } else
    {
        // RLP_LOG_WARN("GIL not held, cannot release!")
    }
}


void
PY_Interp::ping()
{
    // RLP_LOG_DEBUG("")

    // WARNING: only stable with QSG_RENDER_LOOP='basic'
    acquireGIL();
    releaseGIL();
}