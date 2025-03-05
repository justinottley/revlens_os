
#ifndef CORE_PY_INTERP_H
#define CORE_PY_INTERP_H

#include "RlpCore/PY/Global.h"
#include "RlpCore/LOG/Logging.h"


class CORE_PY_API PY_Interp : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    enum State
    {
        PythonUninitialized,
        PythonInitialized,
        AppModuleLoaded
    };

    // enum ThreadType
    // {
    //     TMain,
    //     TRender
    // };

    static void init(int flags=0);
    static void cleanup();
    
    static QVariant callJson(QVariantMap callInfo);
    static QVariant callJson(QString method, QVariantList args=QVariantList());

    static QVariant call(QString method, QVariantList args=QVariantList(), QVariantMap kwargs=QVariantMap());

    static void evalFile(QString pyFile);
    static bool eval(QString content);

    // static bool registerTypeMap(QString name, PyTypeObject** typemap);

    static bool bindAppObject(const QString& moduleName,
                              const QString& name,
                              QObject *o);
    
    static bool bindModule(const char* src, const char* name, PyObject* addmod);

    // Each view/thread needs to initialize its own thread state, passing in
    // an ID to identify the view
    static bool initThreadState(QString tstateId);

    static void acquireGIL(QString tstateId = "main");
    static void releaseGIL();

    static void ping();

    static State state;
    static py::object _PY_CALL_FUNC;
    static PyInterpreterState* istate;

private:
    static void initPyCall();

    static QHash<QString, PyThreadState*> _pytstate;
    static bool _enabled;

};

#endif