#ifndef REVLENS_GUI_TL2_TRACKBUTTON_H
#define REVLENS_GUI_TL2_TRACKBUTTON_H

#include "RlpRevlens/GUI_TL2/Global.h"

#include "RlpGui/BASE/ButtonBase.h"

class GUI_TL2_Track;

class REVLENS_GUI_TL2_API GUI_TL2_TrackButton : public GUI_ButtonBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    static int SIZE;

    GUI_TL2_TrackButton(GUI_ItemBase* parent);

    QRectF boundingRect() const;

    virtual QColor colPen();

public slots:

    qreal size () const { return GUI_TL2_TrackButton::SIZE; }

    void setColHoverOff(QColor col) { _colHoverOff = col; }


protected:

    QColor _colHoverOn;
    QColor _colHoverOff;
    QColor _colBgOn;


};

#endif
