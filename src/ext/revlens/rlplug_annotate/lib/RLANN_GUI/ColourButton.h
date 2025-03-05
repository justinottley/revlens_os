//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef RLANN_GUI_COLOUR_BUTTON_H
#define RLANN_GUI_COLOUR_BUTTON_H

#include "RlpExtrevlens/RLANN_GUI/Global.h"

#include "RlpGui/WIDGET/IconButton.h"


class EXTREVLENS_RLANN_GUI_API RLANN_GUI_ColourButton : public GUI_IconButton {
    Q_OBJECT

signals:
    void colourSelected(QColor col);

public:
    RLP_DEFINE_LOGGER

    RLANN_GUI_ColourButton(QColor col, GUI_ItemBase* parent);

    void paint(GUI_Painter* painter);

private slots:
    void onButtonPressed(QVariantMap metadata);

public slots:

    QColor color() { return _col; }
    void setSelected(bool selected) { _selected = selected; }

private:

    QColor _col;
    bool _selected;

};



#endif

