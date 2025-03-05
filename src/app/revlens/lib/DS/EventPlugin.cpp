//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#include "RlpRevlens/DS/EventPlugin.h"

RLP_SETUP_LOGGER(revlens, DS, EventPlugin)

DS_EventPlugin::DS_EventPlugin(QString name, bool enabled):
    DS_Plugin(name, enabled)
{
    _childPlugins.clear();
}


DS_EventPlugin::~DS_EventPlugin()
{
    // RLP_LOG_DEBUG("")
}


DS_EventPlugin*
DS_EventPlugin::asEventClass()
{
    return dynamic_cast<DS_EventPlugin*>(this);
}


void
DS_EventPlugin::setEnabled(bool enabled)
{
    RLP_LOG_DEBUG(_name << enabled)

    _enabled = enabled;

    for (int ci=0; ci != _childPlugins.size(); ci++)
    {
        _childPlugins.at(ci)->setEnabled(enabled);
    }
}