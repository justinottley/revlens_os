
#include "RlpGui/BASE/FlowLayout.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/BASE/Style.h"

RLP_SETUP_LOGGER(gui, GUI, FlowLayout)

GUI_FlowLayout::GUI_FlowLayout(GUI_ItemBase* parent, int spacing):
    GUI_LayoutBase(parent),
    _spacing(spacing)
{
    _items.clear();

    _scrollArea = new GUI_ScrollArea(this);
    _scrollArea->setBgColour(GUI_Style::PaneBg);
}


void
GUI_FlowLayout::addItem(GUI_ItemBase* item)
{   
    item->setParentItem(_scrollArea->content());
    _items.append(item);
}


void
GUI_FlowLayout::setSpacing(qreal spacing)
{
    _spacing = spacing;
    updateItems();
}


void
GUI_FlowLayout::setOutlined(bool outlined)
{
    _scrollArea->setOutlined(outlined);
}


void
GUI_FlowLayout::clear()
{
    RLP_LOG_DEBUG("")

    for (int i=0; i != _items.size(); ++i) {
        scene()->removeItem(_items.at(i));
        //_items.at(i)->deleteLater();
    }

    _items.clear();
}


void
GUI_FlowLayout::updateItems()
{
    qreal xpos = _spacing;
    qreal ypos = _spacing;
    
    int numItems = _items.size();

    for (int i=0; i != numItems; ++i) {

        // RLP_LOG_DEBUG(i << _items.at(i)->width() << _items.at(i)->height())

        if ((xpos + _items.at(i)->width() + _spacing) > width()) {
            xpos = _spacing;
            ypos += _items.at(i)->height() + _spacing;
        }

        _items.at(i)->setPos(xpos, ypos);
        
        xpos += _items.at(i)->width() + _spacing;
        
    }

    if (numItems > 0)
    {
        _scrollArea->setMaxChildHeight(
            ypos + _items.at(_items.size() - 1)->height() + _spacing
        );
    }
}


void
GUI_FlowLayout::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << nheight)

    setWidth(nwidth);
    setHeight(nheight);

    _scrollArea->onParentSizeChanged(
        nwidth, //  - _scrollArea->vhandleWidth() - 3,
        nheight
    );

    updateItems();
}


QRectF
GUI_FlowLayout::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}

void
GUI_FlowLayout::paint(GUI_Painter* painter)
{
    
}
