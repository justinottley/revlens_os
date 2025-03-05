//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_RADIO_BUTTON_GROUP_H
#define GUI_RADIO_BUTTON_GROUP_H

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/LayoutBase.h"

#include "RlpGui/WIDGET/Global.h"
#include "RlpGui/WIDGET/RadioButton.h"


class GUI_WIDGET_API GUI_RadioButtonGroup : public GUI_ItemBase {
    Q_OBJECT


public:
    RLP_DEFINE_LOGGER

    GUI_RadioButtonGroup(GUI_ItemBase* parent, GUI_LayoutBase* layout, QString labelText="");
    
    QRectF boundingRect() const;

    void paint(GUI_Painter* painter)
    {
    }


public slots:

    static GUI_RadioButtonGroup*
    new_GUI_RadioButtonGroup(GUI_ItemBase* parent, GUI_LayoutBase* layout, QString labelText)
    {
        return new GUI_RadioButtonGroup(parent, layout, labelText);
    }

    // void setRightImage(QPixmap* image) { _rightImage = image; }

    void setLabelText(QString text) { _labelText = text; }

    void onItemAdded(GUI_ItemBase* item);
    void onButtonPressed(QVariantMap metadata);

private:
    
    QString _labelText;
    GUI_LayoutBase* _layout;

};

/*
class GUI_WIDGET_API GUI_RadioButtonGroupDecorator : public QObject {
    Q_OBJECT

public slots:

    GUI_RadioButtonGroup*
    new_GUI_RadioButtonGroup(GUI_ItemBase* parent, GUI_LayoutBase* layout, QString labelText="")
    {
        return new GUI_RadioButtonGroup(parent, layout, labelText);
    }
};
*/

#endif