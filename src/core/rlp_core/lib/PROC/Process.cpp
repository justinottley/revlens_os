

#include "RlpCore/PROC/Process.h"
#include "RlpCore/PY/Interp.h"

RLP_SETUP_LOGGER(core, PROC, Process)

PROC_Process::PROC_Process():
    _proc(),
    _detachedPid(-1)
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
        &PROC_Process::started
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
    _proc.start();

    RLP_LOG_DEBUG(_proc.processId())
}


void
PROC_Process::startDetached()
{
    _proc.startDetached(&_detachedPid);

    RLP_LOG_DEBUG("Running Detached - PID:" << _detachedPid)
}


void
PROC_Process::stop()
{
    if (PY_Interp::state == PY_Interp::PythonInitialized)
    {
        RLP_LOG_DEBUG("Running posix_kill()")
        QVariantList args;
        if (_killTag != "")
        {
            args.append(_killTag);
        } else
        {
            args.append(_proc.processId());
        }

        PY_Interp::call("rlp.core.process.cmds.posix_killall", args);
    } else
    {
        RLP_LOG_DEBUG("Running proc kill")
        _proc.kill();
    }
    
}


void
PROC_Process::onStarted()
{
    RLP_LOG_DEBUG("")
}


bool
PROC_Process::isRunning()
{
    return (_proc.state() == QProcess::Running);
}


void
PROC_Process::onError(QProcess::ProcessError error)
{
    RLP_LOG_DEBUG(error);
}


void
PROC_Process::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    RLP_LOG_DEBUG("")

    emit finished();
}