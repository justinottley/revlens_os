

#include "RlpExtrevlens/RLQTDS/OnTheGoNode.h"

#include "RlpRevlens/MEDIA/Factory.h"
#include "RlpRevlens/MEDIA/ReadEvent.h"

RLQTDS_OnTheGoNode::RLQTDS_OnTheGoNode(DS_NodePtr baseNode):
    DS_Node("OnTheGo", new QVariantMap())
{
    MEDIA_Factory* factory = MEDIA_Factory::instance();
    factory->insertInitialMediaProperties(_properties);

    QVariantMap* inProps = baseNode->getPropertiesPtr();

    _properties->insert("media.frame_count", inProps->value("media.frame_count"));
    _properties->insert("media.name", inProps->value("media.name"));
    // _properties->insert("video.plugin", inProps->value("video.plugin"));

    // Update properties for lookahead
    //
    QStringList keyList = inProps->keys();
    QStringList::iterator it;

    for (it = keyList.begin(); it != keyList.end(); ++it)
    {
        QString keyName = (*it);
        if (keyName.startsWith("video."))
        {
            _properties->insert(keyName, inProps->value(keyName));
        }
    }

    _baseNode = baseNode;
}


void
RLQTDS_OnTheGoNode::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    // RLP_LOG_DEBUG(timeInfo->videoFrame())

    _baseNode->readVideo(timeInfo, destFrame, postUpdate);
}


void
RLQTDS_OnTheGoNode::scheduleReadTask(
    DS_TimePtr timeInfo,
    DS_BufferPtr destBuffer,
    DS_Node::NodeDataType dataType,
    bool optional)
{
    // RLP_LOG_DEBUG(timeInfo->videoFrame())

    _baseNode->scheduleReadTask(timeInfo, destBuffer, dataType, optional);
}