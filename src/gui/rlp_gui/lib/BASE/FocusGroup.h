#ifndef GUI_FOCUSGROUP_H
#define GUI_FOCUSGROUP_H

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/Global.h"


class GUI_BASE_API GUI_FocusGroup : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_FocusGroup(GUI_ItemBase* parent);

public slots:

    GUI_ItemBase* next();
    GUI_ItemBase* previous();
    GUI_ItemBase* focused();

    void addItem(GUI_ItemBase* item);

    int focusedItemIdx() { return _focusedItemIdx; }
    bool setFocusedItemIdx(int idx);

    void onTabKeyPressed(QKeyEvent* kevent);

    // void onItemFocusChanged(GUI_ItemBase* item, Qt::FocusReason reason, bool isFocused);
    void onItemFocusChanged(QVariantMap focusInfo);
    void onItemEnterPressed(QVariantMap itemInfo);


private:
    QList<GUI_ItemBase*> _items;
    int _focusedItemIdx;


};

#endif