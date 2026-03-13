
#include "RlpGui/WIDGET/QWidgetWrapper.h"

RLP_SETUP_LOGGER(gui, GUI, QWidgetWrapper)


GUI_QWidgetWrapper::GUI_QWidgetWrapper(GUI_ItemBase* parent):
    QQuickPaintedItem(parent),
    _widget(nullptr),
    _activeWidget(nullptr)
{
    RLP_LOG_DEBUG("")

    setAcceptHoverEvents(true);
    setFlag(QQuickItem::ItemIsFocusScope);

    setWidth(parent->width());
    setHeight(parent->height());

    connect(this, &QQuickPaintedItem::widthChanged, this, &GUI_QWidgetWrapper::onSizeChangedInternal);
    connect(this, &QQuickPaintedItem::heightChanged, this, &GUI_QWidgetWrapper::onSizeChangedInternal);

    if ((parent != nullptr) && (parent->scene() != nullptr))
    {
        connect(
            parent,
            SIGNAL(sizeChanged(qreal, qreal)),
            this,
            SLOT(onParentSizeChanged(qreal, qreal))
        );
    }
}


void
GUI_QWidgetWrapper::setWidget(QWidget* widget)
{
    RLP_LOG_DEBUG(widget)

    _widget = widget;
    _widget->setGeometry(x(), y(), width(), height());
    _widget->show();
    _widget->update();
    update();
}


QPointF
GUI_QWidgetWrapper::mapToReceiver(const QPointF &pos, const QWidget *receiver)
{
    QPointF p = pos;
    // Map event position from us to the receiver, preserving its
    // precision (don't use QWidget::mapFrom here).
    while (receiver && receiver != _widget) {
        p -= QPointF(receiver->pos());
        receiver = receiver->parentWidget();
    }
    return p;
}


QWidget*
GUI_QWidgetWrapper::widgetAt(const QPointF& pos, QWidget* parent)
{
    if (parent == nullptr)
    {
        parent = _widget;
    }

    bool found = false;
    QList<QObject*> children = parent->children();
    if (children.size() == 0)
    {
        return parent;
    }

    for (int i=0; i != children.size(); ++i)
    {
        QWidget* w = qobject_cast<QWidget*>(children.at(i));
        if (w)
        {
            // RLP_LOG_DEBUG(w << w->pos())

            QRect wgeo = w->rect().translated(w->pos());
            if (wgeo.contains(pos.x(), pos.y()))
            {
                return widgetAt(pos, w);
            }
        }
    }

    return nullptr;
}


void
GUI_QWidgetWrapper::forwardMouseEventToWidget(QMouseEvent* event)
{
    // RLP_LOG_DEBUG(event)

    QWidget* currentWidget = widgetAt(event->position());
    if (currentWidget != nullptr)
    {
        // RLP_LOG_DEBUG(currentWidget)

        QPointF rpos = mapToReceiver(event->position(), currentWidget);
        QMouseEvent evt(
            event->type(),
            rpos,
            event->position(),
            event->position(),
            event->button(),
            event->buttons(),
            event->modifiers()
        );

        QApplication::sendEvent(currentWidget, &evt);
        currentWidget->update();

        event->setAccepted(true);

        update();
    }
}


void
GUI_QWidgetWrapper::forwardEventToActiveWidget(QEvent* event)
{
    // RLP_LOG_DEBUG(event)

    if (_activeWidget != nullptr)
    {
        QApplication::sendEvent(_activeWidget, event);
        _activeWidget->update();

        update();
    }
}


void
GUI_QWidgetWrapper::hoverMoveEvent(QHoverEvent* event)
{
    if (event->position() == event->oldPosF())
        return;

    // RLP_LOG_DEBUG(event->position() << event->oldPosF())

    QWidget* currentWidget = widgetAt(event->position());
    if (!currentWidget)
        return;

    if (_activeWidget != currentWidget)
    {
        RLP_LOG_VERBOSE("New active widget:" << currentWidget)

        if (_activeWidget != nullptr)
        {
            _activeWidget->setAttribute(Qt::WA_UnderMouse, false);
            QPointF rpos = mapToReceiver(event->position(), _activeWidget);
            QPointF roldPos = mapToReceiver(event->oldPosF(), _activeWidget);
            QHoverEvent hleave(
                QEvent::HoverLeave,
                rpos,
                event->position(),
                roldPos,
                event->modifiers()
            );

            QApplication::sendEvent(_activeWidget, &hleave);

            _activeWidget->update();
        }

        currentWidget->setAttribute(Qt::WA_UnderMouse, true);
        QPointF rpos = mapToReceiver(event->position(), currentWidget);
        QPointF roldPos = mapToReceiver(event->oldPosF(), currentWidget);
        QHoverEvent henter(
            QEvent::HoverEnter,
            rpos,
            event->position(),
            roldPos,
            event->modifiers()
        );

        QApplication::sendEvent(currentWidget, &henter);

        currentWidget->update();

        _activeWidget = currentWidget;

    } else
    {
        QApplication::sendEvent(currentWidget, event);
        currentWidget->update();
    }

    update();
}


void
GUI_QWidgetWrapper::hoverEnterEvent(QHoverEvent* event)
{
    hoverMoveEvent(event);
    setFocus(true);
}


void
GUI_QWidgetWrapper::hoverLeaveEvent(QHoverEvent* event)
{
    hoverMoveEvent(event);
    setFocus(false);
}


void
GUI_QWidgetWrapper::keyPressEvent(QKeyEvent* event)
{
    forwardEventToActiveWidget(event);
}


void
GUI_QWidgetWrapper::keyReleaseEvent(QKeyEvent* event)
{
    forwardEventToActiveWidget(event);
}


void
GUI_QWidgetWrapper::mousePressEvent(QMouseEvent* event)
{
    forwardMouseEventToWidget(event);
}


void
GUI_QWidgetWrapper::mouseMoveEvent(QMouseEvent* event)
{
    forwardMouseEventToWidget(event);
}


void
GUI_QWidgetWrapper::mouseReleaseEvent(QMouseEvent* event)
{
    forwardMouseEventToWidget(event);
}


void
GUI_QWidgetWrapper::onSizeChangedInternal()
{
    // RLP_LOG_DEBUG("")

    if (_widget != nullptr)
    {
        // RLP_LOG_DEBUG(width << height)

        _widget->setGeometry(x(), y(), width(), height());
    }
}


void
GUI_QWidgetWrapper::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG("")

    setWidth(width);
    setHeight(height);

    if (_widget != nullptr)
    {
        // RLP_LOG_DEBUG(width << height)

        _widget->setGeometry(x(), y(), width, height);
    }
}


void
GUI_QWidgetWrapper::paint(QPainter* painter)
{
    // RLP_LOG_DEBUG(painter->getPainterType())

    // if (painter->getPainterType() != GUI_Painter::PAINTER_TYPE_QT)
    //     return;
    // RLP_LOG_DEBUG("Painter type OK")

    if (_widget != nullptr)
    {
        // RLP_LOG_DEBUG("")
        _widget->render(painter);
    }
}


QQuickItem*
GUI_QWidgetWrapper::create(GUI_ItemBase* parent, QVariantMap metadata)
{
    return new GUI_QWidgetWrapper(parent);
}