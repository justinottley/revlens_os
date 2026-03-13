
#ifndef GUI_MENU_PANE_BASE_H
#define GUI_MENU_PANE_BASE_H

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"

class GUI_BASE_API GUI_MenuPaneBase : public GUI_ItemBase {
    Q_OBJECT

public:
    GUI_MenuPaneBase(GUI_ItemBase* parent);

    virtual void onParentSizeChanged(qreal width, qreal height);


public slots:
    virtual void setPos(qreal xpos, qreal ypos) {}
    virtual void clear() {}

};

#endif