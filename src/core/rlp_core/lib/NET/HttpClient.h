
#ifndef CORE_CoreNet_HTTP_CLIENT_H
#define CORE_CoreNet_HTTP_CLIENT_H

#include "RlpCore/NET/Global.h"
#include "RlpCore/NET/HttpRequest.h"

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>



class CoreNet_HttpClient : public QObject {
    Q_OBJECT

signals:
    void requestReady(QVariantMap data);

public:
    RLP_DEFINE_LOGGER

    CoreNet_HttpClient();

public slots:
    static CoreNet_HttpClient* new_CoreNet_HttpClient()
    {
        return new CoreNet_HttpClient();
    }

    QString requestImage(QString url, QString runId="");
    QString requestPOST(QString url, QVariantMap data, QString runId="");

private slots:
    void onImageRequestFinished(QString runId);
    void onRequestFinished(QString runId);

private:
    QNetworkAccessManager* _mgr;
    QMap<QString, CoreNet_HttpRequest*> _replyMap;

};

#endif
