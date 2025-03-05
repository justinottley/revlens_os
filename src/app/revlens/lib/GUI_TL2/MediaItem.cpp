
#include "RlpRevlens/GUI_TL2/MediaItem.h"

#include "RlpCore/PY/Interp.h"

#include "RlpGui/BASE/Style.h"

#include "RlpRevlens/GUI_TL2/View.h"


RLP_SETUP_LOGGER(revlens, GUI_TL2, MediaItemForeground)
RLP_SETUP_LOGGER(revlens, GUI_TL2, MediaItemBackground)
RLP_SETUP_LOGGER(revlens, GUI_TL2, MediaItem)

GUI_TL2_MediaItemForeground::GUI_TL2_MediaItemForeground(GUI_TL2_MediaItem* item):
    GUI_ItemBase(item),
    _item(item)
{
    setWidth(item->width());
    setHeight(item->height());

    setZValue(5);
    initThumbnail();
}


void
GUI_TL2_MediaItemForeground::generateThumbnail(bool append)
{
    RLP_LOG_DEBUG(append)

    if (!append)
    {
        _thumbnails.clear();
    }

    QVariantMap props = _item->node()->getProperties();

    QVariantList frameList;
    qlonglong midFrame = int(props.value("media.frame_count").toLongLong() / 2.0);
    if (midFrame != 0)
    {
        frameList.append(midFrame);
    } else
    {
        frameList.append(1);
    }


    QVariantMap extra;
    extra.insert("track_uuid", _item->track()->uuid());
    extra.insert("media_uuid", _item->uuid());

    QVariantList argList;
    argList.append(props.value("media.video"));
    argList.append(QVariant(frameList));
    argList.append(QString(""));
    argList.append(extra);

    QVariant result = PY_Interp::call("revlens.gui.timeline.request_thumbnail_gen", argList);

    // TODO FIXME: We don't need to serialize the thumbnail to disk, can just
    // return a QImage

    QString thumbPath = result.toString();

    if (thumbPath.size() > 0)
    {
        loadThumbnailFromPath(thumbPath);
    }

}


void
GUI_TL2_MediaItemForeground::initThumbnail()
{
    QVariantMap props = _item->node()->getProperties();

    if (props.contains("media.thumbnail.data"))
    {
        QString thumbData = props.value("media.thumbnail.data").toString();
        if (thumbData.size() > 0)
        {
            loadThumbnailFromData(thumbData);
        } else
        {
            generateThumbnail();
        }
        
    } else if (props.contains("media.thumbnail.path"))
    {
        QString thumbPath = props.value("media.thumbnail.path").toString();
        loadThumbnailFromPath(thumbPath);

    } else 
    {
        generateThumbnail();
    }
}


void
GUI_TL2_MediaItemForeground::loadThumbnailFromPath(QString thumbnailPath, bool primary)
{
    RLP_LOG_DEBUG(thumbnailPath);

    QImage p(thumbnailPath);
    p = p.scaledToHeight(
        _item->heightThumbnail() - 2,
        Qt::SmoothTransformation
    );

    _thumbnails.push_back(p);

    if (!primary)
    {
        return;
    }


    // save base64 data into media for shipping to DB
    //
    QByteArray ba;
    QBuffer rbuf(&ba);
    rbuf.open(QIODevice::WriteOnly);
    p.save(&rbuf, "JPG", -1);
    rbuf.close();

    QVariantMap* props = _item->node()->getPropertiesPtr();
    QString result = QString(ba.toBase64());

    props->insert("media.thumbnail.data", result);
    props->insert("media.thumbnail.path", thumbnailPath);
    props->insert("media.thumbnail.image", p);

    // RLP_LOG_DEBUG(result)
    // RLP_LOG_DEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
    // RLP_LOG_DEBUG(*props)


    // emit dataReady from the nodegraph so subsequent clients can see that
    // thumbnail data is now available
    //
    QVariantMap dataInfo;
    dataInfo.insert("data_type", "thumbnail_ready");
    _item->node()->graph()->emitDataReady(dataInfo);

    update();
}


void
GUI_TL2_MediaItemForeground::loadThumbnailFromData(QString thumbData)
{
    // RLP_LOG_DEBUG("");

    QImage p;
    p.loadFromData(QByteArray::fromBase64(thumbData.toLatin1()));
    p = p.scaledToHeight(
        _item->heightThumbnail() - 2,
        Qt::SmoothTransformation
    );

    _thumbnails.push_back(p);

    _item->node()->getPropertiesPtr()->insert("media.thumbnail.image", p);

    update();
}




QRectF
GUI_TL2_MediaItemForeground::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_TL2_MediaItemForeground::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG(globalPosition())
    // RLP_LOG_DEBUG(pos())
    // RLP_LOG_DEBUG(boundingRect())
    
    QRectF brect = boundingRect();

    // painter->setRenderHint(QPainter::Antialiasing, true);

    // painter->save();
    // painter->setClipRect(_item->itemRect(), Qt::IntersectClip);

    painter->setPen(QPen(QColor(200, 200, 200)));

    //int textPos = 20;
    int thumbnailOffset = 0;


    if ((_thumbnails.size() > 0) && (_item->track()->isThumbnailVisible())) {
        // RLP_LOG_DEBUG("draw thumbnail");
        // thumbnailOffset = _thumbnail.width() + 2;
        painter->setPen(Qt::black);
        // painter->drawRect(brect.topLeft().x() + 4, 4, _thumbnail.width() + 2, _thumbnail.height() + 2);

        int tx = 0;
        for (int ti = 0; ti != _thumbnails.size(); ++ ti)
        {
            painter->drawImage(
                brect.topLeft().x() + 3 + tx,
                5,
                const_cast<QImage*>(&_thumbnails.at(ti))
            );

            tx += _thumbnails.at(ti).width() + 3;
        }
        
        //textPos += _item->track()->heightVideo();
    }



    // RLP_LOG_DEBUG(_item->label());

    // painter->save();

    QFont font = painter->font();
    font.setPixelSize(12);
    painter->setFont(font);

    painter->setPen(Qt::white);
    painter->drawText(
        brect.topLeft().x() + thumbnailOffset + 5,
        _item->track()->heightText() + 2,
        _item->label());

    // painter->restore();

}


// --------

GUI_TL2_MediaItemBackground::GUI_TL2_MediaItemBackground(GUI_TL2_MediaItem* item):
    GUI_ItemBase(item),
    _item(item),
    _thumbCachePercent(0.0)
{
    setAcceptHoverEvents(true);

    setWidth(item->width());
    setHeight(item->height());
    setZValue(3);

    initWaveform();
}


void
GUI_TL2_MediaItemBackground::initWaveform()
{
    RLP_LOG_DEBUG("")
    
    QVariantMap* props = _item->node()->getPropertiesPtr();
    if ((props->contains("audio.waveform.path")) && (_waveform.width() == 0))
    {
        loadWaveform(props->value("audio.waveform.path").toString());
    }
}


void
GUI_TL2_MediaItemBackground::loadWaveform(QString waveformPath)
{
    RLP_LOG_DEBUG(waveformPath);

    _waveform.load(waveformPath);

    update();
}


void
GUI_TL2_MediaItemBackground::setThumbCachePercent(float tcpercent)
{
    // RLP_LOG_DEBUG(tcpercent)

    _thumbCachePercent = tcpercent;
}


void
GUI_TL2_MediaItemBackground::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _inHover = true;
    update();
}


void
GUI_TL2_MediaItemBackground::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _inHover = false;
    update();
}


QRectF
GUI_TL2_MediaItemBackground::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_TL2_MediaItemBackground::paint(GUI_Painter* painter)
{
    // painter->setClipRect(_item->boundingRect(), Qt::IntersectClip);

    // painter->setClipRect(_item->itemRect(), Qt::IntersectClip); 
    // brect); // _item->view()->timelineClipRegion());

    QPen p(Qt::black);
    if (_inHover)
    {
        p.setColor(QColor(80, 80, 80));
        p.setWidth(2);
    }

    if (_item->selected())
    {
        p.setColor(QColor(120, 120, 120));
        p.setWidth(3);
    }
    painter->setPen(p);
    painter->setBrush(QBrush(GUI_Style::TrackMediaBg));

    painter->drawRect(_item->boundingRect());

    // p.setColor(Qt::red);
    // p.setBrush(QBrush(GUI_Style::TabBg));

    painter->setPen(p);
    painter->setBrush(QBrush(GUI_Style::TabBg));
    QRectF textBg = _item->boundingRect();
    textBg.setHeight(20);
    textBg.moveBottom(_item->track()->heightText() + 8);

    painter->drawRect(textBg);

    // Why is the waveform drawn in the background?
    //
    if ((_waveform.width() > 0) && (_item->track()->isWaveformVisible()))
    {
        QRectF brect = boundingRect();
        // QRect pixRect(
        //     brect.topLeft().x(),
        //     _item->track()->heightVideo() + 5,
        //     brect.width(),
        //     20
        // );
        // painter->drawImage(pixRect, _waveform);

        painter->setPen(QColor(Qt::white));
        painter->drawImage(
            brect.topLeft().x(),
            _item->track()->heightVideo() + 5,
            brect.width(),
            20,
            &_waveform
        );
    }

    if (_thumbCachePercent > 0)
    {
        float regionSize = (_item->view()->pixelsPerFrame() * _item->view()->zoom());
        float tcwidth = (width() * _thumbCachePercent) - 2;
        
        QColor tccol(40, 40, 100);
        if (_thumbCachePercent == 1)
        {
            tccol = QColor(40, 80, 40);
        }
        painter->setPen(tccol);
        painter->drawRect(QRectF(1, height() - 25, tcwidth, 2));
    }
}

// --------

GUI_TL2_MediaItem::GUI_TL2_MediaItem(GUI_TL2_Track* track, DS_NodePtr node, qlonglong startFrame, qlonglong frameCount):
    GUI_TL2_Item(track, node->graph()->uuid(), startFrame, frameCount),
    _node(node)
{
    setWidth(itemWidth());
    setHeight(track->height());
    setZValue(3);

    _bg = new GUI_TL2_MediaItemBackground(this);
    _fg = new GUI_TL2_MediaItemForeground(this);

    connect(
        node->graph(),
        SIGNAL(dataReady(QVariantMap)),
        this,
        SLOT(onDataReady(QVariantMap))
    );

    connect(track, &GUI_TL2_Track::sizeChanged, this, &GUI_TL2_MediaItem::onParentSizeChanged);

    onParentSizeChanged(itemWidth(), track->height());
}


void
GUI_TL2_MediaItem::clear()
{
    // RLP_LOG_DEBUG("");

    _track->view()->scene()->removeItem(_fg);
    _track->view()->scene()->removeItem(_bg);
    
}


void
GUI_TL2_MediaItem::updateItem()
{
    RLP_LOG_DEBUG("")

    _fg->update();
    _bg->update();
    update();
}


GUI_TL2_MediaTrack*
GUI_TL2_MediaItem::track()
{
    return dynamic_cast<GUI_TL2_MediaTrack*>(_track);
}



void
GUI_TL2_MediaItem::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << nheight)

    setHeight(_track->height());
    refresh();
}


QString
GUI_TL2_MediaItem::label()
{
    return _node->getPropertiesPtr()->value("media.name").toString();
}


void
GUI_TL2_MediaItem::refresh()
{
    GUI_TL2_Item::refresh();

    float tvwidth = _track->view()->width();
    float w = width();

    _bg->setWidth(w);
    _bg->setHeight(height());

    float fgWidth = w;
    if (w > tvwidth)
    {
        fgWidth = tvwidth;
    }
    _fg->setWidth(fgWidth); // width());
    _fg->setHeight(height());

}


QRectF
GUI_TL2_MediaItem::boundingRect() const
{
    // RLP_LOG_DEBUG("")
    return QRectF(0, 0, width(), height());
}


QRectF
GUI_TL2_MediaItem::itemRect()
{
    return boundingRect();

    /*
    QRectF brect = boundingRect();
    QRectF tr = _track->view()->timelineClipRect();

    QRectF ti = brect.intersected(tr);
    ti.setHeight(brect.height());

    return ti;
    */

    /*
    QRegion r(brect.x(), brect.y(), brect.width(), brect.height());
    QRegion t = _track->view()->timelineClipRegion();
    QRegion i = r.intersected(t);

    return QRegion(i.x(), i.y(), i.width(), brect.height());
    */
    // i.setHeight(brect.height());

    // RLP_LOG_DEBUG(r << " --- " << t)
    // RLP_LOG_DEBUG(i)

    // return i;
}


void
GUI_TL2_MediaItem::setYPos(int ypos)
{
    // RLP_LOG_DEBUG(ypos)

    setPos(0, ypos);
    _bg->setPos(0, ypos);
    _fg->setPos(0, ypos);
}


void
GUI_TL2_MediaItem::onDataReady(QVariantMap data)
{
    QString dataType = data.value("data_type").toString();

    // RLP_LOG_DEBUG(dataType)

    if (dataType == "audio_waveform")
    {
        QString waveformPath = data.value("waveform.path").toString();
        _bg->loadWaveform(waveformPath);
    }

    else if (dataType == "thumbnail")
    {
        QString thumbnailPath = data.value("thumbnail.path").toString();
        _fg->loadThumbnailFromPath(thumbnailPath);
    }

    else if (dataType == "thumbnail_cache")
    {
        qlonglong numSoFar = data.value("thumbnail_cache.count").toLongLong();
        qlonglong total = data.value("thumbnail_cache.total").toLongLong();
        float tcpercent = (float)numSoFar / (float)total;

        _bg->setThumbCachePercent(tcpercent);
        _bg->update();

        // int complete = rint(((float)numSoFar / (float)total) * 100);

        // QString text = QString("Thumbnail Cache: %1% complete").arg(complete);

        // qInfo() << text;
        // QVariantList args;
        // args.append(text);

        // PY_Interp::call("rlplug_qtbuiltin.cmds.display_message", args);
    }

    else if (dataType == "media_input")
    {

        QVariantList args;
        QVariantMap extra;
        extra.insert("media_node", data.value("node"));

        QVariantMap kwargs;
        kwargs.insert("extra", extra);

        args.append(data.value("frame_list"));
        // args.append(extra);

        // TODO FIXME: We don't need to serialize the thumbnail to disk, can just
        // return a QImage
        QVariant result = PY_Interp::call(
            "revlens.gui.timeline._gen_thumbnail_with_previewmaker", args, kwargs);
        QString thumbPath = result.toString();
        _fg->loadThumbnailFromPath(thumbPath, false);
        _fg->update();
    }

    else if (dataType == "video_source")
    {
        _fg->generateThumbnail(/*append=*/false);
    }

    update();
}




void
GUI_TL2_MediaItem::mousePressEvent(QMouseEvent* event)
{
    _isSelected = true;
    _bg->update();

    QVariantMap info;
    QVariant nodeWrapper;
    nodeWrapper.setValue(_node);
    info.insert("item_type", "Media");
    info.insert("node", nodeWrapper);
    info.insert("media_uuid", _uuid.toString());

    emit itemSelected(info);
}


void
GUI_TL2_MediaItem::mouseReleaseEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")
}


void
GUI_TL2_MediaItem::paint(GUI_Painter* painter)
{
    // painter->setBrush(QBrush(Qt::red));
    // painter->drawRect(boundingRect());
}