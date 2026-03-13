
#ifndef GUI_TEST_ITEMBASE_H
#define GUI_TEST_ITEMBASE_H

#include "RlpGui/TEST/Global.h"

class GUI_TEST_API GUI_QuickItemBase : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT

public:
    GUI_QuickItemBase(QQuickItem* parent);

    void paint(GUI_Painter* painter);

};

#endif