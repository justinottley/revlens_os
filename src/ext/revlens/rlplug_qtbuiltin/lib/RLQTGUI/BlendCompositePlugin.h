
#ifndef EXTREVLENS_RLQTGUI_BLENDCOMPOSITE_PLUGIN_H
#define EXTREVLENS_RLQTGUI_BLENDCOMPOSITE_PLUGIN_H

#include "RlpExtrevlens/RLQTGUI/Global.h"

#include "RlpRevlens/DISP/CompositePlugin.h"

#include "RlpExtrevlens/RLQTGUI/BlendCompositeSlider.h"


class RLQTGUI_BlendCompositePlugin : public DISP_CompositePlugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTGUI_BlendCompositePlugin();

    DISP_CompositePlugin* duplicate()
    {
        return new RLQTGUI_BlendCompositePlugin();
    }

    void setDisplay(DISP_GLView* display);

    void layout(DS_BufferPtr destFrame, QVariantMap* displayMetadata);

    // void paintGLInternalRecursive(DS_BufferPtr buf, QVariantMap* bufferMd, QVariantMap* displayMd, int recDepth);

    // void readVideo(DS_Node* node, DS_TimePtr timeInfo, DS_BufferPtr destFrame);

public slots:

    void onCompositeModeChanged(QString compositeMode);
    void onBlendPosChanged(float val);


private:
    RLQTGUI_BlendCompositeSlider* _bslider;

};

#endif
