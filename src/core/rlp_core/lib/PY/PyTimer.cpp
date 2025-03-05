


#include "RlpCore/PY/PyTimer.h"
#include "RlpCore/PY/Interp.h"

PyTimer::PyTimer():
    _pyslot(py::none())
{
    PY_Interp::acquireGIL();

    connect(
        &_timer,
        &QTimer::timeout,
        this,
        &PyTimer::onTimeout
    );
}


void
PyTimer::onTimeout()
{
    PY_Interp::acquireGIL();

    if (_pyslot != py::none())
    {
        _pyslot();
    }
    
}


void
PyTimer::start()
{
    PY_Interp::acquireGIL();

    QMetaObject::invokeMethod(
        &_timer,
        "start",
        Qt::AutoConnection
    );
}


void
PyTimer::stop()
{
    PY_Interp::acquireGIL();

    QMetaObject::invokeMethod(
        &_timer,
        "stop",
        Qt::AutoConnection
    );
}