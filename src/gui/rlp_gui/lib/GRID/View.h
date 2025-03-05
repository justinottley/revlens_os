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
    void selectionChanged(QVariantMap info);
    void cellEditRequested(QVariantMap cellEditInfo);


public:
    RLP_DEFINE_LOGGER

    GUI_GRID_View(GUI_ItemBase* parent);

    void onSizeChangedInternal();


public slots:

    static GUI_GRID_View*
    new_GUI_GRID_View(GUI_ItemBase* parent)
    {
        return new GUI_GRID_View(parent);
    }

    void onCellEditRequested(GUI_GRID_Cell* cell);
    void onGridWidthChanged(qreal width);
    void onContentPosChanged(qreal xpos, qreal ypos);

    void onParentSizeChanged(qreal width, qreal height);
    void setAutoResize(bool doAutoResize) { _autoResize = doAutoResize; }

    bool cellsEditable() { return _editable; }
    void setCellsEditable(bool editable) { _editable = editable; }

    void setModelData(CoreDs_ModelDataItemPtr data);
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
    void updateRows();
    void updateHeader();

    void setSelection(CoreDs_ModelDataItemPtr data, int rowIdx);
    void updateSelection(CoreDs_ModelDataItemPtr data, int rowIdx);
    void removeSelection(CoreDs_ModelDataItemPtr data, int rowIdx);

    CoreDs_ModelDataItemPtr selection(); // DEPRECATED
    QVariantMap selectionValues(); // DEPRECATED

    CoreDs_ModelDataItemPtr startSelection() { return _selected; }
    CoreDs_ModelDataItemPtr endSelection() { return _endSelected; }
    ModelItemList fullSelection() { return _fullSelection; }
    QVariantList fullSelectionValues();

    

protected:

    GUI_GRID_Toolbar* _toolbar;
    GUI_GRID_Header* _header;
    GUI_GRID_ColModel* _colModel;
    
    GUI_ScrollArea* _scrollArea;

    CoreDs_ModelDataItemPtr _data;
    QList<GUI_GRID_RowPtr> _rowList;

    CoreDs_ModelDataItemPtr _selected;
    int _selectedRow;

    CoreDs_ModelDataItemPtr _endSelected;
    int _endSelectedRow;

    ModelItemList _fullSelection;
    QList<GUI_GRID_RowPtr> _selectedRows;

    // TODO FIXME TEMP
    //
    QVariantMap _entityNameMap; 

    bool _autoResize;
    bool _editable;
};


/*

class  GUI_GRID_API GUI_GRID_ViewDecorator : public QObject {
    Q_OBJECT

public slots:

    GUI_GRID_View*
    new_GUI_GRID_View(GUI_ItemBase* parent)
    {
        return new GUI_GRID_View(parent);
    }

};

*/


#endif