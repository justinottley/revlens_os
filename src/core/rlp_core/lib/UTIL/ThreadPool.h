
#ifndef CORE_UTIL_WASMCOMPAT_H
#define CORE_UTIL_WASMCOMPAT_H

#ifdef SCAFFOLD_WASM

#include <QtCore/QObject>
#include <QtCore/QRunnable>

class QThreadPool : public QObject {
    Q_OBJECT

public:
    QThreadPool(QObject* parent = nullptr) {}
    void setMaxThreadCount(int max) {}
    void start(QRunnable* task) { task->run(); }
    void tryStart(QRunnable *task) { task->run(); }

};

#else

#include <QtCore/QThreadPool>

#endif

#endif