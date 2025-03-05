
#ifndef GUI_BASE_TOOLTIP_H
#define GUI_BASE_TOOLTIP_H

#include "RlpGui/PANE/Global.h"

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/FrameOverlay.h"

class GUI_BASE_API GUI_ToolTip : public GUI_ItemBase {
    Q_OBJECT

public:
    GUI_ToolTip(GUI_ItemBase* parent);

    void paint(GUI_Painter* painter);

public slots:

    void setText(QString text);

protected:
    QFont _font;
    QString _text;
};


#endif
