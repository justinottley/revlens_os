

// #include "RlpCore/LOG/PythonUtils.h"
// #include "RlpCore/PY/PythonUtils.h"

#include "RlpCore/PY/Interp.h"

#include "RlpCore/UTIL/AppGlobals.h"

/*
#include "RlpCore/LOG/Logging.h"

#include "RlpCore/NET/PythonUtils.h"

#include "RlpCore/CNTRL/Auth.h"
#include "RlpCore/CNTRL/PythonUtils.h"
*/





#include <Python.h>


#include <QtWidgets/QApplication>

#ifdef SCAFFOLD_WASM_TARGET
#include <QtCore/QtPlugin>
Q_IMPORT_PLUGIN(QWasmIntegrationPlugin)
Q_IMPORT_PLUGIN(QSvgIconPlugin)
Q_IMPORT_PLUGIN(QGifPlugin)
Q_IMPORT_PLUGIN(QICNSPlugin)
Q_IMPORT_PLUGIN(QICOPlugin)
Q_IMPORT_PLUGIN(QJpegPlugin)
Q_IMPORT_PLUGIN(QSvgPlugin)
Q_IMPORT_PLUGIN(QTgaPlugin)
Q_IMPORT_PLUGIN(QTiffPlugin)
Q_IMPORT_PLUGIN(QWbmpPlugin)
Q_IMPORT_PLUGIN(QWebpPlugin)
#endif

// extern "import rlp.coreC" PyObject* PyInit_Shiboken();
// extern "C" PyObject* PyInit_QtCore();
// extern "C" PyObject* PyInit_QtGui();
// extern "C" PyObject* PyInit_QtWidgets();


int main(int argc, char** argv)
{
    qInfo() << "hello";

    PY_Interp::init();

    PY_Interp::evalFile(argv[1]);


    /*
    QVariantList args;
    args.append(QString("the quick brown fox"));

    QVariant result = PY_Interp::call("rlp.core.util.slugify", args);
    qInfo() << result;
    */
}


int main_static(int argc, char** argv)
{
    //LOG_PythonUtils::registerModule();
    //UTIL_PythonUtils::registerModule();

    PY_Interp::init(0);

    //LOG_PythonUtils::bootstrap();
    //UTIL_PythonUtils::bootstrap();



    QString icode = "import RlpCoreLOGmodule;print(RlpCoreLOGmodule);";
    PY_Interp::eval(icode);

    UTIL_AppGlobals* ag = new UTIL_AppGlobals();
    // PY_Interp::bindAppObject("__main__", "uglobals", "core.UTIL", UTIL_PythonUtils::UTIL_AppGlobalsType, ag);
    // 
    // CoreCntrl_Auth* auth = new CoreCntrl_Auth();
    // PY_Interp::bindAppObject("__main__", "AUTH", "core.CNTRL", CoreCntrl_PythonUtils::CoreCntrl_AuthType, auth);
// 
    QString bcode = "print(uglobals);print(uglobals.globals());";
    //PY_Interp::eval(bcode);
// 
    // QString ncode = "from RlpCoreNETmodule import CoreNet_WebSocketClient;c = CoreNet_WebSocketClient();print(c)";
    // PY_Interp::eval(ncode);

    return 0;
}


int main_sbk(int argc, char **argv)
{
    // setenv("PYTHONHOME", "/", 1);

    #ifdef SCAFFOLD_WASM
    setenv("PYTHONPATH", "/wasm_sitelib", 1);
    #endif
    // LOG_PythonUtils::registerModule();

    // const char* sbkmod = "Shiboken";

    // if (PyImport_AppendInittab("Shiboken", PyInit_Shiboken) == -1) {
    //     qWarning("Failed to add the module 'Shiboken' to the table of built-in modules.");
    // }


    // if (PyImport_AppendInittab("QtCore", PyInit_QtCore) == -1) {
    //    qWarning("Failed to add the module 'QtCore' to the table of built-in modules.");
    // }

    // if (PyImport_AppendInittab("QtGui", PyInit_QtGui) == -1) {
    //     qWarning("Failed to add the module 'QtCore' to the table of built-in modules.");
    // }

    // if (PyImport_AppendInittab("QWidgets", PyInit_QtWidgets) == -1) {
    //     qWarning("Failed to add the module 'QtCore' to the table of built-in modules.");
    // }

    PY_Interp::init(0);
    // Py_InitializeEx(0);


    // PyObject* mod = PyInit_Shiboken();

    // PyObject* mod = LOG_PythonUtils::bootstrap();
    // PY_Interp::bindModule("shiboken6", "Shiboken", mod);

    // PyObject* modQtCore = PyInit_QtCore();
    //if (modQtCore == nullptr)
    //{
    //    qInfo() << "ERROR IMPORTING QtCore";
    //}

    // PY_Interp::bindModule("PySide6", "QtCore", modQtCore);

    // PyObject* modQtWidgets = PyInit_QtWidgets();
    // 
    // if (modQtWidgets == nullptr)
    // {
    //     qInfo() << "ERROR IMPORTING QtWidgets";
    // }

    const char * sysconfig_str =
	"import sysconfig;\n"
	"v = sysconfig.get_config_vars();\n"
	"for a in v:\n"
	"	print ( f'{a:30} {sysconfig.get_config_var(a)}' );\n";


    const char* modtest = 
    "import RlpCoreLOGmodule as rlpmod;print(rlpmod);"
    "import rlp;print(dir(rlp));";

    const char* pysidetest =
    "import PySide6;print(PySide6);";

    PyRun_SimpleString ( pysidetest );
    
    /*
    const char* moduleName = "rlp.LOG";
    PyObject* module = PyImport_ImportModule(moduleName);
    if (!module)
        PyErr_Format(PyExc_ImportError,"could not import module '%s'", moduleName);

    qInfo() << "import" << moduleName << "OK";
    */

    const char * done = "print('hello world 1');";

    PyRun_SimpleString(done);
    
    


    QApplication a(argc, argv);

    qInfo() << "HELLO WORLD QT";

    // LOG_Logging::init();

    

    
    /*

    LOG_PythonUtils::registerModule();
    UTIL_PythonUtils::registerModule();
    CoreNet_PythonUtils::registerModule();
    CoreCntrl_PythonUtils::registerModule();

    */

    // PY_Interp::init();

    // PY_Interp::eval("hello world from py");


    // qInfo() << "hello world 2";

    /*
    LOG_PythonUtils::bootstrap();
    UTIL_PythonUtils::bootstrap();
    CoreNet_PythonUtils::bootstrap();
    CoreCntrl_PythonUtils::bootstrap();
    */

    // QString pycode = "print('hello world');import RlpCoreUTILmodule;print(dir(RlpCoreUTILmodule));";
    // QString pycode = "import RlpCoreLOGmodule as rlp_core_log;print(rlp_core_log.LOG_Logging.getLogger('core').setLevel(40));";
    




    Py_Finalize();
    // return a.exec();

    
    return 0;
}
