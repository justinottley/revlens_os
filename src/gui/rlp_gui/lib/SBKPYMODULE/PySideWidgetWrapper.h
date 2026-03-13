
#ifndef GUI_WIDGET_QWIDGET_WRAPPER_H
#define GUI_WIDGET_QWIDGET_WRAPPER_H

#include "RlpGui/SBKPYMODULE/Global.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/WIDGET/QWidgetWrapper.h"


class GUI_PySideWidgetWrapper : public GUI_QWidgetWrapper {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_PySideWidgetWrapper(GUI_ItemBase* parent);

    static QQuickItem* create(GUI_ItemBase* parent, QVariantMap metadata);

public slots:

    static GUI_PySideWidgetWrapper* new_GUI_PySideWidgetWrapper(GUI_ItemBase* parent)
    {
        return new GUI_PySideWidgetWrapper(parent);
    }

    void setPySideWidget(py::object obj);

};

#endif
