
#ifndef REVLENS_GUI_TLT_MEDIA_MANAGER_H
#define REVLENS_GUI_TLT_MEDIA_MANAGER_H

#include "RlpRevlens/GUI_TLT/Global.h"

#include "RlpRevlens/GUI_TLT/MediaToolTip.h"
#include "RlpRevlens/GUI_TLT/MediaItem.h"

#include "RlpRevlens/GUI_TL2/Track.h"

class GUI_TLT_View;



class GUI_TLT_MediaManager : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    typedef QHash<QString, GUI_TLT_MediaItem*>::iterator ItemIterator;

    GUI_TLT_MediaManager(GUI_TLT_View* view);

    void clearItems();
    void refresh();

    void handlePlaylistChanged(QVariantMap evtInfo);
    void handleSelectionUpdated(QVariantMap evtInfo);

    void onNoticeEvent(QString evtName, QVariantMap evtInfo);

public slots:

    void setMediaEnabled(bool isEnabled) { _mediaEnabled = isEnabled; }

    void onPlayheadHoverPosChanged(int x, int y);
    void onPlayheadHoverLeave();


private:
    GUI_TLT_View* _view;
    DS_TrackPtr _currTrack;

    bool _mediaEnabled;

    QHash<QString, GUI_TLT_MediaItem*> _mediaItemMap;
    QHash<qlonglong, QString> _frameNodeMap;
    // QString _currTrack;

    int _hx;
    GUI_TLT_MediaToolTip* _mediaToolTip;

};

#endif