#ifndef CORE_LOG_LOGGER_H
#define CORE_LOG_LOGGER_H

#include "RlpCore/LOG/Global.h"

class CORE_LOG_API LOG_Logger : public QObject {
    Q_OBJECT

public:

    LOG_Logger(QString nameIn, int loglevelIn=-1, LOG_Logger* parent=nullptr);
    
    static QStringList topslice(QStringList logNameList);


public slots:

    QString name() { return _name; }
    int loglevel() { return _loglevel; }
    QString loglevelName(int loglevel);

    LOG_Logger* getLogger(QStringList logNameList);
    bool isActive(int loglevelIn);

    void setLevel(int loglevel) { _loglevel = loglevel; }

private:

    QString _name;
    int _loglevel;

    QMap<QString, LOG_Logger*> _loggers;
    LOG_Logger* _parent;
    
    QMutex* _lock;
};


#endif