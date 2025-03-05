
#ifndef CORE_UTIL_TEXT_H
#define CORE_UTIL_TEXT_H

#include "RlpCore/UTIL/Global.h"

class CORE_UTIL_API UTIL_Text : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    UTIL_Text() {}

public:
    static int getWidth(QString text, QString fontStr=QString(""));
    static QStringList wrapToWidth(QString text, int maxWidth);

public slots:

    static UTIL_Text* new_UTIL_Text() { return new UTIL_Text(); }

    QFont currFont() { return QApplication::font(); }

    int getTextWidth(QString text, QString fontStr=QString("")) { return UTIL_Text::getWidth(text, fontStr); }
    int getTextWidthF(QString text, QFont f);
    QString clipToWidth(QString text, int maxWidth);
    QStringList wrapTextToWidth(QString text, int maxWidth) { return UTIL_Text::wrapToWidth(text, maxWidth); }

};


/*
class CORE_UTIL_API  UTIL_TextDecorator : public QObject {
    Q_OBJECT

public slots:

    
};
*/


#endif