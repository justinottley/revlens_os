//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/CNTRL/PluginManager.h"

#include "RlpRevlens/DS/EventPlugin.h"
#include "RlpRevlens/DS/NodeFactoryPlugin.h"

// #include "RlpRevlens/DISP/GlslWidget.h"
#include "RlpRevlens/DISP/GLView.h"

#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/MEDIA/Plugin.h"
#include "RlpRevlens/MEDIA/Factory.h"
#include "RlpRevlens/MEDIA/Locator.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/MediaManager.h"


RLP_SETUP_LOGGER(revlens, CNTRL, PluginManager)

CNTRL_PluginManager::CNTRL_PluginManager(CNTRL_MainController* controller) :
    _controller(controller)
{
}


bool
CNTRL_PluginManager::registerEventPlugin(DS_EventPlugin* plugin)
{
    RLP_LOG_DEBUG(plugin->name());
    
    plugin->setMainController(_controller);
    _controller->registerEventPlugin(plugin);
    _controller->getVideoManager()->registerEventPlugin(plugin);
    
    return true;
}


bool
CNTRL_PluginManager::registerMediaPlugin(MEDIA_Plugin* plugin)
{
    RLP_LOG_DEBUG(plugin->name());

    plugin->setMainController(_controller);
    return _controller->getMediaManager()
                      ->getMediaFactory()
                      ->registerMediaPlugin(plugin);
}


bool
CNTRL_PluginManager::registerMediaLocator(MEDIA_Locator* plugin)
{
    RLP_LOG_DEBUG(plugin->name())

    return _controller->getMediaManager()->getMediaFactory()->registerMediaLocator(plugin);
}


QVariantList
CNTRL_PluginManager::getMediaPlugins()
{
    QList<MEDIA_Plugin*> plugins = _controller->getMediaManager()
                                               ->getMediaFactory()
                                               ->getPlugins();
    
    QList<QVariant> result;
    for (int i=0; i != plugins.size(); ++i)
    {
        QVariant vw;
        vw.setValue(plugins.at(i));
        result.append(vw);
    }

    return result;
}


QVariant
CNTRL_PluginManager::getMediaPlugin(QString name)
{
    QList<MEDIA_Plugin*> plugins = _controller->getMediaManager()
                                               ->getMediaFactory()
                                               ->getPlugins();

    for (int i=0; i != plugins.size(); ++i)
    {
        if (plugins.at(i)->name() == name)
        {
            QVariant vw;
            vw.setValue(plugins.at(i));
            return vw;
        }
    }

    return QVariant();
}


bool
CNTRL_PluginManager::registerNodeFactoryPlugin(DS_NodeFactoryPlugin* plugin)
{
    RLP_LOG_DEBUG(plugin->name());
    
    plugin->setMainController(_controller);
    return _controller->getMediaManager()
                      ->getMediaFactory()
                      ->registerNodeFactoryPlugin(plugin);
}


bool
CNTRL_PluginManager::registerVideoProbe(DS_SyncProbe* probe)
{
    probe->setMainController(_controller);
    return _controller->getVideoManager()->registerProbe(probe);
}


bool
CNTRL_PluginManager::registerPaintEngine(QString name, PaintEngineCreateFunc func)
{
    _controller->getVideoManager()->registerPaintEngine(name, func);
    return true;
}


bool
CNTRL_PluginManager::registerToolGUI(QVariantMap toolInfo)
{
    QString toolName = toolInfo.value("name").value<QString>();
    RLP_LOG_DEBUG(toolName)


    emit registerToolRequested(toolInfo);

    return true;
}
