
#include "RlpRevlens/GUI_TL2/Cache.h"

#include "RlpRevlens/GUI_TL2/View.h"
#include "RlpRevlens/CNTRL/Video.h"

#include <QtCore/QMutexLocker>

RLP_SETUP_LOGGER(revlens, GUI_TL2, Cache)

GUI_TL2_Cache::GUI_TL2_Cache(GUI_TL2_View* view):
    GUI_ItemBase(view->timelineArea()),
    _view(view),
    _lock(new QMutex())
{
    setWidth(view->width() - 1);
    setHeight(10);

    setZValue(5);

    _lookaheadIndex.clear();
    _audioIndex.clear();

    initializeCachedFrames();

}


void
GUI_TL2_Cache::initializeCachedFrames()
{
    RLP_LOG_DEBUG("")

    QList<qlonglong> cachedVideoFrames = _view->controller()
                                              ->getVideoManager()
                                              ->cache()
                                              ->cachedFrames();

    for (auto it : cachedVideoFrames)
    {
        onLookaheadAdd(it, false);
    }
}


void
GUI_TL2_Cache::onLookaheadAdd(qlonglong frameNum, bool update)
{
    QMutexLocker l(_lock);
    // RLP_LOG_DEBUG(frameNum);

    _lookaheadIndex[frameNum] = true;

    if (update)
    {
        _view->updateIfNecessary();
    }
}


void
GUI_TL2_Cache::onLookaheadRelease(qlonglong frameNum)
{
    QMutexLocker l(_lock);

    // RLP_LOG_DEBUG(frameNum);

    if (_lookaheadIndex.contains(frameNum)) {
        _lookaheadIndex.remove(frameNum);

        _view->updateIfNecessary();
    }
}


void
GUI_TL2_Cache::onLookaheadClear()
{
    QMutexLocker l(_lock);

    // RLP_LOG_DEBUG("");

    _lookaheadIndex.clear();
    _view->updateIfNecessary();
}


void
GUI_TL2_Cache::onAudioAdd(QList<qlonglong> &audioList)
{
    QMutexLocker l(_lock);

    // RLP_LOG_DEBUG("");

    _audioIndex.clear();
    _audioIndex = audioList;
    
    _view->updateIfNecessary();
}


void
GUI_TL2_Cache::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << nheight)

    setWidth(_view->width() - 1);
    setHeight(10);// _view->timelineHeight() - 1);
}


QRectF
GUI_TL2_Cache::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_TL2_Cache::paint(GUI_Painter* painter)
{
    if (!isVisible())
    {
        return;
    }

    // In case we're operating on a separate thread here,
    // make a copy and lock only around the copy
    _lock->lock();
    QHash<qlonglong, bool> lookaheadIndex = _lookaheadIndex;
    QList<qlonglong> audioIndex = _audioIndex;
    _lock->unlock();


    // painter->save();
    // painter->setClipRect(_view->timelineClipRect(), Qt::IntersectClip);

    // painter->setRenderHint(QPainter::Antialiasing, false);

    QColor lColor(50, 50, 140);
    painter->setPen(QPen(lColor));
    painter->setBrush(QBrush(lColor));

    //
    // Video Cache
    //

    float regionSize = (_view->pixelsPerFrame() * _view->zoom());
    int maxWidth = _view->width();

    QHash<qlonglong, bool>::iterator lit;
    for ( lit = lookaheadIndex.begin(); lit != lookaheadIndex.end(); ++lit) {

        float xpos = _view->getXPosAtFrame(lit.key());
        if (xpos > maxWidth) {
            continue;
        }

        // RLP_LOG_DEBUG("{ " << lit.key() << " }");
        // RLP_LOG_DEBUG(xpos << " " << regionSize << "maxWidth:" << maxWidth)

        painter->drawRect(QRectF(xpos, 0, regionSize, 2)); // height : 2
    }


    //
    // Audio Cache
    //
    
    QColor acColor(70, 70, 30);
    QPen acPen = QPen(acColor, 1); // , penStyle, cap, join);
    
    QBrush audioBrush = QBrush(acColor, Qt::SolidPattern);
    painter->setBrush(audioBrush);
    painter->setPen(acPen);
    
    // painter->setRenderHint(QPainter::Antialiasing, false);
    
    
    QList<qlonglong>::iterator ait;
    
    // RLP_LOG_DEBUG("-----" );

    for ( ait = audioIndex.begin(); ait != audioIndex.end(); ++ait) {
        
        // RLP_LOG_DEBUG("[ " << *ait << " ]" );
        
        float xpos = _view->getXPosAtFrame((*ait) - 1);
        if (xpos <= maxWidth) {
            // RLP_LOG_DEBUG(xpos << " " << _pixelsPerFrame <<  std::endl;
            painter->drawRect(QRectF(xpos, 4, regionSize, 2));
        }
    }


    // painter->restore();


}