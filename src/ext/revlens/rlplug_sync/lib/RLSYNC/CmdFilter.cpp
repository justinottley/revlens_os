

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
RLSYNC_CmdFilter::setCategoryPermission(QString user, QString cmdCategory, bool enabled)
{
    RLP_LOG_DEBUG(user << "category:" << cmdCategory << "value:" << enabled)

    CategoryMap* cmap = getCategoryMapForUser(user);
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

    QString cuser = _roomCntrl->currentUser();

    if (_driverMap.contains(cuser) &&
        _driverMap.value(cuser)) // == true
    {
        // We are a driver. everything is allowed
        return CoreCntrl_CommandFilterBase::FilterYes;
    }

    CategoryMap* cm = getCategoryMapForUser(cuser);
    if (cm->contains(comm->category()) &&
        cm->value(comm->category())) // == true
    {
        return CoreCntrl_CommandFilterBase::FilterYes;
    }

    return RLSYNC_CmdFilter::InvalidPermissions;

}

