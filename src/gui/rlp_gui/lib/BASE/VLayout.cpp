
#include "RlpGui/BASE/VLayout.h"
#include "RlpGui/BASE/Style.h"

RLP_SETUP_LOGGER(gui, GUI, VLayout)

GUI_VLayout::GUI_VLayout(QQuickItem* parent):
    GUI_LayoutBase(parent),
    _itemWidth(1),
    _itemHeight(1),
    _hspacing(0),
    _vspacing(5)
{
    setSize(parent->size());
}


void
GUI_VLayout::addItem(GUI_ItemBase* item, int hoffset)
{
    // RLP_LOG_DEBUG(item << " " << _itemHeight + _vspacing)

    if (hoffset != 0)
    {
        _offsetMap.insert(item, hoffset);
    }

    item->setParentItem(this);
    item->setPos(_hspacing + hoffset, _itemHeight + _vspacing);

    item->setHeightOffset(_itemHeight + _vspacing);

    _itemHeight += item->boundingRect().height() + _vspacing;

    // items max width
    if (item->width() > _itemWidth) {
        _itemWidth = item->width();
    }

    _items.push_back(item);

    connect(
        item,
        &GUI_ItemBase::sizeChanged,
        this,
        &GUI_VLayout::onItemSizeChanged
    );

    emit itemAdded(item);

}


void
GUI_VLayout::addSpacer(int size)
{
    addItem(new GUI_SpacerItem(0, size, this));
}


void
GUI_VLayout::clear()
{
    RLP_LOG_DEBUG("")

    for (int i=0; i != _items.size(); ++i)
    {
        _items.at(i)->deleteLater();
    }
    
    _items.clear();
    _offsetMap.clear();
    _itemHeight = 1;
}


bool
GUI_VLayout::removeItem(GUI_ItemBase* item)
{
    RLP_LOG_DEBUG(item)

    int foundIdx = -1;
    for (int i=0; i != _items.size(); ++i)
    {
        if (_items.at(i) == item)
        {
            foundIdx = i;
            break;
        }
    }

    if (foundIdx != -1)
    {
        RLP_LOG_DEBUG("FOUND ITEM AT" << foundIdx)

        _items.at(foundIdx)->setParentItem(nullptr);
        _items.removeAt(foundIdx);

        onItemSizeChanged();

        return true;
    }

    return false;
}


void
GUI_VLayout::onItemSizeChanged()
{
    // RLP_LOG_DEBUG("")
    
    _itemHeight = 1;

    for (int i = 0;  i != _items.size(); ++i) {
        
        GUI_ItemBase* itm = _items.at(i);

        int hoffset = 0;
        if (_offsetMap.contains(itm))
        {
            hoffset += _offsetMap.value(itm);
        }
        itm->setPos(_hspacing + hoffset, _itemHeight + _vspacing);
        itm->setHeightOffset(_itemHeight + _vspacing);

        _itemHeight += itm->boundingRect().height() + _vspacing;
    }
}


void
GUI_VLayout::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(width << height)

    setWidth(width);
    setHeight(height);

    onItemSizeChanged();
}


QRectF
GUI_VLayout::boundingRect() const
{
    return QRectF(1, 1, width() - 2, height() - 2);
}


void
GUI_VLayout::paint(GUI_Painter* painter)
{
}
