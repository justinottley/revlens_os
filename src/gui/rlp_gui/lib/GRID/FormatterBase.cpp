

#include "RlpGui/GRID/FormatterBase.h"
#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/Cell.h"


RLP_SETUP_LOGGER(gui, GUI_GRID, FormatterBase)

GUI_GRID_FormatterBase::GUI_GRID_FormatterBase()
{
    _editIcon = new GUI_SvgIcon(":misc/edit.svg", nullptr, 20);
    _editIcon->setBgColour(QColor(200, 200, 200));
    _editIcon->setFgColour(QColor(240, 140, 140));

    _copyIcon = new GUI_SvgIcon(":feather/lightgrey/copy.svg", nullptr, 20);
    _copyIcon->setBgColour(QColor(200, 200, 200));
    _copyIcon->setFgColour(QColor(240, 140, 140));

}


void
GUI_GRID_FormatterBase::paintHeaderCell(GUI_GRID_Cell* cell, GUI_Painter* painter)
{
    

    // if (!cell->hasValue("name")) {
    //     return;
    // }
    
    // qInfo() << "paintCell()";
    
    // qInfo() << "entity formatter";
    // qInfo() << props;


    
    QString val = cell->colType()->displayName();
    
    // qInfo() << val;

    // int pos = props.value("xpos").toInt();
    // int width = props.value("width").toInt();
    // int height = 20;
    
    painter->setPen(Qt::black);
    painter->setBrush(QBrush(GUI_Style::BgLoMidGrey));
    painter->drawRect(cell->boundingRect());

    painter->setPen(Qt::white);
    painter->drawText(
        5,
        15,
        val
    );
}



void
GUI_GRID_FormatterBase::paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter)
{   
}


GUI_ItemBase*
GUI_GRID_FormatterBase::makeDragItem(GUI_GRID_Cell* cell)
{
    RLP_LOG_ERROR("SHOULD NOT REACH HERE")

    return nullptr;
}