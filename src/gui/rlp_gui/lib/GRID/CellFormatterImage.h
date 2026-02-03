//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_CELL_FORMATTER_IMAGE_H
#define GUI_GRID_CELL_FORMATTER_IMAGE_H


#include "RlpGui/GRID/Global.h"
#include "RlpGui/GRID/FormatterBase.h"

class GUI_GRID_Cell;
class GUI_GRID_CellFormatterImage;

class GUI_GRID_API GUI_GRID_CellFormatterImageReadTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_CellFormatterImageReadTask(GUI_GRID_CellFormatterImage* img, GUI_GRID_Cell* cell, QString path);

    void run();


private:
    GUI_GRID_CellFormatterImage* _fmt;
    GUI_GRID_Cell* _cell;
    QString _path;
};


class GUI_GRID_API GUI_GRID_CellFormatterImage : public GUI_GRID_FormatterBase {
    Q_OBJECT

signals:
    void imageReady(GUI_GRID_Cell* cell, QImage img, QString path);


public:
    RLP_DEFINE_LOGGER
    
    enum CellValueType {
        FMT_VAL_B64,
        FMT_VAL_PATH
    };

    GUI_GRID_CellFormatterImage(CellValueType=FMT_VAL_B64, QString keyName="value");
    
    void emitImageReady(GUI_GRID_Cell* cell, QImage img, QString path) { emit imageReady(cell, img, path); }


    void paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter);

private:
    QImage updateImage(GUI_GRID_Cell* cell, QImage image);

private slots:
    void onImageReady(GUI_GRID_Cell* cell, QImage img, QString path);


protected:
    CellValueType _cellValueType;
    QString _keyName;
    QThreadPool _loadPool;

    QMap<QString, QImage> _imageCache;
};

#endif