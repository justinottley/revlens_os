//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#include <iostream>

#include "RlpRevlens/MEDIA/Factory.h"
#include "RlpRevlens/MEDIA/AVInputNode.h"
#include "RlpRevlens/MEDIA/CompositeNode.h"
#include "RlpRevlens/MEDIA/CacheNode.h"
#include "RlpRevlens/MEDIA/Locator.h"

// #include "RlpRevlens/MEDIA/RetimeNode.h"

#include "RlpCore/PY/Interp.h"


RLP_SETUP_LOGGER(revlens, MEDIA, Factory)

MEDIA_Factory* MEDIA_Factory::_instance;

MEDIA_Factory::MEDIA_Factory(DS_ControllerPtr controller):
    DS_Factory(),
    _controller(controller)
{
    _mediaPlugins.clear();
    _mediaPrefs.clear();
    _nodeFactoryPlugins.clear();

    // Plugins are loaded at runtime via python binding
}


MEDIA_Factory*
MEDIA_Factory::instance(DS_ControllerPtr controller)
{
    if (MEDIA_Factory::_instance == NULL)
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
            RLP_LOG_DEBUG("creating video reader using" << _mediaPlugins[i]->name())

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
    RLP_LOG_DEBUG("looking for plugin "
              << properties->value("audio.format.reader").toString());

    DS_NodePtr node = nullptr;

    for (int i=0; i != _mediaPlugins.size(); i++) {

        if (properties->value("audio.format.reader").toString() == _mediaPlugins[i]->name()) {

            RLP_LOG_DEBUG("creating audio reader using \""
                      << _mediaPlugins[i]->name());

            node = _mediaPlugins[i]->createAudioReader(properties);

            QVariant pluginVariantWrapper;
            pluginVariantWrapper.setValue(_mediaPlugins[i]);

            node->setProperty("audio.plugin", pluginVariantWrapper);

            break;
        }

    }

    if (node == nullptr) {
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
MEDIA_Factory::identifyMedia2(QVariantMap mediaInput, DS_Node::NodeDataType nodeDataType)
{
    RLP_LOG_DEBUG(nodeDataType)

    insertInitialMediaProperties(&mediaInput);

    for (int i=0; i != _mediaLocators.size(); ++i)
    {
        RLP_LOG_DEBUG("Attempting locator" << _mediaLocators.at(i)->name())

        if (_mediaLocators.at(i)->locate(&mediaInput))
        {
            break;
        }
    }

    QVariantMap result; // final result
    result.insert(mediaInput);

    // keep track of the media types identified so far,
    // don't attempt to identify the same type twice via two different plugins
    QList<int> mediaTypeList; 

    for (int pi=0; pi != _mediaPlugins.size(); pi++)
    {
        MEDIA_Plugin* pl = _mediaPlugins[pi];
        if (!pl->isEnabled())
        {
            RLP_LOG_WARN("Skipping" << pl->name() << " - disabled")
            continue;
        }

        RLP_LOG_INFO("Attempting to identify media data using" << pl->name());

        QVariantList streamResult = pl->identifyMedia(mediaInput, nodeDataType);
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

    return result;
}


void
MEDIA_Factory::insertInitialMediaProperties(QVariantMap* properties)
{
    RLP_LOG_DEBUG("");

    QVariantMap nodeMap;

    QString graphUuidStr = "";
    if (properties->contains("graph.uuid"))
    {
        QString graphUuidIn = properties->value("graph.uuid").toString();
        RLP_LOG_DEBUG("Found incoming media graph uuid: " << graphUuidIn)
        graphUuidStr = graphUuidIn;
    }

    if (!properties->contains("graph"))
    {
        DS_Graph* graph = new DS_Graph(properties, graphUuidStr);
        QVariant graphWrapper;
        graphWrapper.setValue(graph);
        properties->insert("graph", graphWrapper);
        properties->insert("graph.uuid", graph->uuid());
        properties->insert("graph.nodes", nodeMap);
    }
}


DS_NodePtr
MEDIA_Factory::createMedia(QVariantMap mediaIn, bool avOnly)
{
    RLP_LOG_DEBUG("")

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


    /*
    if (!properties->contains("audio.format.reader")) {

        RLP_LOG_WARN("creating null audio reader");
        properties->insert("audio.format.reader", QString("NullReader"));

    }
    */

    if (videoNode != nullptr)
    {

        RLP_LOG_DEBUG("creating AV Input node");

        DS_NodePtr avNode(new MEDIA_AVInputNode(properties, videoNode, audioNode));

        resultNode = avNode;

        if (!avOnly)
        {
            // RLP_LOG_WARN("DiskCacheNode and RetimeNode BYPASSED")

            DS_NodePtr cacheNode(new MEDIA_CacheNode(properties, avNode));
            // DS_NodePtr retimeNode(new MEDIA_RetimeNode(properties, cacheNode));

            DS_NodePtr compositeNode(new MEDIA_CompositeNode(properties, cacheNode));

            // resultNode = avNode; // retimeNode;
            resultNode = compositeNode;

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


void
MEDIA_Factory::loadMediaList(QVariantList mediaInputList, DS_Track* track)
{
    RLP_LOG_DEBUG("")

    MEDIA_Plugin* dloader = getMediaPluginByName("DeferredLoad");

    qlonglong nextFrame = 1;

    for (int i=0; i != mediaInputList.size(); ++i)
    {
        QVariantMap minfo = mediaInputList.at(i).toMap();
        QVariantList streamList = dloader->identifyMedia(minfo);
        QVariantMap result = streamList[0].toMap();

        DS_NodePtr node = createMedia(result);
        if (node != nullptr)
        {
            RLP_LOG_DEBUG("Inserting at" << nextFrame)
            track->insertNode(node, nextFrame);
        }

        nextFrame += result.value("media.frame_count").toLongLong();
    }

    RLP_LOG_DEBUG("DONE")
}


void
MEDIA_Factory::loadMediaList(QVariantMap allMediaData, DS_Track* track)
{
    MEDIA_Plugin* dloader = getMediaPluginByName("DeferredLoad");

    int numLoaded = 0;

    QStringList mediaUuids = allMediaData.keys();
    QStringList::iterator mit;

    int mediaCount = 0;
    int mediaTotal = mediaUuids.size();

    for (mit = mediaUuids.begin(); mit != mediaUuids.end(); ++mit)
    {

        QString mediaUuidIn = *mit;

        RLP_LOG_DEBUG("media UUID in: " << mediaUuidIn)

        QVariantMap mediaData = allMediaData.value(*mit).toMap();
        qlonglong frame = mediaData.value("frame").toLongLong();

        QVariantMap allprops = mediaData.value("properties").toMap();
        QVariantMap props;

        props.insert("graph.uuid", mediaUuidIn);
        // props.insert("graph.frame", frame);
        // props.insert("track.uuid", track->uuid().toString());

        // Seed some data for progress
        //
        // props.insert("progress.media_total", mediaTotal);
        // props.insert("progress.media_count", mediaCount);

        QStringList keyList = allprops.keys();
        QStringList::iterator it;

        // Transfer all media.* properties, leave everything else
        //
        for (it=keyList.begin(); it != keyList.end(); ++it)
        {
            QString keyName = (*it);

            // if (keyName == "media.video")
            // {
            //     RLP_LOG_DEBUG("SKIPPING" << keyName << "=" << allprops.value(keyName))
            //     continue;
            // }

            // if (keyName == "media.audio")
            // {
            //     RLP_LOG_DEBUG("SKIPPING" << keyName << "=" << allprops.value(keyName))
            //     continue;
            // }


            if (keyName.startsWith("media."))
            {
                if (keyName.indexOf("locator") == -1)
                {
                    props.insert(keyName, allprops.value(keyName));
                }

            }
        }

        QVariantList streamList = dloader->identifyMedia(props);
        QVariantMap result = streamList[0].toMap();

        // QVariantMap iprops = mediaFactory->identifyMedia(props);

        DS_NodePtr node = createMedia(result);
        if (node != nullptr)
        {
            track->insertNode(node, frame);
            numLoaded++;
        }

        mediaCount++;
    }
}