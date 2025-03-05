//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_BASE_ITEMBASE_H
#define GUI_BASE_ITEMBASE_H

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/Painter.h"
#include "RlpCore/PY/PyQVariant.h"

#include "qnanoquickitem.h"
#include "qnanoquickitempainter.h"


class GUI_FrameBase;
class GUI_Scene;

class GUI_ItemBase;
class GUI_PaintedItemBase;

class GUI_NanoPainterProxy : public QNanoQuickItemPainter {

public:
    RLP_DEFINE_LOGGER

    GUI_NanoPainterProxy(const GUI_ItemBase* item);

    void paint(QNanoPainter* p);

private:
    GUI_ItemBase* _item;
    //GUI_NanoPainter* _painter;

};


class GUI_PaintedItemProxy : public QQuickPaintedItem {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_PaintedItemProxy(GUI_ItemBase* parent);

    void setPos(qreal x, qreal y)
    {
        setX(x);
        setY(y);
    }

    void paint(QPainter* painter);

private:
    GUI_ItemBase* _ib;


};



class GUI_BASE_API GUI_ItemBase : public QNanoQuickItem {
    Q_OBJECT

signals:

    void sizeChangedInternal();

    void sizeChanged(qreal width, qreal height);
    // void focusChanged(GUI_ItemBase* item, Qt::FocusReason reason, bool isFocused);
    void focusChanged(QVariantMap info);
    void enterPressed(QVariantMap info);
    void escapePressed();

    void pySig(QVariantMap sigInfo);


public:

    enum PaintMode
    {
        PAINT_QT,
        PAINT_NANOPAINTER
    };

    RLP_DEFINE_LOGGER

    GUI_ItemBase(QQuickItem* parent, PaintMode paintMode=PaintMode::PAINT_NANOPAINTER);

    virtual ~GUI_ItemBase();

    static QQuickItem* create(GUI_ItemBase* parent, QVariantMap toolInfo);

    static const int UserType;

    GUI_Scene* scene() { return _scene; }
    void setScene(GUI_Scene* scene);

    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual bool dragStarted(QMouseEvent* event);

    virtual void hoverEnterEvent(QHoverEvent* event) override;
    virtual void hoverLeaveEvent(QHoverEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

    virtual void keyPressEvent(QKeyEvent* event) override;

    // test for WASM
    // virtual bool event(QEvent* event);

    QNanoQuickItemPainter* createItemPainter() const override;


public slots:

    static GUI_ItemBase* new_GUI_ItemBase(QQuickItem* parent)
    {
        GUI_ItemBase* item = new GUI_ItemBase(parent);

        // This constructor is really only used by python binding, so set pymode here
        // so python clients dont have to
        //
        item->setPyMode(true);

        return item;
    }


    PaintMode paintMode() { return _paintMode; }
    void setPaintMode(int paintMode);

    virtual void setPos(qreal xpos, qreal ypos);
    virtual void setZValue(qreal zval);
    QPointF pos() { return QPointF(x(), y()); }
    virtual QRectF boundingRect() const;


    // to allow subclassing of width and height
    //
    virtual void setItemWidth(qreal width) { setWidth(width); }
    virtual void setItemHeight(qreal height) { setHeight(height); }

    void setHeightOffset(qreal heightOffset) { _heightOffset = heightOffset; }
    qreal heightOffset() { return _heightOffset; }

    void setWidthOffset(qreal widthOffset) { _widthOffset = widthOffset; }
    qreal widthOffset() { return _widthOffset; }

    void showItem() { setVisible(true); }
    void hideItem() { setVisible(false); }
    bool isItemVisible() { return isVisible(); }
    void deleteItemLater() { deleteLater(); }

    virtual bool inHover() { return _inHover; }
    void setInHover(bool ih) { _inHover = ih; _moveTime = QTime::currentTime(); }

    // A way to do custom updates if necessary
    virtual void updateItem();

    bool containsItem(const QPointF& point) { return contains(point); }

    void setItemAcceptHoverEvents(bool hover) { setAcceptHoverEvents(hover); }

    virtual void setPaintBackground(bool doPaintBg);
    bool paintBackground() { return _paintBg; }
    void setPaintBg(bool doPaintBg) { _paintBg = doPaintBg; } // non recursive

    void setPyMode(bool pymode);

    virtual void setFocused(bool isFocused) {}

    virtual void onSizeChangedInternal();
    virtual void onParentSizeChanged(qreal width, qreal height);
    virtual void onParentVisibilityChanged(QString name, bool isVisible);

    virtual QString toolTipText() { return _toolTipText; }
    virtual void setToolTipText(QString text) { _toolTipText = text; }
    virtual void showToolTip(QPointF pos);
    virtual void clearToolTip();
    virtual void checkForToolTip();
    virtual void delayedCheckForToolTip();

    void setMetadata(QString key, QVariant val) { _metadata.insert(key, val); }
    QVariantMap metadata() { return _metadata; }
    QVariant metadataValue(QString key) { return _metadata.value(key); }

    virtual QImage exportToImage();

    virtual void paint(GUI_Painter* painter);

    void deleteChildren();

protected:

    PaintMode _paintMode;
    GUI_PaintedItemProxy* _proxy;
    GUI_Scene* _scene;

    QString _toolTipText;

    bool _inHover;
    QTime _moveTime;
    QPointF _movePos;

    bool _paintBg;

    bool _pymode;

    QVariantMap _metadata;

    // for use in sizing calculations when parent's size changes.
    // maybe this item is in a layout and this item's dimensions
    // are not the whole parent size
    qreal _heightOffset;
    qreal _widthOffset;

    QPointF _pressPos; // for drag and drop
};


class GUI_PaintedItem : public GUI_ItemBase {
    Q_OBJECT

public:
    GUI_PaintedItem(QQuickItem* parent):
        GUI_ItemBase(parent, GUI_ItemBase::PAINT_QT)
    {
    }

public slots:
    static GUI_PaintedItem* new_GUI_PaintedItem(QQuickItem* parent)
    {
        return new GUI_PaintedItem(parent);
    }
};


Q_DECLARE_METATYPE(GUI_ItemBase*)



#endif