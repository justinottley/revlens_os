

#ifndef GUI_BASE_GLITEM_H
#define GUI_BASE_GLITEM_H

#include "RlpGui/BASE/Global.h"

#include "RlpGui/BASE/ItemBase.h"

class GUI_BASE_API GUI_GLItem : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_GLItem(GUI_ItemBase* parent);

    virtual void paint(GUI_Painter* painter);

public slots:

    virtual void onParentVisibilityChanged(QString name, bool isVisible);

    virtual void sync();
    virtual void initGL();
    virtual void paintGL();
    

protected:
    qreal _wpr; // Window Pixel Aspect Ratio - required to position correctly on hidpi devices
    QRect _srect; // screen coords

};

#endif