
#include "RlpGui/BASE/PaintedItemBase.h"
#include "RlpGui/BASE/ViewBase.h"
#include "RlpGui/BASE/FrameBase.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/BASE/Style.h"

#include "RlpCore/PY/Interp.h"

RLP_SETUP_LOGGER(gui, GUI, PaintedItemBase)


GUI_PaintedItemBase::GUI_PaintedItemBase(QQuickItem* parent):
    QQuickPaintedItem(parent),
    _toolTipText(""),
    _inHover(false),
    _scene(nullptr),
    _paintBg(true),
    _pymode(false),
    _heightOffset(0),
    _widthOffset(0)
{
    // test for wasm
    // setAcceptTouchEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);

    connect(this, &GUI_PaintedItemBase::widthChanged, this, &GUI_PaintedItemBase::onSizeChangedInternal);
    connect(this, &GUI_PaintedItemBase::heightChanged, this, &GUI_PaintedItemBase::onSizeChangedInternal);

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
    



    //RLP_LOG_DEBUG(parent)

    /*
    if (parent != nullptr)
    {
        
        RLP_LOG_DEBUG("pulling scene:" << parent->scene())
        

        //connect(parent, &GUI_PaintedItemBase::sizeChanged, this, &GUI_PaintedItemBase::onParentSizeChanged);
    }
    */
}


GUI_PaintedItemBase::~GUI_PaintedItemBase()
{
    if (_pymode)
    {
        RLP_LOG_WARN("DESTRUCTING IN PYMODE")
        PY_Interp::acquireGIL();

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


GUI_PaintedItemBase*
GUI_PaintedItemBase::create(GUI_ItemBase* parent, QVariantMap toolInfo)
{
    RLP_LOG_INFO(toolInfo)

    GUI_PaintedItemBase* iresult = nullptr;

    if (toolInfo.contains("pyitem"))
    {

        QString callback = toolInfo.value("pyitem").toString();

        RLP_LOG_INFO("Creating pyitem: " << callback)
        

        // QStringList funcParts = callback.split(".");
        // QStringList funcNsParts;
        
        // for (int i=0; i < (funcParts.size() - 1); ++i) {
        //     funcNsParts.append(funcParts.at(i));
        // }
        
        // QString funcNs = funcNsParts.join('.');
        // QString widgetCls = funcParts.at(funcParts.size() - 1);


        // QString widgetCreate = callback;
        // widgetCreate += ".create";
        //
        // RLP_LOG_DEBUG(importStr)
        // RLP_LOG_DEBUG(widgetCreate)
        //
        // QVariantList argInputList;
        // QVariant pv;
        // pv.setValue(parent);
        //
        // argInputList.append(pv);

        // QVariant return_result = PY_Interp::call(widgetCreate, argInputList);
        
        PY_Interp::acquireGIL();
        {
            py::object cbMod = py::module_::import(callback.toLocal8Bit().constData());
            py::object cbObj = cbMod.attr("create");
            py::object param = py::cast(parent);

            py::object result = cbObj(param);

            if (!result.is(py::none()))
            {
                iresult = result.cast<GUI_PaintedItemBase*>();
            }
        }

        PY_Interp::releaseGIL();

    } else
    {
        RLP_LOG_ERROR("invalid tool info")
    }

    return iresult;
}


void
GUI_PaintedItemBase::setScene(GUI_Scene* scene)
{
    // RLP_LOG_DEBUG(scene)

    _scene = scene;
}


void
GUI_PaintedItemBase::setPos(qreal xpos, qreal ypos)
{
    QString clsname = metaObject()->className();

    /*
    if ((clsname == "GUI_PaneBody") ||
        (clsname == "GUI_Pane") ||
        (clsname == "GUI_PaneHeader") ||
        (clsname == "GUI_PaneSplitter"))
    {
        RLP_LOG_DEBUG(metaObject()->className() << xpos << ypos)
    }
    */

    setX(xpos);
    setY(ypos);
}


void
GUI_PaintedItemBase::setZValue(qreal zval)
{
    setZ(zval);
}


void
GUI_PaintedItemBase::setPaintBackground(bool doPaintBg)
{
    // RLP_LOG_DEBUG(this << doPaintBg << parentItem())

    
    _paintBg = doPaintBg;

    if (parentItem() != nullptr)
    {
        QString clsName = parentItem()->metaObject()->className();
        if (clsName != "GUI_Scene")
        {
            qobject_cast<GUI_PaintedItemBase*>(parentItem())->setPaintBackground(doPaintBg);
        }
    }

    update();
}


void
GUI_PaintedItemBase::setPyMode(bool pymode)
{
    // RLP_LOG_DEBUG(pymode)

    _pymode = pymode;
}


void
GUI_PaintedItemBase::onSizeChangedInternal()
{
    // RLP_LOG_DEBUG(metaObject()->className() << width() << height())

    emit sizeChanged(width(), height());
}


void
GUI_PaintedItemBase::onParentSizeChanged(qreal width, qreal height)
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
GUI_PaintedItemBase::onParentVisibilityChanged(QString name, bool isVisible)
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
            qobject_cast<GUI_PaintedItemBase*>(parentItem())->setPaintBackground(true);
            // // qobject_cast<GUI_PaintedItemBase*>(parentItem())->setPaintBackground(false);
        }
    } else {
        qobject_cast<GUI_PaintedItemBase*>(parentItem())->setPaintBackground(true);
    }

}


void
GUI_PaintedItemBase::checkForToolTip()
{
    // RLP_LOG_DEBUG(_inHover)

    if (!_inHover) return;

    QTime now = QTime::currentTime();
    int diff = _moveTime.msecsTo(now);

    // RLP_LOG_DEBUG("time diff:" << diff)

    if (diff > 300)
    {
        showToolTip(_movePos);
    }
}


void
GUI_PaintedItemBase::delayedCheckForToolTip()
{
    QTimer::singleShot(500, this, &GUI_PaintedItemBase::checkForToolTip);
}


void
GUI_PaintedItemBase::showToolTip(QPointF pos)
{
    if (_toolTipText.size() == 0) return;

    RLP_LOG_DEBUG(_toolTipText << pos)

    QPointF spos = mapToScene(pos);

    GUI_Scene* s = qobject_cast<GUI_Scene*>(scene());
    s->showToolTip(
        _toolTipText,
        spos.x() + 20,
        spos.y() + 10
    );

}


void
GUI_PaintedItemBase::clearToolTip()
{
    GUI_Scene* s = qobject_cast<GUI_Scene*>(scene());
    s->hideToolTip();
}



void
GUI_PaintedItemBase::dragEnterEvent(QDragEnterEvent* event)
{
    RLP_LOG_DEBUG(this)

    event->setAccepted(true);
}


void
GUI_PaintedItemBase::dragMoveEvent(QDragMoveEvent* event)
{
    // RLP_LOG_DEBUG("")

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
GUI_PaintedItemBase::dragStarted(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return false;

    if ((event->pos() - _pressPos).manhattanLength()
         < QApplication::startDragDistance())
        return false;

    return true;
}


void
GUI_PaintedItemBase::hoverEnterEvent(QHoverEvent* event)
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
GUI_PaintedItemBase::hoverLeaveEvent(QHoverEvent* event)
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
GUI_PaintedItemBase::mousePressEvent(QMouseEvent* event)
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
GUI_PaintedItemBase::mouseMoveEvent(QMouseEvent* event)
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
GUI_PaintedItemBase::mouseReleaseEvent(QMouseEvent* event)
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


void GUI_PaintedItemBase::keyPressEvent(QKeyEvent* event)
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
GUI_PaintedItemBase::updateItem()
{

    update();
}


QRectF
GUI_PaintedItemBase::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


/*
 * test for wasm
 *
bool
GUI_PaintedItemBase::event(QEvent* event)
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
GUI_PaintedItemBase::paint(QPainter* painter)
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

                py::object pypainter = py::cast(painter);
                pypaint(pypainter);

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
GUI_PaintedItemBase::exportToImage()
{
    RLP_LOG_DEBUG("")

    QRectF br = boundingRect();

    // QPdfWriter writer("/tmp/test.pdf");
    // writer.setPageSize(QPageSize(QSize(br.width(), br.height()))); // QPageSize(QPageSize::A4)); // QPagedPaintDevice::A4);
    // writer.setPageMargins(QMargins(30, 30, 30, 30));

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
GUI_PaintedItemBase::deleteChildren()
{
    RLP_LOG_DEBUG("")

    for (QQuickItem* child : childItems())
    {
        child->deleteLater();
    }
}
