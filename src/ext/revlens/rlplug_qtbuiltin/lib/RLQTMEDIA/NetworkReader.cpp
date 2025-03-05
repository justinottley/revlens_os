
#include "RlpExtrevlens/RLQTMEDIA/NetworkReader.h"
#include "RlpExtrevlens/RLQTMEDIA/DeferredLoadPlugin.h"
#include "RlpExtrevlens/RLQTMEDIA/NetworkReadHandler.h"

#include "RlpRevlens/CNTRL/MainController.h"

#include "RlpRevlens/MEDIA/ReadEvent.h"

RLP_SETUP_LOGGER(ext, RLQTMEDIA, NetworkReader)


RLQTMEDIA_NetworkReader::RLQTMEDIA_NetworkReader(
    QVariantMap* properties, DS_Node::NodeDataType dataType
):
    DS_Node("RLQTMEDIA_NetworkReader", properties),
    _dataType(dataType)
{
    _srcInfo = properties->value("media.source_info").toMap();
    _srcPath = _srcInfo.value("media.video").toString();

    RLQTMEDIA_NetworkReadManager* nmgr = RLQTMEDIA_NetworkPool::instance()->nmgr();

    nmgr->requestLoad(_srcPath, this);
}


void
RLQTMEDIA_NetworkReader::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    RLQTMEDIA_NetworkReadManager* nmgr = RLQTMEDIA_NetworkPool::instance()->nmgr();
    nmgr->requestFrame(_srcPath, this, timeInfo, destFrame);
}
