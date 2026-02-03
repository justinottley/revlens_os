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
    void rowExpandRequested();
    void rowCollapseRequested();
    void cellWidgetEvent(QString evtName, QVariantMap evtInfo);


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

    virtual void clear();

    void updateCells(QStringList colNames);

    virtual GUI_GRID_Cell* makeCell(QString colName);

    virtual bool hasChildren();

    virtual bool isExpanded() { return _expanded; }
    virtual void setExpanded(bool expanded) { _expanded = expanded; }

    virtual void setHover(bool inHover) { _inHover = inHover; }

    virtual bool isPopulated() { return _populated; }
    virtual void setPopulated(bool populated);

    virtual int treeDepth() { return _treeDepth; }
    virtual void setTreeDepth(int depth) { _treeDepth = depth; }

    virtual void setItemHeight(qreal height);

public:

    virtual void mousePressEvent(QMouseEvent* event);

    virtual QRectF boundingRect() const;

    virtual void paint(GUI_Painter* painter);

public slots:

    int rowIdx() { return _rowIdx; }

    virtual CoreDs_ModelDataItemPtr rowData() { return _rowData; }

    virtual QVariant modelValue(QString key) { return _rowData->value(key); }
    virtual bool hasModelValue(QString key) { return _rowData->contains(key); }

    virtual QString modelMimeType(QString key);
    virtual bool hasModelMimeType(QString key);
    virtual void setMimeTypeKey(QString key) { _mimeTypeKey = key; } // which column / model key should be used for mimeType
    virtual QString mimeTypeKey() { return _mimeTypeKey; }

    virtual void setMimeData(QVariant mimeData) { _mimeData = mimeData; }
    virtual QVariant mimeData() { return _mimeData; }

    GUI_GRID_ColType* colType(QString key);
    GUI_GRID_Cell* cell(QString colName);

    virtual void setRowData(CoreDs_ModelDataItemPtr rowData);

    bool setFilterString(QString filterStr, QVariantList colFilterList);
    void setFilteredOut(bool fout) { _filteredOut = fout; }

    void setSelected(bool selected) { _selected = selected; update(); }
    void setRowSelected(bool selected, int selMode); // for easier use from python

    void setBaseHeight(int height) { _baseHeight = height; }
    void setEvenRow(bool isEvenRow) { _evenRow = isEvenRow; }
    void setBgColour(QColor col);


    void onSiblingRowSelected(CoreDs_ModelDataItemPtr data);
    void onGridWidthChanged(qreal width);
    void onParentVisibleChanged();
    void onCellSelected(QString colName, GridSelectionMode selMode);

public slots: // TreeGrid

    void appendRow(GUI_GRID_RowPtr row) { _rowList.push_back(row); }

    virtual qreal rowHeight();

    virtual int rowCount() { return _rowList.size(); }
    GUI_GRID_RowPtr row(int rowIdx) { return _rowList.at(rowIdx); }
    QList<GUI_GRID_RowPtr>* rowListPtr() { return &_rowList; }

    void onDataPopulated();
    void expandRow();
    void expandRowRecursive();
    void collapseRow();

    void updateRows();

private:
    void updateRowVisibilityForFilter(QString filterStr, QVariantList colFilterList);
    void collapseRowRecursive(QList<GUI_GRID_RowPtr>* rowList);

protected:

    GUI_GRID_View* _view;
    GUI_GRID_Row* _rowParent;

    CoreDs_ModelDataItemPtr _rowData;
    QString _mimeTypeKey;
    QVariant _mimeData;

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
    bool _filteredOut; // filtered out

    QColor _bgCol;
    QColor _bgHoverCol;

};

#endif