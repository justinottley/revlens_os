
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_VIEWER_PANE_H
#define REVLENS_GUI_VIEWER_PANE_H

#include "RlpRevlens/GUI/Global.h"

#include "RlpGui/BASE/ToolGroup.h"
#include "RlpGui/PANE/View.h"

#include "RlpGui/WIDGET/IconButton.h"
#include "RlpGui/WIDGET/ToggleIconButton.h"
#include "RlpGui/WIDGET/ToggleSvgButton.h"

#include "RlpRevlens/GUI/PlayModeButton.h"
#include "RlpRevlens/GUI/ViewerLoadOverlay.h"
#include "RlpRevlens/GUI/ViewerTimeline.h"

#include "RlpRevlens/DISP/GLView.h"


#ifdef SCAFFOLD_IOS
static const int TOOLBAR_HEIGHT = 40;
#else
static const int TOOLBAR_HEIGHT = 30;
#endif


class REVLENS_GUI_API GUI_ViewerPane : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_ViewerPane(GUI_ItemBase* parent);
    ~GUI_ViewerPane();

    static QQuickItem* create(GUI_ItemBase* parent, QVariantMap toolInfo);

    void onViewHoverEnter();
    void onViewHoverLeave();
    void onViewHoverMove();
    void onViewContextMenuItemSelected(QVariantMap cmiInfo);

    void dropEvent(QDropEvent* event);
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;

    QRectF boundingRect() const;


public slots:

    static GUI_ViewerPane*
    new_GUI_ViewerPane(GUI_ItemBase* parent)
    {
        return new GUI_ViewerPane(parent);
    }

    DISP_GLView* view() { return _view; }
    GUI_ToolGroup* toolbar() { return _toolbar; }
    GUI_ViewerTimeline* overlayTimeline() { return _viewerTimeline; }

    void initToolbar();

    void onParentSizeChanged(qreal width, qreal height);
    void onParentVisibilityChanged(QString name, bool isVisible);
    void onPlayPausePressed(QVariantMap metadata);
    void onPrevFrame(QVariantMap metadata);
    void onNextFrame(QVariantMap metadata);
    void onStartFrame(QVariantMap metadata);
    void onEndFrame(QVariantMap metadata);
    void onMuteToggled(QVariantMap md);
    void onVolumeChangeRequested(float val);
    void onReceiveVolumeChange(float val); // volume changed via hotkey or api, for UI update
    void onViewerToolsToggled(QVariantMap md);

    void onNoticeEvent(QString evtName, QVariantMap evtInfo);

    void toggleOverlayTimeline(QVariantMap metadata);
    void checkHideOverlayTimeline();
    
    void setToolbarVisible(bool visToolbar) { _showToolbar = visToolbar; }

    void setPresentationMode(bool pmode, bool showOverlayTimeline=true);

private:

    DISP_GLView* _view;
    GUI_ToolGroup* _toolbar;
    bool _showToolbar;
    bool _presentationMode;

    GUI_ViewerLoadOverlay* _loadOverlay;
    GUI_ViewerTimeline* _viewerTimeline;

    GUI_SvgButton* _viewerToolsButton;
    GUI_IconButton* _playButton;
    GUI_SvgButton* _vtbutton;
    GUI_ToggleSvgButton* _volButton;
    GUI_PlayModeButton* _playModeButton;

    // Viewer Timeline stuff
    //

    QTimer _vttimer;
    bool _vthidden;
    bool _playh;
    QPointF _mpos;
};


#endif