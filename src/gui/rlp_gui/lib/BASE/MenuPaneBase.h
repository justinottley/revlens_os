
#ifndef GUI_MENU_PANE_BASE_H
#define GUI_MENU_PANE_BASE_H

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"

class GUI_BASE_API GUI_MenuPaneBase : public GUI_ItemBase {
    Q_OBJECT
    QML_ELEMENT

public:
    GUI_MenuPaneBase(GUI_ItemBase* parent);

    virtual void onParentSizeChanged(qreal width, qreal height);

public slots:
    virtual void setPos(QPointF pos) = 0;
    virtual void clear() {}

};

#endif