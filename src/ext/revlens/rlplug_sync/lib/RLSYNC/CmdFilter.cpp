

#include "RlpExtrevlens/RLSYNC/CmdFilter.h"
#include "RlpExtrevlens/RLSYNC/RoomController.h"

RLP_SETUP_LOGGER(extrevlens, RLSYNC, CmdFilter)

const CmdFilterResult RLSYNC_CmdFilter::InvalidPermissions = {false, "invalid permissions"};

RLSYNC_CmdFilter::RLSYNC_CmdFilter(RLSYNC_RoomController* roomCntrl):
    CoreCntrl_CommandFilterBase("Sync"),
    _roomCntrl(roomCntrl)
{
    _userMap.clear();
    _driverMap.clear();
}


CategoryMap*
RLSYNC_CmdFilter::getCategoryMapForUser(QString user)
{
    if (!_userMap.contains(user))
    {
        RLP_LOG_DEBUG("Creating new CategoryMap for" << user)
        CategoryMap* cm = new CategoryMap();
        _userMap.insert(user, cm);
    }

    return _userMap.value(user);
}


void
RLSYNC_CmdFilter::setDriver(QString username, bool isDriver)
{
    RLP_LOG_DEBUG(username << ":" << isDriver)

    _driverMap.insert(username, isDriver);
}


void
RLSYNC_CmdFilter::setCategoryPermission(QString userIdent, QString cmdCategory, bool enabled)
{
    RLP_LOG_DEBUG(userIdent << "category:" << cmdCategory << "value:" << enabled)

    CategoryMap* cmap = getCategoryMapForUser(userIdent);
    cmap->insert(cmdCategory, enabled);
}


CmdFilterResult
RLSYNC_CmdFilter::checkCommand(CoreCntrl_CommandBase* comm, QVariant execParams)
{
    // RLP_LOG_DEBUG(comm->name() << comm->category())

    if (!_roomCntrl->inSyncSession())
    {
        return CoreCntrl_CommandFilterBase::FilterYes;
    }

    QString cuserIdent = _roomCntrl->currentUserIdent();

    // RLP_LOG_DEBUG(cuserIdent)

    if (_driverMap.contains(cuserIdent) &&
        _driverMap.value(cuserIdent)) // == true
    {
        // We are a driver. everything is allowed
        return CoreCntrl_CommandFilterBase::FilterYes;
    }

    CategoryMap* cm = getCategoryMapForUser(cuserIdent);
    if (cm->contains(comm->category()) &&
        cm->value(comm->category())) // == true
    {
        return CoreCntrl_CommandFilterBase::FilterYes;
    }

    return RLSYNC_CmdFilter::InvalidPermissions;

}

