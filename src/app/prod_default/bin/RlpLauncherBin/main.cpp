


#ifndef _WIN32
#include "RlpCore/UTIL/StacktraceHandler.h"
#endif

#include "RlpCore/UTIL/AppGlobals.h"

#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/Stylesheet.h"
#include "RlpGui/PANE/View.h"

// #include "RlpGui/WIDGET/PyScripting.h"


#include "RlpProd/LAUNCH/Controller.h"


#include <QtCore/QtGlobal>
#include <QtCore/QDebug>
#include <QtGui/QFontDatabase>

#include <QtWidgets/QMenu>
#include <QtWidgets/QSystemTrayIcon>


const char* _logName = "prod.Launcher";


void
quit()
{
    RLP_LOG_DEBUG("Exiting")
    exit(0);
}


int main( int argc, char **argv )
{
    #ifndef _WIN32
    UTIL_StacktraceHandler::installSignalHandler();
    #endif

    Q_INIT_RESOURCE(gui_BASE_resources);

    // needed before logging init
    QCoreApplication::setApplicationName("launcher");

    LOG_Logging::init();

    // LOG_Logging::getLogger("edb.EdbStorSql")->setLevel(LOG_Logging::WARN);

    int EdbFsLogLevel = LOG_Logging::DEBUG; // LOG_Logging::DEBUG; //  LOG_Logging::INFO; // LOG_Logging::VERBOSE;

    LOG_Logging::getLogger("edb.EdbFs")->setLevel(EdbFsLogLevel);
    LOG_Logging::getLogger("edb.EdbStorSql")->setLevel(LOG_Logging::WARN);
    LOG_Logging::getLogger("edb.EdbDs")->setLevel(LOG_Logging::DEBUG);
    LOG_Logging::getLogger("core")->setLevel(LOG_Logging::DEBUG);
    

    
    // NEEDED FOR WASM MULTITHREADING
    //
    // QApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);
    // QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication qapp(argc, argv);
    qapp.setQuitOnLastWindowClosed(false);
    
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    // qapp.setAttribute(Qt::AA_DisableHighDpiScaling);


    GUI_Style::init();

    // TODO FIXME: move to GUI_Style
    //
    qapp.setStyleSheet(QString(RLP_GUI_STYLESHEET));

    int fontId = QFontDatabase::addApplicationFont(":misc/font_nanum_gothic.ttf"); // fontPath);
    if (fontId != -1)
    {
        QStringList fontList = QFontDatabase::applicationFontFamilies(fontId);
        QString appFontName = fontList[0];
        // qDebug() << fontList;
        QFont appFont(appFontName);
        appFont.setPixelSize(14);
        qapp.setFont(appFont);

        RLP_LOG_DEBUG("Set App Font: " << appFontName)
    }
    else
    {
        QFont newFont("Verdana", 10); // ("Bitstream Vera Sans", 10); // , QFont::Bold, true);
        newFont.setStyleStrategy(QFont::PreferAntialias);

        qapp.setFont(newFont);
    }

    //
    // INIT APPGLOBALS
    //

    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
    QString appRoot = e.value("REVLENS_APP_ROOT");

    QVariantMap globals;
    // globals.insert("startup.args", args);
    globals.insert("app.pid", QCoreApplication::applicationPid());
    globals.insert("app.rootpath", appRoot);
    
    RLP_LOG_DEBUG("APP_ROOT:" << appRoot)


    UTIL_AppGlobals* appGlobals = new UTIL_AppGlobals();
    appGlobals->setInstance(appGlobals);
    appGlobals->setGlobals(globals);


    GUI_View* v = new GUI_View();
    LAUNCH_Controller* lc = new LAUNCH_Controller(v);

    v->resize(400, 600);

    lc->startup();

    // QTimer::singleShot(250, lc, &LAUNCH_Controller::startupReady);

    v->show();

    QMenu* m = new QMenu();
    QAction* actShowWindow = m->addAction("Show Window");
    QObject::connect(
        actShowWindow,
        &QAction::triggered,
        lc,
        &LAUNCH_Controller::showWindow
    );


    QAction* actQuit = m->addAction("Quit");
    QObject::connect(
        actQuit,
        &QAction::triggered,
        lc,
        &LAUNCH_Controller::quit
    );

    QSystemTrayIcon* ico = new QSystemTrayIcon(QIcon(":misc/revlens_logo_only_2025.png"));

    ico->setToolTip("Revlens Platform Launcher");
    ico->setContextMenu(m);
    ico->show();

    return qapp.exec();


}