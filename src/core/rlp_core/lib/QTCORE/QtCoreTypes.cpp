
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "RlpCore/PY/Global.h"

#include "RlpCore/QTCORE/QtCoreTypes.h"

#include "RlpCore/PY/Interp.h"
#include "RlpCore/PY/Signal.h"
#include "RlpCore/PY/PyQVariant.h"
#include "RlpCore/PY/PyQVariantMap.h"
#include "RlpCore/PY/PyTimer.h"


#ifdef SCAFFOLD_WASM
#define SCAFFOLD_PYMODULE PYBIND11_EMBEDDED_MODULE
#endif

void
RlpCoreQTCOREmodule_bootstrap()
{
    qInfo() << "core.QtCore - bootstrap";
}

SCAFFOLD_PYMODULE(RlpCoreQTCORE, m) {

    py::class_<QCoreApplication>(m, "QCoreApplication")
        .def_static("instance", &QCoreApplication::instance, py::return_value_policy::reference)
        .def("processEvents",
            [](const QCoreApplication& app)
            {
                app.processEvents();
            }
        )
        .def_static("pingPyInterp",
            []()
            {
                PY_Interp::ping();
            }
        )
    ;

    py::class_<QObject>(m, "QObject")
        .def(py::init<>())
    ;

    py::class_<QString>(m, "QString")
        .def(py::init<>())
        .def(py::init<const char*>())
        .def(py::init<QString>())
        .def("toStdString", &QString::toStdString)
    ;

    py::class_<QStringList>(m, "QStringList")
        .def(py::init<>())
        .def("toPy",
            [](const QStringList& sl)
            {
                py::list result;
                for (int i=0; i != sl.size(); ++i)
                {
                    result.append(py::cast(sl.at(i).toStdString()));
                }
                return result;
            }
        )
    ;

    py::class_<QRectF>(m, "QRectF")
        .def(py::init<qreal, qreal, qreal, qreal>())
        .def("x", &QRectF::x)
        .def("y", &QRectF::y)
        .def("width", &QRectF::width)
        .def("height", &QRectF::height)
    ;

    py::class_<QRect>(m, "QRect")
        .def("x", &QRect::x)
        .def("y", &QRect::y)
        .def("width", &QRect::width)
        .def("height", &QRect::height)
    ;

    py::class_<QPointF>(m, "QPointF")
        .def(py::init<qreal, qreal>())
        .def("x", &QPointF::x)
        .def("y", &QPointF::y)
    ;

    py::class_<QUuid>(m, "QUuid")
        .def("toString",
            [](const QUuid& u) {
                return u.toString().toStdString();
            }
        )
        .def_static("createUuid", &QUuid::createUuid)
    ;

    py::class_<QUrl>(m, "QUrl")
        .def("toString",
            [](const QUrl& u) {
                return u.toString();
            }
        )
    ;


    py::class_<QDate>(m, "QDate")
        .def("day",
            [](const QDate& d)
            {
                return d.day();
            }
        )
        .def("month",
            [](const QDate& d)
            {
                return d.month();
            }
        )
        .def("year",
            [](const QDate& d)
            {
                return d.year();
            }
        )
    ;

    py::class_<QDateTime>(m, "QDateTime")
        .def_static("fromSecsSinceEpoch",
            [](qint64 secs) {
                return QDateTime::fromSecsSinceEpoch(secs);
            }
        )
        .def("toString",
            [](const QDateTime& d) {
                return d.toString();
            }
        )
        .def("date", &QDateTime::date)
    ;

    py::class_<QTimer>(m, "QTimer")
        .def("start",
            static_cast<void (QTimer::*)(int msec)>(&QTimer::start)
        )
        .def("start",
            static_cast<void (QTimer::*)()>(&QTimer::start)
        )
        .def("stop", &QTimer::stop)
    ;


    py::class_<PyQVariant>(m, "PyQVariant")
        .def(py::init<>())
        .def(py::init<int>())
        .def(py::init<float>())
        .def(py::init<double>())
        .def(py::init<const char *>())
        .def(py::init<QVariant>())
        .def(py::init<PyQVariantMap*>())
        .def(py::init<QString>())
        .def(py::init<py::list>())
        .def(py::init<py::dict>())
        .def("toInt", &PyQVariant::toInt)
        .def("toBool", &PyQVariant::toBool)
        .def("toString", &PyQVariant::toString)
        .def("toDouble", &PyQVariant::toDouble)
        .def("toList", &PyQVariant::toList)
        .def("toStringList", &PyQVariant::toStringList)
        .def("toPyList", &PyQVariant::toPyList)
        .def("toDict", &PyQVariant::toDict)
        .def("pyval", &PyQVariant::pyval)
        .def("val", &PyQVariant::val)
    ;

    py::class_<PyQVariantMap>(m, "PyQVariantMap")
        .def(py::init<>())
        .def(py::init<py::dict>())
        .def("insert", 
            static_cast<void (PyQVariantMap::*)(const char*, const char*)>(&PyQVariantMap::insertPy)
        )
        .def("insert",
            static_cast<void (PyQVariantMap::*)(const char*, float)>(&PyQVariantMap::insertPy)
        )
        .def("insert",
            static_cast<void (PyQVariantMap::*)(const char*, double)>(&PyQVariantMap::insertPy)
        )
        .def("insert",
            static_cast<void (PyQVariantMap::*)(const char*, bool)>(&PyQVariantMap::insertPy)
        )
        .def("toDict", &PyQVariantMap::toDict)
        .def("val", &PyQVariantMap::val)
    ;

    py::class_<QVariant>(m, "QVariant")
        .def("isValid", &QVariant::isValid)
        .def("toPy",
            [](const QVariant& l) {
                return PyQVariant(l).pyval();
            }
        )
        .def("toList", &QVariant::toList)
    ;

    py::class_<QVariantMap>(m, "QVariantMap")
        .def(py::init<>())
        .def("toDict",
            [](const QVariantMap& m) {
                return PyQVariantMap(m).toDict();
            }
        )
        .def("toPy",
            [](const QVariantMap& m) {
                return PyQVariantMap(m);
            }
        );

    py::class_<QVariantList>(m, "QVariantList")
        .def("toPy",
            [](const QVariantList& l) {
                return PyQVariant(l).toPyList();
            }
        );

    py::class_<QJsonObject>(m, "QJsonObject")
        .def("toString",
            [](const QJsonObject& jobj) {
                return QString(QJsonDocument(jobj).toJson(QJsonDocument::Compact)).toStdString();
            }
        );

    py::class_<PY_Signal>(m, "PY_Signal")
        .def(py::init(
            [](QObject* parent)
            {
                return new PY_Signal(parent);
            }),
            py::return_value_policy::reference
        )
        .def("connect", &PY_Signal::connect)
        .def("emit", &PY_Signal::emit_)
        .def("disconnectAll", &PY_Signal::disconnectAll)
    ;

    py::class_<PyTimer>(m, "PyTimer")
        .def(py::init<>())
        .def("connectTimeout", &PyTimer::connectTimeout)
        .def("setInterval", &PyTimer::setInterval)
        .def("setSingleShot", &PyTimer::setSingleShot)
        .def("start", &PyTimer::start)
        .def("stop", &PyTimer::stop)
    ;

    py::class_<QMimeData>(m, "QMimeData")
        .def("formats",
            [](const QMimeData& md)
            {
                QVariantList result;
                QStringList fmts = md.formats();

                for (int fi = 0; fi != fmts.size(); ++fi)
                {
                    result.append(fmts.at(fi));

                }

                return PyQVariant(result).toPyList();
            }
        )
        .def("hasFormat",
            [](const QMimeData& md, const char* format)
            {
                return md.hasFormat(QString(format));
            }
        )
        .def("hasText", &QMimeData::hasText)
        .def("text",
            [](const QMimeData& md)
            {
                return md.text().toStdString();
            }
        )
        .def("dataAsText",
            [](const QMimeData& md, const char* format)
            {
                return QString(md.data(QString(format))).toStdString();
            }
        )
    ;
}