//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_HEADER_H
#define GUI_GRID_HEADER_H

#include "RlpGui/GRID/Global.h"

#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/Cell.h"

class GUI_GRID_View;
class GUI_GRID_ColModel;

class GUI_GRID_API GUI_GRID_Header : public GUI_GRID_Row {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER

    GUI_GRID_Header(GUI_GRID_View* parent);
    
    QRectF boundingRect() const;
    
    CoreDs_ModelDataItemPtr rowData();
    QVariant modelValue(QString key);
    bool hasModelValue(QString key);
    
    QString modelMimeType(QString key);
    bool hasModelMimeType(QString key);
    
    GUI_GRID_Cell* makeCell(QString colName);
    
    
    
public slots:

    void onColAdded(QString colInfo);
    
private:
    
    GUI_GRID_ColModel* _colModel;
    


};

#endif