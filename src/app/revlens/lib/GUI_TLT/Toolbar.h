#ifndef REVLENS_GUI_TLT_TOOLBAR_H
#define REVLENS_GUI_TLT_TOOLBAR_H

#include "RlpRevlens/GUI_TLT/Global.h"
#include "RlpRevlens/GUI_TLT/View.h"

#include "RlpRevlens/DS/SyncProbe.h"

#include "RlpGui/BASE/HLayout.h"
#include "RlpGui/WIDGET/Label.h"
#include "RlpGui/WIDGET/ToggleIconButton.h"
#include "RlpGui/WIDGET/ToggleSvgButton.h"

#include "RlpGui/PANE/SvgButton.h"

class GUI_TLT_Toolbar : public GUI_ItemBase, DS_SyncProbe {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    static const int TB_HEIGHT;

    GUI_TLT_Toolbar(GUI_ItemBase* parent);
    ~GUI_TLT_Toolbar();

    int timelineWidthOffset();

    void onParentSizeChanged(qreal width, qreal height);
    void paint(GUI_Painter* painter);

public: // DS_SyncProbe

    void syncUpdate(qlonglong frameNum); // called during playback
    void syncUpdateImmediate(qlonglong frameNum); // called if immediate update (scrubbing)

public slots:

    static GUI_TLT_Toolbar*
    new_GUI_TLT_Toolbar(GUI_ItemBase* parent)
    {
        return new GUI_TLT_Toolbar(parent);
    }

    GUI_TLT_View* timelineView() { return _tltView; }

    void onPlayPausePressed(QVariantMap md);
    void onPrevFrame(QVariantMap metadata);
    void onNextFrame(QVariantMap metadata);

    void onMuteToggled(QVariantMap md=QVariantMap());
    void onVolumeChangeRequested(float val);

    // void onToolsToggled(QVariantMap md);

    void onInFrameChanged(qlonglong frame);
    void onOutFrameChanged(qlonglong frame);
    void onFrameCountChanged(qlonglong frameCount);
    void onPlayStateChanged(int playbackState);
    void onNoticeEvent(QString evtName, QVariantMap evtInfo);

private:
    void initConnections();


private:

    GUI_ToggleIconButton* _playButton;
    GUI_IconButton* _prevFrameButton;
    GUI_IconButton* _nextFrameButton;

    GUI_ToggleSvgButton* _volButton;

    GUI_Label* _currFrameLabel;
    GUI_Label* _startFrameLabel;
    GUI_Label* _endFrameLabel;

    GUI_TLT_View* _tltView;

    QString _currTrackUuidStr;

};

#endif
