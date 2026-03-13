

#include "RlpCore/NET/HttpRequest.h"

RLP_SETUP_LOGGER(core, CoreNet, HttpRequest)

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

