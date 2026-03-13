#ifndef REVLENS_GUI_TL2_ZOOMBAR_PANBUTTON_H
#define REVLENS_GUI_TL2_ZOOMBAR_PANBUTTON_H


#include "RlpRevlens/GUI_TL2/Global.h"

class GUI_TL2_View;
class GUI_TL2_ZoomBar;


class REVLENS_GUI_TL2_API GUI_TL2_ZoomBarPanButton : public GUI_ItemBase {
    Q_OBJECT

public:

    enum ButtonSide {
        ZOOM_BTN_PAN_LEFT,
        ZOOM_BTN_PAN_RIGHT
    };

    GUI_TL2_ZoomBarPanButton(ButtonSide side, GUI_TL2_ZoomBar* zoomBar);
    
    
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void resizeEvent(QResizeEvent* event);

    virtual QRectF boundingRect() const;
    virtual void paint(GUI_Painter* painter);

    
    void setXPos(int xpos) { _xpos = xpos; }

public slots:

    ButtonSide side() { return _side; }
    int width() const;
    int xpos(bool offset=true) const;
    

signals:

    void buttonPressed(int side);

private:

    ButtonSide _side;
    GUI_TL2_ZoomBar* _zoomBar;

    bool _inDrag;
    int _dragPos;
    int _xpos;
    int _xposOffset; // fixed offset for placing button - left or right

};

#endif

