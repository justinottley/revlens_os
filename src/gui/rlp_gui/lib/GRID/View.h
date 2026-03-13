//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_VIEW_H
#define GUI_GRID_VIEW_H

#include "RlpCore/DS/ModelDataItem.h"

#include "RlpGui/BASE/ScrollArea.h"

#include "RlpGui/GRID/Global.h"
#include "RlpGui/GRID/ColModel.h"
#include "RlpGui/GRID/Toolbar.h"
#include "RlpGui/GRID/Header.h"
#include "RlpGui/GRID/Row.h"


class GUI_GRID_API GUI_GRID_View : public GUI_ItemBase {
    Q_OBJECT

signals:
    void modelDataChanged(); // VariantList);
    void rowSelected(CoreDs_ModelDataItemPtr row);
    void selectionChanged(QString selMode, QVariantList selectionInfo);
    void cellEditRequested(QVariantMap cellEditInfo);
    void cellWidgetEvent(QString evtName, QVariantMap evtInfo);

public:
    RLP_DEFINE_LOGGER

    enum VerticalScrollbarPolicy {
        GRID_VSCROLLBAR_POLICY_COLMODEL_WIDTH,
        GRID_VSCROLLBAR_POLICY_PARENT_WIDTH
    };

    GUI_GRID_View(GUI_ItemBase* parent, GUI_GRID_ToolbarInit* tbi=nullptr);

    void onSizeChangedInternal();


public slots:

    static GUI_GRID_View*
    new_GUI_GRID_View(GUI_ItemBase* parent, GUI_GRID_ToolbarInit* tbi=nullptr)
    {
        return new GUI_GRID_View(parent, tbi);
    }

    void onCellEditRequested(GUI_GRID_Cell* cell);
    void onGridWidthChanged(qreal width);
    void onContentPosChanged(qreal xpos, qreal ypos);

    void onParentSizeChanged(qreal width, qreal height);
    void setAutoResize(bool doAutoResize) { _autoResize = doAutoResize; }

    void setVerticalScrollbarPolicy(int vScrollbarPolicy)
    {
        _vscrollbarPolicy = (VerticalScrollbarPolicy)vScrollbarPolicy;
    }

    bool cellsEditable() { return _editable; }
    void setCellsEditable(bool editable) { _editable = editable; }

    void setModelData(
        CoreDs_ModelDataItemPtr data,
        QString stashName="",
        bool clearAllData=true,
        bool emitModelDataChanged=true
    );

    void setModelDataList(QVariantList data);

    void clearData();

    GUI_GRID_ColModel* colModel() { return _colModel; }
    GUI_GRID_Toolbar* toolbar() { return _toolbar; }
    GUI_GRID_Header* header() { return _header; }
    GUI_ScrollArea* scrollArea() { return _scrollArea; }

    GUI_GRID_Row* rowAt(int idx);

    void onColAdded(QString colInfo);
    void onColToggled(QVariantMap sigData);
    bool setColVisible(QString colName, bool isVisible);

    QVariant getEntityByUuid(QUuid u)
    {
        return _entityNameMap.value(u.toString());
    }

    void onRowExpanded(int rowIdx);
    void onRowCollapsed(int rowIdx);
    void updateRows(bool force=false);
    void updateHeader();

    void setSelection(CoreDs_ModelDataItemPtr data, int rowIdx);
    void updateSelection(CoreDs_ModelDataItemPtr data, int rowIdx, GridSelectionMode selMode, bool emitSignal=false);
    void removeSelection(CoreDs_ModelDataItemPtr data, int rowIdx);
    void clearSelection();

    CoreDs_ModelDataItemPtr selection(); // DEPRECATED
    QVariantMap selectionValues(); // DEPRECATED

    CoreDs_ModelDataItemPtr startSelection() { return _selected; }

    // TODO FIXME: clean this up?
    ModelItemList fullSelection() { return _fullSelection; }
    QVariantList fullSelectionValues();
    QVariantList fullSelectionRows();

    QString currFilterStr() { return _currFilterStr; }
    QVariantList currColFilterList() { return _currColFilterList; }

    void expandRowRecursive();

    void setFilterString(QString str, QVariantList colFilterList);
    bool updateRowVisibilityForFilter(GUI_GRID_RowPtr row, QString filterStr, QVariantList colFilterList);

    int rowCount() { return _rowList->size(); }

    void forceRefresh();

    bool hasStashedRowList(QString stashName);
    void stashCurrentRowList(QString stashName);
    bool loadStashedRowList(QString stashName);
    QString currStashedRowList() { return _currStashedRowList; }
    void setCurrStashedRowListName(QString stashName);
    GUI_GRID_Row* rowAtStashedRowList(QString stashName, int rowIdx);
    void clearStashedRowList(QString stashName);

protected:

    GUI_GRID_Toolbar* _toolbar;
    GUI_GRID_Header* _header;
    GUI_GRID_ColModel* _colModel;
    
    GUI_ScrollArea* _scrollArea;

    CoreDs_ModelDataItemPtr _data;
    QList<GUI_GRID_RowPtr>* _rowList;

    // stashed rowlists
    QHash<QString, RowList*> _rowListMap;
    QHash<QString, CoreDs_ModelDataItemPtr> _rowDataMap;
    QString _currStashedRowList;

    CoreDs_ModelDataItemPtr _selected;
    int _selectedRow;

    ModelItemList _fullSelection;
    QList<GUI_GRID_RowPtr> _selectedRows;

    // TODO FIXME TEMP
    //
    QVariantMap _entityNameMap; 

    bool _autoResize;
    bool _editable;
    VerticalScrollbarPolicy _vscrollbarPolicy;

    // Filtering
    QString _currFilterStr;
    QVariantList _currColFilterList;

};


#endif