
#include "RlpRevlens/GUI_TLT/MediaItem.h"
#include "RlpRevlens/GUI_TLT/View.h"

#include "RlpGui/BASE/Style.h"

RLP_SETUP_LOGGER(revlens, GUI_TLT, MediaItem)

GUI_TLT_MediaItem::GUI_TLT_MediaItem(GUI_TLT_View* view,
                                     QVariantMap mediaInfo,
                                     qlonglong startFrame,
                                     qlonglong frameCount,
                                     bool isOdd):
    GUI_ItemBase(view),
    _view(view),
    _mediaInfo(mediaInfo),
    _startFrame(startFrame),
    _frameCount(frameCount),
    _endFrame(startFrame + frameCount),
    _isEnabled(true),
    _isOdd(isOdd)
{
    setHeight(25);

    if (mediaInfo.contains("media.name"))
    {
        _name = mediaInfo.value("media.name").toString();
    }

    if (mediaInfo.contains("media.thumbnail.image"))
    {
        _thumbnail = mediaInfo.value("media.thumbnail.image").value<QImage>();
    }
}


qreal
GUI_TLT_MediaItem::itemWidth()
{
    return _view->getXPosAtFrame(_endFrame) - _view->getXPosAtFrame(_startFrame);
}


void
GUI_TLT_MediaItem::refresh()
{
    qreal iwidth = itemWidth();
    qreal xpos = _view->getXPosAtFrame(_startFrame);

    setWidth(iwidth);
    setPos(xpos, 3); // position().y());
}


void
GUI_TLT_MediaItem::paint(GUI_Painter* painter)
{
    QColor bgcol(40, 40, 50); //  = GUI_Style::TabBg;
    
    if (!_isEnabled)
    {
        bgcol = QColor(10, 10, 10);

    }
    else if (_isOdd)
    {
        bgcol = QColor(50, 50, 60);
    }

    QPen p(GUI_Style::BgMidGrey);
    p.setWidth(2);

    painter->setPen(p);
    painter->setBrush(QBrush(bgcol));

    painter->drawRoundedRect(boundingRect(), 3, 3);
    // painter->drawRect(boundingRect());
}