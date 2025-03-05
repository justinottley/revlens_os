
#include <iostream>
// #include <pybind11/pybind11.h>
// #include <pybind11/embed.h> // everything needed for embedding

// #include "RlpCore/LOG/Logging.h"
// #include "RlpCore/PY/Interp.h"

// #include "RlpGui/PANE/View.h"

#include "RlpProd/GUI/Startup.h"

#include <QApplication>
#include <QQuickWindow>

// #include <QDir>
// #include <QFileInfo>

// #include "analogclock.h"

namespace py = pybind11;

int main(int argc, char *argv[])
{
    const char* execPath = argv[0];
    
    QFileInfo execFile(execPath);

    QDir parentDir = execFile.absoluteDir();
    QString pyHomeString = parentDir.absolutePath();
    pyHomeString += "/python";
    
    QString pyPathString = pyHomeString;
    pyPathString += "/lib/python3.11";
    
    QString appRoot = parentDir.absolutePath();
    qDebug() << "Setting APP_ROOT";
    setenv("APP_ROOT", appRoot.toUtf8().constData(), 1);

    qDebug() << "Setting PYTHONHOME:" << pyHomeString;
    setenv("PYTHONHOME", pyHomeString.toUtf8().constData(), 1);
    
    qDebug() << "Setting PYTHONPATH:" << pyPathString;
    setenv("PYTHONPATH", pyPathString.toUtf8().constData(), 1);
    
    QString revlensPath = parentDir.absolutePath();
    revlensPath += "/revlens";
    
    revlensPath += ":";
    revlensPath += pyHomeString;

    qDebug() << "Setting REVLENS_PATH:" << revlensPath;
    setenv("REVLENS_PATH", revlensPath.toUtf8().constData(), 1);

    setenv("QSG_RENDER_LOOP", "basic", 1);
    
    QCoreApplication::setApplicationName("revlens");

    QApplication app(argc, argv);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    
    Q_INIT_RESOURCE(core_CNTRL_resources);
    Q_INIT_RESOURCE(gui_BASE_resources);
    Q_INIT_RESOURCE(revlens_GUI_resources);
    Q_INIT_RESOURCE(extrevlens_RLANN_GUI_resources);
    Q_INIT_RESOURCE(extrevlens_RLANN_MP_resources);
        

    GUI_Startup s;

    bool result = s.start(argc, argv);
    
    //    QQuickWindow* w = new QQuickWindow();
    //    w->resize(800, 600);
    //    w->show();
    //    GUI_View* v = new GUI_View();
    //    v->resize(800, 600);
    //    v->show();
    
    // QApplication app(argc, argv);
    // AnalogClock clock;
    // clock.show();
    
    
    return app.exec();
}
