
#include "RlpCore/LOG/Logging.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>

#include "RlpCore/NET/WebSocketClient.h"
#include "RlpCore/CNTRL/ControllerBase.h"
#include "RlpCore/CNTRL/Auth.h"

static CoreNet_WebSocketClient* wsClient;

void onClientConnected()
{
    qInfo() << "CONNECTED";

    QVariantList args;
    QVariantList filters;
    QVariantList returnFields;
    returnFields.append("code");

    args.append("Project");
    args.append(QVariant(filters));
    args.append(QVariant(returnFields));

    wsClient->sendCall("sg.find", args);
}


void onMessageReceived(QVariantMap msg)
{
    qInfo() << msg;
}


int main(int argc, char** argv)
{
    LOG_Logging::init();


    QCoreApplication app(argc, argv);

    // qInfo() << "QCoreApplication constructed";

    int port = 8009;

    // QString url = "wss://studio2.revlens.io:8003";
    QString url = "ws://127.0.0.1:8003";

    // CoreCntrl_Auth* auth = new CoreCntrl_Auth();
    // CoreNet_WebSocketClient* wsClient = new CoreNet_WebSocketClient();
    wsClient = new CoreNet_WebSocketClient();
    wsClient->setModeEncrypted(true);

    QObject::connect(
        wsClient,
        &CoreNet_WebSocketClient::onClientConnected,
        &onClientConnected
    );

    QObject::connect(
        wsClient,
        &CoreNet_WebSocketClient::messageReceived,
        &onMessageReceived
    );

    wsClient->setUrl(url);
    // wsClient->sendCall("test_call");

    app.exec();

}