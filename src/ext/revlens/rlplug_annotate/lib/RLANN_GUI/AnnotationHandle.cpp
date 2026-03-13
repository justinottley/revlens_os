
#include "RlpExtrevlens/RLANN_GUI/AnnotationHandle.h"

#include "RlpGui/BASE/Scene.h"

#include "RlpExtrevlens/RLANN_DS/Track.h"

#include "RlpExtrevlens/RLANN_GUI/AnnotationItem.h"
#include "RlpExtrevlens/RLANN_GUI/Track.h"

RLP_SETUP_EXT_LOGGER(RLANN_GUI, AnnotationHandle)

RLANN_GUI_AnnotationHandle::RLANN_GUI_AnnotationHandle(HandleSide side, RLANN_GUI_AnnotationItem* item):
    GUI_ItemBase(item),
    _side(side),
    _item(item),
    _colHover(QColor(128, 94, 51, 255)),
    _colNormal(QColor(64, 47, 26, 255)),
    _inHandle(false)
{
    setZValue(4);
    setAcceptHoverEvents(true);

    setWidth(item->itemWidth() / 2);
    setHeight(item->track()->height());

    if (side == HNDL_RIGHT)
    {
        setPos(item->itemWidth() / 2, 0);
    }
}


GUI_TL2_View*
RLANN_GUI_AnnotationHandle::view()
{
    return _item->track()->view();
}


RLANN_GUI_Track*
RLANN_GUI_AnnotationHandle::track()
{
    return _item->track();
}


void
RLANN_GUI_AnnotationHandle::mousePressEvent(QMouseEvent* event)
{
    event->accept();

    _inDrag = true;
    QPointF p = mapToItem(view(), event->position());
    _pressPos = QPointF(
        p.x(),
        p.y()
    );

    _inHandle = true;

    // RLP_LOG_DEBUG(_side << event->position())
}


void
RLANN_GUI_AnnotationHandle::mouseMoveEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG(_side << event->position())

    if (!_inDrag) {
        return;
    }

    event->accept();
    QPointF tlPos = mapToItem(view(), event->position());
    qreal xpos = tlPos.x();
    qlonglong frame = view()->getFrameAtXPos(xpos - view()->timelineXPosStart());

    qlonglong startFrame = _item->startFrame();
    qlonglong endFrame = _item->endFrame();

    if (xpos > _pressPos.x()) // Dragging right
    { 

        if ((_side == HNDL_RIGHT) &&
            (frame >= endFrame) &&
            (!track()->hasAnnotationItem(frame)))
        {

            for (qlonglong fx = endFrame - 1; fx != frame; ++fx)
            {

                // RLP_LOG_DEBUG("HOLD: " << endFrame - 1 << " -> " << fx + 1);

                track()->holdFrame(endFrame - 1, fx + 1);
                track()->drawController()->holdFrame(
                    track()->view()->uuid(),
                    track()->uuid().toString(),
                    endFrame - 1,
                    fx + 1);
            }

            // RLP_LOG_DEBUG("SET ENDFRAME: " << frame + 1)

            _pressPos = tlPos;
            _item->setEndFrame(frame + 1);

        } else if ((_side == HNDL_LEFT) && 
                   (frame > startFrame) && 
                   (frame < endFrame) &&
                   (track()->hasAnnotationItem(frame)))
        {

            for (qlonglong fx = startFrame; fx != frame; ++fx)
            {

                // RLP_LOG_DEBUG("RELEASE: " << fx);

                track()->releaseFrame(fx);
                track()->drawController()->releaseFrame(
                    track()->view()->uuid(),
                    track()->uuid().toString(),
                    fx
                );
            }
            
            // RLP_LOG_DEBUG("SET STARTFRAME: " << frame);

            _pressPos = tlPos;
            _item->setStartFrame(frame);
        }

    } else
    {
        // RLP_LOG_DEBUG("LEFT:" << xpos << " frame: " << frame << "start:" << startFrame << "end:" << endFrame << "HAI:" << track()->hasAnnotationItem(frame))

        if ((_side == HNDL_LEFT) &&
            (frame < startFrame) &&
            (!track()->hasAnnotationItem(frame)))
        {

            for (qlonglong fx = startFrame; fx != frame; --fx)
            {

                // RLP_LOG_DEBUG("HOLD: " << _item->startFrame() << " -> " << fx - 1);

                track()->holdFrame(startFrame, fx - 1);
                track()->drawController()->holdFrame(
                    track()->view()->uuid(),
                    track()->uuid().toString(),
                    startFrame,
                    fx - 1
                );
                
            }

            // RLP_LOG_DEBUG("SET STARTFRAME: " << frame);

            _pressPos = tlPos;
            _item->setStartFrame(frame);

        } else if ((_side == HNDL_RIGHT) &&
                   ((frame + 1) < endFrame) &&
                   (frame >= startFrame) &&
                   (track()->hasAnnotationItem(frame + 1)))
        {

            for (qlonglong fx = (frame + 1); fx != endFrame; ++fx)
            {

                // RLP_LOG_DEBUG("RELEASE: " << fx);

                track()->releaseFrame(fx);
                track()->drawController()->releaseFrame(
                    track()->view()->uuid(),
                    track()->uuid().toString(),
                    fx
                );
            }


            _pressPos = tlPos;
            _item->setEndFrame(frame + 1);

        }

    }

    _item->update();
}


void
RLANN_GUI_AnnotationHandle::mouseReleaseEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("");

    track()->strack()->setAnnotationFrameRange(_item->annotation());

    event->accept();

    _inDrag = false;
    _inHandle = false;
}


void
RLANN_GUI_AnnotationHandle::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("");

    _inHandle = true;
    event->setAccepted(false);

    update();
}


void
RLANN_GUI_AnnotationHandle::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("");
    
    _inHandle = false;
    event->setAccepted(false);

    update();
}


QRectF
RLANN_GUI_AnnotationHandle::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
RLANN_GUI_AnnotationHandle::paint(GUI_Painter* painter)
{

    if (_inHandle)
    {
        // painter->setPen(QPen(_colHover));
        painter->setPen(QPen(_colNormal));
        painter->setBrush(QBrush(_colHover));
        painter->setOpacity(0.4);
        painter->drawRoundedRect(QRectF(3, 4, width() - 6, height() - 6), 4, 8);

    } else
    {
        // painter->setPen(QPen(_colNormal));
        // painter->setBrush(QBrush(_colNormal));
    }
}