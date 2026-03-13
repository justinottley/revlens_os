
#include "RlpRevlens/GUI/RowMediaOverlay.h"

#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/View.h"

#include "RlpCore/UTIL/Convert.h"
#include "RlpCore/PY/Interp.h"

RLP_SETUP_LOGGER(revlens, GUI, RowMediaOverlay)
RLP_SETUP_LOGGER(revlens, GUI, RowMediaWidgetPlugin)


const int GUI_RowMediaOverlay::INSERT_POS_ABOVE = 0;
const int GUI_RowMediaOverlay::INSERT_POS_BELOW = 1;

GUI_RowMediaOverlay::GUI_RowMediaOverlay(GUI_GRID_Row* row):
    GUI_GRID_RowWidget(row),
    _validDragEnter(false),
    _text(""),
    _inInsertAbove(false),
    _inInsertBelow(false),
    _inComposite(false)
{
    setFlag(QQuickItem::ItemAcceptsDrops);
    setZ(5);
    setOpacity(0.8);
    setWidth(row->width());
    setHeight(row->height());

    _font = QApplication::font();
    _font.setBold(true);
    _font.setPixelSize(20);

    _insertAboveButton = new GUI_SvgIcon(":feather/arrow-up.svg", this, 40);
    _insertAboveButton->setPos(40, 5);

    _insertBelowButton = new GUI_SvgIcon(":feather/arrow-down.svg", this, 40);
    _insertBelowButton->setPos(90, 5);

    _compositeButton = new GUI_SvgIcon(":misc/compare.svg", this, 40);
    _compositeButton->setPos(140, 5);

    reset();
}


void
GUI_RowMediaOverlay::reset()
{
    _validDragEnter = false;

    _text = "";

    _insertAboveButton->setVisible(false);
    _insertBelowButton->setVisible(false);
    _compositeButton->setVisible(false);

    _inInsertAbove= false;
    _inInsertBelow = false;
    _inComposite = false;
}


void
GUI_RowMediaOverlay::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/filename") ||
        event->mimeData()->hasFormat("text/plain") ||
        event->mimeData()->hasFormat("text/uri-list") ||
        event->mimeData()->hasFormat("text/json"))
    {
        event->setAccepted(true);

        _validDragEnter = true;
        _insertAboveButton->setVisible(true);
        _insertBelowButton->setVisible(true);
        _compositeButton->setVisible(true);

    } else
    {
        reset();
    }

    update();
}


void
GUI_RowMediaOverlay::dragLeaveEvent(QDragLeaveEvent* event)
{
    // RLP_LOG_DEBUG("")

    reset();
    update();
}


void
GUI_RowMediaOverlay::dragMoveEvent(QDragMoveEvent* event)
{
    GUI_GRID_RowWidget::dragMoveEvent(event);

    int xpos = event->position().x();

    _insertAboveButton->setHover(false);
    _insertBelowButton->setHover(false);
    _compositeButton->setHover(false);

    _inInsertAbove = false;
    _inInsertBelow = false;
    _inComposite = false;

    if ((xpos >= 40) && (xpos <= 85))
    {
        _insertAboveButton->setHover(true);
        _text = "Insert Above";
        _inInsertAbove = true;
    }

    else if ((xpos >= 90) && (xpos <= 135))
    {
        _inInsertBelow = true;
        _insertBelowButton->setHover(true);
        _text = "Insert Below";
    }

    else if ((xpos >= 140) && (xpos <= 185))
    {
        _inComposite = true;
        _compositeButton->setHover(true);
        _text = "Add to Composite";
    } else 
    {
        _text = "";
    }

    _insertAboveButton->update();
    _insertBelowButton->update();
    _compositeButton->update();

    update();
}


void
GUI_RowMediaOverlay::addToComposite(QDropEvent* event)
{
    RLP_LOG_DEBUG(event)
    const QMimeData* md = event->mimeData();

    QString path;

    if (md->hasFormat("text/json"))
    {
        QVariantMap payload = UTIL_Convert::fromJsonString(event->mimeData()->data("text/json"));
        // LOG_Logging::pprint(payload);
        
        if (payload.contains("data"))
        {
            QVariantMap data = payload.value("data").toMap();
            

            if (data.contains("sg_path_to_frames"))
            {
                path = data.value("sg_path_to_frames").toString();

                RLP_LOG_DEBUG("GOT PATH:" << path)
            } else  if (data.contains("source_info"))
            {
                QVariantMap sourceInfo = data.value("source_info").toMap();
                path = sourceInfo.value("media.video").toString();

                RLP_LOG_DEBUG("GOT PATH:" << path)
            }

        }
    }


    if (path != "")
    {
        QVariantMap values = _row->rowData()->values();
        QString nodeUuid = _row->modelValue("uuid").toString();

        QVariantList argList;
        argList.append(path);
        argList.append(nodeUuid);

        PY_Interp::call("revlens.media.append_to_composite", argList);
    }
}


void
GUI_RowMediaOverlay::insertMedia(QDropEvent* event, int pos)
{

    RLP_LOG_DEBUG(event)
    const QMimeData* md = event->mimeData();

    QString path;

    if (md->hasFormat("text/json"))
    {
        QVariantMap payload = UTIL_Convert::fromJsonString(event->mimeData()->data("text/json"));
        // LOG_Logging::pprint(payload);

        if (payload.contains("data"))
        {
            QVariantMap data = payload.value("data").toMap();
            
            int insertIdx = _row->rowIdx();
            if ((pos == GUI_RowMediaOverlay::INSERT_POS_BELOW) &&
                (insertIdx < (_row->view()->rowCount() - 1)))
            {
                insertIdx++;
            }

            QString nodeUuid = _row->modelValue("uuid").toString();

            RLP_LOG_DEBUG(insertIdx << nodeUuid)
            
            QVariantMap md;
            md.insert("at_idx", insertIdx);
            md.insert("data", data);

            emit _row->view()->cellWidgetEvent("row.insert_requested", md);
        }
    }
    
}


void
GUI_RowMediaOverlay::dropEvent(QDropEvent *event)
{

    if (_inComposite)
    {
        addToComposite(event);

    } else if (_inInsertAbove)
    {
        insertMedia(event, GUI_RowMediaOverlay::INSERT_POS_ABOVE);

    } else if (_inInsertBelow)
    {
        insertMedia(event, GUI_RowMediaOverlay::INSERT_POS_BELOW);
    }

    reset();
    update();
}


void
GUI_RowMediaOverlay::paint(GUI_Painter* painter)
{
    QPen pw(Qt::white);
    pw.setWidth(4);

    if (_validDragEnter)
    {
        painter->setPen(pw);
        painter->setBrush(QBrush(QColor(80, 80, 120)));

        painter->drawRoundedRect(boundingRect(), 10, 10);
        
        painter->setBrush(QBrush(Qt::white));
        painter->setFont(_font);
        painter->drawText(250, 32, _text);
    }
    // painter->drawText(10, 10, "HELLO WORLD");
}


// ----


GUI_RowMediaWidgetPlugin::GUI_RowMediaWidgetPlugin():
    GUI_GRID_RowWidgetPlugin("media")
{
}


void
GUI_RowMediaWidgetPlugin::initRowWidget(GUI_GRID_Row* row, QVariantMap widgetInfo)
{
    GUI_RowMediaOverlay* rmo = new GUI_RowMediaOverlay(row);
    row->addRowWidget("media", rmo);
}