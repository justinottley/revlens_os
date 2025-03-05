//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_PANE_PANESPLITTER_H
#define GUI_PANE_PANESPLITTER_H

#include "RlpCore/LOG/Logging.h"


#include "RlpGui/PANE/Global.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/FrameBase.h"

class GUI_Pane;

class GUI_PaneSplitter;
typedef std::shared_ptr<GUI_PaneSplitter> GUI_PaneSplitterPtr;

class GUI_PANE_API GUI_PaneSplitter : public GUI_ItemBase {
    Q_OBJECT
    QML_ELEMENT

public:
    static const int SIZE;
    static bool AUTOHIDE; // switch for toggling all splitters visible or not

public:
    RLP_DEFINE_LOGGER

    GUI_PaneSplitter(GUI_ItemBase* parent, int paneStartIndex);

    SplitOrientation orientation() const;

    QRectF boundingRect() const;
    
    void onParentSizeChanged(qreal width, qreal height);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

	void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    void paint(GUI_Painter* painter);


public slots:

    void setPosPercent(float pos, bool doUpdatePanes=true);
    void setPosPixels(int pixels);

    void setAutohide(bool doAutohide) { _autoHide = doAutohide; }

    void setActive(bool active) { _active = active; }
    bool isActive() { return _active; }

    void setDrawLine(bool drawLine) { _drawLine = drawLine; }

    float maxPos();
    float minPos();
    float splitterPos() { return _pos; }
    
    // qreal width() const;
    // qreal height() const;

    void updatePanes();
    void updatePos();
    void setPosFromPixels();


    void setPaneStartIndex(int idx) { _paneStartIndex = idx; }
    int paneStartIndex() { return _paneStartIndex; }


private:

    int _paneStartIndex;

    float _pos;
    int _posPixels;

    GUI_Pane* _parentPane;

    bool _inDrag;
    QPointF _dragStartPos;
    QPointF _dragStartPosG;
    bool _focused;

    bool _active;
    bool _autoHide;

    bool _drawLine;
};


#endif