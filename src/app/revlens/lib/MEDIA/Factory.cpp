//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#include <iostream>

#include "RlpRevlens/MEDIA/Factory.h"
#include "RlpRevlens/MEDIA/AVInputNode.h"
#include "RlpRevlens/MEDIA/CompositeNode.h"
#include "RlpRevlens/MEDIA/CacheNode.h"
#include "RlpRevlens/MEDIA/Locator.h"
#include "RlpRevlens/DISP/CompositePlugin.h"

// #include "RlpRevlens/MEDIA/RetimeNode.h"
#include "RlpRevlens/DS/AudioFormat.h"

#include "RlpCore/PY/Interp.h"


RLP_SETUP_LOGGER(revlens, MEDIA, Factory)

// -----

MEDIA_Factory* MEDIA_Factory::_instance = nullptr;

MEDIA_Factory::MEDIA_Factory(DS_ControllerPtr controller):
    DS_Factory(),
    _controller(controller)
{
    // Plugins are loaded at runtime via python binding

    _mediaPlugins.clear();
    _mediaPrefs.clear();
    _nodeFactoryPlugins.clear();

}


MEDIA_Factory::~MEDIA_Factory()
{
    RLP_LOG_DEBUG("")
}


MEDIA_Factory*
MEDIA_Factory::instance(DS_ControllerPtr controller)
{
    if (MEDIA_Factory::_instance == nullptr)
    {
        MEDIA_Factory::_instance = new MEDIA_Factory(controller);
    }

    return MEDIA_Factory::_instance;
}


bool
MEDIA_Factory::registerMediaPlugin(MEDIA_Plugin* mediaPlugin)
{
    RLP_LOG_DEBUG("Registering media plugin:" << mediaPlugin->name());
    _mediaPlugins.push_back(mediaPlugin);
    _mediaPluginMap.insert(mediaPlugin->name(), mediaPlugin);

    return true;
}


bool
MEDIA_Factory::registerNodeFactoryPlugin(DS_NodeFactoryPlugin* nodePlugin)
{
    RLP_LOG_DEBUG("registering node factory plugin:" << nodePlugin->name());
    _nodeFactoryPlugins.push_back(nodePlugin);

    return true;
}


bool
MEDIA_Factory::registerMediaLocator(MEDIA_Locator* locator)
{
    RLP_LOG_DEBUG("Registering locator:" << locator->name());

    _mediaLocators.push_back(locator);
    return true;
}


QList<MEDIA_Plugin*>
MEDIA_Factory::getPlugins()
{
    return _mediaPlugins;
}


MEDIA_Plugin*
MEDIA_Factory::getMediaPluginByName(QString name)
{
    if (_mediaPluginMap.contains(name))
    {
        return _mediaPluginMap.value(name);
    }

    return nullptr;
}


DS_NodeFactoryPlugin*
MEDIA_Factory::getNodeFactoryPluginByName(QString name)
{
    QList<DS_NodeFactoryPlugin*>::iterator it;
    for (it=_nodeFactoryPlugins.begin(); it != _nodeFactoryPlugins.end(); ++it)
    {
        DS_NodeFactoryPlugin* nplug = *it;
        if (nplug->name() == name)
        {
            return nplug;
        }
    }

    return nullptr;
}


QVariant
MEDIA_Factory::getMediaPref(QString prefName)
{
    if (_mediaPrefs.contains(prefName)) {
        return _mediaPrefs.value(prefName);
    }
    return QVariant();
}


void
MEDIA_Factory::setMediaPref(QString prefName, QVariant prefValue)
{
    _mediaPrefs.insert(prefName, prefValue);
}


bool
MEDIA_Factory::isValidFrameSequence(QString seqTemplateStr)
{
    SEQ_SequencePtr seq = SEQ_Util::getSequenceFromTemplateString(seqTemplateStr);
    return (seq != nullptr);
}


DS_NodePtr
MEDIA_Factory::createVideoReader(QVariantMap* properties)
{
    // RLP_LOG_DEBUG("");

    DS_NodePtr node = nullptr;

    for (int i=0; i != _mediaPlugins.size(); i++)
    {

        if (properties->value("video.format.reader").value<QString>() == _mediaPlugins[i]->name())
        {
            // RLP_LOG_DEBUG("creating video reader using" << _mediaPlugins[i]->name())

            node = _mediaPlugins[i]->createVideoReader(properties);

            QVariant pluginVariantWrapper;
            pluginVariantWrapper.setValue(_mediaPlugins[i]);

            node->setProperty("video.plugin", pluginVariantWrapper);

            break;
        }
    }

    if (node == nullptr)
    {
        RLP_LOG_ERROR("VIDEO READER CREATION FAILED");
    }

    return node;
}


DS_NodePtr
MEDIA_Factory::createAudioReader(QVariantMap* properties)
{
    // RLP_LOG_DEBUG("looking for plugin "
    //           << properties->value("audio.format.reader").toString());

    if (properties->contains("video.frame_count"))
    {
        qlonglong frameCount = properties->value("video.frame_count").toLongLong();
        float frameRate = properties->value("video.frame_rate").toFloat();
        DS_AudioFormat audioFormat(frameRate);

        qint64 maxSize = audioFormat.bytePositionForFrame(frameCount);

        // RLP_LOG_DEBUG("Inititalizing empty audio buffer for " << frameCount << "video frames -" << maxSize)

        QByteArray* emptyAudio = new QByteArray();
        emptyAudio->fill('\0', maxSize);

        QVariant v;
        v.setValue(emptyAudio);
        properties->insert("audio.data", v);
    }


    DS_NodePtr node = nullptr;

    for (int i=0; i != _mediaPlugins.size(); i++) {

        if (properties->value("audio.format.reader").toString() == _mediaPlugins[i]->name()) {

            // RLP_LOG_DEBUG("creating audio reader using \""
            //           << _mediaPlugins[i]->name());

            node = _mediaPlugins[i]->createAudioReader(properties);

            QVariant pluginVariantWrapper;
            pluginVariantWrapper.setValue(_mediaPlugins[i]);

            node->setProperty("audio.plugin", pluginVariantWrapper);

            break;
        }

    }

    if (node == nullptr)
    {
        RLP_LOG_ERROR("AUDIO READER CREATION FAILED");
    }

    return node;
}


QVariantMap
MEDIA_Factory::identifyMediaItem(QString mediaInput)
{
    RLP_LOG_DEBUG(mediaInput);

    //
    // PATH REMAPPING via rlp_core.path.Path
    //
    // Call out to python for now to do path remapping on input
    // TODO FIXME: implement in C++
    //

    QVariantList l;
    l.push_back(mediaInput);
    l.push_back(true); // resolve_components

    QVariant pyresult = PY_Interp::call("revlens.media.locate", l);


    QVariantMap result;
    if (pyresult.canConvert<QVariantMap>()) {
        result.insert(pyresult.value<QVariantMap>());

        return identifyMedia(result);

    } else {
        RLP_LOG_ERROR("revlens.media.locate - Python processing of media input failed");

    }

    return result;
}


QVariantMap
MEDIA_Factory::identifyMedia(QVariantMap mediaInput)
{
    RLP_LOG_DEBUG("");

    insertInitialMediaProperties(&mediaInput);

    QVariantList l;
    l.push_back(mediaInput);
    l.push_back(true); // resolve_components

    QVariant pyResult = PY_Interp::call("revlens.media.locate", l);

    QVariantMap result;

    if (pyResult.canConvert<QVariantMap>())
    {

        RLP_LOG_DEBUG("GOT RESULT BACK FROM resolve_components()");
        mediaInput = pyResult.value<QVariantMap>();

    } else
    {
        RLP_LOG_ERROR("Python processing of media input failed");

    }

    result.insert(mediaInput);

    QList<int> mediaTypeList;

    // RLP_LOG_DEBUG(result)


    for (int pi=0; pi != _mediaPlugins.size(); pi++)
    {
        MEDIA_Plugin* pl = _mediaPlugins[pi];
        if (!pl->isEnabled())
        {
            continue;
        }

        RLP_LOG_INFO("Attempting to identify media data using" << pl->name());

        QVariantList streamResult = pl->identifyMedia(mediaInput);

        if (streamResult.size()  == 0)
        {
            continue;
        }

        for (int si=0; si != streamResult.size(); ++si)
        {

            QVariantMap streamInfo = streamResult[si].toMap();
            if (streamInfo.contains("media_type"))
            {

                MEDIA_Plugin::MediaType resultType = (MEDIA_Plugin::MediaType)streamInfo.value("media_type").value<int>();

                if ((resultType != MEDIA_Plugin::UNKNOWN) && (mediaTypeList.indexOf((int)resultType) == -1))
                {
                    RLP_LOG_INFO("Found media type "
                                << (int)resultType << " by plugin "
                                << _mediaPlugins[pi]->name());

                    result.insert(streamInfo);

                    mediaTypeList.push_back(resultType);
                }
            }
        }


    }

    RLP_LOG_DEBUG("Identify complete")
    // RLP_LOG_DEBUG(result)

    return result;

}


QVariantMap
MEDIA_Factory::locate(QVariantMap mediaInput)
{
    // RLP_LOG_DEBUG(_mediaLocators.size() << mediaInput)

    for (int i=0; i != _mediaLocators.size(); ++i)
    {
        // RLP_LOG_DEBUG("Attempting locator" << _mediaLocators.at(i)->name())

        if (_mediaLocators.at(i)->locate(&mediaInput))
        {
            break;
        }
    }

    return mediaInput;
}


bool
MEDIA_Factory::appendMedia(QVariantMap mediaIn, DS_Track* track)
{
    // LOG_Logging::pprint(mediaIn);

    QVariantMap result = identifyMedia2(mediaIn); // , (int)DS_Node::DATA_VIDEO);

    // LOG_Logging::pprint(result);

    DS_NodePtr node = createMedia(result);
    if (node != nullptr)
    {
        track->insertNode(node, track->getMaxFrameNum() + 1);
        return true;
    }

    return false;
}


QVariantMap
MEDIA_Factory::identifyMedia2(QVariantMap mediaInput)
{
    // RLP_LOG_DEBUG(nodeDataType)

    insertInitialMediaProperties(&mediaInput);

    for (int i=0; i != _mediaLocators.size(); ++i)
    {
        // RLP_LOG_DEBUG("Attempting locator" << _mediaLocators.at(i)->name())

        if (_mediaLocators.at(i)->locate(&mediaInput))
        {
            break;
        }
    }

    QVariantMap result; // final result
    result.insert(mediaInput);

    // keep track of the media types identified so far,
    // don't attempt to identify the same type twice via two different plugins
    QList<MEDIA_Plugin::MediaType> mediaTypeList; 
    QList<DS_Node::NodeDataType> nodeTypeList;

    //  nodeDataType = (DS_Node::NodeDataType)inodeDataType;
    QList<DS_Node::NodeDataType> nodeDataTypes = { DS_Node::DATA_VIDEO, DS_Node::DATA_AUDIO };

    for (int ndt = 0; ndt != nodeDataTypes.size(); ++ndt)
    {

        DS_Node::NodeDataType inodeDataType = nodeDataTypes.at(ndt);

        for (int pi=0; pi != _mediaPlugins.size(); pi++)
        {
            MEDIA_Plugin* pl = _mediaPlugins[pi];
            if (!pl->isEnabled())
            {
                // RLP_LOG_WARN("Skipping" << pl->name() << " - disabled")
                continue;
            }

            // RLP_LOG_INFO("Attempting to identify media data using" << pl->name());

            if (nodeTypeList.contains(inodeDataType))
            {
                // RLP_LOG_DEBUG("Data type" << inodeDataType << "identified, breaking")
                break;
            }
            QVariantList streamResult = pl->identifyMedia(mediaInput, inodeDataType);
            if (streamResult.size()  == 0)
            {
                continue;
            }

            for (int si=0; si != streamResult.size(); ++si)
            {
                QVariantMap streamInfo = streamResult[si].toMap();
                if (streamInfo.contains("media_type"))
                {

                    MEDIA_Plugin::MediaType resultType = (MEDIA_Plugin::MediaType)streamInfo.value("media_type").value<int>();

                    if ((resultType != MEDIA_Plugin::UNKNOWN) && (mediaTypeList.indexOf((int)resultType) == -1))
                    {
                        // RLP_LOG_INFO("Found media type "
                        //             << (int)resultType << " by plugin "
                        //             << _mediaPlugins[pi]->name());

                        result.insert(streamInfo);
                        mediaTypeList.push_back(resultType);
                        nodeTypeList.push_back(inodeDataType);
                    }
                }
            }
        }


    }

    // RLP_LOG_DEBUG("Identify complete")

    return result;
}


void
MEDIA_Factory::insertInitialMediaProperties(QVariantMap* properties)
{
    // RLP_LOG_DEBUG("");

    QVariantMap nodeMap;

    QString graphUuidStr = "";
    if (properties->contains("graph.uuid"))
    {
        QString graphUuidIn = properties->value("graph.uuid").toString();
        // RLP_LOG_DEBUG("Found incoming media graph uuid: " << graphUuidIn)
        graphUuidStr = graphUuidIn;
    }

    if (properties->contains("graph"))
    {
        QVariant gv = properties->value("graph");
        if (gv.typeName() == QString("std::nullptr_t"))
        {
            RLP_LOG_WARN("Invalid graph object, constructing new one")
            properties->remove("graph");
        }
    }


    if (!properties->contains("graph"))
    {
        DS_Graph* graph = new DS_Graph(properties, graphUuidStr);
        QVariant graphWrapper;
        graphWrapper.setValue(graph);
        properties->insert("graph", graphWrapper);
        properties->insert("graph.uuid", graph->uuid());
        properties->insert("graph.nodes", nodeMap);

        connect(
            graph,
            &DS_Graph::dataReady,
            this,
            &MEDIA_Factory::onMediaDataReady
        );
    } else
    {
        DS_Graph* graph = properties->value("graph").value<DS_Graph*>();
        graph->setProperties(properties);
    }
}


DS_NodePtr
MEDIA_Factory::createMedia(QVariantMap mediaIn, bool avOnly)
{
    // RLP_LOG_DEBUG("")

    _controller->licenseCheck();

    // Create copy
    QVariantMap* properties = new QVariantMap();
    properties->insert(mediaIn);


    insertInitialMediaProperties(properties);


    DS_NodePtr videoNode = nullptr;
    DS_NodePtr audioNode = nullptr;
    DS_NodePtr resultNode = nullptr;

    videoNode = createVideoReader(properties);
    audioNode = createAudioReader(properties);

    // #ifdef SCAFFOLD_IOS
    // avOnly = true;
    // #endif

    /*
    if (!properties->contains("audio.format.reader")) {

        RLP_LOG_WARN("creating null audio reader");
        properties->insert("audio.format.reader", QString("NullReader"));

    }
    */

    if (videoNode != nullptr)
    {

        // RLP_LOG_DEBUG("creating AV Input node");

        DS_NodePtr avNode(new MEDIA_AVInputNode(properties, videoNode, audioNode));

        resultNode = avNode;

        if (!avOnly)
        {
            // RLP_LOG_WARN("DiskCacheNode and RetimeNode BYPASSED")

            DS_NodePtr cacheNode(new MEDIA_CacheNode(properties, avNode));
            // DS_NodePtr retimeNode(new MEDIA_RetimeNode(properties, cacheNode));

            #ifdef SCAFFOLD_IOS
            resultNode = cacheNode;
            #else
            DS_NodePtr compositeNode(new MEDIA_CompositeNode(properties, cacheNode));

            // resultNode = avNode; // retimeNode;
            resultNode = compositeNode;

            #endif

            NodeFactoryPluginListIterator it;

            for (it=_nodeFactoryPlugins.begin(); it!=_nodeFactoryPlugins.end(); ++it) {
                resultNode = (*it)->runCreateNode(resultNode, properties);
            }
        }

    }

    if (resultNode != nullptr)
    {
        emit mediaCreated(resultNode, avOnly);
    }

    return resultNode;
}


int
MEDIA_Factory::loadMediaList(QVariantList mediaInputList, DS_Track* track, qlonglong atFrame)
{
    RLP_LOG_DEBUG("")

    int numLoaded = 0;
    MEDIA_Plugin* dloader = getMediaPluginByName("DeferredLoad");

    qlonglong nextFrame = atFrame;

    track->setLoading(true);
    MEDIA_LoadGroup* lg = new MEDIA_LoadGroup(_controller, track, mediaInputList.size());
    connect(
        lg,
        &MEDIA_LoadGroup::complete,
        this,
        &MEDIA_Factory::onLoadComplete
    );

    _loadGroupMap.insert(lg->uuid(), lg);

    for (int i=0; i != mediaInputList.size(); ++i)
    {
        QVariantMap minfo = mediaInputList.at(i).toMap();

        // LOG_Logging::pprint(minfo);

        QVariantList streamList = dloader->identifyMedia(minfo);
        if (streamList.size() == 0)
        {
            RLP_LOG_ERROR("No streams found")
            continue;
        }
        QVariantMap result = streamList[0].toMap();

        QVariantMap sourceInfoProps = result.value("media.source_info").toMap();
        auto forwardSourceInfoKeys = {"media.metadata"};
        for (auto key : forwardSourceInfoKeys)
        {
            if (sourceInfoProps.contains(key))
            {
                result.insert(key, sourceInfoProps.value(key));
            }
        }
    
        result.insert("load_uuid", lg->uuid());

        DS_NodePtr node = createMedia(result);
        if (node != nullptr)
        {
            RLP_LOG_DEBUG("Inserting at" << nextFrame)
            track->insertNode(node, nextFrame);

            numLoaded++;
        }

        nextFrame += result.value("media.frame_count").toLongLong();
    }

    RLP_LOG_DEBUG("Done - emitting media_list")

    QVariantMap md;
    md.insert("track_uuid", track->uuid());
    emit mediaDataReady("media_list", md);

    
    return numLoaded;
}


int
MEDIA_Factory::loadMediaList(QVariantMap allMediaData, DS_Track* track)
{
    MEDIA_Plugin* dloader = getMediaPluginByName("DeferredLoad");

    int numLoaded = 0;

    QStringList mediaUuids = allMediaData.keys();
    QStringList::iterator mit;

    int mediaCount = 0;
    int mediaTotal = mediaUuids.size();

    track->setLoading(true);

    MEDIA_LoadGroup* lg = new MEDIA_LoadGroup(_controller, track, mediaTotal);

    // RLP_LOG_DEBUG("LoadGroup info:" << lg->uuid() << "items:" << lg->numItems())
    connect(
        lg,
        &MEDIA_LoadGroup::complete,
        this,
        &MEDIA_Factory::onLoadComplete
    );

    _loadGroupMap.insert(lg->uuid(), lg);

    for (mit = mediaUuids.begin(); mit != mediaUuids.end(); ++mit)
    {

        QString mediaUuidIn = *mit;

        RLP_LOG_DEBUG("media UUID in: " << mediaUuidIn)

        QVariantMap mediaData = allMediaData.value(*mit).toMap();
        qlonglong frame = mediaData.value("frame").toLongLong();

        QVariantMap allprops = mediaData.value("properties").toMap();
        QVariantMap sourceInfoProps = allprops.value("media.source_info").toMap();

        QVariantMap props;

        props.insert("graph.uuid", mediaUuidIn);
        // props.insert("graph.frame", frame);
        // props.insert("track.uuid", track->uuid().toString());

        // Seed some data for progress
        //
        // props.insert("progress.media_total", mediaTotal);
        // props.insert("progress.media_count", mediaCount);

        QStringList keyList = sourceInfoProps.keys();
        QStringList::iterator it;

        // Transfer all media.* properties, leave everything else
        //
        for (it=keyList.begin(); it != keyList.end(); ++it)
        {
            QString keyName = (*it);
            if (keyName.startsWith("media."))
            {
                if (keyName.indexOf("locator") == -1)
                {
                    props.insert(keyName, sourceInfoProps.value(keyName));
                }
            }
        }


        QVariantList streamList = dloader->identifyMedia(props);

        // RLP_LOG_DEBUG("Got streams:" << streamList.size())

        if (streamList.size() == 0)
        {
            RLP_LOG_ERROR("Identify failed, skipping")
            continue;
        }

        QVariantMap result = streamList[0].toMap();
        result.insert("load_uuid", lg->uuid());
        

        // Forward media keys
        //
        auto forwardKeys = {"media.metadata", "media.notes"};
        for (auto key : forwardKeys)
        {
            if (allprops.contains(key))
            {
                result.insert(key, allprops.value(key));
            }
        }

        // // Forward source info keys
        // //
        // if (mediaInput.contains("media.metadata"))
        // {
        //     properties.insert("media.metadata", mediaInput.value("media.metadata"));
        //     mediaInput.remove("media.metadata");
        // }


        DS_NodePtr node = createMedia(result);
        if (node != nullptr)
        {
            track->insertNode(node, frame);
            numLoaded++;
        }

        mediaCount++;
    }

    RLP_LOG_DEBUG("Done - emitting media_list")

    QVariantMap md;
    md.insert("track_uuid", track->uuid());
    emit mediaDataReady("media_list", md);

    return numLoaded;
}


void
MEDIA_Factory::onMediaDataReady(QString dataType, QVariantMap data)
{
    // RLP_LOG_DEBUG(dataType << data)

    if ((dataType == "video_source") || (dataType == "thumbnail_ready"))
    {
        if (data.contains("load_uuid"))
        {
            QUuid loadUuid = data.value("load_uuid").toUuid();

            if (_loadGroupMap.contains(loadUuid))
            {
                MEDIA_LoadGroup* lg = _loadGroupMap.value(loadUuid);
                lg->onMediaDataReady(dataType, data);

            } else
            {
                RLP_LOG_ERROR("unknown load_uuid:" << loadUuid);
            }

        } else
        {
            RLP_LOG_WARN("load_uuid missing from event info")
            // LOG_Logging::pprint(data);
        }
    }
}


void
MEDIA_Factory::onLoadComplete(QUuid uuid, QString dataType)
{
    // RLP_LOG_DEBUG(uuid)

    if (_loadGroupMap.contains(uuid))
    {
        // MEDIA_LoadGroup* lg = _loadGroupMap.value(uuid);

        // RLP_LOG_DEBUG("Cleaning" << lg)
        // lg->deleteLater();
        // _loadGroupMap.remove(uuid);

        if (dataType == "video_source")
        {
            // For stability
            QTimer::singleShot(10, this, [this]() {

                _controller->fullRefresh();

                // qlonglong fr = this->controller()->currentFrameNum();
                // QVariantList args;
                // args.push_back(fr);

                // _controller->runCommand("Recenter", args);
            });

        }
    }
}
