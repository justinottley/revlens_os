
#ifndef EXTREVLENS_GUI_STREAMING_PANE_H
#define EXTREVLENS_GUI_STREAMING_PANE_H

#include "RlpProd/GUI/Global.h"

#include "RlpRevlens/DISP/GLView.h"

#include "RlpGui/WIDGET/IconButton.h"
#include "RlpGui/WIDGET/ToggleIconButton.h"
#include "RlpGui/PANE/SvgButton.h"

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/ToolGroup.h"
#include "RlpGui/BASE/HLayout.h"
#include "RlpGui/BASE/VLayout.h"
#include "RlpGui/WIDGET/Label.h"

#include "RlpCore/NET/RPCFuture.h"


class GUI_StreamingPane;

class GUI_StreamListCallback : public CoreNet_RPCFuture {

public:
    RLP_DEFINE_LOGGER

    GUI_StreamListCallback(GUI_StreamingPane* pane);

    void onResultReady(QVariantMap result);

private:
    GUI_StreamingPane* _pane;

};


class GUI_StreamingPane : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_StreamingPane(GUI_ItemBase* parent);

public slots:

    static GUI_StreamingPane* new_GUI_StreamingPane(GUI_ItemBase* parent)
    {
        return new GUI_StreamingPane(parent);
    }

    void loadStream();

    void onPlayPausePressed(QVariantMap md);
    void onPlayStateChanged(int state);
    
    void initStreamList(QVariantMap streamInfo);

    void onSnapshotRequested(QVariantMap md);
    void onStreamChanged(QVariantMap streamInfo);
    void onRefreshRequested(QVariantMap md=QVariantMap());
    void onClearAnnotationsRequested(QVariantMap md);

    void onParentSizeChanged(qreal width, qreal height);
    void onParentVisibilityChanged(QString name, bool isVisible);

    void paint(GUI_Painter* painter);


private:
    GUI_ToolGroup* _toolbar;
    GUI_SvgButton* _refreshButton;
    GUI_ToggleIconButton* _playButton;
    GUI_SvgButton* _snapshotButton;
    GUI_SvgButton* _clearAnnotationsButton;

    GUI_IconButton* _streamList;
    GUI_Label* _currentStream;

    DISP_GLView* _view;
    bool _drawOverlay;
    QString _msg;

    QVariantMap _streamInfo;

};

#endif
