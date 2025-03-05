//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_MEDIA_FACTORY_H
#define REVLENS_MEDIA_FACTORY_H

#include "RlpRevlens/MEDIA/Global.h"

#include "RlpCore/SEQ/Util.h"

#include "RlpRevlens/DS/Graph.h"
#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/DS/Factory.h"
#include "RlpRevlens/DS/ControllerBase.h"
#include "RlpRevlens/DS/NodeFactoryPlugin.h"


#include "RlpRevlens/MEDIA/Plugin.h"

Q_DECLARE_METATYPE(QList<QFileInfo>)

class MEDIA_Locator;

class REVLENS_MEDIA_API MEDIA_Factory : public DS_Factory {
    Q_OBJECT

signals:
    void mediaCreated(DS_NodePtr node, bool avOnly);


public:
    RLP_DEFINE_LOGGER

    MEDIA_Factory(DS_ControllerPtr controller = nullptr);

    DS_NodePtr createVideoReader(QVariantMap* properties);
    DS_NodePtr createAudioReader(QVariantMap* properties);


public slots:

    static MEDIA_Factory* instance(DS_ControllerPtr controller = nullptr);

    bool registerMediaPlugin(MEDIA_Plugin* mediaPlugin);
    bool registerNodeFactoryPlugin(DS_NodeFactoryPlugin* nodePlugin);
    bool registerMediaLocator(MEDIA_Locator* locator);

    DS_NodePtr createMedia(QVariantMap mediaIn, bool avOnly=false);

    void loadMediaList(QVariantList mediaInputList, DS_Track* track);
    void loadMediaList(QVariantMap mediaData, DS_Track* track);

    void insertInitialMediaProperties(QVariantMap* properties);

    QList<MEDIA_Plugin*> getPlugins();
    MEDIA_Plugin* getMediaPluginByName(QString name);

    void setMediaPref(QString prefName, QVariant prefValue);
    QVariant getMediaPref(QString prefName);
    QVariantMap getMediaPrefs() { return _mediaPrefs; }

    // convenience function
    bool isValidFrameSequence(QString seqTemplateStr);

    QVariantMap identifyMedia(QVariantMap mediaInput);
    QVariantMap identifyMediaItem(QString mediaIn);

    QVariantMap identifyMedia2(QVariantMap mediaInput, DS_Node::NodeDataType=DS_Node::DATA_UNKNOWN);

private:

    static MEDIA_Factory* _instance;
    
    DS_ControllerPtr _controller;
    QList<MEDIA_Plugin*> _mediaPlugins;
    QMap<QString, MEDIA_Plugin*> _mediaPluginMap;

    QList<MEDIA_Locator*> _mediaLocators;

    QList<DS_NodeFactoryPlugin*> _nodeFactoryPlugins;

    // TODO FIXME: this may need to be refactored into a more general config?
    QVariantMap _mediaPrefs;
};

#endif
