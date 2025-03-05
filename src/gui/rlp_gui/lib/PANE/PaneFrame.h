//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_PANE_WINDOWBORDER_H
#define GUI_PANE_WINDOWBORDER_H


#include "RlpCore/LOG/Logging.h"

#include "RlpGui/PANE/Global.h"
#include "RlpGui/BASE/FrameBase.h"
#include "RlpGui/BASE/FrameOverlay.h"
#include "RlpGui/PANE/Pane.h"


class GUI_SvgButton;

class GUI_PANE_API GUI_PaneFrame : public GUI_FrameBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_PaneFrame(int width, int height);

    QRectF boundingRect() const;

    void initPane();

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    void paint(GUI_Painter* painter);

public slots:

    static GUI_PaneFrame* new_GUI_PaneFrame(int width, int height)
    {
        return new GUI_PaneFrame(width, height);
    }


    GUI_PaneBody* body() { return _pane->body(); }
    GUI_SvgButton* closeButton() { return _btnClose; }

    void setCloseButtonVisible(bool isVisible);
    void setPersistent(bool isPersistent) { _isPersistent = isPersistent; }
    void setOverlay(GUI_FrameOverlay* overlay) { _overlay = overlay; }
    void setText(QString text) { _text = text; }
    void setTextBold(bool doBold) { _textBold = doBold; }
    void setBgCol(QColor col);
    void setHeaderHeight(int height) { _headerHeight = height; }
    void setWidth(qreal width);

    void onCloseRequested(QVariantMap metadata = QVariantMap());
    void requestClose() { onCloseRequested(); };

private:

    GUI_Pane* _pane;
    int _borderSize;
    int _headerHeight;

    bool _closeButtonVisible;
    GUI_SvgButton* _btnClose;

    GUI_FrameOverlay* _overlay;
    QColor _bgCol;

    QString _text;
    bool _textBold;
    bool _isPersistent;

    QPointF _dragStartPosG;
    QPointF _dragStartPos;
};

#endif
