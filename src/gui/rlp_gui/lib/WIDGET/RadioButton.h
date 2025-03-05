//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_RADIO_BUTTON_H
#define GUI_RADIO_BUTTON_H

#include "RlpGui/WIDGET/Global.h"

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/LayoutBase.h"
#include "RlpGui/BASE/SvgIcon.h"


class GUI_WIDGET_API GUI_RadioButton : public GUI_ItemBase {
    Q_OBJECT

signals:
    void buttonPressed(QVariantMap metadata);

public:
    RLP_DEFINE_LOGGER

    GUI_RadioButton(GUI_ItemBase* parent, QString labelText="", qreal size=20, qreal padding=0);

    virtual void mousePressEvent(QMouseEvent* event);

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);


public slots:

    static GUI_RadioButton*
    new_GUI_RadioButton(GUI_ItemBase* parent, QString labelText, qreal size, qreal padding)
    {
        return new GUI_RadioButton(parent, labelText, size, padding);
    }


    void setLabelText(QString text);

    bool isToggled() { return _toggled; }
    void setToggled(bool toggled);

    void emitButtonPressed() { emit buttonPressed(_metadata); }

private:
    
    GUI_SvgIcon* _offIcon;
    GUI_SvgIcon* _onIcon;


    QString _labelText;
    int _textWidth;
    int _size;
    bool _toggled;

};


/*

class GUI_WIDGET_API GUI_RadioButtonDecorator : public QObject {
    Q_OBJECT

public slots:

    GUI_RadioButton*
    new_GUI_RadioButton(GUI_ItemBase* parent, qreal size=20, qreal padding=0, QString labelText="")
    {
        return new GUI_RadioButton(parent, size, padding, labelText);
    }
};

*/

#endif