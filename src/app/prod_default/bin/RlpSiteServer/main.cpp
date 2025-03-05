

#include "RlpProd/SITE/ServerController.h"



#include "RlpCore/LOG/Logging.h"
#include "RlpCore/NET/WebSocketServer.h"
#include "RlpCore/PY/Interp.h"

#include "RlpEdb/CNTRL/Query.h"

int main(int argc, char** argv)
{
    // needed before logging init
    QCoreApplication::setApplicationName("site_server");
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    LOG_Logging::init();

    LOG_Logging::getLogger("edb")->setLevel(10);
    LOG_Logging::getLogger("edb.EdbStorSql")->setLevel(LOG_Logging::DEBUG);

    // edb_server port db_path db_name db_type
    // e.g.,
    //
    // edb_server 45303 /path/to/production.db rlp_site production_db

    // qInfo() << "Arg Count: " << argc;

    int port = 8003;
    if (argc > 1)
    {
        qInfo() << "Loading port: " << argv[1];
        port = atoi(argv[1]);
    }

    qInfo();
    qInfo() << "rlp_site_server starting up";
    qInfo();

    qInfo() << "rlp_site_server - port: " << port;
    
    QCoreApplication app(argc, argv);


    SITE_ServerController* cntrl = new SITE_ServerController(port, /*runIOService=*/true);

    CoreNet_WebSocketServer* server = new CoreNet_WebSocketServer(port, /*secure=*/false); // false);
    server->setModeEncrypted(true);

    cntrl->setWsServer(server);


    QObject::connect(
        cntrl,
        SIGNAL(dataReady(QVariantMap)),
        server,
        SLOT(sendCallToClient(QVariantMap))
    );

    // Run init AFTER router is setup so sig / slot connections through
    // the router's server can be properly made
    cntrl->initPython();

    QThread t;

    QObject::connect(
        &t,
        &QThread::started,
        server,
        &CoreNet_WebSocketServer::start
    );

    t.start();


    app.exec();

}