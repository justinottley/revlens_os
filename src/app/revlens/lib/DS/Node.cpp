//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#include "RlpRevlens/DS/Node.h"

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
    // DS_Graph* graph = _properties->value("graph").value<DS_Graph*>();

    // connect(
    //     graph,
    //     SIGNAL(dataReady(QVariantMap)),
    //     this,
    //     SLOT(onGraphDataReady(QVariantMap))
    // );

    QVariantMap localMap;
    _properties->value("graph.nodes").value<QVariantMap>().insert(_uuid.toString(), localMap);
}   


void
DS_Node::onGraphDataReady(QVariantMap data)
{
    // Meant to be implemented by subclasses if a node needs to respond to graph data
    // RLP_LOG_DEBUG(_name);
}


void
DS_Node::emitDataReadyToGraph(QVariantMap data)
{
    RLP_LOG_DEBUG("");

    graph()->emitDataReady(data);
}


DS_Graph*
DS_Node::graph()
{
    return _properties->value("graph").value<DS_Graph*>();
}


DS_NodePtr
DS_Node::input(int idx)
{
    DS_NodePtr result = nullptr;
    if ((idx >= 0) && (idx < _inputs.size())) {
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
    
    for (it=_inputs.begin(); it != _inputs.end(); ++it) {
        if ((*it) == nullptr) {
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