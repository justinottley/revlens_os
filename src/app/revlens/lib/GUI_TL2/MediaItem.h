//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_TL2_MEDIA_ITEM_H
#define REVLENS_GUI_TL2_MEDIA_ITEM_H

#include "RlpRevlens/GUI_TL2/Global.h"


#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/GUI_TL2/Item.h"
#include "RlpRevlens/GUI_TL2/Track.h"
#include "RlpRevlens/GUI_TL2/MediaTrack.h"

class GUI_TL2_View;
class GUI_TL2_MediaItem;

class REVLENS_GUI_TL2_API GUI_TL2_MediaItemBackground : public GUI_ItemBase {

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_MediaItemBackground(GUI_TL2_MediaItem* item);

    void initWaveform();
    void loadWaveform(QString waveformPath);

    void setThumbCachePercent(float tcpercent);

    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);

private:
    GUI_TL2_MediaItem* _item;
    QImage _waveform;
    float _thumbCachePercent;

};

class REVLENS_GUI_TL2_API GUI_TL2_MediaItemForeground : public GUI_ItemBase {

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_MediaItemForeground(GUI_TL2_MediaItem* parent);
    
    void generateThumbnail(bool append=true);
    void initThumbnail();
    
    // TODO FIXME: clean up primary, this is a hack
    void loadThumbnailFromPath(QString thumbnailPath, bool primary=true);
    void loadThumbnailFromData(QString thumbData);


    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);

    
private:
    GUI_TL2_MediaItem* _item;
    QList<QImage> _thumbnails;
};


class REVLENS_GUI_TL2_API GUI_TL2_MediaItem : public GUI_TL2_Item {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_MediaItem(GUI_TL2_Track* track, DS_NodePtr node, qlonglong startFrame, qlonglong frameCount);

    void refresh();

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    

    QRectF boundingRect() const;
    QRectF itemRect();

    void paint(GUI_Painter* painter);
    
public slots:

    void onParentSizeChanged(qreal width, qreal height);

    QString label();

    GUI_TL2_MediaTrack* track();
    DS_NodePtr node() { return _node; }

    int heightThumbnail() { return 40; }
    
    void clear();
    void updateItem();

public slots:

    void setYPos(int ypos);
    void onDataReady(QVariantMap data);


private:

    GUI_TL2_MediaItemBackground* _bg;
    GUI_TL2_MediaItemForeground* _fg;

    DS_NodePtr _node;

};

#endif
