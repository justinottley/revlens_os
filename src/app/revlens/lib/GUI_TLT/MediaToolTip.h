
#ifndef GUI_TLT_MEDIA_TOOLTIP_H
#define GUI_TLT_MEDIA_TOOLTIP_H

#include "RlpRevlens/GUI_TLT/Global.h"

class GUI_TLT_MediaToolTip : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_TLT_MediaToolTip(GUI_ItemBase* parent);

    void setTextTop(QString text) { _textTop = text; }
    void setTextBottom(QString text) { _textBottom = text; }

    void setThumbnail(QImage thumbnail) { _thumbnail = thumbnail; }

    void paint(GUI_Painter* painter);

private:
    QString _textTop;
    QString _textBottom;
    QImage _thumbnail;

};


#endif

