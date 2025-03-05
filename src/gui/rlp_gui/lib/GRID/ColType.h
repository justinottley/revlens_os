//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_COLTYPE_H
#define GUI_GRID_COLTYPE_H

#include "RlpGui/GRID/Global.h"

#include "RlpGui/GRID/FormatterBase.h"


class GUI_GRID_ColType;

typedef QMap<QString, GUI_GRID_ColType*> ColTypeMap;

class GUI_GRID_API GUI_GRID_ColType : public QObject {
    Q_OBJECT

signals:
    void widthChanged(QString name, qreal width);
    void xposChanged(QString name, qreal xpos);
    void visibleChanged(bool isVisible);
    
public:
    RLP_DEFINE_LOGGER

    GUI_GRID_ColType(
        QString typeStr,
        GUI_GRID_FormatterBase* headerFormatter=nullptr,
        GUI_GRID_FormatterBase* dataFormatter=nullptr
    );
    
    GUI_GRID_ColType(
        QString colName,
        QString displayName,
        QString typeStr,
        QVariantMap metadata=QVariantMap(),
        int width=80,
        bool visible=true
    );


public slots:

    QString colName() { return _colName; }
    QString displayName() { return _displayName; }
    QString typeStr() { return _typeStr; }
    QVariantMap metadata() { return _metadata; }
    bool hasMetadataValue(QString key) { return _metadata.contains(key); }

    QVariantMap toMap();

    bool isVisible() { return _visible; }
    void setVisible(bool visible);

    int width() { return _width; }
    void setWidth(qreal width);

    qreal xpos() { return _xpos; }
    void setXPos(qreal xpos);



private:

    QString _colName;
    QString _displayName;
    QString _typeStr;
    QVariantMap _metadata;

    qreal _width;
    bool _visible;
    
    qreal _xpos;

    // ColFormatterMap _formatters;
    
};

#endif