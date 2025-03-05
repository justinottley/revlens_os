#ifndef REVLENS_GUI_TL2_CACHE_H
#define REVLENS_GUI_TL2_CACHE_H

#include "RlpRevlens/GUI_TL2/Global.h"


class GUI_TL2_View;

class REVLENS_GUI_TL2_API GUI_TL2_Cache : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_Cache(GUI_TL2_View* view);

    void initializeCachedFrames();

    virtual QRectF boundingRect() const;
    virtual void paint(GUI_Painter* painter);

public slots:

    void onParentSizeChanged(qreal nwidth, qreal nheight);

    void onLookaheadAdd(qlonglong frameNum, bool update=true);
    void onLookaheadRelease(qlonglong frameNum);
    void onLookaheadClear();
    void onAudioAdd(QList<qlonglong> &audioList);


private:
    GUI_TL2_View* _view;

    QMutex* _lock;

    QHash<qlonglong, bool> _lookaheadIndex;
    QList<qlonglong>  _audioIndex;
};

#endif
