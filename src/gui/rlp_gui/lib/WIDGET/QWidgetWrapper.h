
#ifndef GUI_WIDGET_QWIDGET_WRAPPER_H
#define GUI_WIDGET_QWIDGET_WRAPPER_H

#include "RlpGui/WIDGET/Global.h"
#include "RlpGui/BASE/ItemBase.h"


class GUI_QWidgetWrapper : public QQuickPaintedItem {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_QWidgetWrapper(GUI_ItemBase* parent);

    static QQuickItem* create(GUI_ItemBase* parent, QVariantMap metadata);

    QWidget* getWidget() { return _widget; }

    void hoverEnterEvent(QHoverEvent* event);
    void hoverMoveEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    void paint(QPainter* painter);

public slots:

    static GUI_QWidgetWrapper* new_GUI_QWidgetWrapper(GUI_ItemBase* parent)
    {
        return new GUI_QWidgetWrapper(parent);
    }

    void onSizeChangedInternal();
    void onParentSizeChanged(qreal width, qreal height);

    void setWidget(QWidget* widget);

    void setPaintBackground(bool doBg) {}

    void forwardMouseEventToWidget(QMouseEvent* event);


protected:

    QPointF mapToReceiver(const QPointF &pos, const QWidget *receiver);
    void forwardEventToActiveWidget(QEvent* event);

    QWidget* widgetAt(const QPointF& pos, QWidget* parent=nullptr);

    QWidget* _widget;
    QWidget* _activeWidget; // widget under mouse

};

#endif
