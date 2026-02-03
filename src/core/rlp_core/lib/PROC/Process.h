#ifndef CORE_PROC_PROCESS_H
#define CORE_PROC_PROCESS_H

#include "RlpCore/PROC/Global.h"

class CORE_PROC_API PROC_Process : public QObject {
    Q_OBJECT

signals:
    void outputReady(QString output);
    void started();
    void finished();

public:
    RLP_DEFINE_LOGGER
    
    PROC_Process();


public slots:

    static PROC_Process*
    new_PROC_Process()
    {
        return new PROC_Process();
    }


    void setCommand(QString cmd);
    void setProgram(QString prog) { _proc.setProgram(prog); }
    void setArguments(QStringList args) { _proc.setArguments(args); }
    void setKillTag(QString tag) { _killTag = tag; }

    void start();
    void startDetached();
    void stop();

    void readStderr();
    void readStdout();

    void onError(QProcess::ProcessError error);
    void onStarted();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

    int pid() { return _proc.processId(); }

    bool isRunning();
    void setRunning(bool isRunning) { _running = isRunning; }

protected:
    QProcess _proc;
    qint64 _detachedPid;
    bool _running;
    QString _killTag;

};

#endif