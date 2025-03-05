
#include "RlpGui/GRID/CellFormatterDate.h"

#include "RlpGui/GRID/Cell.h"


RLP_SETUP_LOGGER(gui, GUI_GRID, CellFormatterDate)

GUI_GRID_CellFormatterDate::GUI_GRID_CellFormatterDate(QString keyName, int xoffset, int yoffset):
    GUI_GRID_CellFormatterStr(keyName, xoffset, yoffset)
{
}


void
GUI_GRID_CellFormatterDate::paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter)
{

    int xoffset = 5;
    if (cell->doIndent())
    {
        xoffset += (cell->treeDepth() * 15);
        if (cell->hasChildren())
        {
            xoffset += 15; //  + (cell->ltreeDepth() * 15);
        }
    }
    
    xoffset += _xoffset;

    painter->setFont(_font);
    
    QString val = "-";
    if (!cell->hasValue("date_str"))
    {
        float dateVal = cell->value(_keyName).toFloat();
        val = QDateTime::fromSecsSinceEpoch(dateVal).toString();
        cell->insertValue("date_str", val);

    } else {
        val = cell->value("date_str").toString();
    }


    QColor col = Qt::white;

    painter->setPen(col);
    painter->drawText(
        xoffset,
        15 + _yoffset,
        val
    );


}