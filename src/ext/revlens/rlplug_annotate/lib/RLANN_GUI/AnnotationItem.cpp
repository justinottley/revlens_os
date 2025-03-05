

#include "RlpExtrevlens/RLANN_GUI/AnnotationItem.h"

#include "RlpExtrevlens/RLANN_GUI/Track.h"

#include "RlpGui/BASE/Style.h"

RLP_SETUP_EXT_LOGGER(RLANN_GUI, AnnotationItem)

RLANN_GUI_AnnotationItem::RLANN_GUI_AnnotationItem(
        GUI_TL2_Track* track,
        RLANN_DS_AnnotationPtr ann,
        qlonglong startFrame):
    GUI_TL2_Item(track, ann->uuid(), startFrame, 1),
    _annotation(ann),
    _mouseMode(MM_SCALE),
    _colPen(QColor(80, 80, 80))
    // _colPen(QColor(73, 52, 28, 255))
{
    setZValue(3);

    connect(
        ann.get(),
        &RLANN_DS_Annotation::strokeEnd,
        this,
        &RLANN_GUI_AnnotationItem::onStrokeEnd
    );

    _handleLeft = new RLANN_GUI_AnnotationHandle(RLANN_GUI_AnnotationHandle::HNDL_LEFT, this);
    _handleRight = new RLANN_GUI_AnnotationHandle(RLANN_GUI_AnnotationHandle::HNDL_RIGHT, this);

    setWidth(itemWidth());
    setHeight(track->height());
    setZValue(3);

    onParentSizeChanged(itemWidth(), track->height());

    RLP_LOG_DEBUG("")
}


RLANN_GUI_Track*
RLANN_GUI_AnnotationItem::track()
{
    return dynamic_cast<RLANN_GUI_Track*>(_track);
}



void
RLANN_GUI_AnnotationItem::clear()
{
    RLP_LOG_DEBUG("");

    _track->view()->scene()->removeItem(_handleLeft);
    _track->view()->scene()->removeItem(_handleRight);
}


void
RLANN_GUI_AnnotationItem::setYPos(int ypos)
{
    GUI_TL2_Item::setYPos(ypos + 2);

    _handleLeft->setYPos(ypos + 2);
    _handleRight->setYPos(ypos + 2);
}


void
RLANN_GUI_AnnotationItem::refresh()
{
    // RLP_LOG_DEBUG("")

    // GUI_TL2_Item::refresh();

    setWidth(itemWidth());
    setPos(_track->view()->getXPosAtFrame(_startFrame), position().y());

    _handleLeft->setWidth(width() / 2);
    _handleRight->setWidth(width() / 2);
    _handleRight->setPos(width() / 2, 0);
}


void
RLANN_GUI_AnnotationItem::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << nheight)

    setWidth(itemWidth());
    setHeight(_track->height());

    // RLP_LOG_DEBUG("track height:" << _track->height())

    refresh();
}


void
RLANN_GUI_AnnotationItem::onStrokeEnd()
{
    RLP_LOG_DEBUG("")

    _thumb = _annotation->getQImage()->scaledToHeight(40, Qt::SmoothTransformation);
    _thumb.rgbSwap();

    update();
}


QRectF
RLANN_GUI_AnnotationItem::boundingRect() const
{
   return QRectF(0, 0, width(), height());
}


void
RLANN_GUI_AnnotationItem::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")

    painter->setPen(QColor(60, 60, 60));
    painter->setBrush(QBrush(QColor(60, 60, 60)));
    painter->drawRoundedRect(QRectF(3, 4, width() - 6, height() - 6), 4, 8);

    if ((width() - 20) > _thumb.width())
    {
        float midx = (float)(width() - _thumb.width()) / 2.0;
        float midy = (float)(height() - _thumb.height()) / 2.0;

        // RLP_LOG_DEBUG(midx << midy)
        painter->drawImage(midx - 1, midy, &_thumb);
    }
}