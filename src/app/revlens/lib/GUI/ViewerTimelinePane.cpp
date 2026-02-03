
#include "RlpRevlens/GUI/ViewerTimelinePane.h"

#include "RlpRevlens/CNTRL/MainController.h"

RLP_SETUP_LOGGER(revlens, GUI, ViewerTimelinePane)

GUI_ViewerTimelinePane::GUI_ViewerTimelinePane(GUI_ItemBase* parent):
    GUI_ItemBase(parent)
{
    _rpane = new GUI_Pane(this, "view_tl_root", O_VERTICAL);
    _rpane->initPane();
    _rpane->split(false);
    _rpane->setHeaderVisible(false, false);
    _rpane->pane(1)->setHeaderVisible(false, false);

    QVariantMap ti;
    _viewer = qobject_cast<GUI_ViewerPane*>(
        GUI_ViewerPane::create(_rpane->pane(0)->body(), ti));

    _timeline = qobject_cast<GUI_TL2_View*>(
        GUI_TL2_View::create(_rpane->pane(1)->body(), ti));

    _toggleTimelineBtn = new GUI_SvgButton(
        ":misc/project-plan.svg",
        this,
        DEFAULT_ICON_SIZE
    );
    _toggleTimelineBtn->setToolTipText("Toggle Timeline");

    connect(
        _toggleTimelineBtn,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_ViewerTimelinePane::onTimelineToggled
    );

    _viewer->toolbar()->addDivider(20, TOOLBAR_HEIGHT - 10);
    _viewer->toolbar()->addItem(_toggleTimelineBtn, 5);

    onTimelineToggled();

    onParentSizeChanged(parent->width(), parent->height());

    _rpane->updatePanes();
}


QQuickItem*
GUI_ViewerTimelinePane::create(GUI_ItemBase* parent, QVariantMap toolInfo)
{
    GUI_ViewerTimelinePane* vp = new GUI_ViewerTimelinePane(parent);
    
    QVariant v;
    v.setValue(vp->view());

    vp->setMetadata("view", v);

    CNTRL_MainController::instance()->emitToolCreated(QString("TLViewer"), vp);

    return vp;
}


void
GUI_ViewerTimelinePane::onTimelineToggled(QVariantMap md)
{
    RLP_LOG_DEBUG(md)

    bool toggled = md.value("toggled").toBool();
    if (toggled)
    {
        // _timeline->setVisible(true);
        _rpane->splitter(0)->setPosPercent(0.7);
    }
    else
    {
        // _splitterPos = _rpane->splitter(0)->splitterPos();
        _rpane->splitter(0)->setPosPercent(0.995);
        // _timeline->setVisible(false);
    }
    
    _toggleTimelineBtn->setToggled(!toggled);
    _rpane->update();
    _rpane->splitter(0)->update();
    _rpane->updatePanes();
} 


void
GUI_ViewerTimelinePane::onParentVisibilityChanged(QString name, bool isVisible)
{
    RLP_LOG_DEBUG(name << isVisible)

    QString tabName;
    if (_metadata.contains("tab.name"))
    {
        tabName = _metadata.value("tab.name").toString();
        if ((tabName == name) && (isVisible))
        {
            qobject_cast<GUI_ItemBase*>(parentItem())->setPaintBackground(false);
        }
    }
}


void
GUI_ViewerTimelinePane::onParentSizeChanged(qreal width, qreal height)
{
    setWidth(width);

    GUI_PaneHeader* ph = _metadata.value("pane.header").value<GUI_PaneHeader*>();
    if ((ph != nullptr) && ph->isVisible())
    {
        height -= 25;
    }

    setHeight(height);

    _rpane->updatePanes();
}


