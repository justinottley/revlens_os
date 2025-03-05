
#include "RlpGui/WIDGET/RadioButtonGroup.h"


RLP_SETUP_LOGGER(gui, GUI, RadioButtonGroup)

GUI_RadioButtonGroup::GUI_RadioButtonGroup(
    GUI_ItemBase* parent,
    GUI_LayoutBase* layout,
    QString labelText):
        GUI_ItemBase(parent),
        _layout(layout),
        _labelText(labelText)
{
    connect(
        layout,
        &GUI_LayoutBase::itemAdded,
        this,
        &GUI_RadioButtonGroup::onItemAdded
    );
}


void
GUI_RadioButtonGroup::onItemAdded(GUI_ItemBase* item)
{
    RLP_LOG_DEBUG("")

    QString clsName = item->metaObject()->className();

    if (clsName == "GUI_RadioButton")
    {
        GUI_RadioButton* button = qobject_cast<GUI_RadioButton*>(item);

        connect(
            button,
            &GUI_RadioButton::buttonPressed,
            this,
            &GUI_RadioButtonGroup::onButtonPressed
        );
    }
    setWidth(_layout->itemWidth());
    setHeight(_layout->itemHeight());
}


void
GUI_RadioButtonGroup::onButtonPressed(QVariantMap /* metadata */)
{
    RLP_LOG_DEBUG("")

    QObject* sender = QObject::sender();

    QList<GUI_ItemBase*> litems = _layout->items();

    // special case for one item
    if (litems.size() == 1)
    {
        GUI_ItemBase* item = litems.at(0);
        QString clsName = item->metaObject()->className();
        if (clsName == "GUI_RadioButton")
        {
            GUI_RadioButton* btn = qobject_cast<GUI_RadioButton*>(item);
            btn->setToggled(!btn->isToggled());
        }

        return;
    }


    // untoggle all items
    LayoutItemIterator it;
    
    for (it = litems.begin(); it != litems.end(); ++it)
    {
        GUI_ItemBase* item = *it;
        QString clsName = item->metaObject()->className();
        if (clsName == "GUI_RadioButton")
        {
            GUI_RadioButton* btn = qobject_cast<GUI_RadioButton*>(item);
            btn->setToggled(false);
        }
        
    }

    GUI_RadioButton* button = qobject_cast<GUI_RadioButton*>(sender);
    button->setToggled(true);

}


QRectF
GUI_RadioButtonGroup::boundingRect() const
{
    return QRectF(0, 0, width(), height()); // _layout->itemWidth(), _layout->itemHeight());
}