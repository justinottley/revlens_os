
#include <QtCore/QCoreApplication>

#include "RlpCore/NET/HttpServer.h"
#include "RlpCore/NET/NullRequestHandler.h"


int
main(int argc, char** argv)
{
    LOG_Logging::init();
    
    NET_NullRequestHandler* h = new NET_NullRequestHandler();
    NET_HttpServer* s = new NET_HttpServer(h);

    // NOTE: moveToThread() is not necessary, can result in thread
    // affinity issues with Qt objects since the mongoose request
    // handler will be running in the main thread
    //
    QThread t;
    // s->moveToThread(&t);
    //

    QObject::connect(
        &t,
        &QThread::started,
        s,
        &NET_HttpServer::startServer
    );

    t.start();

    QCoreApplication app(argc, argv);
    app.exec();
    
    /*
     * python rlp_core.remoting.Client example:
     * 
     * c = Client('rlp', server='localhost:PORT')
     * c.handle({'test':'the'})
     */
}