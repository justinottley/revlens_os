//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_TL2_MEDIA_TRACK_H
#define REVLENS_GUI_TL2_MEDIA_TRACK_H

#include "RlpCore/LOG/Logging.h"

#include "RlpRevlens/DS/Track.h"
#include "RlpRevlens/GUI_TL2/Global.h"
#include "RlpRevlens/GUI_TL2/Track.h"

class GUI_TL2_TrackManager;
class GUI_TL2_View;

class REVLENS_GUI_TL2_API GUI_TL2_MediaTrack : public GUI_TL2_Track {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_MediaTrack(GUI_TL2_TrackManager* mgr, DS_TrackPtr strack, QString name);
    
    void initCurrSession();
    void initTrackConnections();
    void initButtons();

    ~GUI_TL2_MediaTrack();
    void addMediaItem(DS_NodePtr node, qlonglong startFrame, qlonglong mediaFrameCount);

    // virtual void paint(GUI_Painter* painter);

public slots:

    virtual qreal trackHeight();
    void onParentSizeChanged(qreal nwidth, qreal nheight);

    int heightAudio() { return _heightAudio; }
    int heightVideo();
    int heightText();

    void onVisibilityChanged(QVariantMap md);
    void onMediaRemoved(DS_NodePtr node, QString trackUuidStr, qlonglong startFrame);    
    void onItemSelected(QVariantMap mediaInfo);
    
    bool isTextVisible() { return _visText; }
    void setTextVisible(bool vis);

    bool isWaveformVisible() { return _visWaveform; }
    void setWaveformVisible(bool vis);

    bool isThumbnailVisible() { return _visThumbnail; }
    void onThumbnailClicked(QVariantMap md);
    void setThumbnailVisible(bool vis);

    void onPrevItemClicked(bool vis);
    void onNextItemClicked(bool vis);
    
private:

    int _heightText;
    int _heightAudio;
    int _heightVideo;

    bool _visText;
    bool _visWaveform;
    bool _visThumbnail;

};

// Q_DECLARE_METATYPE(GUI_TL2_MediaTrack)

#endif
