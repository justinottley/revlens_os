
#include "RlpExtrevlens/RLOCIO/OverlayTools.h"
#include "RlpGui/BASE/Style.h"
#include "RlpGui/WIDGET/Slider.h"

#include <OpenColorIO/OpenColorIO.h>

namespace OCIO = OCIO_NAMESPACE;


RLP_SETUP_LOGGER(extrevlens, RLOCIO, OverlayTools)

RLOCIO_OverlayTools::RLOCIO_OverlayTools(GUI_ItemBase* parent):
    GUI_ItemBase(parent)
{
    // setOpacity(0.75);

    _inputSel = new GUI_IconButton("", this);
    _inputSel->setSvgIconPath(":feather/lightgrey/image.svg", 15);
    _inputSel->setSvgIconRight(false);

    GUI_SvgIcon* iicon = _inputSel->svgIcon();
    iicon->setPos(5, 3);
    iicon->setBgColour(QColor(140, 140, 140));

    _inputSel->setTextHOffset(4);
    _inputSel->setTextYOffset(0);
    _inputSel->setText("Input:");
    _inputSel->setOutlined(true);

    _transformSel = new GUI_IconButton("", this);
    _transformSel->setSvgIconPath(":misc/filter.svg", 15);
    _transformSel->setSvgIconRight(false);
    
    GUI_SvgIcon* ticon = _transformSel->svgIcon();
    ticon->setPos(5, 3);
    ticon->setBgColour(QColor(140, 140, 140));

    _transformSel->setTextHOffset(4);
    _transformSel->setTextYOffset(0);
    _transformSel->setText("Transform:");
    _transformSel->setOutlined(true);
    

    _displaySel = new GUI_IconButton("", this);
    _displaySel->setSvgIconPath(":feather/lightgrey/monitor.svg", 15);
    _displaySel->setSvgIconRight(false);

    GUI_SvgIcon* dicon = _displaySel->svgIcon();
    dicon->setPos(5, 3);
    dicon->setBgColour(QColor(140, 140, 140));

    _displaySel->setTextHOffset(4);
    _displaySel->setTextYOffset(0);
    _displaySel->setText("Display:");
    _displaySel->setOutlined(true);


    _colBg = new GUI_Label(this, "");
    _colBg->setBgColor(GUI_Style::BgLoMidGrey);
    _colBg->setWidth(50);
    _colBg->setHeight(80);
    _colBg->setOpacity(0.75);

    _exposureLbl = new GUI_SvgButton(":misc/brightness.svg", this, 25);
    _exposureLbl->icon()->setBgColour(QColor(180, 180, 180));
    _exposureLbl->setToolTipText("Exposure");
    _exposureLbl->setPos(20, parent->height());

    _exposureSlider = new GUI_Slider(this, GUI_Slider::O_SL_HORIZONTAL);
    _exposureSlider->setBgColour(GUI_Style::BgLoMidGrey);
    _exposureSlider->setOpacity(0.75);
    _exposureSlider->setPos(60, parent->height());
    

    connect(
        _exposureSlider,
        &GUI_Slider::moving,
        this,
        &RLOCIO_OverlayTools::setExposure
    );

    connect(
        _exposureSlider,
        &GUI_Slider::endMove,
        this,
        &RLOCIO_OverlayTools::setExposure
    );


    // _exposureMinusBtn = new GUI_SvgButton(":feather/lightgrey/minus.svg", this, 25);
    // _exposureMinusBtn->icon()->setBgColour(QColor(180, 180, 180));
    // _exposureMinusBtn->setPos(60, parent->height());

    // _exposurePlusBtn = new GUI_SvgButton(":feather/lightgrey/plus.svg", this, 25);
    // _exposurePlusBtn->icon()->setBgColour(QColor(180, 180, 180));
    // _exposurePlusBtn->setPos(90, parent->height());

    // _exposureVal = new GUI_Label(this, "");
    QFont f = QApplication::font(); // _exposureVal->font();
    f.setPixelSize(f.pixelSize() + 4);
    // _exposureVal->setFont(f);
    // _exposureVal->setPos(110, parent->height());

    _gammaLbl = new GUI_SvgButton(":misc/gamma.svg", this, 25);
    _gammaLbl->icon()->setBgColour(QColor(180, 180, 180));
    _gammaLbl->setToolTipText("Gamma");
    _gammaLbl->setOverlayTextPos(0, -10);
    _gammaLbl->setPos(20 + (parent->width() / 2), parent->height() - 20);

    _gammaSlider = new GUI_Slider(this, GUI_Slider::O_SL_HORIZONTAL);
    _gammaSlider->setBgColour(GUI_Style::BgLoMidGrey);
    _gammaSlider->setOpacity(0.75);
    _gammaSlider->setPos(60, parent->height());
    
    _gammaSlider->setSliderValue(1.0);
    // connect(
    //     _gammaSlider,
    //     &GUI_Slider::moving,
    //     this,
    //     &RLOCIO_OverlayTools::setGamma
    // );

    connect(
        _gammaSlider,
        &GUI_Slider::endMove,
        this,
        &RLOCIO_OverlayTools::setGamma
    );

    // _gammaMinusBtn = new GUI_SvgButton(":feather/lightgrey/minus.svg", this, 25);
    // _gammaMinusBtn->icon()->setBgColour(QColor(180, 180, 180));
    // _gammaMinusBtn->setPos(60 + (parent->width() / 2), parent->height() - 20);

    // _gammaPlusBtn = new GUI_SvgButton(":feather/lightgrey/plus.svg", this, 25);
    // _gammaPlusBtn->icon()->setBgColour(QColor(180, 180, 180));
    // _gammaPlusBtn->setPos(90 + (parent->width() / 2), parent->height() - 20);

    // _gammaVal = new GUI_Label(this, "");
    // _gammaVal->setPos(120 + (parent->width() / 2), parent->height() - 20);
    // _gammaVal->setFont(f);

    _chanBg = new GUI_Label(this, "");
    _chanBg->setBgColor(GUI_Style::BgLoMidGrey);
    _chanBg->setWidth(40);
    _chanBg->setHeight(80);
    _chanBg->setOpacity(0.75);

    _chanBg1 = new GUI_Label(this, "");
    _chanBg1->setBgColor(GUI_Style::BgLoMidGrey);
    _chanBg1->setWidth(140);
    _chanBg1->setHeight(40);
    _chanBg1->setOpacity(0.75);

    _chanRgb = new GUI_IconButton("", this);
    _chanRgb->setSvgIconRight(false);
    _chanRgb->setSvgIconPath(":misc/colour.svg", 20, false);
    _chanRgb->setPos(parent->width() - 40, parent->height() - 20);
    _chanRgb->setToolTipText("Toggle Full Color");
    _chanRgb->svgIcon()->setPos(2, 2);
    _chanRgb->setWidth(25);
    //_chanRgb->svgIcon()->setPos(parent->width() - 60, parent->height() - 20);


    _chanR = new GUI_SvgButton(":feather/lightgrey/circle.svg", this, 24);
    _chanR->icon()->setBgColour(QColor(250, 100, 100));
    _chanR->setOverlayText("R");
    _chanR->setOverlayTextPos(8, 11);
    _chanR->setPos(parent->width() - 20, parent->height() - 20);

    _chanG = new GUI_SvgButton(":feather/lightgrey/circle.svg", this, 24);
    _chanG->icon()->setBgColour(QColor(100, 250, 100));
    _chanG->setOverlayText("G");
    _chanG->setOverlayTextPos(7, 11);
    _chanG->setPos(parent->width() - 20, parent->height() - 20);

    _chanB = new GUI_SvgButton(":feather/lightgrey/circle.svg", this, 24);
    _chanB->icon()->setBgColour(QColor(100, 100, 250));
    _chanB->setOverlayText("B");
    _chanB->setOverlayTextPos(8, 11);
    _chanB->setPos(parent->width() - 20, parent->height() - 20);

    _chanSel = new GUI_Label(this, "RGB");

    _saturationSel = new GUI_SvgButton(":misc/saturation.svg", this, 20);
    _saturationSel->setToolTipText("Saturation / Grayscale");
    _saturationSlider = new GUI_Slider(this, GUI_Slider::O_SL_HORIZONTAL, 140);
    _saturationSlider->setOpacity(0.75);
    _saturationSlider->setBgColour(GUI_Style::BgLoMidGrey);
    _saturationSlider->setPos(60, parent->height());

    connect(
        _saturationSlider,
        &GUI_Slider::moving,
        this,
        &RLOCIO_OverlayTools::setSaturation
    );

    _layout = new GUI_HLayout(this);
    _layout->setPos(10, 10);
    _layout->addItem(_inputSel);
    _layout->addSpacer(30);
    _layout->addItem(_transformSel);
    _layout->addSpacer(30);
    _layout->addItem(_displaySel);

    GUI_ItemBase* pitem = qobject_cast<GUI_ItemBase*>(parentItem());
    DISP_GLView* view = pitem->metadataValue("view").value<DISP_GLView*>();
    
    connect(
        qobject_cast<GUI_MenuPane*>(_inputSel->menu()),
        &GUI_MenuPane::menuItemSelected,
        this,
        &RLOCIO_OverlayTools::onInputColorSpaceChanged
    );

    connect(
        qobject_cast<GUI_MenuPane*>(_transformSel->menu()),
        &GUI_MenuPane::menuItemSelected,
        this,
        &RLOCIO_OverlayTools::onTransformChanged
    );

    connect(
        qobject_cast<GUI_MenuPane*>(_displaySel->menu()),
        &GUI_MenuPane::menuItemSelected,
        this,
        &RLOCIO_OverlayTools::onDisplayChanged
    );

    // connect(
    //     _exposureMinusBtn,
    //     &GUI_SvgButton::buttonPressed,
    //     this,
    //     &RLOCIO_OverlayTools::decExposure
    // );

    // connect(
    //     _exposurePlusBtn,
    //     &GUI_SvgButton::buttonPressed,
    //     this,
    //     &RLOCIO_OverlayTools::incExposure
    // );

    // connect(
    //     _gammaPlusBtn,
    //     &GUI_SvgButton::buttonPressed,
    //     this,
    //     &RLOCIO_OverlayTools::incGamma
    // );

    // connect(
    //     _gammaMinusBtn,
    //     &GUI_SvgButton::buttonPressed,
    //     this,
    //     &RLOCIO_OverlayTools::decGamma
    // );

    connect(
        _chanR,
        &GUI_SvgButton::buttonPressed,
        this,
        &RLOCIO_OverlayTools::setChanR
    );

    connect(
        _chanG,
        &GUI_SvgButton::buttonPressed,
        this,
        &RLOCIO_OverlayTools::setChanG
    );

    connect(
        _chanB,
        &GUI_SvgButton::buttonPressed,
        this,
        &RLOCIO_OverlayTools::setChanB
    );


    connect(
        _chanRgb,
        &GUI_SvgButton::buttonPressed,
        this,
        &RLOCIO_OverlayTools::setChanRGB
    );


    connect(
        view,
        &DISP_GLView::paintEngineCreated,
        this,
        &RLOCIO_OverlayTools::OCIOInit
    );

    connect(
        parent,
        &GUI_ItemBase::sizeChanged,
        this,
        &RLOCIO_OverlayTools::onParentSizeChanged
    );

    OCIOPopulateMenus();
}


void
RLOCIO_OverlayTools::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(width << height << "layout:" << _layout->width())

    setWidth(width);
    setHeight(height);

    _colBg->setPos(10, height - 90 - 50);

    _exposureLbl->setPos(20, height - 90 - 40);
    _exposureSlider->setPos(60, height - 100 - 40);
    // _exposureMinusBtn->setPos(60, height - 90 - 40);
    // _exposurePlusBtn->setPos(90, height - 90 - 40);
    // _exposureVal->setPos(130, height - 85 - 40);

    _gammaLbl->setPos(20, height - 90);
    _gammaSlider->setPos(60, height - 100);
    // _gammaMinusBtn->setPos(60, height - 90);
    // _gammaPlusBtn->setPos(90, height - 90);
    // _gammaVal->setPos(130, height - 85);
    
    
    /*
    _gammaLbl->setPos((width / 2) - 40, height - 90);
    _gammaMinusBtn->setPos(0 + (width / 2), height - 90);
    _gammaPlusBtn->setPos(30 + (width / 2), height - 90);
    _gammaVal->setPos(70 + (width / 2), height - 85);
    */

    _chanRgb->setPos(width - 180, height - 90);
    _chanR->setPos(width - 140, height - 90);
    _chanG->setPos(width - 110, height - 90);
    _chanB->setPos(width - 80, height - 90);

    // _chanSel->setPos(width - 75, height - 120);
    _chanSel->setPos(width - 45, height - 88);
    _chanBg->setPos(width - 190, height - 135);
    _chanBg1->setPos(width - 150, height - 95);

    _saturationSel->setPos(width - 180, height - 120);
    _saturationSlider->setPos(width - 150, height - 135);
}


void
RLOCIO_OverlayTools::OCIOPopulateMenus()
{
    RLP_LOG_DEBUG("")

    OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();

    GUI_MenuPane* imenu = qobject_cast<GUI_MenuPane*>(_inputSel->menu());

    imenu->addItem("scene_linear");

    // Source Image
    for (int i=0; i<config->getNumColorSpaces(); ++i)
    {
        const char * csName = config->getColorSpaceNameByIndex(i);
        if (csName && *csName)
        {
            QVariantMap info;
            info.insert("idx", i);
            imenu->addItem(QString(csName), info);
        }
    }

    // Transforms
    GUI_MenuPane* tmenu = qobject_cast<GUI_MenuPane*>(_transformSel->menu());
    const char * defaultDisplay = config->getDefaultDisplay();
    for (int i=0; i<config->getNumViews(defaultDisplay); ++i)
    {
        tmenu->addItem(QString(config->getView(defaultDisplay, i)));
    }

    // Display
    GUI_MenuPane* dmenu = qobject_cast<GUI_MenuPane*>(_displaySel->menu());
    for (int i=0; i<config->getNumDisplays(); ++i)
    {
        dmenu->addItem(QString(config->getDisplay(i)));
    }

}


void
RLOCIO_OverlayTools::OCIOInit(DISP_GLPaintEngine2D* pe)
{
    RLP_LOG_DEBUG(pe)
    if (pe->name() != "OCIO")
    {
        RLP_LOG_ERROR("OCIO Paint Engine not enabled, aborting")
        return;
    }


    RLOCIO_PaintEngine* ope = qobject_cast<RLOCIO_PaintEngine*>(pe);

    // Stash Paint Engine Object
    //
    _pe = ope;

    QString ispace = ope->inputColorSpace().c_str();
    _inputSel->setText(ispace);

    QString xform = ope->transform().c_str();
    _transformSel->setText(xform);

    QString display = ope->display().c_str();
    _displaySel->setText(display);

    // _exposureVal->setText(QString("%1").arg(ope->exposure()));
    // _gammaVal->setText(QString("%1").arg(ope->gamma()));
}


void
RLOCIO_OverlayTools::onInputColorSpaceChanged(QVariantMap info)
{
    RLP_LOG_DEBUG(info)

    QString icspace = info.value("text").toString();
    int index = info.value("data").toMap().value("idx").toInt();
    _pe->setInputColorSpaceByIndex(index);

    _inputSel->setText(icspace);
}


void
RLOCIO_OverlayTools::onTransformChanged(QVariantMap info)
{
    RLP_LOG_DEBUG("")

    QString transform = info.value("text").toString();
    _pe->setTransform(transform);

    _transformSel->setText(transform);
}


void
RLOCIO_OverlayTools::onDisplayChanged(QVariantMap info)
{
    RLP_LOG_DEBUG("")

    QString display = info.value("text").toString();
    _pe->setDisplay(display);

    _displaySel->setText(display);
    
}


void
RLOCIO_OverlayTools::incExposure(QVariantMap info)
{
    RLP_LOG_DEBUG(info)

    float val = _pe->exposure();
    val += 0.25f;
    _pe->setExposure(val);

    // _exposureVal->setText(QString("%1").arg(val));
}


void
RLOCIO_OverlayTools::decExposure(QVariantMap info)
{
    RLP_LOG_DEBUG(info)

    float val = _pe->exposure();
    val -= 0.25f;
    _pe->setExposure(val);

    // _exposureVal->setText(QString("%1").arg(val));

}


void
RLOCIO_OverlayTools::setExposure(float val)
{
    val *= 5;

    RLP_LOG_DEBUG(val);

    _pe->setExposure(val);
}


void
RLOCIO_OverlayTools::setSaturation(float val)
{
    // put this in the node under the current frame?
    _pe->setDefaultSaturation(1 - val);
}


void
RLOCIO_OverlayTools::incGamma(QVariantMap info)
{
    RLP_LOG_DEBUG(info)

    float val = _pe->gamma();
    val *= 1.1f;
    
    // _gammaVal->setText(QString("%1").arg(val));
    // _gammaVal->update();

    _pe->setGamma(val);

}


void
RLOCIO_OverlayTools::decGamma(QVariantMap info)
{
    RLP_LOG_DEBUG(info)

    float val = _pe->gamma();
    val /= 1.1f;
    
    // _gammaVal->setText(QString("%1").arg(val));
    // _gammaVal->update();

    _pe->setGamma(val);

}


void
RLOCIO_OverlayTools::setGamma(float val)
{
    val *= 2;

    if (val < 0.1)
    {
        RLP_LOG_WARN("Clamping gamma to 0.1")
        val = 0.1;
    }

    RLP_LOG_DEBUG(val)
    _pe->setGamma(val);
}


void
RLOCIO_OverlayTools::setChanR(QVariantMap info)
{
    _pe->setChannelR();
    _chanSel->setText("R");
    _chanSel->setPos(width() - 45, height() - 88); // 120);
    _chanSel->update();
}

void
RLOCIO_OverlayTools::setChanG(QVariantMap info)
{
    _pe->setChannelG();
    _chanSel->setText("G");
    _chanSel->setPos(width() - 45, height() - 88); // 120);
    _chanSel->update();
}

void
RLOCIO_OverlayTools::setChanB(QVariantMap info)
{
    _pe->setChannelB();
    _chanSel->setText("B");
    _chanSel->setPos(width() - 45, height() - 88); // 120);
    _chanSel->update();
}

void
RLOCIO_OverlayTools::setChanRGB(QVariantMap info)
{
    _pe->setChannelRGB();
    _chanSel->setText("RGB");
    _chanSel->setPos(width() - 45, height() - 88); // 120);
    _chanSel->update();
}
