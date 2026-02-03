

#include "RlpExtrevlens/RLANN_CNTRL/DisplayPlugin.h"

#include "RlpExtrevlens/RLANN_CNTRL/DrawController.h"

RLP_SETUP_LOGGER(extrevlens, RLANN_CNTRL, DisplayPlugin)

RLANN_CNTRL_DisplayPlugin::RLANN_CNTRL_DisplayPlugin(RLANN_CNTRL_DrawController* annCntrl):
    DS_EventPlugin("RlpAnnotationDisplayMedia", true),
    _annCntrl(annCntrl),
    _drawing(false),
    _windowWidth(0),
    _windowHeight(0),
    _displayZoom(1.0),
    _relZoom(0.0)
{
    _username = UTIL_AppGlobals::instance()->globalsPtr()->value("username").toString();
}


RLANN_CNTRL_DisplayPlugin::~RLANN_CNTRL_DisplayPlugin()
{
    RLP_LOG_DEBUG("")
}


bool
RLANN_CNTRL_DisplayPlugin::setDisplay(DISP_GLView* display)
{
    // RLP_LOG_DEBUG(display)

    DS_EventPlugin::setDisplay(display);

    // _displayZoom = _display->getDisplayZoom() + (1.0 * _display->getRelZoom());

    connect(
        _display,
        SIGNAL(displayZoomEnd(float)),
        this,
        SLOT(onDisplayZoomEnd(float))
    );

    connect(
        _display,
        &DISP_GLView::hoverEnter,
        this,
        &RLANN_CNTRL_DisplayPlugin::onDisplayHoverEnter
    );


    return true;
}



void
RLANN_CNTRL_DisplayPlugin::strokeTo(RLANN_DS_Track* track, qlonglong frame, const QPointF &endPoint, QVariantMap mdata)
{
    // RLP_LOG_DEBUG(frame << endPoint.x() << endPoint.y());

    if (!track->hasAnnotation(frame))
    {
        RLP_LOG_ERROR("NO CURRENT ANNOTATION ON FRAME " << frame);
        return;
    }

    RLANN_DS_AnnotationPtr ann = track->getAnnotation(frame); // getAnnotationMap()->value(_currFrame);

    // track->brush()->strokeTo(&painter, endPoint, mdata);
    track->brush()->strokeTo(ann, endPoint, mdata);

    // Update all Views
    _annCntrl->getMainController()->getVideoManager()->update();

}


bool
RLANN_CNTRL_DisplayPlugin::hoverMoveEventInternal(QHoverEvent* event)
{
    // iOS pen hover not sending event?
    #ifdef SCAFFOLD_IOS
    return false;
    #endif

    // RLP_LOG_DEBUG(event->position())

    qlonglong currFrame = _annCntrl->currFrame();
    DS_TrackPtr atrack = _annCntrl->activeTrack();
    if (atrack == nullptr)
    {
        // RLP_LOG_WARN("no active track")
        return false;
    }

    RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(atrack.get());

    if (anntrack->brush() == nullptr)
    {
        RLP_LOG_WARN("No brush on current annotation track")
        return false;
    }

    RLANN_DS_AnnotationPtr ann = anntrack->getAnnotation(currFrame);
    if (ann == nullptr)
    {
        // RLP_LOG_WARN("no annotation")
        return false;
    }


    // ann->setValid();
    // ann->setDirty();

    QPointF panPercent = _display->getPanPercent();
    
    QPointF transformedPoint = ann->mapToImage(
        _display->uuid(),
        event->position(),
        panPercent
    );


    QVariantMap extra;
    QPointF epos = event->position();
    extra.insert("event.pos", epos);
    extra.insert("event.gpos", event->globalPosition());

    anntrack->brush()->strokeMove(transformedPoint, extra);

    // RLP_LOG_DEBUG(transformedPoint << ann->getWidth() << ann->getHeight())

    QVariantMap kwargs;
    kwargs.insert("x", transformedPoint.x());
    kwargs.insert("y", transformedPoint.y());
    kwargs.insert("w", ann->getWidth());
    kwargs.insert("h", ann->getHeight());
    kwargs.insert("user", _username);


    // RLP_LOG_DEBUG("emit hoverMove for sync" << kwargs)
    _controller->emitSyncAction("hoverMove", kwargs);

    return false;
}


bool
RLANN_CNTRL_DisplayPlugin::strokeBegin(QPointF pos, QPointF globalpos)
{
    qlonglong currFrame = _annCntrl->currFrame();

    // RLP_LOG_DEBUG(currFrame << pos << globalpos)

    // NOTE: hardcode disable plugin for now
    // return true;

    RLANN_DS_Track* atrack = dynamic_cast<RLANN_DS_Track*>(_annCntrl->activeTrack().get());
    if (!atrack)
    {
        RLP_LOG_WARN("No active track!")
        return false;
    }

    QVariantMap settings = atrack->brush()->settings();
    _annCntrl->emitBrushInfoChanged(settings);

    if ((atrack != nullptr) &&
        (atrack->isEditable()) && 
        (atrack->hasAnnotation(currFrame)))
    {

        RLANN_DS_AnnotationPtr ann = atrack->getAnnotation(currFrame);
        ann->setValid();
        ann->setDirty();

        QPointF panPercent = _display->getPanPercent();

        QPointF lastPoint = ann->mapToImage(
            _display->uuid(),
            pos,
            panPercent
        );

        float imgSizeRatio = ann->sizeRatio(_display->uuid());

        RLP_LOG_DEBUG(pos << imgSizeRatio)
        
        QVariantMap mdata;
        mdata.insert("event.pos", pos); // event->pos());
        mdata.insert("event.gpos", globalpos); // event->globalPosition());
        mdata.insert("image.ratio", imgSizeRatio);
        mdata.insert("display.uuid", _display->uuid());
        mdata.insert("display.pan", panPercent);

        _drawing = true;
        atrack->brush()->setupFrame(ann);
        atrack->brush()->strokeBegin(lastPoint, mdata);

        _annCntrl->stopUpdateTimer();
        _annCntrl->emitSyncDrawStart(currFrame, atrack->uuid().toString());


        QVariantMap kwargs;
        kwargs.insert("ann_uuid", ann->uuid());
        kwargs.insert("frame", currFrame);
        kwargs.insert("x", lastPoint.x());
        kwargs.insert("y", lastPoint.y());
        kwargs.insert("track_uuid", atrack->uuid().toString());
        kwargs.insert("brush", atrack->brush()->settings());

        _controller->emitSyncAction("drawStart", kwargs);

        // RLP_LOG_DEBUG("LAST POS: " << lastPoint.x() << " " << lastPoint.y());

        return true;
    } else
    {
        RLP_LOG_DEBUG("?no annotation on" << currFrame)
        if (!atrack->isEditable())
        {
            RLP_LOG_ERROR("TRACK NOT EDITABLE")
        }
    }

    return false;
}


void
RLANN_CNTRL_DisplayPlugin::strokeMove(QPointF pos, QPointF globalpos, QVariantMap mdataIn)
{
    // RLP_LOG_DEBUG(pos << mdataIn)

    qlonglong currFrame = _annCntrl->currFrame();

    RLANN_DS_Track* track = dynamic_cast<RLANN_DS_Track *>(_annCntrl->activeTrack().get());
    RLANN_DS_AnnotationPtr ann = track->getAnnotation(currFrame);
    
    QPointF panPercent = _display->getPanPercent();
    float displayZoom = _display->getZoom();

    // RLP_LOG_DEBUG(pp.x() << " " << pp.y());
    
    // localPos() is correct (instead of event->pos()) if event is QMouseEvent
    //
    QPointF imgPoint = ann->mapToImage(
        _display->uuid(),
        pos,
        panPercent
    );

    // RLP_LOG_DEBUG(event->pos() << " " << imgPoint << " " << displayZoom << " pan: " << panPercent)

    QVariantMap mdata;
    mdata.insert(mdataIn);
    mdata.insert("event.pos", pos); // event->pos());
    mdata.insert("event.gpos", globalpos); // event->globalPosition());

    strokeTo(track, currFrame, imgPoint, mdata);

    QVariantMap kwargs;
    
    kwargs.insert("frame", (qlonglong)currFrame);
    kwargs.insert("x", imgPoint.x());
    kwargs.insert("y", imgPoint.y());
    kwargs.insert("w", ann->getWidth());
    kwargs.insert("h", ann->getHeight());
    kwargs.insert("user", _username);
    kwargs.insert("track_uuid", track->uuid().toString());

    if (mdataIn.contains("pressure"))
    {
        kwargs.insert("p", mdataIn.value("pressure"));
        kwargs.insert("xt", mdataIn.value("xtilt"));
        kwargs.insert("yt", mdataIn.value("ytilt"));
    }

    _controller->emitSyncAction("draw", kwargs);

}


void
RLANN_CNTRL_DisplayPlugin::strokeEnd(QPointF pos, QPointF globalpos)
{
    // RLP_LOG_DEBUG("")

    _drawing = false;

    DS_TrackPtr atrack = _annCntrl->activeTrack();
    if (atrack == nullptr)
    {
        RLP_LOG_ERROR("no active track!");
        return;
    }

    RLANN_DS_Track* track = dynamic_cast<RLANN_DS_Track *>(atrack.get());

    if (!track->isEditable())
    {
        track->brush()->strokeEnd(nullptr);
        RLP_LOG_ERROR("Track not editable, skipping draw")
        // return false;
    }

    qlonglong currFrame = _annCntrl->currFrame();
    RLANN_DS_AnnotationPtr ann = track->getAnnotation(currFrame);

    QVariantMap mdata;
    mdata.insert("event.pos", pos);
    mdata.insert("event.gpos", globalpos);

    track->brush()->strokeEnd(ann, mdata);

    if (ann != nullptr)
    {

        // QString annImageFilePath = track->getCacheFilePath(ann->fileName());
        // ann->save(annImageFilePath);
        // QString annCacheImagePath = track->getAnnotationCacheFilePathQString("_currFrame);
        // ann->getQImage()->save(annCacheImagePath);

        // RLP_LOG_DEBUG("RLANN_CNTRL_DrawController::mouseReleaseEventInternal(): SAVED: " << annImageFilePath.toStdString().c_str());
        ann->emitStrokeEnd();

        QVariantMap kwargs;
        kwargs.insert("frame", (qlonglong)currFrame);
        kwargs.insert("ann_uuid", ann->uuid().toString());
        kwargs.insert("track_uuid", track->uuid().toString());

        _controller->emitSyncAction("drawEnd", kwargs);

        _annCntrl->registerAnnotationForSync(track->uuid().toString(), ann->uuid().toString());
    }
    

    _annCntrl->emitSyncDrawEnd(currFrame, track->uuid().toString());
    
}


bool
RLANN_CNTRL_DisplayPlugin::mousePressEventInternal(QMouseEvent* event)
{
    // return false;
    // RLP_LOG_DEBUG(_name)

    // if (_annCntrl->isPicking())
    // {
    //     RLANN_DS_Track* atrack = dynamic_cast<RLANN_DS_Track*>(_annCntrl->activeTrack().get());
    //     QVariantMap settings = atrack->brush()->settings();
    //     _annCntrl->emitBrushInfo(settings);

    //     return false;
    // }

    CmdFilterResult res = _controller->checkCommand("annotate");
    if (!res.first)
    {
        RLP_LOG_ERROR("Permission denied - Draw")
        return false;
    }

    // Ignore if any key modifiers are pressed
    //
    if (!event->modifiers().testFlag(Qt::NoModifier))
    {
        return false;
    }

    if (event->button() != Qt::LeftButton)
    {
        return false;
    }


    return strokeBegin(event->position(), event->globalPosition());
}


bool
RLANN_CNTRL_DisplayPlugin::mouseMoveEventInternal(QMouseEvent* event)
{
    // return false;
    // RLP_LOG_DEBUG(_name)

    CmdFilterResult res = _controller->checkCommand("annotate");
    if (!res.first)
    {
        RLP_LOG_ERROR("annotate permission denied")
        return false;
    }

    // Ignore if any key modifiers are pressed
    //
    if (!event->modifiers().testFlag(Qt::NoModifier))
    {
        return false;
    }


    if ((event->buttons() & Qt::LeftButton) && _drawing)
    {

        // RLP_LOG_DEBUG("RLANN_CNTRL_DrawController::mouseMoveEventInternal(): [SYNC ACTION] " << event->pos().x() << " " << event->pos().y());
        strokeMove(event->position(), event->globalPosition());
    }

    return true;
}


bool
RLANN_CNTRL_DisplayPlugin::mouseReleaseEventInternal(QMouseEvent* event)
{
    // return false;
    // RLP_LOG_DEBUG("")

    CmdFilterResult res = _controller->checkCommand("annotate");
    if (!res.first)
    {
        RLP_LOG_ERROR("annotate not available - permission denied")
        return false;
    }

    // if (_annCntrl->isPicking())
    // {
    //     DS_TrackPtr atrack = _annCntrl->activeTrack();
    //     if (atrack == nullptr) {
    //         RLP_LOG_ERROR("no active track!");
    //         return false;
    //     }

    //     RLANN_DS_Track* track = dynamic_cast<RLANN_DS_Track *>(atrack.get());

    //     track->brush()->strokeEnd();
    //     return false;

    // }


    // Ignore if any key modifiers are pressed
    //
    if (!event->modifiers().testFlag(Qt::NoModifier))
    {
        return false;
    }

    // RLP_LOG_DEBUG("");

    strokeEnd(event->position(), event->globalPosition());

    return true;
}


bool
RLANN_CNTRL_DisplayPlugin::tabletEventInternal(QTabletEvent* event)
{
    // RLP_LOG_DEBUG(event)
    // RLP_LOG_DEBUG(event->position() << "pressure:" << event->pressure() << "tilt:" << event->xTilt() << event->yTilt())

    // RLP_LOG_DEBUG(event->position() << event->globalPosition())
    QPointF epos = event->position();
    QPointF dpos = _display->mapFromGlobal(epos);

    float pressure = event->pressure();
    int xtilt = event->xTilt();
    int ytilt = event->yTilt();

    CmdFilterResult res = _controller->checkCommand("annotate");
    if (!res.first)
    {
        RLP_LOG_ERROR("Permission denied - Draw")
        return false;
    }

    int xOffset = 3;
    int yOffset = 0;

    QPointF pos(
        dpos.x() - xOffset, // not sure why, determined empiricaly
        epos.y() - yOffset // not sure why, determined empiricaly
    );

    QPointF gpos(
        event->globalPosition().x() - xOffset,
        event->globalPosition().y() - yOffset
    );

    if (pressure == 0)
    {
        // RLP_LOG_DEBUG("pen release")
        strokeEnd(pos, gpos);
        return true;
    }

    if (!_drawing)
    {
        return strokeBegin(pos, gpos);
    } else
    {
        QVariantMap mdata;
        mdata.insert("pressure", pressure);
        mdata.insert("xtilt", xtilt);
        mdata.insert("ytilt", ytilt);

        strokeMove(pos, gpos, mdata);
    }


    return true;
}


void
RLANN_CNTRL_DisplayPlugin::updateAnnDisplayWindow(RLANN_DS_AnnotationPtr ann)
{
    // RLP_LOG_DEBUG(_windowWidth << _windowHeight)

    ann->setWindow(
        _display->uuid(),
        _windowWidth,
        _windowHeight,
        _display->pos().y(),
        _displayZoom,
        _relZoom
    );

}


QPointF
RLANN_CNTRL_DisplayPlugin::mapToDisplay(MousePos& annPos)
{
    // Adjust for zoom
    //
    float nLeft = (float)(_windowWidth - (annPos.w * _zoom)) / 2.0;
    float nTop = (float)(_windowHeight - (annPos.h * _zoom)) / 2.0;

    // RLP_LOG_DEBUG(_zoom << _windowWidth << _windowHeight << "left/top:" << nLeft << nTop)

    float xp = nLeft + (annPos.x * _zoom);
    float yp = nTop + (annPos.y * _zoom);

    // Adjust for pan
    //
    QPointF panPercent = _display->getPanPercent();

    xp += (_windowWidth * (panPercent.x() / 2.0));
    yp += (_windowHeight * (panPercent.y() / -2.0));

    QPointF result = QPointF(xp, yp);

    return result;
}


void
RLANN_CNTRL_DisplayPlugin::onDisplayZoomEnd(float displayZoom)
{
    // RLP_LOG_DEBUG(displayZoom << "current zoom:" << _display->getZoom())

    // For some reason, we may get a delayed sig-slot connection delivered here
    // for a zoom value that has since been outdated (maybe because of a destructed view or a resized view)
    // use the zoom value that is currently on the display, and not the incoming value
    _displayZoom = _display->getDisplayZoom(); // + (1.0 * _display->getRelZoom());
    _relZoom = _display->getRelZoom();

    _zoom = _displayZoom * (1.0 + _relZoom);

    resizeGLInternal(_windowWidth, _windowHeight);
}


void
RLANN_CNTRL_DisplayPlugin::onDisplayHoverEnter()
{
    // RLP_LOG_DEBUG("")

    RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(_annCntrl->activeTrack().get());
    if (anntrack != nullptr)
    {
        anntrack->brush()->activate(_display, _annCntrl);
    }
}


void
RLANN_CNTRL_DisplayPlugin::resizeGLInternal(int width, int height)
{
    // RLP_LOG_DEBUG(width << " " << height << _display->pos())

    _windowWidth = width;
    _windowHeight = height;
    
    qlonglong currFrame = _annCntrl->currFrame();

    DS_TrackPtr track = _annCntrl->activeTrack();
    if (track != nullptr)
    {
        RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(track.get());
        if (anntrack->hasAnnotation(currFrame))
        {

            anntrack->getAnnotation(currFrame)->setWindow(
                _display->uuid(),
                width,
                height,
                _display->pos().y(),
                _displayZoom,
                _relZoom
            );

        }
    }
}


void
RLANN_CNTRL_DisplayPlugin::paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata)
{
    DS_TrackPtr track = _annCntrl->activeTrack();
    if (track != nullptr)
    {
        RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(track.get());
        RLANN_DS_BrushBase* brush = anntrack->brush();
        if (brush != nullptr)
        {
            brush->paintGLPostDraw(painter, mdata);
        } else
        {
            RLP_LOG_ERROR("INVALID BRUSH ON TRACK" << anntrack)
        }
        
    }

    if (_annCntrl->property("pref.show_remote_cursors").toBool())
    {
        StringMousePosMap* spm = _annCntrl->userPositionMap();
        StringMousePosMap::iterator it;

        for (it = spm->begin(); it != spm->end(); ++it)
        {
            QString username = it.key();
            MousePos posp = it.value();

            QPointF dpos = mapToDisplay(posp);

            int xpos = rint(dpos.x());
            int ypos = rint(dpos.y());
            // RLP_LOG_DEBUG(dpos)

            painter->setPen(Qt::black);
            painter->drawCircle(xpos + 1, ypos + 1, 3);
            painter->drawText(xpos + 14, ypos + 14, username);
            painter->setPen(Qt::white);
            painter->drawCircle(xpos, ypos, 3);
            painter->drawText(xpos + 12, ypos + 12, username);
        }
    }
}
