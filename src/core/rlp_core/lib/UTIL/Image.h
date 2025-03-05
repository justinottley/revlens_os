
#ifndef CORE_UTIL_IMAGE_H
#define CORE_UTIL_IMAGE_H


#include "RlpCore/UTIL/Global.h"
#include "RlpCore/LOG/Logging.h"


class CORE_UTIL_API UTIL_Image : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    UTIL_Image() {}


public:
    static QString s_toBase64(QImage pIn, QString format="JPG");
    static QImage s_fromBase64(QString b64input);
    static QImage s_fitImage(int width, int height, QImage image);

public slots:

    QString toBase64(QImage p, QString format="JPG") { return UTIL_Image::s_toBase64(p, format); }
    QImage fromBase64(QString b64data) { return UTIL_Image::s_fromBase64(b64data); }

    static UTIL_Image* new_UTIL_Image() { return new UTIL_Image(); }

};


#endif