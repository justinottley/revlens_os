//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_SCROLLAREA_H
#define CORE_GUI_SCROLLAREA_H

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"

class GUI_ScrollArea;

class GUI_BASE_API GUI_ScrollBarHandle : public GUI_ItemBase {
    Q_OBJECT

signals:
    void handlePosChanged(float posPercent, qreal val, bool clamp);

public:
    RLP_DEFINE_LOGGER

    GUI_ScrollBarHandle(GUI_ScrollArea* parent);

    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    QRectF boundingRect() const;
    
    void paint(GUI_Painter* painter);


public slots:

    //int width() { return _width; }
    //int height() { return _height; }
    float opacity() { return _opacity; }
    void setOpacity(float opacity) { _opacity = opacity; }

    float posPercent() { return _posPercent; }
    virtual void resetPosition();

    virtual void onSizeRatioChanged(qreal size, qreal maxSize) {}

    void setAutoScroll(bool doAutoScroll) { _autoScroll = doAutoScroll; }
    bool autoScroll() { return _autoScroll; }

protected:

    bool _hover;
    // int _width;
    // int _height;

    QPointF _dragPos;
    QPointF _dragPosG;

    float _posPercent;
    float _opacity;

    bool _autoScroll;
};


class GUI_BASE_API GUI_VerticalScrollBarHandle : public GUI_ScrollBarHandle {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    static const int MIN_HEIGHT = 20;

    GUI_VerticalScrollBarHandle(GUI_ScrollArea* parent);

    void mouseMoveEvent(QMouseEvent* event);
    void setYPosDelta(qreal ypos);
    

public slots:    
    void onSizeRatioChanged(qreal size, qreal maxSize);
    void resetPosition();

};


class GUI_BASE_API GUI_HorizontalScrollBarHandle : public GUI_ScrollBarHandle {
    Q_OBJECT

signals:
    void handlexPosChanged(qreal xpos);

public:
    RLP_DEFINE_LOGGER

    GUI_HorizontalScrollBarHandle(GUI_ScrollArea* parent);

    void mouseMoveEvent(QMouseEvent* event);
    void setXPosDelta(qreal ypos);

public slots:
    void onSizeRatioChanged(qreal size, qreal maxSize);
    void resetPosition();
};


class GUI_BASE_API GUI_ScrollAreaContent : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_ScrollAreaContent(GUI_ItemBase* parent);

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

    void setOutlined(bool outlined) { _outlined = outlined; }
    bool isOutlined() { return _outlined; }


private:
    bool _outlined;
};


class GUI_BASE_API GUI_ScrollArea : public GUI_ItemBase {
    Q_OBJECT


signals:
    void focusChanged(Qt::FocusReason reason, bool isFocused);
    void contentPosChanged(qreal xpos, qreal ypos);
public:
    RLP_DEFINE_LOGGER

    GUI_ScrollArea(QQuickItem* parent, qreal width=300, qreal height=200);

    void onSizeChangedInternal();

    
    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void scrollBy(int ypos);
    void wheelEvent(QWheelEvent* event);

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

public slots:

    static GUI_ScrollArea* new_GUI_ScrollArea(QQuickItem* parent) { return new GUI_ScrollArea(parent); }

    GUI_ScrollAreaContent* content() { return _content; }
    GUI_VerticalScrollBarHandle* vhandle() { return _vhandle; }
    GUI_HorizontalScrollBarHandle* hhandle() { return _hhandle; }


    // qreal width() { return _width; } // + _vhandle->width() + 1; }
    // void setWidth(qreal width) { _width = width; }

    // qreal height() { return _height; } //  + _hhandle->height() + 1; }
    void setSAHeight(qreal height);

    // void setSize(qreal width, qreal height) { _width = width; _height = height; }

    qreal maxChildHeight() { return _maxChildHeight; }
    void setMaxChildHeight(qreal height);
    void updateVHandleVis();

    qreal maxChildWidth() { return _maxChildWidth; }
    void setMaxChildWidth(qreal width);
    void updateHHandleVis();

    qreal vhandleWidth() { return _vhandle->width(); }

    void setAutoHide(bool autoHide);

    void setOutlined(bool outlined) { _outlined = outlined; }
    bool isOutlined() { return _outlined; }

    void setFocused(bool isFocused) { _inHover = isFocused; }

    void setHScrollBarVisible(bool isVisible);

    void setBgColour(QColor col) { _bgCol = col; }

    void resetPosition();

    void onParentSizeChanged(qreal width, qreal height);

    void onChildPosChanged(GUI_ItemBase* child, qreal xpos, qreal ypos);

    void onVHandlePosChanged(float pos);
    void onHHandlePosChanged(float pos, bool clamp=true);

    void setYPosPercent(float pos);

    void setVHandlePosOffset(qreal offset) { _vhandlePosOffset = offset; }


protected:

    GUI_VerticalScrollBarHandle* _vhandle;
    GUI_HorizontalScrollBarHandle* _hhandle;

    GUI_ScrollAreaContent* _content;
    GUI_ItemBase* _contentWrapper;

    qreal _maxChildHeight;
    qreal _maxChildWidth;

    bool _autoHide;
    bool _inHover;
    bool _outlined;

    bool _hScrollBarVisible;

    qreal _vhandlePosOffset;

    QColor _bgCol;
};



class GUI_BASE_API GUI_ScrollAreaDecorator : public QObject {
    Q_OBJECT

public slots:

    GUI_ScrollArea*
    new_GUI_ScrollArea(GUI_ItemBase* parent, qreal width=300, qreal height=200)
    {
        return new GUI_ScrollArea(parent, width, height);
    }
};


#endif