

#include "RlpGui/GRID/ColType.h"


RLP_SETUP_LOGGER(gui, GUI_GRID, ColType)

GUI_GRID_ColType::GUI_GRID_ColType(
    QString colName,
    QString displayName,
    QString typeStr,
    QVariantMap metadata,
    int width,
    bool visible):
        _colName(colName),
        _displayName(displayName),
        _typeStr(typeStr),
        _metadata(metadata),
        _width(width),
        _visible(visible)
{
}


void
GUI_GRID_ColType::setWidth(qreal width)
{
    // RLP_LOG_DEBUG(width)

    _width = width;
    emit widthChanged(_colName, width);
}


void
GUI_GRID_ColType::setXPos(qreal xpos)
{
    // RLP_LOG_DEBUG(_colName << " " << xpos)

    _xpos = xpos;

    emit xposChanged(_colName, xpos);
}


void
GUI_GRID_ColType::setVisible(bool isVisible)
{
    _visible = isVisible;

    emit visibleChanged(isVisible);
}


QVariantMap
GUI_GRID_ColType::toMap()
{
    QVariantMap result;
    result.insert("name", _colName);
    result.insert("display_name", _displayName);
    result.insert("type_str", _typeStr);

    return result;
}