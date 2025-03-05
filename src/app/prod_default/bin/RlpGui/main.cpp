
#include "RlpProd/GUI/Startup.h"

#include <QtQuick/QQuickWindow>

#ifndef SCAFFOLD_WASM
#include <QtWebEngineQuick/QtWebEngineQuick>
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
    //QApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);

    // QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    #ifndef SCAFFOLD_WASM
    QtWebEngineQuick::initialize();
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

    GUI_Startup s;

    bool result = s.start(argc, argv);

    return qapp.exec();
}