#ifndef CORE_PROC_PROCESS_H
#define CORE_PROC_PROCESS_H

#include "RlpCore/PROC/Global.h"

class CORE_PROC_API PROC_Process : public QObject {
    Q_OBJECT

signals:
    void outputReady(QString output);

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
    void start();

    void readStderr();
    void readStdout();

    void onError(QProcess::ProcessError error);
    void onStarted();

protected:
    QProcess _proc;

};

#endif