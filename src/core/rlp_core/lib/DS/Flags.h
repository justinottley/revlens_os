
#ifndef CORE_DS_FLAGS_H
#define CORE_DS_FLAGS_H

#include "RlpCore/DS/Global.h"

class CORE_DS_API CoreDs_Flags : public QObject {
    Q_OBJECT

signals:
    void flagStateChanged(QString flagName, bool flagVal);

public:
    RLP_DEFINE_LOGGER

    CoreDs_Flags();

public slots:

    static bool registerFlag(QString flagName);

    bool checkFlagByName(QString flagName);
    bool checkFlagByEnum(int flagEnum);

    bool setFlagByName(QString flagName, bool val);
    bool setFlagByEnum(int flagEnum, bool val);

    int getFlagEnum(QString name);
    QString getFlagName(int flagEnum);

    QVariantList flagInfo();


protected:
    static int _ENUMS;
    static QMutex* _allLock;

    QMutex* _lock;


    QHash<QString, bool> _flagNameValMap;
    QHash<int, bool> _flagEnumValMap;

    static QHash<QString, int> _flagNameToEnumMap;
    static QHash<int, QString> _flagEnumToNameMap;

};

#endif
