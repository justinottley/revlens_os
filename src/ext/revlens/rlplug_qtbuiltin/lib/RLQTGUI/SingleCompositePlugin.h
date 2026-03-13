
#ifndef EXTREVLENS_SINGLE_SELECT_COMPOSITE_PLUGIN_H
#define EXTREVLENS_SINGLE_SELECT_COMPOSITE_PLUGIN_H

#include "RlpExtrevlens/RLQTGUI/Global.h"

#include "RlpRevlens/DISP/CompositePlugin.h"


class RLQTGUI_SingleCompositePlugin : public DISP_CompositePlugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTGUI_SingleCompositePlugin();

    DISP_CompositePlugin* duplicate()
    {
        return new RLQTGUI_SingleCompositePlugin();
    }

    void layout(DS_BufferPtr destFrame, QVariantMap* displayMetadata);


public slots:

    static RLQTGUI_SingleCompositePlugin*
    new_RLQTGUI_SingleCompositePlugin()
    {
        return new RLQTGUI_SingleCompositePlugin();
    }


};

#endif
