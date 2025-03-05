//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_TABHEADER_H
#define CORE_GUI_TABHEADER_H

#include "RlpGui/PANE/Global.h"
#include "RlpGui/BASE/ItemBase.h"

class GUI_SvgButton;

class GUI_PANE_API GUI_TabHeader : public GUI_ItemBase {
    Q_OBJECT

signals:
    void tabActivateRequested(QString tabName);
    void closeTab(QString tabName, bool destruct);
    void tabMoveRequested(QString myName, QString tabName, QVariant tabW);


public:
    RLP_DEFINE_LOGGER

    GUI_TabHeader(QString name, GUI_ItemBase* parent);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    void dragEnterEvent(QDragEnterEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dropEvent(QDropEvent *event);

    QRectF boundingRect() const;
    
    void paint(GUI_Painter* painter);

    GUI_ItemBase* makeDragItem();

public slots:

    QString name() { return _name; }

    bool isActive() { return _active; }
    void setActive(bool active) { _active = active; updateItem(); }

    bool isFirst() { return _first; }
    void setFirst(bool isFirst) { _first = isFirst; updateItem(); }

public slots:

    void onCloseButtonPressed(QVariantMap metadata = QVariantMap());


private:

    QString _name;

    GUI_SvgButton* _btnClose;

    bool _active;
    bool _inHover;
    bool _first; // for painting..
    bool _highlightDrop; // for drag and drop
};

#endif
