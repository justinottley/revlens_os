//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include <iostream>

#include "RlpCore/NET/HttpServer.h"

#include "RlpCore/UTIL/Convert.h"


RLP_SETUP_LOGGER(core, NET, HttpServer)

static struct mg_serve_http_opts s_http_server_opts;

const char* NET_HttpServer::RPC_URL = "/rlp/default/call/json/rpc";


static void ev_handler(struct mg_connection *nc, int ev, void *p) {
    
  NET_HttpServer* server = (NET_HttpServer *)nc->mgr->user_data;
  server->eventHandler(nc, ev, p);
  
}


NET_HttpServer::NET_HttpServer(QObject* handler) :
    _handler(handler)
{
}


QVariantMap
NET_HttpServer::handleRequest(const char* method, QVariantList argList)
{
    bool result = 0;
    QVariant retVal;

    //
    // Some argument handling
    //
    if (argList.size() > 0) {
               
        QVariant firstArg = argList.at(0);
        
        // std::cout << firstArg.toJsonDocument().toJson().constData() << std::endl;
        
        if (firstArg.canConvert<QVariantMap>()) {
            
            QVariantMap fm = firstArg.toMap();
            
            result = QMetaObject::invokeMethod(
                _handler,
                method,
                Qt::AutoConnection,
                Q_ARG(QVariantMap, fm));
            
            
        } else if (firstArg.canConvert<QVariantList>()) {
            
            QVariantList fl = firstArg.toList();
            
            result = QMetaObject::invokeMethod(
                _handler,
                method,
                Qt::AutoConnection,
                Q_ARG(QVariantList, fl));
            
        } else if (firstArg.canConvert<QString>()) {

            RLP_LOG_VERBOSE("First arg is string")

            

            result = QMetaObject::invokeMethod(
                _handler,
                method,
                Qt::AutoConnection,
                Q_RETURN_ARG(QVariant, retVal),
                Q_ARG(QString, firstArg.toString())
            );

        } else if (firstArg.canConvert<float>()) {
            
            result = QMetaObject::invokeMethod(
                _handler,
                method,
                Qt::AutoConnection,
                Q_ARG(float, firstArg.toFloat()));
            
        } 
        
    } else {
        
        result = QMetaObject::invokeMethod(
                _handler,
                method,
                Qt::AutoConnection);
        
    }

    RLP_LOG_VERBOSE(method << " result: " << retVal)

    QVariantMap fresult;
    fresult.insert("result", retVal);

    return fresult;
}


void
NET_HttpServer::eventHandler(struct mg_connection *nc, int ev, void *ev_data)
{
    
    
  if (ev == MG_EV_HTTP_REQUEST) {
       
        RLP_LOG_VERBOSE("Got HTTP request")
      
        struct http_message *hm = (struct http_message *) ev_data;
        // std::cout << hm->uri.p << std::endl;
       
        if (mg_vcmp(&hm->uri, RPC_URL) == 0) {
           
            // TODO FIXME: not sure if the transfer encoding should be changed
            //
            mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            
            char method[2048];
            char args[4096];
            
            
            // Get form variables
            mg_get_http_var(&hm->body, "method", method, sizeof(method));
            mg_get_http_var(&hm->body, "args", args, sizeof(args));
            
            RLP_LOG_VERBOSE("Method: " << method)
            RLP_LOG_VERBOSE("Args: " << args)
            
            
            QJsonDocument jsonArgs(QJsonDocument::fromJson(QByteArray(args)));
            QVariantList argList = jsonArgs.toVariant().toList();
            
            RLP_LOG_VERBOSE("json raw data: " << jsonArgs.toJson().constData())
            
            QVariantMap result = handleRequest(method, argList);
            
            RLP_LOG_VERBOSE("Attempting to convert: " << result)

            QJsonObject jresult;
            UTIL_Convert::toJsonObject(result, &jresult);
        
            RLP_LOG_VERBOSE("convert done")

            QJsonObject envelope;
            envelope.insert("status", 0);
            envelope.insert("result", jresult);

            QJsonDocument doc(envelope);

            const char* response = doc.toJson(QJsonDocument::Compact).constData();

            // const char *response = "{\"status\": 0, \"result\": \"{}\"}";
            
            mg_printf_http_chunk(nc, response);
            mg_send_http_chunk(nc, "", 0); // Send empty chunk, the end of response
            
            
            
            
            
            
       } else {
           mg_serve_http(nc, hm, s_http_server_opts);
       }
  }
  
  
  
}



void
NET_HttpServer::writeServerPort()
{
    qint64 appPid = QCoreApplication::applicationPid();
    QString pidFile = QString("/tmp/rlp_http_server_%1.pid").arg(appPid);
    
    RLP_LOG_DEBUG("writing port: " << _serverPort << " to pid file " << pidFile)
    
    QFile pf(pidFile);
    
    if (pf.open(QIODevice::WriteOnly)) {
        
        QTextStream out(&pf);
        
        out << _serverPort;
        
        pf.flush();
        pf.close();
    }
}


void
NET_HttpServer::startServer()
{
    RLP_LOG_DEBUG("")
    
    // Start on a random port
    //
    const char *s_http_port = "0"; // "8000";
    
    struct mg_mgr mgr;
    struct mg_connection *nc;
    
    mg_mgr_init(&mgr, this);
    
    nc = mg_bind(&mgr, s_http_port, ev_handler);
    
    
    // Set up HTTP server parameters
    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.document_root = ".";      // Serve current directory
    s_http_server_opts.dav_document_root = ".";  // Allow access via WebDav
    s_http_server_opts.enable_directory_listing = "yes";
    
    // Get the server port
    //
    
    struct sockaddr_in sin;
    socklen_t slen = sizeof(sin);
    getsockname(mgr.active_connections->sock, (struct sockaddr *)&sin, &slen);
    _serverPort = ntohs(sin.sin_port);
    
    
    // Write it out to a file
    //
    writeServerPort();
    
    RLP_LOG_INFO("http server running port " << _serverPort)
    
    for (;;) {
        mg_mgr_poll(&mgr, 100);
    }
    
    mg_mgr_free(&mgr);
    
    
}