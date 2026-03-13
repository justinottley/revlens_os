
#include "RlpRevlens/GUI_TL2/MediaItem.h"

#include "RlpCore/PY/Interp.h"

#include "RlpGui/BASE/Style.h"

#include "RlpRevlens/GUI_TL2/View.h"
#include "RlpRevlens/MEDIA/Plugin.h"

RLP_SETUP_LOGGER(revlens, GUI_TL2, MediaItemForeground)
RLP_SETUP_LOGGER(revlens, GUI_TL2, MediaItemBackground)
RLP_SETUP_LOGGER(revlens, GUI_TL2, MediaItem)
RLP_SETUP_LOGGER(revlens, GUI_TL2, MediaItemThumbnailGenTask)

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
GUI_TL2_MediaItemForeground::generateThumbnail(
    GUI_TL2_MediaItemThumbnailGenTask::ThumbnailGenMode mode,
    bool append)
{

    // RLP_LOG_DEBUG("mode:" << (int)mode << "append:" << append)

    QVariantMap* props = _item->node()->getPropertiesPtr();
    qlonglong midFrame = int(props->value("media.frame_count").toLongLong() / 2.0);

    QRunnable* thumbGen = new GUI_TL2_MediaItemThumbnailGenTask(
        _item,
        mode,
        midFrame
    );

    _item->view()->tpool()->start(thumbGen);

}


void
GUI_TL2_MediaItemForeground::initThumbnail()
{
    // RLP_LOG_DEBUG("")

    QVariantMap props = _item->node()->getProperties();

    if (props.contains("media.thumbnail.image"))
    {
        loadThumbnailFromImage();
    }
    else if (props.contains("media.thumbnail.data"))
    {
        QString thumbData = props.value("media.thumbnail.data").toString();
        if (thumbData.size() > 0)
        {
            generateThumbnail(GUI_TL2_MediaItemThumbnailGenTask::THUMBNAIL_GEN_DATA);
        }
    } else if (props.contains("media.thumbnail.path"))
    {
        generateThumbnail(GUI_TL2_MediaItemThumbnailGenTask::THUMBNAIL_GEN_PATH);
    } else 
    {
        generateThumbnail(GUI_TL2_MediaItemThumbnailGenTask::THUMBNAIL_GEN_NODE);
    }
}


void
GUI_TL2_MediaItemForeground::loadThumbnailFromImage()
{
    // RLP_LOG_DEBUG("")

    QVariantMap* props = _item->node()->getPropertiesPtr();
    if (props->contains("media.thumbnail.image"))
    {
        QImage i = props->value("media.thumbnail.image").value<QImage>();
        _thumbnails.clear();
        _thumbnails.push_back(i);

        update();
    }
}


QRectF
GUI_TL2_MediaItemForeground::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_TL2_MediaItemForeground::paint(GUI_Painter* painter)
{

    QRectF brect = boundingRect();

    // painter->setRenderHint(QPainter::Antialiasing, true);

    // painter->save();
    // painter->setClipRect(_item->itemRect(), Qt::IntersectClip);

    if ((_item->isItemEnabled()) && (_item->track()->isTrackEnabled()))
    {
        painter->setOpacity(1.0);
    } else
    {
        painter->setOpacity(0.2);
    }


    painter->setPen(QPen(QColor(200, 200, 200)));

    //int textPos = 20;
    int thumbnailOffset = 0;


    if ((_thumbnails.size() > 0) && (_item->track()->isThumbnailVisible()))
    {
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
    // RLP_LOG_DEBUG("")
    
    QVariantMap* props = _item->node()->getPropertiesPtr();
    if ((props->contains("audio.waveform.path")) && (_waveform.width() == 0))
    {
        loadWaveformByPath(props->value("audio.waveform.path").toString());
    }
}


void
GUI_TL2_MediaItemBackground::loadWaveformByPath(QString waveformPath)
{
    //  RLP_LOG_DEBUG(waveformPath);

    _waveform.load(waveformPath);

    update();
}


void
GUI_TL2_MediaItemBackground::loadWaveformByImage(QImage waveformImage)
{
    // RLP_LOG_DEBUG(waveformImage.width())

    _waveform.swap(waveformImage);
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

    // RLP_LOG_DEBUG(_item->isEnabled())

    if ((_item->isItemEnabled()) && (_item->track()->isTrackEnabled()))
    {
        painter->setOpacity(1.0);
    } else
    {
        painter->setOpacity(0.2);
    }

    QPen p(Qt::black);
    if (_inHover)
    {
        p.setColor(QColor(80, 80, 80));
        p.setWidth(2);
    }

    if (_item->isSelected())
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
        &DS_Graph::dataReady,
        this,
        &GUI_TL2_MediaItem::onMediaDataReady
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
    // RLP_LOG_DEBUG("")

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
GUI_TL2_MediaItem::onMediaDataReady(QString dataType, QVariantMap data)
{
    // RLP_LOG_DEBUG(dataType)

    if (dataType == "audio_waveform")
    {
        if (data.contains("audio.waveform.path"))
        {
            QString waveformPath = data.value("audio.waveform.path").toString();
            _bg->loadWaveformByPath(waveformPath);

        } else if (data.contains("audio.waveform.image"))
        {
            QImage waveformImage = data.value("audio.waveform.image").value<QImage>();
            _bg->loadWaveformByImage(waveformImage);
        }
        
    }

    else if (dataType == "thumbnail")
    {
        _fg->generateThumbnail(
            GUI_TL2_MediaItemThumbnailGenTask::THUMBNAIL_GEN_PATH,
            /*append=*/false
        );
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
        // QVariantList args;
        // QVariantMap extra;
        // extra.insert("media_node", data.value("node"));

        // QVariantMap kwargs;
        // kwargs.insert("extra", extra);

        // args.append(data.value("frame_list"));



        // args.append(extra);

        // TODO FIXME: We don't need to serialize the thumbnail to disk, can just
        // return a QImage
        // QVariant result = PY_Interp::call(
        //     "revlens.gui.timeline._gen_thumbnail_with_previewmaker", args, kwargs);
        // QString thumbPath = result.toString();
        // _fg->loadThumbnailFromPath(thumbPath, false);
        
        
        //_fg->update();
    }

    else if (dataType == "video_source")
    {
        _fg->generateThumbnail(
            GUI_TL2_MediaItemThumbnailGenTask::THUMBNAIL_GEN_NODE,
            /*append=*/false
        );
    }

    else if (dataType == "thumbnail_ready")
    {
        _fg->loadThumbnailFromImage();
    }
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


// ----------


GUI_TL2_MediaItemThumbnailGenTask::GUI_TL2_MediaItemThumbnailGenTask(
    GUI_TL2_MediaItem* item,
    ThumbnailGenMode mode,
    qlonglong frameNum,
    QString thumbInput
):
    _item(item),
    _mode(mode),
    _frameNum(frameNum),
    _thumbInput(thumbInput)
{
}


void
GUI_TL2_MediaItemThumbnailGenTask::loadFromNode()
{
    MEDIA_Plugin* plugin = _item->node()->getProperty<MEDIA_Plugin*>("video.plugin");
    QVariantMap frameInfo = _item->node()->getProperty<QVariantMap>("video.frame_info.one");
    long sessionFrame = _item->node()->getProperty<long>("session.frame");

    DS_FrameBufferPtr n = plugin->getOrCreateFrameBuffer(
        _item,
        frameInfo,
        /* force create new framebuffer */ true
    );
    n->getBufferMetadata()->insert("video.preview", true);
    n->reallocate();

    DS_TimePtr ftime = _item->node()->makeTime(_frameNum + sessionFrame);

    // postUpdate would probably be a reasonable data delivery mechanism here,
    // the MediaItem would need to implement event() and catch it though.
    // For now we will just add it to the node's properties and run update on the 
    // item
    _item->node()->readVideo(ftime, n, /*postUpdate=*/false); 

    QImage fimage = n->toQImage();
    QImage image = fimage.scaledToHeight(
        _item->heightThumbnail() - 2,
        Qt::SmoothTransformation
    );

    if (image.width() > 0)
    {
        // save base64 da into media for shipping to DB
        //
        QByteArray ba;
        QBuffer rbuf(&ba);
        rbuf.open(QIODevice::WriteOnly);
        image.save(&rbuf, "JPG", -1);
        rbuf.close();
        QString result = QString(ba.toBase64());


        QVariantMap nprops;

        nprops.insert("media.thumbnail.data", result);
        nprops.insert("media.thumbnail.image", image);
        nprops.insert("media.thumbnail.image_full", fimage);

        QVariantMap evtInfo;
        if (n->getBufferMetadata()->contains("image.loading"))
        {
            evtInfo.insert("image_type", 0); // IMAGE_TYPE_TEMP
        } else
        {
            evtInfo.insert("image_type", 1); // IMAGE_TYPE_FINAL
        }

        QVariantMap md;
        md.insert("evt_name", "thumbnail_ready");
        md.insert("evt_info", evtInfo);
        md.insert("props", nprops);

        _item->node()->emitDataReadyToGraph("update_properties", md);

    } else 
    {
        RLP_LOG_ERROR("Could not convert to image from video framebuffer")
    }
}


void
GUI_TL2_MediaItemThumbnailGenTask::loadFromData()
{
    QImage p;
    p.loadFromData(QByteArray::fromBase64(_thumbInput.toLatin1()));
    p = p.scaledToHeight(
        _item->heightThumbnail() - 2,
        Qt::SmoothTransformation
    );

    QVariantMap nprops;

    nprops.insert("media.thumbnail.data", _thumbInput);
    nprops.insert("media.thumbnail.image", p);
    
    QVariantMap md;
    md.insert("evt_name", "thumbnail_ready");
    md.insert("props", nprops);

    _item->node()->emitDataReadyToGraph("update_properties", md);
    
}


void
GUI_TL2_MediaItemThumbnailGenTask::loadFromPath()
{
    RLP_LOG_DEBUG(_thumbInput);

    QImage p(_thumbInput);
    p = p.scaledToHeight(
        _item->heightThumbnail() - 2,
        Qt::SmoothTransformation
    );

    // _thumbnails.push_back(p);

    // if (!primary)
    // {
    //     return;
    // }


    // save base64 da into media for shipping to DB
    //
    QByteArray ba;
    QBuffer rbuf(&ba);
    rbuf.open(QIODevice::WriteOnly);
    p.save(&rbuf, "JPG", -1);
    rbuf.close();
    QString result = QString(ba.toBase64());

    QVariantMap* props = _item->node()->getPropertiesPtr();

    props->insert("media.thumbnail.path", _thumbInput);
    props->insert("media.thumbnail.data", result);
    props->insert("media.thumbnail.image", p);


    // emit dataReady from the nodegraph so subsequent clients can see that
    // thumbnail data is now available
    //
    _item->node()->emitDataReadyToGraph("thumbnail_ready");
}


void
GUI_TL2_MediaItemThumbnailGenTask::run()
{
    // RLP_LOG_DEBUG(_mode)

    if (_mode == THUMBNAIL_GEN_NODE)
    {
        loadFromNode();
    }
    else if (_mode == THUMBNAIL_GEN_DATA)
    {
        loadFromData();
    }
    else if (_mode == THUMBNAIL_GEN_PATH)
    {
        loadFromPath();
    }
    else
    {
        RLP_LOG_ERROR("invalid thumbnail gen mode:" << (int)_mode)
    }
}
