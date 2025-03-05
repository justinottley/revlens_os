
#ifndef EXTREVLENS_RLSYNC_CMDFILTER_H
#define EXTREVLENS_RLSYNC_CMDFILTER_H

#include "RlpExtrevlens/RLSYNC/Global.h"
#include "RlpCore/CNTRL/CommandBase.h"

typedef QMap<QString, bool> CategoryMap;

class RLSYNC_RoomController;

class RLSYNC_CmdFilter : public CoreCntrl_CommandFilterBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    static const CmdFilterResult InvalidPermissions;

    RLSYNC_CmdFilter(RLSYNC_RoomController* roomCntrl);

    virtual CmdFilterResult checkCommand(CoreCntrl_CommandBase* comm, QVariant execParams);

public slots:

    void setDriver(QString user, bool isDriver);
    void setCategoryPermission(QString user, QString category, bool enabled);

private:
    CategoryMap* getCategoryMapForUser(QString user);

    RLSYNC_RoomController* _roomCntrl;
    QMap<QString, CategoryMap*> _userMap;
    QMap<QString, bool> _driverMap;

};

#endif
