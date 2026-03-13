
#include "RlpCore/NET/Global.h"


class CORE_NET_API NET_NullRequestHandler : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

public slots:

    void handle(QVariantMap payload);

};