
#include "RlpGui/BASE/HLayout.h"
#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/DividerItem.h"

RLP_SETUP_LOGGER(gui, GUI, HLayout)

GUI_HLayout::GUI_HLayout(QQuickItem* parent, int hspacing):
    GUI_LayoutBase(parent),
    _itemWidth(1),
    _itemHeight(1),
    _hspacing(hspacing),
    _vspacing(0),
    _hoff(0),
    _woff(0),
    _bgcolour(Qt::transparent)
{
    _items.clear();
}


void
GUI_HLayout::clear()
{
    RLP_LOG_DEBUG("")

    for (int i=0; i != _items.size(); ++i)
    {
        GUI_ItemBase* it = _items.at(i);
        it->setVisible(false);
        it->setParentItem(nullptr);

        if (_safeDelete)
        {
            RLP_LOG_VERBOSE("SAFE DELETE")

            _deletedItems.append(it);

        } else
        {
            it->deleteLater();
        }
    }

    _items.clear();
    _itemWidth = 0;
    _itemHeight = 0;
}


void
GUI_HLayout::addItemAt(GUI_ItemBase* item, int voffset, InsertPosition pos)
{
    if (voffset != 0)
    {
        _offsetMap.insert(item, voffset);
    }


    item->setParentItem(this);
    item->setPos(_itemWidth + _hspacing, _vspacing + voffset);

    _itemWidth += item->boundingRect().width() + _hspacing;

    int iheight = item->height() + voffset;

    // max height
    if (iheight > _itemHeight)
    {
        _itemHeight = iheight;
    }


    if (pos == POS_END)
    {
        _items.push_back(item);

    } else if (pos == POS_START)
    {
        RLP_LOG_DEBUG("Adding item at start")
        _items.insert(0, item);
        onItemSizeChanged();

    } else
    {
        RLP_LOG_ERROR("invalid insertion position")
    }
    
    connect(
        item,
        &GUI_ItemBase::sizeChanged,
        this,
        &GUI_HLayout::onItemSizeChanged
    );

    if (item->isPyMode())
    {
        _safeDelete = true;
    }

    emit itemAdded(item);
    
    // RLP_LOG_DEBUG(item << " " << _itemWidth << " " << _itemHeight)

    setWidth(_itemWidth + _woff);
    setHeight(_itemHeight + _hoff);

}


void
GUI_HLayout::addItem(GUI_ItemBase* item, int voffset)
{
    addItemAt(item, voffset, POS_END);
}


void
GUI_HLayout::addItemAtStart(GUI_ItemBase* item, int voffset)
{
    addItemAt(item, voffset, POS_START);

}


void
GUI_HLayout::addSpacer(int size)
{
    addItem(new GUI_SpacerItem(size, 0, this));
}


void
GUI_HLayout::addDivider(int size, int height, int voffset)
{
    addItem(new GUI_DividerItem(size, height, this), voffset);
}


QVariantList
GUI_HLayout::itemList()
{
    QVariantList result;
    for (int i=0; i != _items.size(); ++i)
    {
        QVariant v;
        v.setValue(_items.at(i));
        result.append(v);
    }

    return result;
}


void
GUI_HLayout::onItemSizeChanged()
{
    _itemWidth = 1;

    for (int i = 0;  i != _items.size(); ++i)
    {
        GUI_ItemBase* itm = _items.at(i);

        int voffset = 0;
        if (_offsetMap.contains(itm))
        {
            voffset += _offsetMap.value(itm);
        }

        itm->setPos(_itemWidth + _hspacing, _vspacing + voffset);
        _itemWidth += itm->boundingRect().width() + _hspacing;

        // RLP_LOG_DEBUG(itm << _itemWidth)
    }

    setWidth(_itemWidth + _woff + 1);
}


QRectF
GUI_HLayout::boundingRect() const
{
    return QRectF(0, 0, _itemWidth + _woff, (_itemHeight + _hoff));
}


void
GUI_HLayout::paint(GUI_Painter* painter)
{
    // if (_bgcolour != Qt::transparent)
    // {
    //     // RLP_LOG_DEBUG(boundingRect())

    //     QBrush b(_bgcolour);
    //     painter->setPen(_bgcolour);
    //     painter->setBrush(b);
    //     painter->drawRoundedRect(boundingRect(), 5, 5);
    // }
}
