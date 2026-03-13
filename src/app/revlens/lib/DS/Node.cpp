//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/DS/Track.h"

RLP_SETUP_LOGGER(revlens, DS, Node)

DS_Node::DS_Node(QString name) :
    _name(name),
    _properties(new QVariantMap()),
    _uuid(QUuid::createUuid())
{
    initLocalProperties();
}


DS_Node::DS_Node(QString name, QVariantMap* properties) :
    _name(name),
    _properties(properties),
    _uuid(QUuid::createUuid())
{
    initLocalProperties();
}


DS_Node::DS_Node(QString name, QVariantMap* properties, QString uuidStr) :
    _name(name),
    _properties(properties),
    _uuid(uuidStr)
{
    initLocalProperties();
}


DS_Node::~DS_Node()
{
    _inputs.clear();
}


void
DS_Node::initLocalProperties()
{
    if (_properties->contains("graph"))
    {
        _graph = _properties->value("graph").value<DS_Graph*>();
    } else
    {
        RLP_LOG_WARN("No graph in properties, this is likely to cause errors later");
    }

    QVariantMap localMap;
    _properties->value("graph.nodes").value<QVariantMap>().insert(_uuid.toString(), localMap);

}


bool
DS_Node::checkFlagByName(QString flagName)
{
    return _graph->flags()->checkFlagByName(flagName);
}


bool
DS_Node::setFlagByName(QString flagName, bool val)
{
    return _graph->flags()->setFlagByName(flagName, val);
}


void
DS_Node::emitDataReadyToGraph(QString dataType, QVariantMap data)
{
    if (_properties->contains("session.track"))
    {
        int trackIdx = _properties->value("session.track").value<DS_Track*>()->index();
        data.insert("track_idx", trackIdx);
    }

    _graph->emitDataReady(dataType, data);
}


/*
DS_Graph*
DS_Node::graph()
{
    return _properties->value("graph").value<DS_Graph*>();
}
*/


DS_NodePtr
DS_Node::input(int idx)
{
    DS_NodePtr result = nullptr;
    if ((idx >= 0) && (idx < _inputs.size()))
    {
        result = _inputs[idx];
    }

    return result;
}


DS_TimePtr
DS_Node::makeTime(long frameNum)
{
    DS_TimePtr result = nullptr;

    float frameRate = getProperty<float>("video.frame_rate");
    long sessionFrame = getProperty<long>("session.frame");

    if ((frameRate > 0) && (sessionFrame > 0))
    {

        // RLP_LOG_DEBUG(frameNum << " with framerate: " << frameRate << " session frame: " << sessionFrame);

        result = DS_TimePtr(new DS_Time(frameRate));
        result->setVideoFrameOffset(sessionFrame);
        result->setVideoFrame(frameNum);
    }

    if (result == nullptr)
    {
        RLP_LOG_ERROR("Failed for" << frameNum << "frameRate:" << frameRate << "sessionFrame:" << sessionFrame)
    }

    return result;
}


QJsonObject
DS_Node::toJson()
{
    QJsonObject result;
    QJsonArray inputList;
    
    NodeInputListIterator it;
    
    for (it=_inputs.begin(); it != _inputs.end(); ++it)
    {
        if ((*it) == nullptr)
        {
            RLP_LOG_DEBUG("NULL NODE, skipping");
            continue;
        }
        inputList.append((*it)->toJson());
    }

    result.insert("uuid", _uuid.toString());
    result.insert("inputs", inputList);
    result.insert("name", name());
    
    return result;
}