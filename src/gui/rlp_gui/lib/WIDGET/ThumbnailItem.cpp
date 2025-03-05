
#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/SvgIcon.h"

#include "RlpGui/WIDGET/ThumbnailItem.h"
#include "RlpGui/WIDGET/Label.h"


RLP_SETUP_LOGGER(gui, GUI, ThumbnailItem)

GUI_ThumbnailItem::GUI_ThumbnailItem(
    GUI_ItemBase* parent,
    QImage p,
    qreal width):
        GUI_ItemBase(parent),
        _srcThumb(p),
        _doOutline(true)
{
    setAcceptHoverEvents(true);

    _thumb = _srcThumb.scaledToWidth(
        width - 4,
        Qt::SmoothTransformation);

    setWidth(width);
    setHeight(_thumb.height());
}


GUI_ThumbnailItem::GUI_ThumbnailItem(
    GUI_ItemBase* parent):
        GUI_ItemBase(parent)
{

}


void
GUI_ThumbnailItem::hoverEnterEvent(QHoverEvent* event)
{
    _inHover = true;

    QTimer::singleShot(1000, this, &GUI_ThumbnailItem::checkForToolTip);

    QVariantMap edata = _data;
    edata.insert("_evt.pos", event->position());
    edata.insert("_evt.scene_pos", event->globalPosition());
    edata.insert("_item.pos", pos());
    edata.insert("_item.scene_pos", position());
    edata.insert("_item.width", width());
    edata.insert("_item.height", height());

    emit hoverEnter(edata);
}


void
GUI_ThumbnailItem::hoverMoveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _moveTime = QTime::currentTime();
    _movePos = event->globalPosition();
}


void
GUI_ThumbnailItem::hoverLeaveEvent(QHoverEvent* event)
{
    clearToolTip();
    _inHover = false;

    QVariantMap edata = _data;
    edata.insert("_evt.pos", event->position());
    edata.insert("_evt.scene_pos", event->globalPosition());

    emit hoverLeave(edata);
}


GUI_ThumbnailItem*
GUI_ThumbnailItem::fromBase64(GUI_ItemBase* parent, QString b64Input, int width)
{
    // RLP_LOG_DEBUG("")

    QImage p;
    p.loadFromData(QByteArray::fromBase64(b64Input.toLatin1()));

    return new GUI_ThumbnailItem(parent, p, width);

}


GUI_ThumbnailItem*
GUI_ThumbnailItem::fromPath(GUI_ItemBase* parent, QString filePath, int width)
{
    /*
    // GUI_SvgIcon rendering not working yet
    if (filePath.endsWith(".svg"))
    {
        GUI_SvgIcon si(filePath, nullptr, width);
        QImage p = si.toImage();
        return new GUI_ThumbnailItem(parent, p, width);

    } else
    {
        
    }
    */


    QImage p(filePath);
    return new GUI_ThumbnailItem(parent, p, width);
}


void
GUI_ThumbnailItem::setLabelWidget(GUI_ItemBase* label)
{
    RLP_LOG_DEBUG("")

    _label = label;
    setHeight(_thumb.height() + _label->height() + 5);
}


void
GUI_ThumbnailItem::setLabel(QString label)
{
    _label = new GUI_Label(this, label);
    _label->setPos(0, _thumb.height() + 7);

    setHeight(_thumb.height() + _label->height() + 5);
}


void
GUI_ThumbnailItem::setThumbImage(QImage thumb)
{
    // RLP_LOG_DEBUG(thumb)

    _thumb = thumb;

    setWidth(_thumb.width());
    setHeight(_thumb.height() + _label->height() + 5);

}


QRectF
GUI_ThumbnailItem::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_ThumbnailItem::paint(GUI_Painter* painter)
{   

    if (_doOutline)
    {
        QPen p;
        p.setWidth(4);
        p.setColor(Qt::black); // GUI_Style::BgLightGrey);

        painter->setPen(p);
        painter->setBrush(QBrush(GUI_Style::BgLightGrey)); // PaneBg

        // painter->drawRoundedRect(boundingRect(), 15, 15);
        painter->drawRect(boundingRect());
        painter->drawImage(2, 2, &_thumb);

    } else
    {
        painter->drawImage(2, 2, &_thumb);
    }
    
}