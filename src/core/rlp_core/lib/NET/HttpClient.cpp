
#include "RlpCore/NET/HttpClient.h"
#include "RlpCore/UTIL/Convert.h"

RLP_SETUP_LOGGER(core, CoreNet, HttpClient)


CoreNet_HttpClient::CoreNet_HttpClient()
{
    _mgr = new QNetworkAccessManager(this);
}


QString
CoreNet_HttpClient::requestPOST(QString url, QVariantMap data, QString runId)
{
    QNetworkRequest req(url);
    QJsonObject obj;
    UTIL_Convert::toJsonObject(data, &obj);
    QJsonDocument doc(obj);
    QByteArray jsonData = doc.toJson();

    QNetworkReply* reply = _mgr->post(req, jsonData);

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
        &CoreNet_HttpClient::onImageRequestFinished
    );

    _replyMap.insert(r->runId(), r);

    return r->runId();
}


void
CoreNet_HttpClient::onImageRequestFinished(QString runId)
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


void
CoreNet_HttpClient::onRequestFinished(QString runId)
{
    CoreNet_HttpRequest* r = _replyMap.value(runId);

    QByteArray result = r->reply()->readAll();

    r->reply()->deleteLater();
    r->deleteLater();
    
    _replyMap.remove(runId);

    QVariantMap data;
    data.insert("run_id", runId);
    data.insert("result", QString(result));

    emit requestReady(data);
}