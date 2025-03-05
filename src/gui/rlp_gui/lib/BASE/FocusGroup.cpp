

#include "RlpGui/BASE/FocusGroup.h"


RLP_SETUP_LOGGER(core, GUI, FocusGroup)

GUI_FocusGroup::GUI_FocusGroup(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _focusedItemIdx(-1)
{
}


void
GUI_FocusGroup::addItem(GUI_ItemBase* item)
{
    // RLP_LOG_DEBUG(item)

    connect(
        item,
        &GUI_ItemBase::focusChanged,
        this,
        &GUI_FocusGroup::onItemFocusChanged
    );

    connect(
        item,
        &GUI_ItemBase::enterPressed,
        this,
        &GUI_FocusGroup::onItemEnterPressed
    );

    _items.append(item);
}


GUI_ItemBase*
GUI_FocusGroup::next()
{
    _focusedItemIdx++;

    if (_focusedItemIdx >= _items.size())
    {
        _focusedItemIdx = 0;
    }

    return focused();
}


GUI_ItemBase*
GUI_FocusGroup::previous()
{
    _focusedItemIdx--;

    if (_focusedItemIdx < 0)
    {
        _focusedItemIdx = _items.size() - 1;
    }

    return focused();
}


GUI_ItemBase*
GUI_FocusGroup::focused()
{
    if ((_focusedItemIdx < 0) ||
        (_focusedItemIdx >= _items.size()) ||
        (_items.size() == 0))
    {
        RLP_LOG_DEBUG("No items, returning")
        return nullptr;
    }


    RLP_LOG_DEBUG("num items:" << _items.size())

    GUI_ItemBase* ni = _items.at(_focusedItemIdx);
    return ni;
}


bool
GUI_FocusGroup::setFocusedItemIdx(int idx)
{
    RLP_LOG_DEBUG(idx)

    if ((idx > 0) && (idx < _items.size()))
    {
        _focusedItemIdx = idx;

        return true;
    }

    return false;
}


void
GUI_FocusGroup::onTabKeyPressed(QKeyEvent* kevent)
{
    RLP_LOG_DEBUG(kevent)

    GUI_ItemBase* f = focused();
    if (f != nullptr)
    {
        RLP_LOG_DEBUG(f)

        f->setFocused(false);
    }

    GUI_ItemBase* item;
    if (kevent->modifiers().testFlag(Qt::ShiftModifier))
    {
        item = previous();

    } else {
        item = next();
    }


    if (item != nullptr)
    {
        RLP_LOG_DEBUG(item)

        item->setFocused(true);
    }
}


void
GUI_FocusGroup::onItemFocusChanged(QVariantMap focusInfo) // GUI_ItemBase* item, Qt::FocusReason reason, bool isFocused)
{
    // RLP_LOG_DEBUG(item << reason << isFocused)
    
    GUI_ItemBase* item = focusInfo.value("item").value<GUI_ItemBase*>();
    Qt::FocusReason reason = (Qt::FocusReason)focusInfo.value("reason").toInt();
    bool isFocused = focusInfo.value("isFocused").toBool();

    if ((isFocused) && (reason != Qt::TabFocusReason))
    {
        for (int i=0; i != _items.size(); ++i)
        {
            if (_items.at(i) == item)
            {
                _focusedItemIdx = i;
                break;
            }
        }
    }
}


void
GUI_FocusGroup::onItemEnterPressed(QVariantMap itemInfo)
{
    RLP_LOG_DEBUG(itemInfo);

    emit enterPressed(itemInfo);
}