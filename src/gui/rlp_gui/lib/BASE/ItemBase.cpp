
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/PaintedItemBase.h"
#include "RlpGui/BASE/ViewBase.h"
#include "RlpGui/BASE/FrameBase.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/FrameOverlay.h"

#include "RlpCore/PY/Interp.h"
#include "RlpCore/PY/SigSlotConnector.h"
#include "RlpCore/UTIL/Text.h"

RLP_SETUP_LOGGER(gui, GUI, PaintedItemProxy)
RLP_SETUP_LOGGER(gui, GUI, NanoPainterProxy)
RLP_SETUP_LOGGER(gui, GUI, ItemBase)

GUI_PaintedItemProxy::GUI_PaintedItemProxy(GUI_ItemBase* parent):
    QQuickPaintedItem(parent),
    _ib(parent)
{
    // RLP_LOG_DEBUG("")
}


void
GUI_PaintedItemProxy::paint(QPainter* painter)
{
    RLP_LOG_DEBUG("")
    GUI_QPainter p(painter);
    _ib->paint(&p);
}



GUI_NanoPainterProxy::GUI_NanoPainterProxy(const GUI_ItemBase* item):
    _item(const_cast<GUI_ItemBase*>(item))
{
}


void
GUI_NanoPainterProxy::paint(QNanoPainter* p)
{
    if (_item->paintMode() == GUI_ItemBase::PAINT_QT)
    {
        RLP_LOG_DEBUG("Found QT Paint mode, skipping")
        p->reset();
        return;
    }

    GUI_NanoPainter painter(p);
    _item->paint(&painter);
}


GUI_ItemBase::GUI_ItemBase(QQuickItem* parent, PaintMode pm):
    QNanoQuickItem(parent),
    _paintMode(pm),
    _proxy(nullptr),
    _toolTipText(""),
    _inHover(false),
    _scene(nullptr),
    _paintBg(true),
    _pymode(false),
    _heightOffset(0),
    _widthOffset(0),
    _inDragAndDrop(false)
{
    if (pm == PaintMode::PAINT_QT)
    {
        _proxy = new GUI_PaintedItemProxy(this);
        _proxy->setPos(0, 0);
    }
    // test for wasm
    // setAcceptTouchEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);

    connect(this, &GUI_ItemBase::widthChanged, this, &GUI_ItemBase::onSizeChangedInternal);
    connect(this, &GUI_ItemBase::heightChanged, this, &GUI_ItemBase::onSizeChangedInternal);

    if (parent != nullptr)
    {
        QString parentClsName = parent->metaObject()->className();

        if (parent->inherits("GUI_PaintedItemBase"))
        {
            RLP_LOG_DEBUG("Got painted item:" << parentClsName)

            GUI_PaintedItemBase* iparent = qobject_cast<GUI_PaintedItemBase*>(parent);
            if (iparent->scene() != nullptr)
            {
                // RLP_LOG_DEBUG("PARENT OK:" << parent->metadata())
                _scene = iparent->scene();

                connect(
                    iparent,
                    SIGNAL(sizeChanged(qreal, qreal)),
                    this,
                    SLOT(onParentSizeChanged(qreal, qreal))
                );
            }
        } else
        {
            GUI_ItemBase* iparent = qobject_cast<GUI_ItemBase*>(parent);
            if (iparent->scene() != nullptr)
            {
                // RLP_LOG_DEBUG("PARENT OK:" << parent->metadata())
                _scene = iparent->scene();

                connect(
                    iparent,
                    SIGNAL(sizeChanged(qreal, qreal)),
                    this,
                    SLOT(onParentSizeChanged(qreal, qreal))
                );
            }

        }
    }
}


GUI_ItemBase::~GUI_ItemBase()
{
    if (_pymode)
    {
        // RLP_LOG_WARN("DESTRUCTING IN PYMODE")

        PY_Interp::acquireGIL();

        disconnectPySlots();

        // Try to look up the overridden method on the Python side.
        pybind11::function pyfunc = pybind11::get_override(this, "destruct");
        if (pyfunc)
        {
            try {
                pyfunc();

            } catch (py::error_already_set &e) {

                RLP_LOG_ERROR("")
                RLP_LOG_ERROR("Error running destruct()")
                RLP_LOG_ERROR("")
                RLP_LOG_ERROR(e.what())
            }
        }
    }
}


void
GUI_ItemBase::disconnectPySlots()
{
    // RLP_LOG_DEBUG("")

    // TODO FIXME: CopyPasta from PY_Signal
    //
    if (property("pyslots").isValid())
    {
        QVariantList pyslots = property("pyslots").toList();

        // RLP_LOG_DEBUG(pyslots)

        for (auto pyslot : pyslots)
        {
            PyConnInfo pci = pyslot.value<PyConnInfo>();
            if (pci.second != py::none())
            {
                PY_SigSlotConnector* pssc = qobject_cast<PY_SigSlotConnector*>(pci.first);

                if (pssc->removePySlot(pci.second))
                {
                    // RLP_LOG_DEBUG("Slot disconnected")
                } else
                {
                    RLP_LOG_WARN("SLOT NOT DISCONNECTED")
                }
            }
        }

    }

}


QQuickItem*
GUI_ItemBase::create(GUI_ItemBase* parent, QVariantMap toolInfo)
{
    // RLP_LOG_INFO(toolInfo)

    QQuickItem* iresult = nullptr;

    if (toolInfo.contains("pyitem"))
    {
        QString callback = toolInfo.value("pyitem").toString();

        PY_Interp::acquireGIL();
        {
            py::object cbMod = py::module_::import(callback.toLocal8Bit().constData());
            py::object cbObj = cbMod.attr("create");
            py::object param = py::cast(parent);

            py::object result = cbObj(param);

            // prevent garbage collection of UI pane widgets when python function returns
            result.inc_ref();

            if (!result.is(py::none()))
            {
                iresult = result.cast<QQuickItem*>();
            }
        }

        PY_Interp::releaseGIL();

    } else
    {
        RLP_LOG_ERROR("invalid tool info")
    }

    return iresult;
}


QNanoQuickItemPainter*
GUI_ItemBase::createItemPainter() const
{
    return new GUI_NanoPainterProxy(this);
}

void
GUI_ItemBase::setScene(GUI_Scene* scene)
{
    // RLP_LOG_DEBUG(scene)

    _scene = scene;
}


void
GUI_ItemBase::setPaintMode(int pmi)
{
    RLP_LOG_DEBUG(pmi)

    if ((pmi == (int)(PaintMode::PAINT_QT)) &&
        (_proxy == nullptr))
    {
        RLP_LOG_DEBUG("Creating QQuickPaintedItem proxy")
        _proxy = new GUI_PaintedItemProxy(this);
        _proxy->setPos(0, 0);
        _proxy->setSize(size());
    }

    _paintMode = (PaintMode)(pmi);

    updateItem();
}

void
GUI_ItemBase::setPos(qreal xpos, qreal ypos)
{
    setX(xpos);
    setY(ypos);
}


void
GUI_ItemBase::setZValue(qreal zval)
{
    setZ(zval);
}


void
GUI_ItemBase::setPaintBackground(bool doPaintBg)
{
    // RLP_LOG_DEBUG(this << doPaintBg << parentItem())

    
    _paintBg = doPaintBg;

    if (parentItem() != nullptr)
    {
        QString clsName = parentItem()->metaObject()->className();
        if (clsName != "GUI_Scene")
        {
            qobject_cast<GUI_ItemBase*>(parentItem())->setPaintBackground(doPaintBg);
        }
    }

    update();
}


void
GUI_ItemBase::setPyMode(bool pymode)
{
    // RLP_LOG_DEBUG(pymode)

    _pymode = pymode;
}


void
GUI_ItemBase::onSizeChangedInternal()
{
    // RLP_LOG_DEBUG(metaObject()->className() << width() << height())

    if (_proxy != nullptr)
    {
        _proxy->setSize(size());
    }
    

    emit sizeChanged(width(), height());
}


void
GUI_ItemBase::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(width << height << _pymode)

    if (_pymode)
    {
        PY_Interp::acquireGIL();

        // Try to look up the overridden method on the Python side.
        pybind11::function pyfunc = pybind11::get_override(this, "onParentSizeChangedItem");
        if (pyfunc)
        {
            try {
                pyfunc(width, height);

            } catch (py::error_already_set &e) {

                RLP_LOG_ERROR("")
                RLP_LOG_ERROR("Error running onParentSizeChangedItem()")
                RLP_LOG_ERROR("")
                RLP_LOG_ERROR(e.what())
            }
        }
    }
}


void
GUI_ItemBase::onParentVisibilityChanged(QString name, bool isVisible)
{
    // RLP_LOG_DEBUG(name << isVisible)

    QString tabName;
    if (_metadata.contains("tab.name"))
    {
        tabName = _metadata.value("tab.name").toString();
        // RLP_LOG_DEBUG(name << isVisible << "my tab:" << tabName)
        if ((tabName == name) && (isVisible))
        {
            // RLP_LOG_DEBUG("ENABLE")
            qobject_cast<GUI_ItemBase*>(parentItem())->setPaintBackground(true);
        }
    } else
    {
        qobject_cast<GUI_ItemBase*>(parentItem())->setPaintBackground(true);
    }

}


void
GUI_ItemBase::checkForToolTip()
{
    // RLP_LOG_DEBUG(_inHover)

    if (!_inHover) return;

    QTime now = QTime::currentTime();
    int diff = _moveTime.msecsTo(now);

    // RLP_LOG_DEBUG("time diff:" << diff)

    if (diff > 2) // 300)
    {
        showToolTip(_movePos);
    }
}


void
GUI_ItemBase::delayedCheckForToolTip()
{
    QTimer::singleShot(500, this, &GUI_ItemBase::checkForToolTip);
}


void
GUI_ItemBase::showToolTip(QPointF pos)
{
    #ifdef SCAFFOLD_IOS
    return;
    #endif

    if (_toolTipText.size() == 0) return;

    // RLP_LOG_DEBUG(_toolTipText << pos)

    QPointF spos = mapToScene(pos);

    GUI_Scene* s = qobject_cast<GUI_Scene*>(scene());

    int ypos = spos.y() + 10;
    if (ypos + 30 > s->view()->height())
    {
        ypos -= 30;
    }

    s->showToolTip(
        _toolTipText,
        spos.x() + 20,
        ypos
    );

}


void
GUI_ItemBase::clearToolTip()
{
    GUI_Scene* s = qobject_cast<GUI_Scene*>(scene());
    s->hideToolTip();
}


void
GUI_ItemBase::dragEnterEvent(QDragEnterEvent* event)
{
    // RLP_LOG_DEBUG(this)

    if (_pymode)
    {
        PY_Interp::acquireGIL();

        // Try to look up the overridden method on the Python side.
        pybind11::function pyfunc = pybind11::get_override(this, "dragEnterEventItem");
        if (pyfunc)
        {
            try
            {
                py::object pyevent = py::cast(event);
                pyfunc(pyevent);

            } catch (py::error_already_set &e)
            {

                RLP_LOG_ERROR("")
                RLP_LOG_ERROR("Error running dragEnterEventItem()")
                RLP_LOG_ERROR("")
                RLP_LOG_ERROR(e.what())
            }
            

        } else{
            RLP_LOG_VERBOSE("dragEnterEventItem() not implemented")
        }

    } else
    {
       event->setAccepted(false);
       QNanoQuickItem::dragEnterEvent(event);
    }
    
}


void
GUI_ItemBase::dragLeaveEvent(QDragLeaveEvent* event)
{
    // RLP_LOG_DEBUG(this)

    if (_pymode)
    {
        PY_Interp::acquireGIL();

        // Try to look up the overridden method on the Python side.
        pybind11::function pyfunc = pybind11::get_override(this, "dragLeaveEventItem");
        if (pyfunc)
        {
            try
            {
                py::object pyevent = py::cast(event);
                pyfunc(pyevent);

            } catch (py::error_already_set &e)
            {

                RLP_LOG_ERROR("")
                RLP_LOG_ERROR("Error running dragEnterEventItem()")
                RLP_LOG_ERROR("")
                RLP_LOG_ERROR(e.what())
            }
            

        } else{
            RLP_LOG_VERBOSE("dragEnterEventItem() not implemented")
        }

    } else
    {
       event->setAccepted(false);
       QNanoQuickItem::dragLeaveEvent(event);
    }
    
}


void
GUI_ItemBase::dropEvent(QDropEvent* event)
{
    // RLP_LOG_DEBUG(this)

    if (_pymode)
    {
        PY_Interp::acquireGIL();

        // Try to look up the overridden method on the Python side.
        pybind11::function pyfunc = pybind11::get_override(this, "dropEventItem");
        if (pyfunc)
        {
            try {
                py::object pyevent = py::cast(event);
                pyfunc(pyevent);

            } catch (py::error_already_set &e) {

                RLP_LOG_ERROR("")
                RLP_LOG_ERROR("Error running dropEventItem()")
                RLP_LOG_ERROR("")
                RLP_LOG_ERROR(e.what())
            }
            

        } else{
            RLP_LOG_VERBOSE("dropEventItem() not implemented")
        }
    }
}


void
GUI_ItemBase::dragMoveEvent(QDragMoveEvent* event)
{
    // RLP_LOG_DEBUG(this)

    event->setAccepted(true);

    GUI_Scene* s = scene();
    if (s == nullptr)
    {
        // RLP_LOG_WARN("NO SCENE")
        return;
    }


    GUI_ItemBase* di = s->dragItem();
    if (di != nullptr)
    {
        QPointF pos = event->position();
        QPointF gpos = mapToItem(s, pos);

        di->setPos(gpos.x(), gpos.y());
    }
}


bool
GUI_ItemBase::dragStarted(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return false;

    if ((event->pos() - _pressPos).manhattanLength()
         < QApplication::startDragDistance())
        return false;

    return true;
}


void
GUI_ItemBase::hoverEnterEvent(QHoverEvent* event)
{
    if (_pymode)
    {
        PY_Interp::acquireGIL();

        // Try to look up the overridden method on the Python side.
        pybind11::function pyfunc = pybind11::get_override(this, "hoverEnterEventItem");
        if (pyfunc)
        {
            try {
                py::object pyevent = py::cast(event);
                pyfunc(pyevent);

            } catch (py::error_already_set &e) {

                RLP_LOG_ERROR("")
                RLP_LOG_ERROR("Error running hoverEnterEventItem()")
                RLP_LOG_ERROR("")
                RLP_LOG_ERROR(e.what())
            }
            

        } else{
            RLP_LOG_VERBOSE("hoverEnterEventItem() not implemented")
        }

    }
}


void
GUI_ItemBase::hoverLeaveEvent(QHoverEvent* event)
{
    if (_pymode)
    {
        PY_Interp::acquireGIL();

        // Try to look up the overridden method on the Python side.
        pybind11::function pyfunc = pybind11::get_override(this, "hoverLeaveEventItem");
        if (pyfunc)
        {
            try {
                py::object pyevent = py::cast(event);
                pyfunc(pyevent);

            } catch (py::error_already_set &e) {

                RLP_LOG_ERROR("")
                RLP_LOG_ERROR("Error running hoverLeaveEventItem()")
                RLP_LOG_ERROR("")
                RLP_LOG_ERROR(e.what())
            }
        } else{
            RLP_LOG_VERBOSE("hoverLeaveEventItem() not implemented")
        }

    }
}


void
GUI_ItemBase::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG(metaObject()->className())

    if (_pymode)
    {
        PY_Interp::acquireGIL();

        // Try to look up the overridden method on the Python side.
        pybind11::function pyfunc = pybind11::get_override(this, "mousePressEventItem");
        if (pyfunc)
        {
            event->setAccepted(true);

            try {
                py::object pyevent = py::cast(event);
                pyfunc(pyevent);

            } catch (py::error_already_set &e) {

                RLP_LOG_ERROR("")
                RLP_LOG_ERROR("Error running mousePressEventItem()")
                RLP_LOG_ERROR("")
                RLP_LOG_ERROR(e.what())
            }
        } else{
            RLP_LOG_VERBOSE("mousePressEventItem() not implemented")
        }

    } else {
        event->setAccepted(false);
    }
}


void
GUI_ItemBase::mouseMoveEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG(metaObject()->className())

    if (_pymode)
    {
        PY_Interp::acquireGIL();

        // Try to look up the overridden method on the Python side.
        pybind11::function pyfunc = pybind11::get_override(this, "mouseMoveEventItem");
        if (pyfunc)
        {
            event->setAccepted(true);

            try {
                py::object pyevent = py::cast(event);
                pyfunc(pyevent);

            } catch (py::error_already_set &e) {

                RLP_LOG_ERROR("")
                RLP_LOG_ERROR("Error running mouseMoveEventItem()")
                RLP_LOG_ERROR("")
                RLP_LOG_ERROR(e.what())
            }
        } else{
            RLP_LOG_VERBOSE("mouseMoveEventItem() not implemented")
        }

    } else {
        event->setAccepted(false);
    }
}


void
GUI_ItemBase::mouseReleaseEvent(QMouseEvent* event)
{
    if (_pymode)
    {
        PY_Interp::acquireGIL();

        // Try to look up the overridden method on the Python side.
        pybind11::function pyfunc = pybind11::get_override(this, "mouseReleaseEventItem");
        if (pyfunc)
        {
            try {
                py::object pyevent = py::cast(event);
                pyfunc(pyevent);

            } catch (py::error_already_set &e) {

                RLP_LOG_ERROR("")
                RLP_LOG_ERROR("Error running mouseReleaseEventItem()")
                RLP_LOG_ERROR("")
                RLP_LOG_ERROR(e.what())
            }
        } else{
            RLP_LOG_VERBOSE("mouseReleaseEventItem() not implemented")
        }

    }
}


void GUI_ItemBase::keyPressEvent(QKeyEvent* event)
{
    if (_pymode)
    {
        PY_Interp::acquireGIL();

        // Try to look up the overridden method on the Python side.
        pybind11::function pyfunc = pybind11::get_override(this, "keyPressEventItem");
        if (pyfunc)
        {
            try {
                py::object pyevent = py::cast(event);
                pyfunc(pyevent);

            } catch (py::error_already_set &e) {

                RLP_LOG_ERROR("")
                RLP_LOG_ERROR("Error running keyPressEventItem()")
                RLP_LOG_ERROR("")
                RLP_LOG_ERROR(e.what())
            }
        } else{
            RLP_LOG_VERBOSE("keyPressEventItem() not implemented")
        }

    }
}


void
GUI_ItemBase::updateItem()
{
    if (_proxy != nullptr)
    {
        _proxy->update();
    }

    update();
}


QRectF
GUI_ItemBase::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


/*
 * test for wasm
 *
bool
GUI_ItemBase::event(QEvent* event)
{
    if (event->isInputEvent())
    {
        QInputEvent* ie = (QInputEvent*)event;
        qInfo() << event << "device:" << ie->device();
    }
    // qInfo() << event;
    return QQuickPaintedItem::event(event);
}
*
*/

void
GUI_ItemBase::paint(GUI_Painter* painter)
{
    if (_pymode)
    {
        // GUI_View handles acquiring the GIL in the render thread
        // we should already have it here
        //
        if (!PyGILState_Check())
        {
            RLP_LOG_ERROR("GIL Not held, cannot dispatch to python")
            return;
        }

        // Try to look up the overridden method on the Python side.
        pybind11::function pypaint = pybind11::get_override(this, "paintItem");
        if (pypaint)
        {
            try {
                if (painter->getPainterType() == GUI_Painter::PAINTER_TYPE_QNANOPAINTER)
                {
                    py::object pypainter = py::cast(static_cast<GUI_NanoPainter*>(painter));
                    pypaint(pypainter);
                } else
                {
                    py::object pypainter = py::cast(static_cast<GUI_QPainter*>(painter));
                    pypaint(pypainter);
                }
                
            } catch (py::error_already_set &e) {

                RLP_LOG_ERROR("")
                RLP_LOG_ERROR("Error running paintItem()")
                RLP_LOG_ERROR("")
                RLP_LOG_ERROR(e.what())
            }
            

        } else{
            // RLP_LOG_DEBUG("paintItem() not implemented")
        }

    }
}


QImage
GUI_ItemBase::exportToImage()
{
    RLP_LOG_DEBUG("")

    QRectF br = boundingRect();

    QImage i(int(br.width()), int(br.height()), QImage::Format_ARGB32);
    i.fill(Qt::transparent);

    QPainter painter(&i);
    QPointF sp = mapToScene(QPointF(0, 0));
    
    QRectF sceneRect(
        sp.x(),
        sp.y(),
        br.width(),
        br.height()
    );

    // scene()->render(&painter, br, sceneRect);

    // i.save("/tmp/node_1.png");
    painter.end();

    return i;
}


void
GUI_ItemBase::setupDragAndDrop(QString label, qreal x, qreal y, QString mimeKey, QString mimeVal)
{
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData();

    RLP_LOG_DEBUG(mimeKey << mimeVal)

    mimeData->setData(mimeKey, QByteArray(mimeVal.toUtf8()));
    drag->setMimeData(mimeData);

    int tw = UTIL_Text::getWidth(label);

    GUI_Scene* s = scene();
    GUI_FrameOverlay* di = new GUI_FrameOverlay(tw + 10, 30, nullptr);
    di->setWidth(tw + 10);
    di->setText(label);
    di->setTextPos(5, 20);
    di->setColour(GUI_Style::BgDarkGrey);
    di->setOpacity(0.9);

    // GUI_Label* l = new GUI_Label(di, lbl);
    di->setPos(x + 5, y + 10);

    s->addItem(di);
    s->setDragItem(di);


    _inDragAndDrop = true;

    Qt::DropAction dropAction = drag->exec(); // Qt::CopyAction | Qt::MoveAction);

    RLP_LOG_DEBUG("Drop action: " << dropAction);
    s->clearDragItem();

    _inDragAndDrop = false;

}


void
GUI_ItemBase::deleteChildren()
{
    RLP_LOG_DEBUG("")

    for (QQuickItem* child : childItems())
    {
        child->deleteLater();
    }
}
