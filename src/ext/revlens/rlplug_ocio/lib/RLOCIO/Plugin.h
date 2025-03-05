
#ifndef EXTREVLENS_RLOCIO_PLUGIN_H
#define EXTREVLENS_RLOCIO_PLUGIN_H

#include "RlpExtrevlens/RLOCIO/Global.h"
#include "RlpRevlens/DS/EventPlugin.h"

#include "RlpExtrevlens/RLOCIO/OverlayTools.h"

class EXTREVLENS_RLOCIO_API RLOCIO_Plugin : public DS_EventPlugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLOCIO_Plugin();

    DS_EventPlugin* duplicate() { return new RLOCIO_Plugin(); }

public slots:
    QString getSettingsUI() { return "rlplug_ocio.settings"; }
    void onToolCreated(QVariantMap toolInfo);
    RLOCIO_OverlayTools* overlayTools() { return _otools; }

private:
    RLOCIO_OverlayTools* _otools;

};

#endif