
#ifndef EXTREVLENS_RLOCIO_INIT_H
#define EXTREVLENS_RLOCIO_INIT_H

#include "RlpExtrevlens/RLOCIO/Global.h"

#include "RlpExtrevlens/RLOCIO/OverlayTools.h"

class EXTREVLENS_RLOCIO_API RLOCIO_Init : public QObject {
    Q_OBJECT

    
public:
    RLP_DEFINE_LOGGER

public slots:
    static void init();

    static RLOCIO_Init* instance() { return _INSTANCE; }

    RLOCIO_OverlayTools* overlayTools() { return _otools; }

    void initTools();
    void onToolCreated(QVariantMap toolInfo);

private:
    static RLOCIO_Init* _INSTANCE;
    
    RLOCIO_OverlayTools* _otools;
};


#endif