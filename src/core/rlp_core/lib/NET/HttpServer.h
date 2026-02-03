//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef CORE_NET_SERVER_H
#define CORE_NET_SERVER_H

#include "RlpCore/NET/Global.h"

// mongoose can be obtained from git repo https://github.com/cesanta/mongoose.git
//
#ifdef _WIN32
#include "RlpCore/NET/mongoose.h"
#else
#include "RlpCore/MONGOOSE/mongoose.h"
#endif

class CORE_NET_API NET_HttpServer : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    NET_HttpServer(QObject* handler);

public slots:

    void startServer();

public:

    virtual QVariantMap handleRequest(const char* method, QVariantList argList);

    void eventHandler(struct mg_connection *nc, int ev, void *p);

private:

    void writeServerPort();

protected:

    QThread _serverThread;
    QWaitCondition _stopCondition;

    uint _serverPort;

    static const char* RPC_URL;

    QObject* _handler;

};

#endif
