
#include "RlpProd/GUI/Startup.h"

#include <QtQuick/QQuickWindow>

#ifndef SCAFFOLD_WASM
#ifndef SCAFFOLD_IOS
#include <QtWebEngineQuick/QtWebEngineQuick>
#endif
#endif

int main( int argc, char **argv )
{
    #ifdef SCAFFOLD_WASM
    setenv("PYTHONHOME", "/", 1);
    setenv("PYTHONPATH", "/wasm_fs_root/pylib", 1);
    #endif

    #ifdef __MACH__
    // turn off GPU acceleration for chromium on macOS,
    // prevents QtWebEngine from composing correctly in the GUI
    setenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-gpu", 1);
    #endif

    // NEEDED FOR WASM MULTITHREADING
    //
    // QApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);
    // QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    #ifndef SCAFFOLD_WASM
    #ifndef SCAFFOLD_IOS
    QtWebEngineQuick::initialize();
    #endif
    #endif


    QCoreApplication::setApplicationName("revlens");

    QApplication qapp(argc, argv);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    Q_INIT_RESOURCE(core_CNTRL_resources);
    Q_INIT_RESOURCE(gui_BASE_resources);
    Q_INIT_RESOURCE(revlens_GUI_resources);
    Q_INIT_RESOURCE(extrevlens_RLANN_GUI_resources);
    Q_INIT_RESOURCE(extrevlens_RLANN_MP_resources);
    // Q_INIT_RESOURCE(prod_GUI_resources);

    #ifdef __MACH__

    QString RLP_PY_VERSION = "3.12";

    QString execDir = QApplication::applicationDirPath();
    QDir d(execDir);

    QString appRoot;
    QString pylibRoot;
    QString pyFrameworkRoot;

    bool isAppBundle = false;
    if (d.dirName() == "MacOS")
    {
        qInfo() << "macOS App Bundle Detected";
        d.cdUp();

        appRoot = QString("%1/lib/inst/Darwin").arg(d.absolutePath());
        pylibRoot = QString("%1/lib").arg(d.absolutePath());
        pyFrameworkRoot = QString("%1/Frameworks/Python.framework/Versions/%2").arg(d.absolutePath(), RLP_PY_VERSION);
        isAppBundle = true;

    } else
    {
        d.cdUp();
        d.cdUp();

        appRoot = d.absolutePath();

        // TODO FIXME: Ship this as a config from the build system
        pylibRoot = "/Users/jottley/dev/revlens_root/thirdbase/25_04/Darwin-arm64-26.2";
    }

    
    qInfo() << "App Root:" << appRoot;

    QStringList ssets = {"extrevlens", "prod", "revlens", "edb", "gui", "core"};

    QString ppath = "";
    QString revlensPath = "";
    QString path = "";

    for (auto ssetName : ssets)
    {
        QString ssetPyPath = QString("%1/%2/lib/python%3/site-packages:").arg(appRoot, ssetName, RLP_PY_VERSION);
        QString ssetRevlensPath = QString("%1/%2:").arg(appRoot, ssetName);
        QString ssetPath = QString("%1/%2/bin:").arg(appRoot, ssetName);

        ppath += ssetPyPath;
        revlensPath += ssetRevlensPath;
        path += ssetPath;
    }

    // pylibRoot = QString("%1/lib/pylibs").arg(appRoot);

    // TODO FIXME: Ship this as a config from the build system
    QStringList pylibList = {
        "pylibs/shotgun_api3/3.9.2",
        "pylibs/python-ldap/3.4.4",
        "pylibs/ws4py/0.6.0",
        "pylibs/sqlalchemy/1.4.54",
        "OpenTimelineIO/25_10_py311",
        "PySide6/6.8.0_py311"
    };

    for (auto pylibName : pylibList)
    {
        QString pylibPath = QString("%1/%2/lib/python%3/site-packages:").arg(pylibRoot, pylibName, RLP_PY_VERSION);
        // qInfo() << pylibPath;

        ppath += pylibPath;
    }

    // qInfo() << "PYTHONPATH:" << ppath;
    // Info() << "REVLENS_PATH:" << revlensPath;

    QProcessEnvironment currEnv = QProcessEnvironment::systemEnvironment();

    if (!currEnv.contains("PYTHONPATH"))
    {
        qInfo() << "Setting PYTHONPATH";
        qInfo() << ppath;
        setenv("PYTHONPATH", ppath.toStdString().c_str(), 1);
    }

    if (!currEnv.contains("REVLENS_PATH"))
    {
        qInfo() << "Setting REVLENS_PATH";
        qInfo() << revlensPath;
        setenv("REVLENS_PATH", revlensPath.toStdString().c_str(), 1);
        setenv("RLP_NO_LOGIN", "1", 1);
    }

    if (isAppBundle)
    {
        QString currPath = qgetenv("PATH");
        QString newPath = QString("%1:%2/bin:%3").arg(path, pyFrameworkRoot, currPath);
        qInfo() << "Setting PATH";
        qInfo() << newPath;
        setenv("PATH", newPath.toStdString().c_str(), 1);
        setenv("RLP_INST_DIR", appRoot.toStdString().c_str(), 1);
    }

    #endif


    GUI_Startup s;

    bool result = s.start(argc, argv);

    return qapp.exec();
}