
#ifndef REVLENS_CNTRL_MEDIA_SERVICE_CNTRL_H
#define REVLENS_CNTRL_MEDIA_SERVICE_CNTRL_H

#include "RlpRevlens/CNTRL/Global.h"

#include "RlpRevlens/DS/Node.h"


class CNTRL_MainController;

class CNTRL_MediaServiceController : public QObject {
    Q_OBJECT

signals:
    void sendResult(QVariantMap result);

public:
    RLP_DEFINE_LOGGER

    CNTRL_MediaServiceController(CNTRL_MainController* cntrl);

public slots:
    void onRemoteCallReceived(QVariantMap msgInfo);

    QVariantMap runMediaLoad(QVariantMap mediaInfo);
    QVariantMap runReadFrame(QString path, qlonglong frame);

private:
    CNTRL_MainController* _cntrl;
    QMap<QString, DS_NodePtr> _nodeMap;

};

#endif
