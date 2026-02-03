
#include "RlpExtrevlens/RLQTDS/OnTheGoTrack.h"
#include "RlpExtrevlens/RLQTDS/OnTheGoNode.h"

#include "RlpRevlens/CNTRL/MainController.h"

RLP_SETUP_LOGGER(extrevlens, RLQTDS, OnTheGoTrack)

RLQTDS_OnTheGoTrack::RLQTDS_OnTheGoTrack(DS_Session* session, int idx, QUuid uuid, QString owner):
    DS_Track(session, idx, TRACK_TYPE_ONTHEGO, uuid, owner)
{
    CNTRL_MainController* cntrl = CNTRL_MainController::instance();
    connect(
        cntrl,
        &CNTRL_MainController::noticeEvent,
        this,
        &RLQTDS_OnTheGoTrack::onNoticeEvent
    );
}


void
RLQTDS_OnTheGoTrack::onNoticeEvent(QString evtName, QVariantMap evtInfo)
{
    if (evtName != "playlist.selection_updated")
    {
        return;
    }

    clear();

    RLP_LOG_DEBUG(evtInfo)
    QVariantList sel = evtInfo.value("selection").toList();

    qlonglong currFrame = 1;

    for (int i = 0; i != sel.size(); ++i)
    {
        QString nodeUuid = sel.at(i).toString();
        DS_NodePtr node = _session->getNodeByUuid(nodeUuid);
        if (node != nullptr)
        {
            
            QVariantMap* nprops = node->getPropertiesPtr();
            qlonglong frameCount = nprops->value("media.frame_count").toLongLong();
            qlonglong frame = nprops->value("session.frame").toLongLong();

            RLP_LOG_DEBUG("Got node at" << frame << "length:" << frameCount);

            DS_NodePtr otgNode = DS_NodePtr(new RLQTDS_OnTheGoNode(node));

            insertNode(otgNode, currFrame);
            currFrame += frameCount;
        }
    }
}