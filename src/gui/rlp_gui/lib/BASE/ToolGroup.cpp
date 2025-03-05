
#include "RlpGui/BASE/ToolGroup.h"
#include "RlpGui/BASE/HLayout.h"
#include "RlpGui/BASE/VLayout.h"

#include "RlpGui/BASE/Style.h"

RLP_SETUP_LOGGER(gui, GUI, ToolGroup)

GUI_ToolGroup::GUI_ToolGroup(
    ToolGroupOrientation orientation,
    GUI_ItemBase* parent,
    int size):
        GUI_ItemBase(parent),
        _orientation(orientation)
{
    setSize(QSizeF(size, size + 5));

    if (orientation == TG_VERTICAL) {
        _layout = new GUI_VLayout(this);

    } else if (orientation == TG_HORIZONTAL) {
        _layout = new GUI_HLayout(this);
        // _layout->setPos(0, 5);
    }
}


GUI_ToolGroup::~GUI_ToolGroup()
{
    RLP_LOG_DEBUG("")
    delete _layout;
}


void
GUI_ToolGroup::addItem(GUI_ItemBase* item, int offset)
{
    // RLP_LOG_DEBUG(item << _layout)
    _layout->addItem(item, offset);

    // RLP_LOG_DEBUG(item << item->pos())
}


void
GUI_ToolGroup::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << nheight)

    if (_orientation == TG_VERTICAL)
    {
        setHeight(nheight);

    } else if (_orientation == TG_HORIZONTAL)
    {
        setWidth(nwidth);

        // TODO FIXME: HARDCODE ALIGN CENTER
        qreal midpos = (nwidth / 2) - (_layout->itemWidth() / 2); 
        _layout->setX(midpos);
    
    } else
    {

        RLP_LOG_ERROR("Invalid orientation")
        Q_ASSERT(false);
    }
}


QRectF
GUI_ToolGroup::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_ToolGroup::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")

    painter->setPen(GUI_Style::BgDarkGrey);
    painter->setBrush(GUI_Style::BgDarkGrey);
    painter->drawRect(boundingRect());

}