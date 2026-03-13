
#ifndef RLOCIO_OVERLAY_TOOLS_H
#define RLOCIO_OVERLAY_TOOLS_H

#include "RlpExtrevlens/RLOCIO/Global.h"
#include "RlpExtrevlens/RLOCIO/PaintEngine.h"

#include "RlpRevlens/DISP/GLPaintEngine2D.h"
#include "RlpRevlens/DISP/GLView.h"

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/HLayout.h"
#include "RlpGui/PANE/SvgButton.h"
#include "RlpGui/WIDGET/IconButton.h"
#include "RlpGui/WIDGET/Label.h"
#include "RlpGui/WIDGET/Slider.h"

class EXTREVLENS_RLOCIO_API RLOCIO_OverlayTools : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLOCIO_OverlayTools(GUI_ItemBase* parent);

    void OCIOPopulateMenus();
    void OCIOInit(DISP_GLPaintEngine2D* pe);

    void onParentSizeChanged(qreal width, qreal height);

public slots:

    static RLOCIO_OverlayTools*
    new_RLOCIO_OverlayTools(GUI_ItemBase* parent)
    {
        return new RLOCIO_OverlayTools(parent);
    }

    void onInputColorSpaceChanged(QVariantMap info);
    void onTransformChanged(QVariantMap info);
    void onDisplayChanged(QVariantMap info);

    void incExposure(QVariantMap info);
    void decExposure(QVariantMap info);
    void setExposure(float val);

    void setSaturation(float val);

    void incGamma(QVariantMap info);
    void decGamma(QVariantMap info);
    void setGamma(float val);

    void setChanR(QVariantMap info);
    void setChanG(QVariantMap info);
    void setChanB(QVariantMap info);
    void setChanRGB(QVariantMap info);


private:
    GUI_IconButton* _inputSel;
    GUI_IconButton* _transformSel;
    GUI_IconButton* _displaySel;

    GUI_SvgButton* _exposureLbl;
    GUI_Slider* _exposureSlider;
    // GUI_SvgButton* _exposurePlusBtn;
    // GUI_SvgButton* _exposureMinusBtn;
    // GUI_Label* _exposureVal;

    GUI_SvgButton* _gammaLbl;
    GUI_Slider* _gammaSlider;
    // GUI_SvgButton* _gammaPlusBtn;
    // GUI_SvgButton* _gammaMinusBtn;
    // GUI_Label* _gammaVal;

    GUI_Label* _colBg;

    GUI_IconButton* _chanRgb;
    GUI_SvgButton* _chanR;
    GUI_SvgButton* _chanG;
    GUI_SvgButton* _chanB;
    GUI_Label* _chanSel;
    GUI_Label* _chanBg;
    GUI_Label* _chanBg1;

    GUI_SvgButton* _saturationSel;
    GUI_Slider* _saturationSlider;

    GUI_HLayout* _layout;

    RLOCIO_PaintEngine* _pe;

};

#endif