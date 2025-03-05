

#include "RlpCore/PROC/Process.h"

RLP_SETUP_LOGGER(core, PROC, Process)

PROC_Process::PROC_Process():
    _proc()
{
    _proc.setProcessChannelMode(QProcess::MergedChannels);

    connect(
        &_proc,
        &QProcess::readyReadStandardError,
        this,
        &PROC_Process::readStderr
    );

    connect(
        &_proc,
        &QProcess::readyReadStandardOutput,
        this,
        &PROC_Process::readStdout
    );

    connect(
        &_proc,
        &QProcess::started,
        this,
        &PROC_Process::onStarted
    );

    connect(
        &_proc,
        &QProcess::errorOccurred,
        this,
        &PROC_Process::onError
    );
}


void
PROC_Process::setCommand(QString command)
{
    RLP_LOG_DEBUG(command)

    _proc.setProgram(command);
}


void
PROC_Process::readStderr()
{
    RLP_LOG_DEBUG("")

    QByteArray result = _proc.readAllStandardError();
    QString rstr(result);

    emit outputReady(rstr);
}


void
PROC_Process::readStdout()
{
    
    QByteArray result = _proc.readAllStandardOutput();
    QString rstr(result);

    // RLP_LOG_DEBUG(rstr)

    emit outputReady(rstr);
}


void
PROC_Process::start()
{
    RLP_LOG_DEBUG("")

    _proc.start();
}


void
PROC_Process::onStarted()
{
    RLP_LOG_DEBUG("")
}


void
PROC_Process::onError(QProcess::ProcessError error)
{
    RLP_LOG_DEBUG(error);
}