
#ifndef CORE_GUI_VIEWBASE_H
#define CORE_GUI_VIEWBASE_H


#include "RlpGui/BASE/Global.h"


class GUI_FrameBase;

#ifdef SCAFFOLD_WASM
#define VIEW_QBASE QWidget
#else
#define VIEW_QBASE QQuickWindow // QGraphicsView
#endif

class GUI_BASE_API GUI_ViewBase : public VIEW_QBASE {
    Q_OBJECT

public:
    GUI_ViewBase();

public slots:

    virtual GUI_FrameBase* showToolTip(QString text) { return nullptr; }
    virtual void clearToolTip() {}

protected:

    GUI_FrameBase* _currToolTip;

};


#endif