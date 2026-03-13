
#include "RlpGui/SBKPYMODULE/main.h"
#include "RlpGui/SBKPYMODULE/QWidgetWrapper.h"

#include "RlpCore/PY/Interp.h"

void
RlpGuiSBKPYMODULE_bootstrap()
{
}

PYBIND11_MODULE(RlpGuiSBKPYMODULE, m) {

    m.doc() = "Revlens PySide/Shiboken Extension";
    m.def("bootstrap", &RlpGuiSBKPYMODULE_bootstrap, "Bootstrap Python Bindings");

    py::class_<GUI_QWidgetWrapper>(m, "GUI_QWidgetWrapper")
        .def(py::init(
            [](GUI_ItemBase* parent)
            {
                return new GUI_QWidgetWrapper(parent);
            }),
           py::return_value_policy::reference
        )
        .def("setPySideWidget", &GUI_QWidgetWrapper::setPySideWidget)
        .def("setWidth", &GUI_QWidgetWrapper::setWidth)
        .def("setHeight", &GUI_QWidgetWrapper::setHeight)
        .def("forwardMouseEventToWidget", &GUI_QWidgetWrapper::forwardMouseEventToWidget)
    ;
}