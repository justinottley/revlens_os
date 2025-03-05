
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "RlpCore/PY/PyQVariant.h"
#include "RlpGui/QTGUI/QtGuiTypes.h"

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/PaintedItemBase.h"

#ifdef SCAFFOLD_WASM
#define SCAFFOLD_PYMODULE PYBIND11_EMBEDDED_MODULE
#endif

void
RlpGuiQTGUImodule_bootstrap()
{
    qInfo() << "gui.QTGUI BOOTSTRAP";

    PyQVariant::registerConverter("QQuickItem*", new GUI_QQuickItemPtr_PyTypeConverter());
  
}


SCAFFOLD_PYMODULE(RlpGuiQTGUI, m) {

    py::class_<QImage>(m, "QImage")
        .def(py::init<const QString&>())
        .def("width", &QImage::width)
        .def("height", &QImage::height)
        .def("save",
            [](const QImage& i, py::str path) {
                return i.save(QString(std::string(path).c_str()));
            }
        )
        .def("scaledToHeight",
            [](const QImage& i, int height) {
                return i.scaledToHeight(height, Qt::SmoothTransformation);
            }
        )
        .def("scaledToWidth",
            [](const QImage& i, int width) {
                return i.scaledToWidth(width, Qt::SmoothTransformation);
            }
        )
    ;

    py::class_<QPixmap>(m, "QPixmap")
        .def(py::init<const QString&>())
        .def("width", &QPixmap::width)
        .def("height", &QPixmap::height)
        .def("scaledToHeight",
            [](const QPixmap& p, int height) {
                return p.scaledToHeight(height, Qt::SmoothTransformation);
            }
        )
        .def("scaledToWidth",
            [](const QPixmap& p, int width) {
                return p.scaledToWidth(width, Qt::SmoothTransformation);
            }
        )
        .def_static("fromImage", 
            [](const QImage& i) {
                return QPixmap::fromImage(i);
            }
        )
    ;

    py::class_<QColor>(m, "QColor")
        .def(py::init<const char*>())
        .def(py::init<int, int, int>())
        .def("red", &QColor::red)
        .def("green", &QColor::green)
        .def("blue", &QColor::blue)
        .def("setAlpha", &QColor::setAlpha)
    ;

    py::class_<QPen>(m, "QPen")
        .def(py::init<>())
        .def(py::init<const QColor&>())
        .def("setColor", &QPen::setColor)
        .def("setWidth", &QPen::setWidth)
        .def("setWidthF", &QPen::setWidthF)
    ;

    py::class_<QBrush>(m, "QBrush")
        .def(py::init<const QColor&>())
    ;

    py::class_<QFont>(m, "QFont")
        .def(py::init<const QString&>())
        .def("pointSize", &QFont::pointSize)
        .def("pixelSize", &QFont::pixelSize)
        .def("setPointSize", &QFont::setPointSize)
        .def("setPointSizeF", &QFont::setPointSizeF)
        .def("setPixelSize", &QFont::setPixelSize)
        .def("setBold", &QFont::setBold)
        .def("toString", &QFont::toString)
    ;

    py::class_<QFontMetrics>(m, "QFontMetrics")
        .def(py::init<const QFont&>())
        .def("height", &QFontMetrics::height)
        .def("horizontalAdvance",
            static_cast<int (QFontMetrics::*)(const QString&, int) const>(&QFontMetrics::horizontalAdvance)
        )
    ;

    py::class_<QLinearGradient>(m, "QLinearGradient")
        .def(py::init<const QPointF&, const QPointF&>())
        .def(py::init<qreal, qreal, qreal, qreal>())
        .def("setColorAt", &QLinearGradient::setColorAt)
    ;

    py::class_<QPainterPath>(m, "QPainterPath")
        .def(py::init<>())
        .def("moveTo",
            static_cast<void (QPainterPath::*)(const QPointF& pos)>(&QPainterPath::moveTo)
        )
        .def("moveTo",
            static_cast<void (QPainterPath::*)(qreal x, qreal y)>(&QPainterPath::moveTo)
        )
        .def("cubicTo",
            static_cast<void (QPainterPath::*)(const QPointF&, const QPointF&, const QPointF&)>(&QPainterPath::cubicTo)
        )
        .def("cubicTo",
            static_cast<void (QPainterPath::*)(qreal, qreal, qreal, qreal, qreal, qreal)>(&QPainterPath::cubicTo)
        )
    ;

    py::class_<QPainter>(m, "QPainter")
        .def("setBrush", 
            static_cast<void (QPainter::*)(const QBrush&)>(&QPainter::setBrush)
        )
        .def("setPen",
            static_cast<void (QPainter::*)(const QPen&)>(&QPainter::setPen)
        )
        .def("setPen",
            static_cast<void (QPainter::*)(const QColor&)>(&QPainter::setPen)
        )
        .def("pen", &QPainter::pen)
        .def("setFont", &QPainter::setFont)
        .def("font", &QPainter::font)
        .def("setOpacity", &QPainter::setOpacity)
        .def("opacity", &QPainter::opacity)
        .def("drawRect",
            static_cast<void (QPainter::*)(const QRectF&)>(&QPainter::drawRect)
        )
        .def("drawRect",
            static_cast<void (QPainter::*)(int, int, int, int)>(&QPainter::drawRect)
        )
        .def("drawPixmap",
            static_cast<void (QPainter::*)(int, int, const QPixmap&)>(&QPainter::drawPixmap)
        )
        .def("drawPixmap",
            static_cast<void (QPainter::*)(int, int, int, int, const QPixmap&)>(&QPainter::drawPixmap)
        )
        .def("drawText",
            static_cast<void (QPainter::*)(int, int, const QString&)>(&QPainter::drawText)
        )
        .def("drawEllipse",
            static_cast<void (QPainter::*)(int, int, int, int)>(&QPainter::drawEllipse)
        )
        .def("drawRoundedRect",
            [](QPainter&p, const QRectF& rect, qreal xRadius, qreal yRadius) {
                p.drawRoundedRect(rect, xRadius, yRadius);
            }
        )
        .def("drawLine",
            static_cast<void (QPainter::*)(int, int, int, int)>(&QPainter::drawLine)
        )
        .def("strokePath", &QPainter::strokePath)
        .def("save", &QPainter::save)
        .def("restore", &QPainter::restore)
        .def("setAntiAliasing",
            [](QPainter& p, bool val) {
                p.setRenderHint(QPainter::Antialiasing, val);
            }
        )
        .def("setLinearGradient",
            [](QPainter& p, QLinearGradient& g) {
                QBrush b(g);
                b.setStyle(Qt::LinearGradientPattern);
                p.setBrush(b);
            }
        )
    ;

    py::class_<QMouseEvent>(m, "QMouseEvent")
        .def("position", &QHoverEvent::position)
        .def("globalPosition", &QHoverEvent::globalPosition)
        .def("scenePosition", &QHoverEvent::scenePosition)
        .def("modifiers",
            [](QMouseEvent& m) {
                return (int)m.modifiers();
            })
    ;

    py::class_<QHoverEvent>(m, "QHoverEvent")
        .def("position", &QHoverEvent::position)
        .def("globalPosition", &QHoverEvent::globalPosition)
        .def("scenePosition", &QHoverEvent::scenePosition)
    ;

    py::class_<QKeyEvent>(m, "QKeyEvent")
        .def("key", &QKeyEvent::key)
        .def("text", &QKeyEvent::text)
        .def("modifiers", [](QKeyEvent& e) {
            return (int)e.modifiers();
        })
    ;

    py::class_<QQuickItem>(m, "QQuickItem")
        .def("update", [](QQuickItem& i) {
                i.update();
            }
        )
        .def("parentItem", &QQuickItem::parentItem)
        .def("setVisible", &QQuickItem::setVisible)
        .def("width", &QQuickItem::width)
        .def("height", &QQuickItem::height)
        .def("setWidth", &QQuickItem::setWidth)
        .def("setHeight", &QQuickItem::setHeight)
        .def("x", &QQuickItem::x)
        .def("y", &QQuickItem::y)
        .def("z", &QQuickItem::z)
        .def("position", &QQuickItem::position)
        .def("scale", &QQuickItem::scale)
        .def("setScale", &QQuickItem::setScale)
        .def("mapToScene", &QQuickItem::mapToScene)
        .def("mapToItem",
            [](QQuickItem& i, QQuickItem* toItem, QPointF& point)
            {
                return i.mapToItem(toItem, point);
            }
        )
        .def("mapFromGlobal", 
            [](QQuickItem& i, QPointF& point)
            {
                return i.mapFromGlobal(point);
            }
        )
        .def("mapToGlobal",
            [](QQuickItem& i, QPointF& point)
            {
                return i.mapToGlobal(point);
            }
        )
        .def("deleteLater", &QQuickItem::deleteLater)
        .def("childAt", &QQuickItem::childAt)
        .def("childItems",
            [](QQuickItem& i)
            {
                QList<QQuickItem*> ci = i.childItems();
                QVariantList l;
                for (QQuickItem* i : ci)
                {
                    QVariant v;
                    v.setValue(i);
                    l.append(v);
                }

                return PyQVariant(l).toPyList();
            })
    ;

    py::class_<QQuickPaintedItem, /*parent=*/ QQuickItem >(m, "QQuickPaintedItem")
    ;


    py::class_<QQuickWindow>(m, "QQuickWindow")
        .def("show", &QQuickWindow::show)
        .def("showMinimized", &QQuickWindow::showMinimized)
        .def("showFullScreen", &QQuickWindow::showFullScreen)
        .def("create", &QQuickWindow::create)
        .def("update", &QQuickWindow::update)
        .def("close", &QQuickWindow::close)
    ;
}