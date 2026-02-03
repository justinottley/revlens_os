
#include "RlpProd/GUI/Startup.h"

#include "RlpCore/UTIL/AppGlobals.h"
#include "RlpCore/PY/Interp.h"
#include "RlpCore/SEQ/Init.h"

#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/Stylesheet.h"

#include "RlpCore/UTIL/StacktraceHandler.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/GUI_TL2/Controller.h"
#include "RlpRevlens/GUI/MainWindowArrangement.h"
#include "RlpRevlens/GUI/TabletWindowArrangement.h"

#ifdef SCAFFOLD_PYBIND_STATIC

// Python bindings - static setup for wasm compat
//
#include "RlpCore/DS/pymodule.h"


#include "RlpCore/CNTRL/pymodule.h"
#include "RlpCore/UTIL/pymodule.h"
#include "RlpCore/SEQ/pymodule.h"
#include "RlpCore/NET/pymodule.h"

#ifndef SCAFFOLD_WASM
#ifndef SCAFFOLD_IOS
#include "RlpCore/PROC/pymodule.h"
#endif
#endif

#include "RlpGui/BASE/pymodule.h"
#include "RlpGui/BASE/PainterModule.h"
#include "RlpGui/PANE/pymodule.h"
#include "RlpGui/MENU/pymodule.h"
#include "RlpGui/GRID/pymodule.h"
#include "RlpGui/WIDGET/pymodule.h"


#include "RlpRevlens/DS/pymodule.h"
#include "RlpRevlens/MEDIA/pymodule.h"
#include "RlpRevlens/VIDEO/pymodule.h"
#include "RlpRevlens/AUDIO/pymodule.h"
#include "RlpRevlens/DISP/pymodule.h"
#include "RlpRevlens/CNTRL/pymodule.h"
#include "RlpRevlens/GUI/pymodule.h"
#include "RlpRevlens/GUI_TL2/pymodule.h"
#include "RlpRevlens/GUI_TLT/pymodule.h"
#include "RlpRevlens/GUI_VIEW/pymodule.h"

#include "RlpCore/QTCORE/QtCoreTypes.h"
#include "RlpGui/QTGUI/QtGuiTypes.h"


#include "RlpExtrevlens/RLQTMEDIA/pymodule.h"
#include "RlpExtrevlens/RLQTCNTRL/pymodule.h"
#include "RlpExtrevlens/RLQTDISP/pymodule.h"
#include "RlpExtrevlens/RLQTGUI/pymodule.h"

#include "RlpExtrevlens/RLANN_DS/pymodule.h"
#include "RlpExtrevlens/RLANN_MP/pymodule.h"
#include "RlpExtrevlens/RLANN_MEDIA/pymodule.h"
#include "RlpExtrevlens/RLANN_CNTRL/pymodule.h"
#include "RlpExtrevlens/RLANN_GUI/pymodule.h"

#include "RlpExtrevlens/RLSYNC/pymodule.h"
#include "RlpExtrevlens/RLMEDIA/pymodule.h"
#include "RlpExtrevlens/RLFFMPEG/pymodule.h"
#include "RlpExtrevlens/RLQPSD_MEDIA/pymodule.h"

#ifndef SCAFFOLD_WASM
#ifndef SCAFFOLD_IOS

#include "RlpExtrevlens/RLQPDF_MEDIA/pymodule.h"

#ifndef __MACH__ // EXR not available on macOS (23.03.01)

#include "RlpExtrevlens/RLEXRMEDIA/pymodule.h"
#include "RlpExtrevlens/RLOCIO/pymodule.h"
#include "RlpExtrevlens/RLOIIO/pymodule.h"

#endif // __MACH__
#endif // SCAFFOLD_IOS
#endif // SCAFFOLD_WASM

#include "RlpExtrevlens/RLSGCMD/pymodule.h"

#include "RlpProd/CNTRL/pymodule.h"
#include "RlpProd/GUI/pymodule.h"

#endif // SCAFFOLD_PYBIND_STATIC



// ---



RLP_SETUP_LOGGER(prod, GUI, Startup)

GUI_Startup::GUI_Startup()
{
    qRegisterMetaType<DS_TimePtr>("DS_TimePtr");
    qRegisterMetaType<DS_BufferPtr>("DS_BufferPtr");
}


void
GUI_Startup::pythonInit(CNTRL_MainController* cntrl)
{
    RLP_LOG_DEBUG("")

    #ifdef SCAFFOLD_PYBIND_STATIC

    RlpCoreQTCOREmodule_bootstrap();
    RlpGuiQTGUImodule_bootstrap();
    
    RlpCoreDSmodule_bootstrap();
    RlpCoreUTILmodule_bootstrap();
    RlpCoreSEQmodule_bootstrap();
    RlpCoreNETmodule_bootstrap();
    RlpCoreCNTRLmodule_bootstrap();

    #ifndef SCAFFOLD_WASM
    #ifndef SCAFFOLD_IOS
    RlpCorePROCmodule_bootstrap();
    #endif
    #endif


    RlpGuiBASEmodule_bootstrap();
    RlpGuiPAINTERmodule_bootstrap();
    RlpGuiPANEmodule_bootstrap();
    RlpGuiMENUmodule_bootstrap();
    RlpGuiGRIDmodule_bootstrap();
    RlpGuiWIDGETmodule_bootstrap();

    RlpRevlensDSmodule_bootstrap();
    RlpRevlensMEDIAmodule_bootstrap();
    RlpRevlensAUDIOmodule_bootstrap();
    RlpRevlensVIDEOmodule_bootstrap();
    RlpRevlensDISPmodule_bootstrap();
    RlpRevlensCNTRLmodule_bootstrap();
    RlpRevlensGUI_TL2module_bootstrap();
    RlpRevlensGUImodule_bootstrap();
    RlpRevlensGUI_TLTmodule_bootstrap();
    RlpRevlensGUI_VIEWmodule_bootstrap();

    RlpExtrevlensRLQTMEDIAmodule_bootstrap();
    RlpExtrevlensRLQTCNTRLmodule_bootstrap();
    RlpExtrevlensRLQTDISPmodule_bootstrap();
    RlpExtrevlensRLQTGUImodule_bootstrap();

    RlpExtrevlensRLANN_DSmodule_bootstrap();
    RlpExtrevlensRLANN_MPmodule_bootstrap();
    RlpExtrevlensRLANN_MEDIAmodule_bootstrap();
    RlpExtrevlensRLANN_CNTRLmodule_bootstrap();
    RlpExtrevlensRLANN_GUImodule_bootstrap();

    RlpExtrevlensRLSYNCmodule_bootstrap();

    RlpExtrevlensRLMEDIAmodule_bootstrap();
    RlpExtrevlensRLFFMPEGmodule_bootstrap();
    RlpExtrevlensRLQPSD_MEDIAmodule_bootstrap();

    #ifndef SCAFFOLD_WASM
    #ifndef SCAFFOLD_IOS

    RlpExtrevlensRLQPDF_MEDIAmodule_bootstrap();

    #ifndef __MACH__ // EXR, OCIO, OIIO not available on macOS (23.03.01)
    RlpExtrevlensRLEXRMEDIAmodule_bootstrap();
    RlpExtrevlensRLOCIOmodule_bootstrap();
    RlpExtrevlensRLOIIOmodule_bootstrap();
    #endif // __MACH__
    #endif // SCAFFOLD_IOS
    #endif // SCAFFOLD_WASM

    RlpExtrevlensRLSGCMDmodule_bootstrap();

    RlpProdCNTRLmodule_bootstrap();
    RlpProdGUImodule_bootstrap();

    #endif // SCAFFOLD_PYBIND_STATIC


    PY_Interp::init();

    // -------------- 
    // Global Objects
    // --------------

    // need this before VIEW is exported to register parent classes
    PY_Interp::eval("import RlpCoreQTCORE");
    PY_Interp::eval("import RlpGuiQTGUI");

    // PY_Interp::eval("import RlpCoreCNTRLmodule"); // for CoreCntrl_Auth ?

    // PY_Interp::eval("import RlpGuiBASEmodule");
    // PY_Interp::eval("import RlpGuiPANEmodule");

    // // #ifndef SCAFFOLD_PYBIND_STATIC
    // PY_Interp::eval("import RlpCoreUTILmodule");
    // PY_Interp::eval("import RlpRevlensCNTRLmodule");
    // PY_Interp::eval("import RlpRevlensGUImodule");
    // PY_Interp::eval("import RlpRevlensGUI_TL2module");
    // // #endif

    // PY_Interp::eval("import RlpRevlensDSmodule");

    PY_Interp::eval("import rlp.core;import rlp.util;import rlp.gui;import revlens");

    PY_Interp::bindAppObject("RlpCoreUTILmodule", "APPGLOBALS", UTIL_AppGlobals::instance());
    PY_Interp::bindAppObject("RlpRevlensCNTRLmodule", "CNTRL", cntrl);
    PY_Interp::bindAppObject("RlpRevlensGUI_TL2module", "TL_CNTRL", GUI_TL2_Controller::instance());


    PY_Interp::eval("import rlp.core.rlp_logging;rlp.core.rlp_logging._init_rlp_logging()");


    RLP_LOG_DEBUG("OK")
}


void
GUI_Startup::pythonStartup()
{
    RLP_LOG_DEBUG("")
    PY_Interp::eval("import revlens;revlens.startup()");
}


bool
GUI_Startup::start(int argc, char** argv, bool showGui)
{
    #ifndef SCAFFOLD_WASM
    #ifndef SCAFFOLD_IOS
    UTIL_StacktraceHandler::installSignalHandler();
    #endif
    #endif

    LOG_Logging::init();
    SEQ_Init::initAll();

    QVariantList args;    
    for (int x = 0; x != argc; ++x) {
        args.append(QString(argv[x]));
    }
    
    QProcessEnvironment penv = QProcessEnvironment::systemEnvironment();
    QString appRoot = penv.value("REVLENS_APP_ROOT");

    QVariantMap globals;
    globals.insert("startup.args", args);
    globals.insert("app.pid", QCoreApplication::applicationPid());
    globals.insert("app.rootpath", appRoot);
    QString appPlatform = "linux";
    #ifdef __MACH__
    appPlatform = "darwin";
    #endif
    #ifdef SCAFFOLD_IOS
    appPlatform = "ios";
    #endif
    globals.insert("app.platform", appPlatform);

    setenv("RLP_APP_PLATFORM", appPlatform.toUtf8().constData(), 1);

    UTIL_AppGlobals* appGlobals = new UTIL_AppGlobals();
    appGlobals->setInstance(appGlobals);
    appGlobals->setGlobals(globals);



    int loglevel = 10;
    if (args.contains("--service"))
    {
        RLP_LOG_INFO("Service mode, turning loglevel to ERROR")
        loglevel = 40;
    }

    QStringList logList = {"core", "edb", "gui", "revlens", "prod", "ext"};
    
    for (int li=0; li!=logList.size(); ++li)
    {
        LOG_Logging::getLogger(logList.at(li))->setLevel(loglevel);
    }

    LOG_Logging::getLogger("edb")->setLevel(40);

    // NEEDED FOR WASM MULTITHREADING
    //
    // QApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);
    // QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    // macOS?
    // qapp.setAttribute(Qt::AA_DisableHighDpiScaling);

    // For higher quality antialiasing in OpenGL
    // QSurfaceFormat fmt;
    // fmt.setSamples(10); 
    // QSurfaceFormat::setDefaultFormat(fmt);

    QApplication* qapp = qobject_cast<QApplication*>(QApplication::instance());
    
    GUI_Style::init();

    // For Widgets based on QWidget
    qapp->setStyleSheet(QString(RLP_GUI_STYLESHEET));

    int fontId = QFontDatabase::addApplicationFont(":misc/font_nanum_gothic.ttf");
    if (fontId != -1)
    {
        QStringList fontList = QFontDatabase::applicationFontFamilies(fontId);
        QString appFontName = fontList[0];
        // qDebug() << fontList;
        QFont appFont(appFontName);
        appFont.setPixelSize(14);

        qapp->setFont(appFont);

        RLP_LOG_DEBUG("Set App Font: " << appFontName << "point size:" << appFont.pixelSize());
    }
    else
    {
        QFont newFont("Verdana", 10); // ("Bitstream Vera Sans", 10); // , QFont::Bold, true);
        newFont.setStyleStrategy(QFont::PreferAntialias);

        qapp->setFont(newFont);
    }


    CNTRL_MainController* cntrl = new CNTRL_MainController();
    cntrl->licenseCheck();

    CNTRL_MainController::setInstance(cntrl);

    GUI_TL2_Controller* tlCntrl = new GUI_TL2_Controller(cntrl);
    GUI_TL2_Controller::setInstance(tlCntrl);

    // GUI_View* w = new GUI_View();
    // GUI_View* v = w;

    // w->resize(1000, 600);


    // #if 1
    #ifdef SCAFFOLD_IOS

    appGlobals->globalsPtr()->insert("startup.arrangement", "tablet2");
    GUI_TabletWindowArrangement* mainArr = new GUI_TabletWindowArrangement();
    GUI_TabletWindowArrangement::setInstance(mainArr);

    #else

    GUI_MainWindowArrangement* mainArr = new GUI_MainWindowArrangement();
    GUI_MainWindowArrangement::setInstance(mainArr);

    #endif

    mainArr->init();

    pythonInit(cntrl);

    PY_Interp::bindAppObject("RlpRevlensGUImodule", "GUI", mainArr);

    pythonInitPlugins();
    pythonStartup();

    mainArr->mainView()->resize(1000, 600);
    mainArr->mainView()->show();


    // bool isBatch = appGlobals->globalsPtr()->contains("startup.mode.batch");
    // showGui = !isBatch;


    // if (showGui)
    // {
    //     v->show();

    // } else 
    // {
    //     #ifdef _WIN32
    //     v->show();
    //     v->raise();

    //     QTimer::singleShot(500, v, &GUI_View::showMinimized);

    //     // v->showMinimized();
    //     #else
    //     v->create();
    //     // Trigger view load
    //     v->grabWindow();

    //     #endif
        
    //     RLP_LOG_INFO("Batch Mode Startup OK - " << v->size())

    //     // v->setAttribute(Qt::WA_DontShowOnScreen, true);
    //     // v->setAttribute(Qt::WA_ShowWithoutActivating, true);
        
    //     // v->showMinimized();
    // }


    // return qapp->exec();
    return true;
}