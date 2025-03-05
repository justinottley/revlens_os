//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_ROW_H
#define GUI_GRID_ROW_H

#include "RlpCore/DS/ModelDataItem.h"

#include "RlpGui/GRID/Global.h"
#include "RlpGui/GRID/ColType.h"

class GUI_GRID_View;
class GUI_GRID_Cell;
class GUI_GRID_Row;

typedef std::shared_ptr<GUI_GRID_Row> GUI_GRID_RowPtr;
typedef QMap<QString, GUI_GRID_Cell*>::iterator CellIterator;

class GUI_GRID_API GUI_GRID_Row : public GUI_ItemBase {
    Q_OBJECT

signals:
    void rowExpanded(int rowIdx);
    void rowCollapsed(int rowIdx);


public:
    RLP_DEFINE_LOGGER

    GUI_GRID_Row(
        GUI_ItemBase* parent,
        GUI_GRID_View* view,
        int rowIdx,
        GUI_GRID_Row* rowParent=nullptr,
        int height=30
    );
    
    ~GUI_GRID_Row();
    
public slots:

    GUI_GRID_View* view() { return _view; }
    
    // GUI_GRID_View* view();
    
    virtual void clear();

    // virtual void setPos(qreal x, qreal y);

    void updateCells(QStringList colNames);
    
    virtual GUI_GRID_Cell* makeCell(QString colName);
    
    virtual bool hasChildren();

    virtual bool isExpanded() { return _expanded; }
    virtual void setExpanded(bool expanded) { _expanded = expanded; }

    virtual void setHover(bool inHover) { _inHover = inHover; }

    virtual bool isPopulated() { return _populated; }
    virtual void setPopulated(bool populated) { _populated = populated; }

    virtual int treeDepth() { return _treeDepth; }
    virtual void setTreeDepth(int depth) { _treeDepth = depth; }

    virtual void setItemHeight(qreal height);

public:

    virtual void mousePressEvent(QMouseEvent* event);

    virtual QRectF boundingRect() const;
    
    virtual void paint(GUI_Painter* painter);

public slots:

    int rowIdx() { return _rowIdx; }

    void setBaseHeight(int height) { _baseHeight = height; }
    
    virtual CoreDs_ModelDataItemPtr rowData() { return _rowData; }

    virtual QVariant modelValue(QString key) { return _rowData->value(key); }
    virtual bool hasModelValue(QString key) { return _rowData->contains(key); }

    virtual QString modelMimeType(QString key) { return _rowData->mimeType(key); }
    virtual bool hasModelMimeType(QString key) { return _rowData->hasMimeType(key); }
    
    GUI_GRID_ColType* colType(QString key);
    GUI_GRID_Cell* cell(QString colName);

    virtual void setRowData(CoreDs_ModelDataItemPtr rowData);

    void setSelected(bool selected) { _selected = selected; update(); }

    void onSiblingRowSelected(CoreDs_ModelDataItemPtr data);
    void onGridWidthChanged(qreal width);
    void onParentVisibleChanged();
    void onCellSelected(QString colName, GridSelectionMode selMode);

public slots: // TreeGrid

    void appendRow(GUI_GRID_RowPtr row) { _rowList.push_back(row); }

    virtual qreal rowHeight();

    virtual int rowCount() { return _rowList.size(); }
    GUI_GRID_RowPtr row(int rowIdx) { return _rowList.at(rowIdx); }

    void onDataPopulated();
    void expandRow();
    void collapseRow();

    // void setVisible(bool visible);

    void updateRows();


protected:

    GUI_GRID_View* _view;
    GUI_GRID_Row* _rowParent;

    CoreDs_ModelDataItemPtr _rowData;

    int _rowIdx;
    bool _evenRow; // even or odd row, for alternate coloring
    int _baseHeight;
    
    QMap<QString, GUI_GRID_Cell*> _cells;

    bool _expanded;
    bool _populated; // children rows built
    int _treeDepth; // level / depth into the tree, used for indentation

    QList<GUI_GRID_RowPtr> _rowList;

    bool _selected;
    bool _inHover; // set from cell
};

#endif