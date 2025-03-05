
#include "RlpGui/GRID/CellFormatterFolder.h"
#include "RlpGui/GRID/Cell.h"
#include "RlpGui/GRID/Row.h"
#include "RlpCore/DS/ModelDataItem.h"

GUI_GRID_CellFormatterFolder::GUI_GRID_CellFormatterFolder()
{
    _image = QImage(":feather/lightgrey/folder.png").scaledToWidth(20, Qt::SmoothTransformation);
    _fmtText = new GUI_GRID_CellFormatterStr("value", 35, 5);
}


GUI_ItemBase*
GUI_GRID_CellFormatterFolder::makeDragItem(GUI_GRID_Cell* cell)
{
    GUI_ItemBase* item = _fmtText->makeDragItem(cell);
    CoreDs_ModelDataItemPtr rd = cell->row()->rowData();

    QString root = rd->value("__rootDir__").toString();
    QString fileName = rd->value("name").toString();
    QString path = root + "/" + fileName;

    RLP_LOG_DEBUG(path)

    item->setMetadata("mimeData", QByteArray(path.toUtf8()));

    return item;
}


void
GUI_GRID_CellFormatterFolder::paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter)
{
    int xoffset = 10 + (cell->treeDepth() * 15);

    if (cell->hasChildren()) {
        xoffset += 15; //  + (cell->ltreeDepth() * 15);

        painter->drawImage(
            xoffset,
            4,
            &_image
        );
    }


    _fmtText->paintCell(cell, painter);
}

