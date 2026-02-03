
#ifndef EXTREVLENS_WIPE_COMPOSITE_PLUGIN_H
#define EXTREVLENS_WIPE_COMPOSITE_PLUGIN_H

#include "RlpExtrevlens/RLQTGUI/Global.h"
#include "RlpExtrevlens/RLQTGUI/WipeCompositeSplitter.h"

#include "RlpRevlens/DISP/CompositePlugin.h"


class RLQTGUI_WipeCompositePlugin : public DISP_CompositePlugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTGUI_WipeCompositePlugin();

    DISP_CompositePlugin* duplicate()
    {
        return new RLQTGUI_WipeCompositePlugin();
    }

    void setDisplay(DISP_GLView* display);

    void layout(DS_BufferPtr destFrame, QVariantMap* displayMetadata);

    void paintGLInternalRecursive(DS_BufferPtr buf, QVariantMap* bufferMd, QVariantMap* displayMd, int recDepth);

    // void readVideo(DS_Node* node, DS_TimePtr timeInfo, DS_BufferPtr destFrame);

public slots:

    static RLQTGUI_WipeCompositePlugin*
    new_RLQTGUI_WipeCompositePlugin()
    {
        return new RLQTGUI_WipeCompositePlugin();
    }

    RLQTGUI_WipeCompositeSplitter* splitter() { return _wsplitter; }

    void onCompositeModeChanged(QString compositeMode);
    void onWipePosChanged(float pos);


private:
    RLQTGUI_WipeCompositeSplitter* _wsplitter;
};

#endif
