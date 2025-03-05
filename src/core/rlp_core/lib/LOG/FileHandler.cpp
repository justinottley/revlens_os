
#include "RlpCore/LOG/FileHandler.h"


LOG_FileHandler::LOG_FileHandler(QString filePath)
{
    #ifndef SCAFFOLD_WASM

    _lock = new QMutex();

    if (filePath == "")
    {
        QString fileName("rlp.%1.%2.log");
        fileName = fileName.arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationPid());

        filePath = QDir::tempPath();
        filePath += "/";
        filePath += fileName;
    }
    
    _logfile.setFileName(filePath);

    if (_logfile.open(QIODevice::WriteOnly))
    {
        qInfo() << "Logging to: " << filePath;
        _ok = true;

        #ifndef SCAFFOLD_IOS
        #ifndef SCAFFOLD_WASM

        QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
        QString showLog = e.value("RLP_SHOW_LOG");
        if (showLog.size() != 0)
        {
            QString cmd = "gnome-terminal";
            QStringList args;
            args.append("--");
            args.append("tail");
            args.append("-f");
            args.append(filePath);
            QProcess::startDetached(cmd, args);


            QString ccmd = "code";
            QStringList cargs;
            cargs.append(filePath);
            QProcess::startDetached(ccmd, cargs);

        }

        #endif
        #endif

    } else {
        _ok = false;
        qInfo() << "ERROR: unable to open for write: " << filePath;
    }

    #endif

}


void
LOG_FileHandler::handleLog(QString msg)
{
    QMutexLocker l(_lock);

    if (!_ok)
        return;

    #ifdef SCAFFOLD_WASM
    qInfo() << msg;
    #else
    QTextStream s(&_logfile);
    s << QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss.z ").leftJustified(23) << msg << "\n";
    #endif
}