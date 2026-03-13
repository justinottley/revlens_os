
#ifndef REVLENS_GUI_TLT_MEDIA_ITEM_H
#define REVLENS_GUI_TLT_MEDIA_ITEM_H

#include "RlpRevlens/GUI_TLT/Global.h"

#include "RlpGui/BASE/ItemBase.h"

class GUI_TLT_View;


class GUI_TLT_MediaItem : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_TLT_MediaItem(GUI_TLT_View* view, QVariantMap mediaInfo, qlonglong startFrame, qlonglong frameCount, bool isOdd=false);

    void paint(GUI_Painter* painter);

public slots:

    QString name() { return _name; }
    QImage thumbnail() { return _thumbnail; }

    qlonglong startFrame() { return _startFrame; }
    qlonglong endFrame() { return _endFrame; }
    qlonglong frameCount() { return _frameCount; }

    void setItemEnabled(bool isEnabled) { _isEnabled = isEnabled; }

    void refresh();

    virtual qreal itemWidth();

private:

    GUI_TLT_View* _view;

    QVariantMap _mediaInfo;

    QString _name;
    QImage _thumbnail;

    qlonglong _startFrame;
    qlonglong _endFrame;
    qlonglong _frameCount;

    bool _isEnabled;
    bool _isOdd;
};

#endif
