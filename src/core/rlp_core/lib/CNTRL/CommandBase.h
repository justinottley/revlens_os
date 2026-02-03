
#ifndef CORE_CNTRL_COMMAND_H
#define CORE_CNTRL_COMMAND_H

#include "RlpCore/CNTRL/Global.h"

typedef std::pair<bool, QString> CmdFilterResult;


class CoreCntrl_CommandBase : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    CoreCntrl_CommandBase(QString name, QString category=QString(""));

public slots:

    const QString name() { return _name; }
    const QString category() { return _category; }
    const QList<QString> args() { return _params.keys(); }
    const QVariantMap params() { return _params; }

    virtual QVariant run(QVariant execParams=QVariant()) const;

protected:
    const QString _name;
    const QString _category;
    const QVariantMap _params; // input argument info accepted by this command

};

class CoreCntrl_CommandFilterBase : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    CoreCntrl_CommandFilterBase(QString name):
        _name(name)
    {
    }

    static const CmdFilterResult FilterYes;

    virtual CmdFilterResult checkCommand(CoreCntrl_CommandBase* comm, QVariant execParams)
    {
        return FilterYes;
    }

public slots:
    QString name() { return _name; }

protected:
    QString _name;
};


#endif
