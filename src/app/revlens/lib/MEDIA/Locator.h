

#ifndef REVLENS_MEDIA_LOCATOR_H
#define REVLENS_MEDIA_LOCATOR_H

#include "RlpRevlens/MEDIA/Global.h"
#include "RlpCore/LOG/Logging.h"

class MEDIA_Locator : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    MEDIA_Locator(QString name);

public slots:

    QString name() { return _name; }

    virtual bool locate(QVariantMap* mediaInput);


private:
    QString _name;

};

#endif
