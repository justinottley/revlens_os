

#ifndef REVLENS_GUI_TL2_FRAME_TIME_INFO_H
#define REVLENS_GUI_TL2_FRAME_TIME_INFO_H

#include "RlpRevlens/GUI_TL2/Global.h"


class GUI_TL2_View;


class REVLENS_GUI_TL2_API GUI_TL2_FrameTimeInfo : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_FrameTimeInfo(GUI_TL2_View* view);

    GUI_TL2_View* view() { return _view; }
    
    virtual QRectF boundingRect() const;
    virtual void paint(GUI_Painter* painter);


private:
    GUI_TL2_View* _view;
};

#endif