
#include "RlpRevlens/GUI_TL2/Item.h"
#include "RlpRevlens/GUI_TL2/Track.h"
#include "RlpRevlens/GUI_TL2/View.h"

RLP_SETUP_LOGGER(revlens, GUI_TL2, Item)

GUI_TL2_Item::GUI_TL2_Item(GUI_TL2_Track* track, QUuid uuid, qlonglong startFrame, qlonglong frameCount):
    GUI_ItemBase(track->timelineArea()),
    _track(track),
    _uuid(uuid),
    _startFrame(startFrame),
    _endFrame(startFrame + frameCount),
    _isSelected(false)
{
}


GUI_TL2_View*
GUI_TL2_Item::view()
{
    return _track->view();
}


qlonglong
GUI_TL2_Item::frameCount()
{
    return _endFrame - _startFrame;
}


void
GUI_TL2_Item::setEndFrame(qlonglong endFrame)
{
    // RLP_LOG_DEBUG(endFrame);

    _endFrame = endFrame;

    refresh();
}



void
GUI_TL2_Item::setFrameCount(qlonglong frameCount)
{
    _endFrame = _startFrame + frameCount;
}



void
GUI_TL2_Item::setStartFrame(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum);

    _startFrame = frameNum;

    refresh();
}


qreal
GUI_TL2_Item::itemWidth() const
{
    qreal result = _track->view()->getXPosAtFrame(_endFrame) - _track->view()->getXPosAtFrame(_startFrame);
    // RLP_LOG_DEBUG(result)

    if ((result > _track->view()->timelineWidth()) && (_track->view()->isFocusedOnFrame()))
    {
        // RLP_LOG_WARN(result << " > " << _track->view()->timelineWidth())
        result = _track->view()->timelineWidth();
    }
    return result;
}


void
GUI_TL2_Item::refresh()
{
    // RLP_LOG_DEBUG("")

    setWidth(itemWidth());
    // setPos(0, position().y());

    qreal xpos = _track->view()->getXPosAtFrame(_startFrame);
    if ((xpos < 0) && _track->view()->isFocusedOnFrame())
    {
        xpos = 0;
    }
    setPos(xpos, position().y());

    // RLP_LOG_DEBUG(pos())
}