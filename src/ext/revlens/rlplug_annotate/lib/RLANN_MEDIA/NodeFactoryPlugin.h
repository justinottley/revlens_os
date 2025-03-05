//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_MEDIA_NODEFACTORYPLUGIN_H
#define EXTREVLENS_RLANN_MEDIA_NODEFACTORYPLUGIN_H

#include "RlpExtrevlens/RLANN_MEDIA/Global.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/DS/NodeFactoryPlugin.h"

#include "RlpExtrevlens/RLANN_DS/DrawControllerBase.h"


class RLANN_MEDIA_Plugin;

class EXTREVLENS_RLANN_MEDIA_API RLANN_MEDIA_NodeFactoryPlugin : public DS_NodeFactoryPlugin {
    Q_OBJECT
    
public:

    RLP_DEFINE_LOGGER
    
    RLANN_MEDIA_NodeFactoryPlugin();
    RLANN_MEDIA_NodeFactoryPlugin(RLANN_DS_DrawControllerBase* drawController,
                                 RLANN_MEDIA_Plugin* mediaPlugin);
    
public slots:

    void setDrawController(RLANN_DS_DrawControllerBase* drawController);

    void setMediaPlugin(RLANN_MEDIA_Plugin* plugin);

    DS_NodePtr runCreateNode(DS_NodePtr nodeIn, QVariantMap* properties);
    
private:
    
    RLANN_DS_DrawControllerBase* _drawController;
    RLANN_MEDIA_Plugin* _mediaPlugin;
};

#endif
