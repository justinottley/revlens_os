
#include "RlpCore/UTIL/Text.h"

#include "RlpGui/BASE/Style.h"
#include "RlpGui/GRID/ColType.h"
#include "RlpGui/GRID/Cell.h"
#include "RlpGui/GRID/CellFormatterEntity.h"

RLP_SETUP_LOGGER(gui, GUI_GRID, CellFormatterEntity)

GUI_GRID_CellFormatterEntity::GUI_GRID_CellFormatterEntity()
{
    
}



void
GUI_GRID_CellFormatterEntity::paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter)
{
    bool inHE = cell->inHoverEdit();
    bool inEdit = cell->inEdit();

    if (inEdit)
    {
        return;
    }

    if (!cell->hasValue("value"))
    {
        if (inHE)
        {
            painter->drawImage(cell->width() - 25, 6, _editIcon->fgImage());
        }
        
        // RLP_LOG_WARN("Invalid data, cannot paint" << cell->values())
        return;
    }



    QVariantMap etInfo = cell->value("value").toMap();

    // RLP_LOG_DEBUG(etInfo)

    QString val = QString("[%1] ").arg(etInfo.value("et_name").toString());
    if (etInfo.contains("name"))
    {
        val += etInfo.value("name").toString();
    } else
    {
        val += etInfo.value("uuid").toUuid().toString();
    }


    /*
    QString val = "{ " +
                  cell->colType()->displayName() + " \"" +
                  cell->value("value").toString() + "\""
                  " }";
    */

    // int pos = props.value("xpos").toInt();
    // int width = props.value("width").toInt();
    // int height = 20;
    
    

    int textWidth = UTIL_Text::getWidth(val);

    QRectF textBg(4, 4, textWidth + 15, 20);

    QColor bgcol = GUI_Style::BgLightGrey;
    QColor fgcol = Qt::white;



    if (inHE)
    {
        bgcol = GUI_Style::BgDarkGrey;
        fgcol = GUI_Style::BgLightGrey;
    }

    QBrush b(bgcol);

    painter->setPen(bgcol); // GUI_Style::BgLightGrey);
    painter->setBrush(b);
    painter->drawRect(textBg);

    painter->setPen(fgcol);
    painter->drawText(
        5,
        20,
        val
    );
    
    if (inHE)
    {
        painter->drawImage(cell->width() - 25, 6, _editIcon->fgImage());
    }
}
