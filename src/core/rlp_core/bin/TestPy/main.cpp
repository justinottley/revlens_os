
#include "RlpCore/PY/Interp.h"
#include "RlpCore/UTIL/StacktraceHandler.h"
#include "RlpCore/LOG/Logging.h"

#include <QtGui/QGuiApplication>


#include "RlpCore/QTCORE/QtCoreTypes.h"

int main(int argc, char** argv)
{
    LOG_Logging::init();
    UTIL_StacktraceHandler::installSignalHandler();

    QGuiApplication app(argc, argv);

    // QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    Q_INIT_RESOURCE(core_CNTRL_resources);

    RlpCoreQTCOREmodule_bootstrap();

    PY_Interp::init();

    PY_Interp::eval("import RlpCoreQTCORE");
    PY_Interp::eval("import PySide6");
    PY_Interp::eval("from PySide6 import QtCore;print(QtCore);");
    PY_Interp::eval("print('imports ok')");

    qInfo() << "OK";

    return QGuiApplication::exec();
}