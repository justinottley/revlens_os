#ifndef REVLENS_GUI_TL2_TICKS_H
#define REVLENS_GUI_TL2_TICKS_H


#include "RlpRevlens/GUI_TL2/Global.h"

class GUI_TL2_View;

class REVLENS_GUI_TL2_API GUI_TL2_Ticks : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER
    
    enum HeightPolicy {        
        HEIGHT_FIXED,
        HEIGHT_FULL
    };

    GUI_TL2_Ticks(GUI_TL2_View* view);

    void onParentSizeChanged(qreal nwidth, qreal nheight);

    virtual QRectF boundingRect() const;
    virtual void paint(GUI_Painter* painter);

public slots:

    int tickHeight() const;
    void setTickHeight(int height) { _tickHeight = height; }
    void setHeightPolicy(HeightPolicy policy) { _heightPolicy = policy; }

    bool isLinesVisible() { return _linesVisible; }
    bool isNumbersVisible() { return _numVisible; }

    void setLinesVisible(bool linesVisible) { _linesVisible = linesVisible; }
    void setNumbersVisible(bool numVisible) { _numVisible = numVisible; }

    void refresh();

private:

    GUI_TL2_View* _view;

    int _tickHeight;
    HeightPolicy _heightPolicy;    
    
    bool _linesVisible;
    bool _numVisible;
};

#endif