
#include <QtCore/QCoreApplication>
#include <QtCore/QThread>

#include "RlpCore/NET/WebSocketServer.h"
#include "RlpCore/CNTRL/ControllerBase.h"


int main(int argc, char** argv)
{
    LOG_Logging::init();


    QCoreApplication app(argc, argv);

    // qInfo() << "QCoreApplication constructed";

    int port = 8009;


    CoreNet_WebSocketServer* server = new CoreNet_WebSocketServer(port);
    server->setModeEncrypted(true);

    CoreCntrl_ControllerBase* cntrl = new CoreCntrl_ControllerBase();
    cntrl->setWsServer(server);



    QThread t;
    // s->moveToThread(&t);

    QObject::connect(
        &t,
        &QThread::started,
        server,
        &CoreNet_WebSocketServer::start
    );

    t.start();

    qInfo() << "Listening on port" << port;

    app.exec();

}