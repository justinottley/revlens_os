
#include "RlpGui/GRID/ColModel.h"
#include "RlpGui/GRID/CellFormatterStr.h"
#include "RlpGui/GRID/CellFormatterEntity.h"
#include "RlpGui/GRID/CellFormatterImage.h"
#include "RlpGui/GRID/CellFormatterFolder.h"
#include "RlpGui/GRID/CellFormatterDate.h"

RLP_SETUP_LOGGER(gui, GUI_GRID, ColModel)

GUI_GRID_ColModel::GUI_GRID_ColModel():
    _colWidth(0)
{
    initFormatters();
}


void
GUI_GRID_ColModel::initFormatters()
{
    // RLP_LOG_DEBUG("")

    _fmtrMap.insert("str", new GUI_GRID_CellFormatterStr());
    _fmtrMap.insert("bool", new GUI_GRID_CellFormatterStr());
    _fmtrMap.insert("int", new GUI_GRID_CellFormatterStr());
    _fmtrMap.insert("longint", new GUI_GRID_CellFormatterStr());
    _fmtrMap.insert("float", new GUI_GRID_CellFormatterStr());
    _fmtrMap.insert("entity", new GUI_GRID_CellFormatterEntity());
    _fmtrMap.insert("image_b64", new GUI_GRID_CellFormatterImage());
    _fmtrMap.insert("folder", new GUI_GRID_CellFormatterFolder());
    _fmtrMap.insert("date_epoch", new GUI_GRID_CellFormatterDate());
}

void
GUI_GRID_ColModel::addFormatter(QString fmtName, GUI_GRID_FormatterBase* formatter)
{
    // RLP_LOG_DEBUG(fmtName)

    _fmtrMap.insert(fmtName, formatter);
}

void
GUI_GRID_ColModel::clear()
{
    _colNames.clear();
    _colMap.clear();
}


GUI_GRID_FormatterBase*
GUI_GRID_ColModel::formatterType(QString typeName)
{
    if (_fmtrMap.contains(typeName))
    {
        return _fmtrMap.value(typeName);
    }

    return nullptr;
}



GUI_GRID_FormatterBase*
GUI_GRID_ColModel::formatter(
    QString colName)
{
    QString typeStr = _colMap.value(colName)->typeStr();
    QVariantMap mdata = _colMap.value(colName)->metadata();

    QStringList fmtList = {typeStr};

    if (mdata.contains("data_type"))
    {
        fmtList.insert(0, mdata.value("data_type").toString());
    }

    // RLP_LOG_DEBUG(colName << fmtList)

    for (int fi=0; fi!=fmtList.size(); ++fi)
    {
        QString fmt = fmtList.at(fi);

        if (_fmtrMap.contains(fmt))
        {
            // RLP_LOG_DEBUG(colName << "FORMATTER:" << fmt)
            return _fmtrMap.value(fmt);
        }

        /*
            RLP_LOG_DEBUG("isUpper: " << fmt.at(0).isUpper())

            if (typeStr.at(0).isUpper())
            {
                RLP_LOG_WARN("No formatter for type" << fmt << ", using entity formatter")
                return _fmtrMap.value("entity");
            }
            else
            {
                RLP_LOG_ERROR("No formatter for type" << fmt)
            }
        }
        */
    }

    return nullptr;
}


QString
GUI_GRID_ColModel::displayName(QString colName)
{
    return _colMap.value(colName)->displayName();
}


GUI_GRID_ColType*
GUI_GRID_ColModel::colType(QString colName)
{
    return _colMap.value(colName);
}


void
GUI_GRID_ColModel::setCols(QList<GUI_GRID_ColType*> cols)
{
    
    _colMap.clear();
    _colNames.clear();
    
    for (int i=0; i != cols.size(); ++i) {
        
        GUI_GRID_ColType* col = cols.at(i);
        _colNames.push_back(col->colName());
        _colMap.insert(col->colName(), col);
        
    }
}


void
GUI_GRID_ColModel::setColProperty(QString colName, QString prop, QVariant val)
{
    RLP_LOG_ERROR("Not implemented")
    Q_ASSERT(false);

    /*
    QVariantMap colInfo = _colMap.value(colName).toMap();
    colInfo.insert(prop, val);
    
    _colMap.insert(colName, colInfo);
    */
}



void
GUI_GRID_ColModel::addCol(
    QString colName,
    QString displayName,
    QString typeStr,
    QVariantMap metadata,
    int width,
    bool visible,
    int idx)
{

    GUI_GRID_ColType* col = new GUI_GRID_ColType(
        colName,
        displayName,
        typeStr,
        metadata,
        width,
        visible
    );

    connect(
        col,
        &GUI_GRID_ColType::widthChanged,
        this,
        &GUI_GRID_ColModel::onColWidthChanged
    );

    return addCol(col, idx);

    /*
    QVariantMap colInfo;
    
    colInfo.insert("col_name", colName);
    colInfo.insert("col_type", typeStr);
    colInfo.insert("display_name", displayName);

    addCol(colInfo, idx);
    */

    
}


void
GUI_GRID_ColModel::addCol(QVariantMap colInfo, int idx)
{
    return addCol(
        colInfo.value("col_name").toString(),
        colInfo.value("display_name").toString(),
        colInfo.value("col_type").toString(),
        colInfo.value("metadata").toMap(),
        colInfo.value("width").toInt()
    );
    
}


void
GUI_GRID_ColModel::addCol(GUI_GRID_ColType* col, int idx)
{
    QString colName = col->colName(); // colInfo.value("col_name").toString();
    
    //
    // if (!colInfo.contains("display_name")) {
    //    colInfo.insert("display_name", colName.capitalize());
    // }
    
    
    
    int numCols = _colNames.size();
    
    int xoffset = 0;
    if (idx >= 0) {
        
        if (idx > 0) {
            QString prevColName = _colNames.at(idx - 1);

            GUI_GRID_ColType* prevCol = _colMap.value(prevColName);

            // QVariantMap prevCol = _colMap.value(prevColName).toMap();

            xoffset += prevCol->xpos(); // prevCol.value("xpos").toInt();
            xoffset += prevCol->width(); // prevCol.value("width").toInt();
        
        }
        
        _colNames.insert(idx, colName);
        
        
    } else {
        
        if (numCols > 0) {
            QString prevColName = _colNames.at(numCols - 1);
            
            // QVariantMap prevCol = _colMap.value(prevColName).toMap();

            GUI_GRID_ColType* prevCol = _colMap.value(prevColName);

            xoffset += prevCol->xpos(); // prevCol.value("xpos").toInt();
            xoffset += prevCol->width(); // prevCol.value("width").toInt();
        }

        _colNames.push_back(colName);
        
        
    }
    
    col->setXPos(xoffset);

    // colInfo.insert("xpos", xoffset);
    _colMap.insert(colName, col);
    
    
    emit colAdded(colName);
    
}


void
GUI_GRID_ColModel::onColWidthChanged(QString colName, qreal width)
{
    // RLP_LOG_DEBUG(colName << " " << width)

    updateCols();
}


void
GUI_GRID_ColModel::updateCols()
{
    // RLP_LOG_DEBUG(_colMap.keys())
    // RLP_LOG_DEBUG(_colNames)

    if (_colNames.size() == 0)
    {
        return;
    }


    qreal xoffset = 0;

    _colMap.value(_colNames.at(0))->setXPos(0);
    
    for (int i=1; i != _colNames.size(); ++i) {

        GUI_GRID_ColType* prevCol = _colMap.value(_colNames.at(i - 1));
        GUI_GRID_ColType* col = _colMap.value(_colNames.at(i));

        if (prevCol->isVisible()) {
            xoffset += prevCol->width();
        }
        
        col->setXPos(xoffset);
    }

    GUI_GRID_ColType* lastCol = _colMap.value(_colNames.at(_colNames.size() - 1));
    if (lastCol->isVisible()) {
        xoffset += lastCol->width();
    }
    

    // RLP_LOG_DEBUG(xoffset)
    _colWidth = xoffset;

    emit gridWidthChanged(xoffset);
}