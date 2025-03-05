//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_TL2_TRACK_TOGGLEBUTTON_H
#define REVLENS_GUI_TL2_TRACK_TOGGLEBUTTON_H

#include "RlpRevlens/GUI_TL2/Global.h"
#include "RlpRevlens/GUI_TL2/TrackButton.h"

class GUI_TL2_Track;

class REVLENS_GUI_TL2_API GUI_TL2_TrackToggleButton : public GUI_TL2_TrackButton {
    Q_OBJECT

public:
    GUI_TL2_TrackToggleButton(GUI_ItemBase* parent);

    virtual void paintOn(GUI_Painter* painter) {}
    virtual void paintOff(GUI_Painter* painter) {}

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    
    void paint(GUI_Painter* painter);

    QColor colPen();


};


#endif
