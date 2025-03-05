
#ifndef CORE_CoreNet_HTTP_CLIENT_H
#define CORE_CoreNet_HTTP_CLIENT_H

#include "RlpCore/NET/Global.h"

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

class CoreNet_HttpRequest : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    CoreNet_HttpRequest(QString url, QNetworkReply* reply, QString runId="");

signals:
    void finished(QString runId);

public slots:
    QString url() { return _url; }
    QNetworkReply* reply() { return _reply; }
    QString runId() { return _runId; }

private slots:
    void onReplyFinished();

private:
    QString _url;
    QNetworkReply* _reply;
    QString _runId;
};


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

private slots:
    void onRequestFinished(QString runId);

private:
    QNetworkAccessManager* _mgr;
    QMap<QString, CoreNet_HttpRequest*> _replyMap;

};

#endif
