
#ifndef REVLENS_GUI_LOOPMODE_BUTTON_H
#define REVLENS_GUI_LOOPMODE_BUTTON_H

#include "RlpRevlens/GUI/Global.h"

#include "RlpGui/WIDGET/IconButton.h"

class GUI_PlayModeButton : public GUI_IconButton {
    Q_OBJECT

public:
    GUI_PlayModeButton(GUI_ItemBase* parent, int iconSize=18);


public slots:
    void onPlayModeChanged(QVariantMap md);
    void onPlayModeMenuShown(QVariantMap md);

private:
    void syncMenu();

    GUI_MenuItem* _lmLoop;
    GUI_MenuItem* _lmOnce;
    GUI_MenuItem* _lmSwing;

    GUI_MenuItem* _dirForward;
    GUI_MenuItem* _dirBackward;
    GUI_MenuItem* _dirAuto;

};


#endif
