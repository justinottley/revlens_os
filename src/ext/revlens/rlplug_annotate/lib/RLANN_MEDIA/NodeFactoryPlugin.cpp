//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/MEDIA/Plugin.h"

#include "RlpExtrevlens/RLANN_DS/DrawControllerBase.h"
#include "RlpExtrevlens/RLANN_MEDIA/NodeFactoryPlugin.h"
#include "RlpExtrevlens/RLANN_MEDIA/DrawNode.h"
#include "RlpExtrevlens/RLANN_MEDIA/Plugin.h"

RLP_SETUP_EXT_LOGGER(RLANN_MEDIA, NodeFactoryPlugin)

RLANN_MEDIA_NodeFactoryPlugin::RLANN_MEDIA_NodeFactoryPlugin(
        RLANN_DS_DrawControllerBase* drawController,
        RLANN_MEDIA_Plugin* mediaPlugin):
    DS_NodeFactoryPlugin(QString("RLANN_MEDIA_NodeFactory")),
    _drawController(drawController),
    _mediaPlugin(mediaPlugin)
{
}


RLANN_MEDIA_NodeFactoryPlugin::RLANN_MEDIA_NodeFactoryPlugin():
    DS_NodeFactoryPlugin(QString("RLANN_MEDIA_NodeFactory"))
{
}


void
RLANN_MEDIA_NodeFactoryPlugin::setDrawController(RLANN_DS_DrawControllerBase* drawController)
{
    RLP_LOG_DEBUG(drawController)

    _drawController = drawController;
}


void
RLANN_MEDIA_NodeFactoryPlugin::setMediaPlugin(RLANN_MEDIA_Plugin* plugin)
{
    RLP_LOG_DEBUG(plugin)

    _mediaPlugin = plugin;
}


DS_NodePtr
RLANN_MEDIA_NodeFactoryPlugin::runCreateNode(DS_NodePtr nodeIn, QVariantMap* properties)
{
    RLP_LOG_DEBUG("");
    
    return DS_NodePtr(new RLANN_MEDIA_Node(_drawController, properties, nodeIn));
}

