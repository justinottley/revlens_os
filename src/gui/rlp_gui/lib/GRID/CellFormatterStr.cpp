
#include "RlpGui/GRID/CellFormatterStr.h"

#include "RlpGui/BASE/Style.h"

#include "RlpGui/GRID/Cell.h"
#include "RlpGui/GRID/Row.h"

#include "RlpCore/UTIL/Text.h"


RLP_SETUP_LOGGER(gui, GUI_GRID, CellFormatterStr)

GUI_GRID_CellFormatterStr::GUI_GRID_CellFormatterStr(QString keyName, int xoffset, int yoffset):
    _keyName(keyName),
    _xoffset(xoffset),
    _yoffset(yoffset),
    _font(QApplication::font())
{
}


QString
GUI_GRID_CellFormatterStr::getValue(GUI_GRID_Cell* cell)
{
    QString val;
    QVariant value = cell->value(_keyName);
    QString typeStr = value.typeName();


    if (typeStr == "QString")
    {
        val = value.toString();

    }
    else if (typeStr == "int")
    {
        val = "%1";
        val = val.arg(value.toInt());
    }
    else if (typeStr == "qlonglong")
    {
        val = "%1";
        val = val.arg(value.toLongLong());
    }
    else if (
        (typeStr == "float") ||
        (typeStr == "double")
    )
    {
        val = "%1";
        val = val.arg(value.toDouble());
    }

    return val;
}


QVariantMap
GUI_GRID_CellFormatterStr::getMetaValues(GUI_GRID_Cell* cell)
{

    QVariantMap result;
    QString colName = cell->colName();

    // RLP_LOG_DEBUG(colName)

    if (cell->row()->rowData()->containsItem(colName))
    {
        result = cell->row()->rowData()->item(colName)->values();
    }

    return result;
}


QImage
GUI_GRID_CellFormatterStr::getImage(QString name)
{
    QString longName = QString(":feather/%1.svg").arg(name);

    // RLP_LOG_DEBUG(longName)

    if (_iconMap.contains(longName))
    {
        return _iconMap.value(longName);
    }

    QImage im = GUI_SvgIcon::loadSvgFromPath(longName, 20, QColor(200, 200, 200));
    _iconMap.insert(longName, im);
    
    return im;
}


void
GUI_GRID_CellFormatterStr::hoverMoveEventInternal(GUI_GRID_Cell* cell, QPointF pos)
{
    if (!cell->colType()->hasMetadataValue("selectable"))
    {
        return;
    }


    QString val = getValue(cell);
    int textWidth = UTIL_Text::getWidth(val) + 6;

    if ((pos.x() <= textWidth) && (pos.y() >= 5) && (pos.y() <= 30))
    {
        cell->setCursor(Qt::PointingHandCursor);
        cell->setInItemSelect(true);

    } else
    {
        cell->setCursor(Qt::ArrowCursor);
        cell->setInItemSelect(false);
    }
}


void
GUI_GRID_CellFormatterStr::paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter)
{
    // RLP_LOG_DEBUG(cell->values())
    // return;

    bool inHE = cell->inHoverEdit();
    bool inEdit = cell->inEdit();

    if (inEdit)
    {
        return;
    }

    if (!cell->hasValue(_keyName))
    {
        // RLP_LOG_DEBUG("NO VALUE FOR" << _keyName)
        if (inHE)
        {
            painter->drawImage(cell->width() - 25, 6, _editIcon->fgImage());
        }

        // RLP_LOG_WARN("NO VALUE FOR" << _keyName << "values:" << cell->values())
        return;
    }

    QString val = getValue(cell);
    QVariantMap metaVals = getMetaValues(cell);

    int xoffset = 5;
    if (cell->doIndent())
    {
        xoffset += (cell->treeDepth() * 15);
        if (cell->hasChildren())
        {
            xoffset += 15;
        }
    }
    
    xoffset += _xoffset;
    
    if (metaVals.contains("icon"))
    {
        // RLP_LOG_DEBUG("GOT ICON");
        QImage im = getImage(metaVals.value("icon").toString());
        painter->drawImage(xoffset - 25, 4, &im);
    }

    if (metaVals.contains("style"))
    {
        QFont f = painter->font();

        QVariantMap styleInfo = metaVals.value("style").toMap();
        if (styleInfo.contains("bold"))
        {
            f.setBold(true);
        }

        painter->setFont(f);

        // if (styleInfo.contains("underline"))
        // {
        //     int textWidth = UTIL_Text::getWidth(val);
        //     painter->setPen(GUI_Style::FgGrey);
        //     painter->drawLine(xoffset, 15 + _yoffset + 6, textWidth + 6, 15 + _yoffset + 6);
        // }
    }

    if (cell->colType()->hasMetadataValue("selectable"))
    {

        QFont f = painter->font();
        f.setBold(true);
        painter->setFont(f);

        int textWidth = UTIL_Text::getWidth(val);
        painter->setPen(GUI_Style::FgGrey);
        painter->drawLine(xoffset, 15 + _yoffset + 6, textWidth + 6, 15 + _yoffset + 6);
    }


    QColor col = Qt::white;
    if (inHE)
    {
        col = GUI_Style::BgLightGrey;
    }
    painter->setPen(col);
    painter->drawText(
        xoffset,
        15 + _yoffset,
        val
    );


    if (inHE)
    {
        painter->drawImage(cell->width() - 25, 6, _editIcon->fgImage());
        if (val != "")
        {
            painter->drawImage(cell->width() - 50, 6, _copyIcon->fgImage());
        }
    }
}
