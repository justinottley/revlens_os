

#include "RlpGui/GRID/RowExpanderItem.h"
#include "RlpGui/BASE/Style.h"


RLP_SETUP_LOGGER(gui, GUI_GRID, RowExpanderItem)

GUI_GRID_RowExpanderItem::GUI_GRID_RowExpanderItem(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _size(16),
    _expanded(false)
{
    setWidth(_size);
    setHeight(_size);

    _polyCollapsed << QPoint(2, 2) << QPoint(12, 8) << QPoint(2, 14);
    _polyExpanded << QPoint(0, 4) << QPoint(12, 4) << QPoint(6, 14);

    _imgCollapsed = QImage(":feather/lightgrey/chevron-right.png").scaledToWidth(20, Qt::SmoothTransformation);
    _imgExpanded = QImage(":feather/lightgrey/chevron-down.png").scaledToWidth(20, Qt::SmoothTransformation);

}


QRectF
GUI_GRID_RowExpanderItem::boundingRect() const
{
    return QRectF(0, 0, _size, _size);
}


void
GUI_GRID_RowExpanderItem::mousePressEvent(QMouseEvent* event)
{
    RLP_LOG_DEBUG("")

    _expanded = !_expanded;
    emit triggered();

    update();
}


void
GUI_GRID_RowExpanderItem::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")
    // painter->setPen(Qt::red);
    // painter->drawRect(boundingRect());

    painter->save();

    // painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setBrush(GUI_Style::FgGrey);

    if (_expanded) {
        // painter->drawPolygon(_polyExpanded);
        painter->drawImage(
            1, 1, &_imgExpanded
        );

    } else {
        // painter->drawPolygon(_polyCollapsed);
        painter->drawImage(
            1, 1, &_imgCollapsed
        );
    }
    

    painter->restore();
}