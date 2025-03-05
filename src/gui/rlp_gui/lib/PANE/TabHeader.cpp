
#include "RlpGui/PANE/TabHeader.h"

#include "RlpGui/PANE/SvgButton.h"
#include "RlpGui/PANE/PaneHeader.h"
#include "RlpGui/PANE/Pane.h"
#include "RlpGui/PANE/View.h"

#include "RlpCore/UTIL/Text.h"

#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/BASE/FrameOverlay.h"
#include "RlpGui/BASE/ToolInfo.h"


RLP_SETUP_LOGGER(gui, GUI, TabHeader)

GUI_TabHeader::GUI_TabHeader(QString name, GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _name(name),
    _active(false),
    _inHover(false),
    _first(false),
    _highlightDrop(false)
{
    setSize(QSizeF(100, 25));
    setFlag(QQuickItem::ItemAcceptsDrops);
    setAcceptHoverEvents(true);

    int textWidth = UTIL_Text::getWidth(name);

    if (textWidth > 60) {
        setWidth(textWidth + 60);
    }

    // Q_INIT_RESOURCE(core_GUI_resources);

    _btnClose = new GUI_SvgButton(
        ":/feather/lightgrey/x-circle.svg",
        this
    );
    _btnClose->icon()->setBgColour(GUI_Style::BgLightGrey);
    _btnClose->setPos(width() - _btnClose->width() - 4, 3);
    
    connect(
        _btnClose,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_TabHeader::onCloseButtonPressed
    );

}


GUI_ItemBase*
GUI_TabHeader::makeDragItem()
{
    int tw = UTIL_Text::getWidth(_name);

    GUI_FrameOverlay* di = new GUI_FrameOverlay(tw + 20, 30, nullptr);
    di->setColour(GUI_Style::BgDarkGrey);
    di->setOpacity(0.9);
    di->setText(_name);
    di->setOutlined(true);

    return di;
}


void
GUI_TabHeader::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_INFO("")
    _pressPos = event->pos();

    emit tabActivateRequested(_name);
}


void
GUI_TabHeader::mouseMoveEvent(QMouseEvent* event)
{
    RLP_LOG_DEBUG("")

    if (!dragStarted(event))
        return;

    GUI_PaneHeader* ph = qobject_cast<GUI_PaneHeader*>(parentItem());
    GUI_Pane* pane = ph->pane();

    GUI_TabPtr tab = pane->getTab(_name);
    if (tab == nullptr)
    {
        return;
    }

    QDrag* drag = new QDrag(this);

    GUI_ItemBase* di = makeDragItem();
    QVariant tw;
    tw.setValue(tab);
    di->setProperty("tab", tw);

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(
        QString("text/tab.name"),
        QVariant(_name).toByteArray()
    );

    // RLP_LOG_DEBUG(_cellData.value("mimeType").toString() << " --> " << QString(di->metadataValue("mimeData").toByteArray()))
    drag->setMimeData(mimeData);


    GUI_Scene* s = scene();
    s->addItem(di);
    s->setDragItem(di);

    QPointF pos = event->position();
    QPointF gpos = mapToItem(s, pos);

    di->setPos(gpos.x(), gpos.y());

    // Execute drag/drop action
    //
    Qt::DropAction da = drag->exec();


    s->clearDragItem();
    _highlightDrop = false;

    updateItem();

    if (da == Qt::MoveAction)
    {
        pane->removeTab(_name);
    }
}


void
GUI_TabHeader::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _inHover = true;
    update();
}


void
GUI_TabHeader::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _inHover = false;
    update();
}


void
GUI_TabHeader::dragEnterEvent(QDragEnterEvent* event)
{
    // RLP_LOG_DEBUG("")

    
    event->setAccepted(true);

    if (event->mimeData()->hasFormat("text/tab.name"))
    {
        _highlightDrop = true;
        update();
    }

}


void
GUI_TabHeader::dragLeaveEvent(QDragLeaveEvent* event)
{
    // RLP_LOG_DEBUG("")

    _highlightDrop = false;
    update();
}

void
GUI_TabHeader::onCloseButtonPressed(QVariantMap /* metadata */)
{
    RLP_LOG_INFO("")

    emit closeTab(_name, false);
}


void
GUI_TabHeader::dropEvent(QDropEvent *event)
{
    RLP_LOG_DEBUG("")
    _highlightDrop = false;
    GUI_ItemBase* dragItem = scene()->dragItem();

    QString tabName = event->mimeData()->data("text/tab.name");

    if (dragItem != nullptr)
    {
        QVariant tabW = dragItem->property("tab");
        if (tabW.isValid())
        {
            GUI_TabPtr tab = tabW.value<GUI_TabPtr>();
            GUI_PaneHeader* ph = qobject_cast<GUI_PaneHeader*>(parentItem());
            GUI_Pane* pane = ph->pane();

            if (tab->parentItem() != pane->body())
            {
                // Check that this tool can be moved to another pane
                GUI_Scene* gscene = scene();
                GUI_View* gview = qobject_cast<GUI_View*>(gscene->view());

                GUI_ToolInfo* toolInfo = gview->tool(tabName);
                QVariantMap toolData = toolInfo->data();

                if (toolData.contains("movable"))
                {
                    bool isMovable = toolData.value("movable").toBool();
                    if (!isMovable)
                    {
                        RLP_LOG_DEBUG("Tool not movable, aborting move")
                        return;
                    }
                }

                event->setAccepted(true);
                event->setDropAction(Qt::MoveAction);

            } else
            {
                event->setAccepted(true);
                event->setDropAction(Qt::IgnoreAction);
            }

            emit tabMoveRequested(_name, tabName, tabW);
        }
    }
}


QRectF
GUI_TabHeader::boundingRect() const
{
    return QRectF(1, 1, width() - 2, height());
}


void
GUI_TabHeader::paint(GUI_Painter* painter)
{

    QColor bgcol = GUI_Style::BgDarkGrey;
    if (_inHover)
    {
        bgcol = GUI_Style::BgMidGrey;
    }

    painter->setPen(bgcol);
    painter->setBrush(bgcol);

    QRectF br = boundingRect();

    painter->drawRect(br);


    if (_first)
    {
        QRectF br1 = br.adjusted(-1, 0, 0, 0);
        QPen p(Qt::black); // bgcol);
        p.setWidth(1);
        painter->setPen(p);
        painter->drawLine(br1.topLeft(), br1.bottomLeft());
    }

    painter->setPen(Qt::black);
    painter->drawLine(br.topRight(), br.bottomRight());

    if (!_active)
    {
        br.adjust(0, 0, 0, -1);
        QPen p(Qt::black);
        p.setWidth(2);
        painter->setPen(p); // Qt::black);
        painter->drawLine(br.bottomLeft(), br.bottomRight());
    }

    if (_highlightDrop)
    {
        QPen p(Qt::red);
        p.setWidth(5);
        painter->setPen(p);
        painter->drawLine(br.topLeft(), br.bottomLeft());
    }
    painter->setPen(Qt::white);
    painter->drawText(15, 17, _name);  
}
    