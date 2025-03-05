//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_NODEFACTORY_PLUGIN_H
#define REVLENS_DS_NODEFACTORY_PLUGIN_H

#include "RlpRevlens/DS/Global.h"
#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/DS/Plugin.h"

class DS_NodeFactoryPlugin;
typedef QList<DS_NodeFactoryPlugin*>::iterator  NodeFactoryPluginListIterator;

class REVLENS_DS_API    DS_NodeFactoryPlugin : public DS_Plugin {
    Q_OBJECT
    
public:
    DS_NodeFactoryPlugin(QString name):
        DS_Plugin(name)
    {
    }
    
    
public slots:
    
    virtual DS_NodePtr runCreateNode(DS_NodePtr nodeIn, QVariantMap* properties) 
    {
        return nodeIn;
    }
    
};

#endif
