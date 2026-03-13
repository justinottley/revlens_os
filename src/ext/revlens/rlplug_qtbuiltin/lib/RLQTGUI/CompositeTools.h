
#ifndef EXTREVLENS_RLQTGUI_COMPOSITE_TOOLS_H
#define EXTREVLENS_RLQTGUI_COMPOSITE_TOOLS_H

#include "RlpExtrevlens/RLQTGUI/Global.h"

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/WIDGET/IconButton.h"


class EXTREVLENS_RLQTGUI_API RLQTGUI_ViewerIconButton : public GUI_IconButton {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTGUI_ViewerIconButton(GUI_ItemBase* parent, QString iconPath, int height);

    void onMenuShown(QVariantMap md);

public slots:

    static RLQTGUI_ViewerIconButton* new_RLQTGUI_ViewerIconButton(GUI_ItemBase* parent, QString path, int height)
    {
        return new RLQTGUI_ViewerIconButton(parent, path, height);
    }
};


class EXTREVLENS_RLQTGUI_API RLQTGUI_CompositeModeButton : public RLQTGUI_ViewerIconButton {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTGUI_CompositeModeButton(GUI_ItemBase* parent);

    void onModeMenuItemSelected(QVariantMap itemInfo);


public slots:

    static RLQTGUI_CompositeModeButton* new_RLQTGUI_CompositeModeButton(GUI_ItemBase* parent)
    {
        return new RLQTGUI_CompositeModeButton(parent);
    } 
};


class EXTREVLENS_RLQTGUI_API RLQTGUI_CompositeTools : public GUI_ItemBase {
    Q_OBJECT

// signals:
//     void compositeModeChanged(QString modeName);

public:
    RLP_DEFINE_LOGGER

    RLQTGUI_CompositeTools(GUI_ItemBase* parent);


public slots:

    static RLQTGUI_CompositeTools* new_RLQTGUI_CompositeTools(GUI_ItemBase* parent)
    {
        return new RLQTGUI_CompositeTools(parent);
    }



protected:

    GUI_IconButton* _modeButton;

};

#endif
