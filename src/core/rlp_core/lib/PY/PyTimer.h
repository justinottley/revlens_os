
#ifndef RLP_CORE_PY_TIMER_H
#define RLP_CORE_PY_TIMER_H

#include "RlpCore/PY/Global.h"


class CORE_PY_API PyTimer : public QObject {
    Q_OBJECT

public:
    PyTimer();

public slots:

    void onTimeout();
    void connectTimeout(py::object slot) { _pyslot = slot; }

    void setInterval(int msec) { _timer.setInterval(msec); }
    void setSingleShot(bool ss) { _timer.setSingleShot(ss); }
    void start();
    void stop();

private:
    QTimer _timer;
    py::object _pyslot;

};

#endif