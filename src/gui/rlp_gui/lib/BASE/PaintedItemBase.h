//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_BASE_PAINTEDITEMBASE_H
#define GUI_BASE_PAINTEDITEMBASE_H

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpCore/PY/PyQVariant.h"


class GUI_FrameBase;
class GUI_Scene;


class GUI_BASE_API GUI_PaintedItemBase : public QQuickPaintedItem {
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
    RLP_DEFINE_LOGGER

    GUI_PaintedItemBase(QQuickItem* parent);
    virtual ~GUI_PaintedItemBase();

    static GUI_PaintedItemBase* create(GUI_ItemBase* parent, QVariantMap toolInfo);

    static const int UserType;

    GUI_Scene* scene() { return _scene; }
    void setScene(GUI_Scene* scene);

    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual bool dragStarted(QMouseEvent* event);

    virtual void hoverEnterEvent(QHoverEvent* event);
    virtual void hoverLeaveEvent(QHoverEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

    virtual void keyPressEvent(QKeyEvent* event);


public slots:

    static GUI_PaintedItemBase* new_GUI_PaintedItemBase(QQuickItem* parent)
    {
        GUI_PaintedItemBase* item = new GUI_PaintedItemBase(parent);

        // This constructor is really only used by python binding, so set pymode here
        // so python clients dont have to
        //
        item->setPyMode(true);

        return item;
    }

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

    virtual void paint(QPainter* painter);

    void deleteChildren();

protected:

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

class GUI_QQuickItemPtr_PyTypeConverter : public PyTypeConverter {
  public:

    GUI_QQuickItemPtr_PyTypeConverter():
        PyTypeConverter("QQuickItem*")
    {
    }

    py::object
    toPy(QVariant val)
    {
        QQuickItem* ival = val.value<QQuickItem*>();
        if (ival->inherits("GUI_ItemBase"))
        {
            GUI_ItemBase* iobj = qobject_cast<GUI_ItemBase*>(ival);
            return py::cast(iobj);

        } else if (ival->inherits("GUI_PaintedItemBase"))
        {
            GUI_PaintedItemBase* iobj = qobject_cast<GUI_PaintedItemBase*>(ival);
            return py::cast(iobj);
        }

        qInfo() << "WARNING: Could not cast C++ object to ItemBase or PaintedItemBase, using QQuickItem";
        return py::cast(ival);
    }

    QVariant
    fromPy(py::handle pobj)
    {
        QVariant result;
        QQuickItem* cobj = pobj.cast<QQuickItem*>();

       
        if (cobj->inherits("GUI_ItemBase"))
        {
            GUI_ItemBase* iobj = qobject_cast<GUI_ItemBase*>(cobj);
            result.setValue(iobj);
        } else if (cobj->inherits("GUI_PaintedItemBase"))
        {
            GUI_PaintedItemBase* iobj = qobject_cast<GUI_PaintedItemBase*>(cobj);
            result.setValue(iobj);
        }

        return result;
    }
};



#endif