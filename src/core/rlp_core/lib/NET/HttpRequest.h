
#ifndef CORE_NET_HTTP_REQUEST_H
#define CORE_NET_HTTP_REQUEST_H


#include "RlpCore/NET/Global.h"

#include <QtNetwork/QNetworkReply>

class CoreNet_HttpRequest : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    CoreNet_HttpRequest(QString url, QNetworkReply* reply, QString runId="");

    void insertMetadata(QString key, QVariant val) { _metadata.insert(key, val); }
    QVariantMap metadata() { return _metadata; }

signals:
    void finished(QString runId);

public slots:
    QString url() { return _url; }
    QNetworkReply* reply() { return _reply; }
    QString runId() { return _runId; }

private slots:
    void onReplyFinished();

protected:
    QString _url;
    QNetworkReply* _reply;
    QString _runId;
    QVariantMap _metadata;
};



#endif

