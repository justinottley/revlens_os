//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_CELL_H
#define GUI_GRID_CELL_H

#include "RlpGui/GRID/Global.h"
#include "RlpGui/GRID/FormatterBase.h"
#include "RlpGui/GRID/ColType.h"
#include "RlpGui/GRID/RowExpanderItem.h"


class GUI_GRID_Row;

class GUI_GRID_API GUI_GRID_Cell : public GUI_ItemBase {
    Q_OBJECT

signals:
    void selected(QString colName, GridSelectionMode selMode);
    void editRequested(GUI_GRID_Cell* cell);
    void cellWidgetEvent(QString evtName, QVariantMap evtInfo);


public:
    RLP_DEFINE_LOGGER

    GUI_GRID_Cell(
        GUI_GRID_Row* row,
        QString colName,
        ColFormatterType cfType=CF_DATA);

    ~GUI_GRID_Cell();

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

    virtual void hoverEnterEvent(QHoverEvent* event);
    virtual void hoverMoveEvent(QHoverEvent* event);
    virtual void hoverLeaveEvent(QHoverEvent* event);
    
    virtual QRectF boundingRect() const;
    
    virtual void paint(GUI_Painter* painter);

public slots:

    GUI_GRID_Row* row() { return _row; }
    GUI_GRID_ColType* colType() { return _col; }

    QString colName() { return _colName; }
    
    QVariant value(QString key) { return _cellData.value(key); }
    bool hasValue(QString key) { return _cellData.contains(key); }
    void insertValue(QString key, QVariant val) { _cellData.insert(key, val); }
    void removeValue(QString key) { _cellData.remove(key); }

    QVariantMap values() { return _cellData; }

    virtual bool hasChildren();

    void onHeightChanged();

    bool inHover() { return _inHover; }
    bool inHoverEdit();

    bool inEdit();
    void setInEdit(bool inEdit) { _inEdit = inEdit; update(); }

    bool inItemSelect() { return _inItemSelect; }
    void setInItemSelect(bool inItemSelect) { _inItemSelect = inItemSelect; }

    int treeDepth();
    bool doIndent() { return _doIndent; }


public slots:

    void onColWidthChanged(QString colName, qreal width);
    void onColXPosChanged(QString colName, qreal xpos);
    void onColVisibleChanged(bool isVisible);
    void onRowExpanderTriggered();

    void addWidget(QString name, GUI_ItemBase* widget);
    QVariant widget(QString name) { return _widgets.value(name); }

    void emitCellWidgetEvent(QString evtName, QVariantMap evtInfo)
    {
        emit cellWidgetEvent(evtName, evtInfo);
    }

private slots:
    void onHoverButtonHoverChanged(bool val);
    void onHoverButtonPressed(QVariantMap md);

protected:

    GUI_GRID_Row* _row;
    ColFormatterType _cfType;
    QString _colName;

    GUI_GRID_RowExpanderItem* _ei;

    QVariantMap _cellData;
    GUI_GRID_ColType* _col;

    GUI_GRID_FormatterBase* _fmt;

    bool _inHover;
    bool _inHoverEdit;
    bool _inEdit;
    bool _inItemSelect;

    bool _doIndent;

    QMap<QString, QVariant> _widgets;

};

#endif