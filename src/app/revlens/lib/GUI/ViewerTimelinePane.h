
#ifndef GUI_VIEWER_TIMELINE_PANE_H
#define GUI_VIEWER_TIMELINE_PANE_H

#include "RlpRevlens/GUI/Global.h"
#include "RlpRevlens/GUI/ViewerPane.h"
#include "RlpRevlens/GUI_TL2/View.h"

#include "RlpGui/PANE/SvgButton.h"

class GUI_ViewerTimelinePane : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_ViewerTimelinePane(GUI_ItemBase* parent);

    static QQuickItem* create(GUI_ItemBase* parent, QVariantMap toolInfo);

    void onParentVisibilityChanged(QString name, bool isVisible);
    void onParentSizeChanged(qreal width, qreal height);

public slots:
    DISP_GLView* view() { return _viewer->view(); }
    GUI_ToolGroup* toolbar() { return _viewer->toolbar(); }

    void onTimelineToggled(QVariantMap md=QVariantMap()); 

private:
    GUI_ViewerPane* _viewer;
    GUI_TL2_View* _timeline;

    GUI_Pane* _rpane;

    GUI_SvgButton* _toggleTimelineBtn;
    float _splitterPos;
};

#endif
