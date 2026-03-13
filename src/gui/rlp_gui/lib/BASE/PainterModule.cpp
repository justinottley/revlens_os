
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "RlpGui/BASE/PainterModule.h"
#include "RlpGui/BASE/Painter.h"

#include "RlpCore/PY/Global.h"


RLP_SETUP_MODULE_LOGGER(gui, PAINTER)

void
RlpGuiPAINTERmodule_bootstrap()
{
    RLP_LOG_DEBUG_MODULE(gui, PAINTER, "")
}

void
RlpGuiPAINTERmodule_initmodule()
{
    RLP_LOG_DEBUG_MODULE(gui, PAINTER, "")
}

SCAFFOLD_PYMODULE(RlpGuiPAINTERmodule, m) {
    m.def("init_module", &RlpGuiPAINTERmodule_initmodule, "Bootstrap Python Bindings");
    py::class_<GUI_NanoPainter>(m, "GUI_NanoPainter")
        .def("setBrush", 
            static_cast<void (GUI_NanoPainter::*)(const QBrush&)>(&GUI_NanoPainter::setBrush)
        )
        .def("setPen",
            static_cast<void (GUI_NanoPainter::*)(const QPen&)>(&GUI_NanoPainter::setPen)
        )
        .def("setPen",
            static_cast<void (GUI_NanoPainter::*)(const QColor&)>(&GUI_NanoPainter::setPen)
        )
        .def("pen", &GUI_NanoPainter::pen)
        .def("setFont", &GUI_NanoPainter::setFont)
        .def("font", &GUI_NanoPainter::font)
        .def("setOpacity", &GUI_NanoPainter::setOpacity)
        .def("opacity", &GUI_NanoPainter::opacity)
        .def("drawRect",
            static_cast<void (GUI_NanoPainter::*)(const QRectF&)>(&GUI_NanoPainter::drawRect)
        )
        .def("drawRect",
            static_cast<void (GUI_NanoPainter::*)(int, int, int, int)>(&GUI_NanoPainter::drawRect)
        )
        .def("drawImage",
            static_cast<void (GUI_NanoPainter::*)(int, int, QImage*)>(&GUI_NanoPainter::drawImage)
        )
        .def("drawImage",
            static_cast<void (GUI_NanoPainter::*)(int, int, int, int, QImage*)>(&GUI_NanoPainter::drawImage)
        )
        .def("drawText",
            static_cast<void (GUI_NanoPainter::*)(int, int, const QString&)>(&GUI_NanoPainter::drawText)
        )
        .def("drawEllipse",
            static_cast<void (GUI_NanoPainter::*)(qreal, qreal, qreal, qreal)>(&GUI_NanoPainter::drawEllipse)
        )
        .def("drawRoundedRect",
            [](GUI_NanoPainter&p, const QRectF& rect, qreal xRadius, qreal yRadius) {
                p.drawRoundedRect(rect, xRadius, yRadius);
            }
        )
        .def("drawLine",
            static_cast<void (GUI_NanoPainter::*)(int, int, int, int)>(&GUI_NanoPainter::drawLine)
        )
        .def("strokePath", &GUI_NanoPainter::strokePath)
        .def("moveTo", &GUI_NanoPainter::moveTo)
        .def("cubicTo", &GUI_NanoPainter::cubicTo)
        .def("stroke", &GUI_NanoPainter::stroke)
        .def("beginPath", &GUI_NanoPainter::beginPath)
        .def("closePath", &GUI_NanoPainter::closePath)
        .def("drawCircle", &GUI_NanoPainter::drawCircle)
        .def("save", &GUI_NanoPainter::save)
        .def("restore", &GUI_NanoPainter::restore)
    ;

    py::class_<GUI_QPainter>(m, "GUI_QPainter")
        .def("setBrush", 
            static_cast<void (GUI_QPainter::*)(const QBrush&)>(&GUI_QPainter::setBrush)
        )
        .def("setPen",
            static_cast<void (GUI_QPainter::*)(const QPen&)>(&GUI_QPainter::setPen)
        )
        .def("setPen",
            static_cast<void (GUI_QPainter::*)(const QColor&)>(&GUI_QPainter::setPen)
        )
        .def("pen", &GUI_QPainter::pen)
        .def("setFont", &GUI_QPainter::setFont)
        .def("font", &GUI_QPainter::font)
        .def("setOpacity", &GUI_QPainter::setOpacity)
        .def("opacity", &GUI_QPainter::opacity)
        .def("drawRect",
            static_cast<void (GUI_QPainter::*)(const QRectF&)>(&GUI_QPainter::drawRect)
        )
        .def("drawRect",
            static_cast<void (GUI_QPainter::*)(int, int, int, int)>(&GUI_QPainter::drawRect)
        )
        .def("drawImage",
            static_cast<void (GUI_QPainter::*)(int, int, QImage*)>(&GUI_QPainter::drawImage)
        )
        .def("drawImage",
            static_cast<void (GUI_QPainter::*)(int, int, int, int, QImage*)>(&GUI_QPainter::drawImage)
        )
        .def("drawText",
            static_cast<void (GUI_QPainter::*)(int, int, const QString&)>(&GUI_QPainter::drawText)
        )
        .def("drawEllipse",
            static_cast<void (GUI_QPainter::*)(qreal, qreal, qreal, qreal)>(&GUI_QPainter::drawEllipse)
        )
        .def("drawRoundedRect",
            [](GUI_QPainter&p, const QRectF& rect, qreal xRadius, qreal yRadius) {
                p.drawRoundedRect(rect, xRadius, yRadius);
            }
        )
        .def("drawLine",
            static_cast<void (GUI_QPainter::*)(int, int, int, int)>(&GUI_QPainter::drawLine)
        )
        .def("strokePath", &GUI_QPainter::strokePath)
        .def("moveTo", &GUI_QPainter::moveTo)
        .def("cubicTo", &GUI_QPainter::cubicTo)
        .def("stroke", &GUI_QPainter::stroke)
        .def("beginPath", &GUI_QPainter::beginPath)
        .def("closePath", &GUI_QPainter::closePath)
        .def("drawCircle", &GUI_QPainter::drawCircle)
        .def("save", &GUI_QPainter::save)
        .def("restore", &GUI_QPainter::restore)
    ;

}