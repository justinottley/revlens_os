//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_WIDGET_THUMBNAIL_ITEM_H
#define GUI_WIDGET_THUMBNAIL_ITEM_H

#include "RlpCore/LOG/Logging.h"
#include "RlpGui/BASE/ItemBase.h"

#include "RlpGui/WIDGET/Global.h"

class GUI_WIDGET_API GUI_ThumbnailItem : public GUI_ItemBase {
    Q_OBJECT

signals:
    void hoverEnter(QVariantMap data);
    void hoverLeave(QVariantMap data);

public:
    RLP_DEFINE_LOGGER

    GUI_ThumbnailItem(GUI_ItemBase* parent, QImage p, qreal width);
    GUI_ThumbnailItem(GUI_ItemBase* parent = nullptr);
    
    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);
    void hoverMoveEvent(QHoverEvent* event);

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

public slots:

    static GUI_ThumbnailItem* new_GUI_ThumbnailItem() { return new GUI_ThumbnailItem(); }

    void setLabelWidget(GUI_ItemBase* label);
    void setLabel(QString label);
    void setData(QVariantMap data) { _data = data; }
    void setOutlined(bool doOutline) { _doOutline = doOutline; }
    void setThumbImage(QImage thumbImg);

    QVariantMap data() { return _data; }

    GUI_ThumbnailItem* fromBase64(GUI_ItemBase* parent, QString b64Input, int width);
    GUI_ThumbnailItem* fromPath(GUI_ItemBase* parent, QString filePath, int width);


protected:
    QImage _srcThumb;
    QImage _thumb;
    QString _thumbPath;
    QVariantMap _data;


    GUI_ItemBase* _label;
    bool _doOutline;

};


/*
class GUI_WIDGET_API GUI_ThumbnailItemDecorator : public QObject {
    Q_OBJECT

public slots:

    GUI_ThumbnailItem*
    new_GUI_ThumbnailItem(GUI_ItemBase* parent = nullptr)
    {
        return new GUI_ThumbnailItem(parent);
    }


};
*/

#endif