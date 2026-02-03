
#include "RlpCore/LOG/Logging.h"

#include "RlpCore/NET/WebSocketServer.h"
#include "RlpEdb/CNTRL/Query.h"


int main(int argc, char** argv)
{
    LOG_Logging::init();

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

    QString dbPath = QString("/opt/rlp/edb/production_%1.db").arg(port);
    if (argc > 2)
    {
        dbPath = QString(argv[2]);
    }

    QString dbName = "rlp_site";
    if (argc > 3)
    {
        QString dbType = QString(argv[3]);
    }


    QString dbType = "production_db";
    if (argc > 4)
    {
        dbType = QString(argv[4]);
    }


    qInfo();
    qInfo() << "edb_server starting up";
    qInfo();

    qInfo() << "edb_server - port: " << port;
    qInfo() << "edb_server - DbPath: " << dbPath;
    qInfo() << "edb_server - DbName: " << dbName;
    qInfo() << "edb_server - DbType: " << dbType;
    qInfo();

    QCoreApplication app(argc, argv);

    // qInfo() << "QCoreApplication constructed";


    CoreNet_WebSocketServer* server = new CoreNet_WebSocketServer(port);
    server->setModeEncrypted(true);

    CoreCntrl_ControllerBase* cntrl = new EdbCntrl_Query(dbPath, dbName, dbType);
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


    app.exec();

}