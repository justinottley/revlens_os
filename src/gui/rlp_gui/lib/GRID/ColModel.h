//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_COLMODEL_H
#define GUI_GRID_COLMODEL_H

#include "RlpCore/LOG/Logging.h"

#include "RlpGui/GRID/FormatterBase.h"
#include "RlpGui/GRID/ColType.h"


class GUI_GRID_API GUI_GRID_ColModel : public QObject {
    Q_OBJECT
    
signals:

    void colAdded(QString colName);
    void gridWidthChanged(qreal width);

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_ColModel();
    
    void initFormatters();

    void addCol(
        QString colName,
        QString displayName,
        QString typeStr,
        QVariantMap metadata=QVariantMap(),
        int width=80,
        bool visible=true,
        int idx=-1
    );

    void addCol(GUI_GRID_ColType* col, int idx=-1);

public slots:

    void clear();
    void updateCols();

    void addFormatter(QString fmtName, GUI_GRID_FormatterBase* formatter);
    GUI_GRID_FormatterBase* formatter(QString colName);
    GUI_GRID_FormatterBase* formatterType(QString typeName);

    void setCols(QList<GUI_GRID_ColType*> cols);
    QStringList colNames() { return _colNames; }
    void setColNames(QStringList colNames) { _colNames = colNames; }
    
    QString displayName(QString colName);
    GUI_GRID_ColType* colType(QString colName);
    bool hasColType(QString colName) { return _colMap.contains(colName); }

    void addCol(QVariantMap col, int idx=-1);

    void setColProperty(QString colName, QString prop, QVariant val);
    
    int colCount() { return _colNames.size(); }
    
    qreal colWidth() { return _colWidth; }
    void onColWidthChanged(QString colName, qreal width);
    

private:
    
    //QVariantMap _colMap;
    QMap<QString, GUI_GRID_ColType*> _colMap;
    QStringList _colNames;
    
    QMap<QString, GUI_GRID_FormatterBase*> _fmtrMap;

    qreal _colWidth;

};

#endif