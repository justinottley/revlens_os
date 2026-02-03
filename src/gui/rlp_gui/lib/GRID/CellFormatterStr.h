//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_TEXT_FORMATTER_H
#define GUI_GRID_TEXT_FORMATTER_H


#include "RlpGui/GRID/Global.h"
#include "RlpGui/GRID/FormatterBase.h"

class GUI_GRID_Cell;

class GUI_GRID_API GUI_GRID_CellFormatterStr : public GUI_GRID_FormatterBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_CellFormatterStr(QString keyName="value", int xoffset=0, int yoffset=5);

    QString getValue(GUI_GRID_Cell* cell);
    QVariantMap getMetaValues(GUI_GRID_Cell* cell, QString name);
    QImage getImage(QString name);

    GUI_ItemBase* makeDragItem(GUI_GRID_Cell* cell);

    virtual void paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter);



public slots:

    void setFontSize(int size) { _font.setPixelSize(size); }

protected:
    QString _keyName;
    int _xoffset;
    int _yoffset;
    QFont _font;
    QMap<QString, QImage> _iconMap;

};

#endif