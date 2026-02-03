
#include "RlpProd/CNTRL/Global.h"

class CNTRL_StreamingController;

class CNTRL_LoadStreamTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

    CNTRL_LoadStreamTask(CNTRL_StreamingController* cntrl, QString streamUrl, QVariantMap streamInfo):
        _cntrl(cntrl),
        _streamUrl(streamUrl),
        _streamInfo(streamInfo)
    {
    }

    void setupMediaInfo(QVariantMap* minfo);
    void run();

private:
    CNTRL_StreamingController* _cntrl;
    QString _streamUrl;
    QVariantMap _streamInfo;

};

