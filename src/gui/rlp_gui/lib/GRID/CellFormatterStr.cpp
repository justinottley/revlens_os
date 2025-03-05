
#include "RlpGui/GRID/CellFormatterStr.h"

#include "RlpCore/UTIL/Text.h"

#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/FrameOverlay.h"

#include "RlpGui/WIDGET/Label.h"

#include "RlpGui/GRID/Cell.h"
#include "RlpGui/GRID/Row.h"


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
GUI_GRID_CellFormatterStr::getMetaValues(GUI_GRID_Cell* cell, QString name)
{
    // RLP_LOG_DEBUG(name)
    QVariantMap result;
    if (cell->row()->rowData()->containsItem(name))
    {
        RLP_LOG_DEBUG("  FOUND" << name)
        result = cell->row()->rowData()->item(name)->values();
    }

    return result;
}


QImage
GUI_GRID_CellFormatterStr::getImage(QString name)
{
    QString longName = QString(":feather/lightgrey/%1.svg").arg(name);
    RLP_LOG_DEBUG(longName)

    if (_iconMap.contains(longName))
    {
        return _iconMap.value(longName);
    }

    QImage im = GUI_SvgIcon::loadSvgFromPath(longName, 20, QColor(200, 200, 200));
    _iconMap.insert(longName, im);
    
    return im;
}


GUI_ItemBase*
GUI_GRID_CellFormatterStr::makeDragItem(GUI_GRID_Cell* cell)
{
    RLP_LOG_DEBUG("")

    QString val = getValue(cell);
    int tw = UTIL_Text::getWidth(val);

    GUI_FrameOverlay* di = new GUI_FrameOverlay(tw + 10, 30, nullptr);
    di->setColour(GUI_Style::BgDarkGrey);
    di->setOpacity(0.9);

    GUI_Label* l = new GUI_Label(di, val);

    return di;
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
    QVariantMap metaVals = getMetaValues(cell, val);

    // RLP_LOG_DEBUG(metaVals)


    // int pos = props.value("xpos").toInt();
    // int width = props.value("width").toInt();
    // int height = 20;


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
    
    if (metaVals.contains("icon"))
    {
        // RLP_LOG_DEBUG("GOT ICON");
        QImage im = getImage(metaVals.value("icon").toString());
        painter->drawImage(xoffset - 25, 4, &im);
    }

    // RLP_LOG_DEBUG(xoffset << _yoffset << val)

    // painter->setFont(_font);

    if (metaVals.contains("style"))
    {
        QFont f = painter->font();
        f.setBold(true);
        painter->setFont(f);
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

    
    /*
    painter->drawLine(
        pos + width,
        0,
        pos + width,
        height
    );
    
    */
}
