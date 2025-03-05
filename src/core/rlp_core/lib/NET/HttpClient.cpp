
#include "RlpCore/NET/HttpClient.h"

RLP_SETUP_LOGGER(core, CoreNet, HttpRequest)
RLP_SETUP_LOGGER(core, CoreNet, HttpClient)

CoreNet_HttpRequest::CoreNet_HttpRequest(QString url, QNetworkReply* reply, QString runId):
    _url(url),
    _reply(reply),
    _runId(runId)
{

    if (runId == "")
    {
        _runId = QUuid::createUuid().toString();
    }

    connect(
        reply,
        &QNetworkReply::finished,
        this,
        &CoreNet_HttpRequest::onReplyFinished
    );
}


void
CoreNet_HttpRequest::onReplyFinished()
{
    emit finished(_runId);
}


CoreNet_HttpClient::CoreNet_HttpClient()
{
    _mgr = new QNetworkAccessManager(this);
}


QString
CoreNet_HttpClient::requestImage(QString url, QString runId)
{
    // RLP_LOG_DEBUG(url << runId)

    QNetworkReply* reply = _mgr->get(QNetworkRequest(url));

    CoreNet_HttpRequest* r = new CoreNet_HttpRequest(url, reply, runId);
    connect(
        r,
        &CoreNet_HttpRequest::finished,
        this,
        &CoreNet_HttpClient::onRequestFinished
    );

    _replyMap.insert(r->runId(), r);

    return r->runId();
}


void
CoreNet_HttpClient::onRequestFinished(QString runId)
{
    CoreNet_HttpRequest* r = _replyMap.value(runId);
    
    QByteArray result = r->reply()->readAll();

    QImage i = QImage::fromData(result, "jpg");

    r->reply()->deleteLater();
    r->deleteLater();
    
    _replyMap.remove(runId);

    QVariantMap data;
    data.insert("run_id", runId);
    data.insert("image", i);

    emit requestReady(data);
}