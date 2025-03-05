
#include "RlpGui/SBKPYMODULE/QWidgetWrapper.h"
#include "RlpGui/SBKPYMODULE/Util.h"


RLP_SETUP_LOGGER(gui, GUI, QWidgetWrapper)


GUI_PySideWidgetWrapper::GUI_PySideWidgetWrapper(GUI_ItemBase* parent):
    GUI_QWidgetWrapper(parent)
{
}


QQuickItem*
GUI_PySideWidgetWrapper::create(GUI_ItemBase* parent, QVariantMap metadata)
{
    return new GUI_PySideWidgetWrapper(parent);
}

void
GUI_QWidgetWrapper::setPySideWidget(py::object obj)
{
    RLP_LOG_DEBUG("")

    _widget = PYSBK_Util::getQWidget(obj.ptr());
    _widget->setGeometry(x(), y(), width(), height());

    update();
}
