
#include "RlpRevlens/GUI_TLT/MediaManager.h"
#include "RlpRevlens/GUI_TLT/MediaItem.h"
#include "RlpRevlens/GUI_TLT/View.h"



RLP_SETUP_LOGGER(revlens, GUI_TLT, MediaManager)

GUI_TLT_MediaManager::GUI_TLT_MediaManager(GUI_TLT_View* view):
    _view(view),
    _currTrack(nullptr),
    _mediaEnabled(true),
    _hx(-1)
{
    _mediaItemMap.clear();

    _mediaToolTip = new GUI_TLT_MediaToolTip(view);
    _mediaToolTip->setVisible(false);
    view->scene()->addMenuPane(_mediaToolTip);

    connect(
        _view->playhead(),
        &GUI_TL2_Playhead::hoverPosChanged,
        this,
        &GUI_TLT_MediaManager::onPlayheadHoverPosChanged
    );

    connect(
        _view->playhead(),
        &GUI_TL2_Playhead::hoverLeave,
        this,
        &GUI_TLT_MediaManager::onPlayheadHoverLeave
    );
}


void
GUI_TLT_MediaManager::clearItems()
{
    GUI_TLT_MediaManager::ItemIterator it;
    for (it = _mediaItemMap.begin(); it != _mediaItemMap.end(); ++it)
    {
        it.value()->setParentItem(nullptr);
        it.value()->deleteLater();
    }

    _mediaItemMap.clear();
}


void
GUI_TLT_MediaManager::refresh()
{
    // RLP_LOG_DEBUG("")

    GUI_TLT_MediaManager::ItemIterator it;
    for (it = _mediaItemMap.begin(); it != _mediaItemMap.end(); ++it)
    {
        it.value()->refresh();
    }
}


void
GUI_TLT_MediaManager::onPlayheadHoverPosChanged(int x, int y)
{
    if ((_view->showMediaToolTip()) && (_hx != x))
    {
        qlonglong frame = _view->getFrameAtXPos(x);
        if (_frameNodeMap.contains(frame))
        {
            QString nodeUuidStr = _frameNodeMap.value(frame);
            GUI_TLT_MediaItem* item = _mediaItemMap.value(nodeUuidStr, nullptr);
            
            if (item != nullptr)
            {
                QPointF spos = _view->mapToScene(QPointF(x, y));
                QPointF vspos = _view->mapToScene(_view->position());

                _mediaToolTip->setPos(spos.x(), vspos.y() - _mediaToolTip->height() - 10);
                _mediaToolTip->setTextBottom(item->name());
                _mediaToolTip->setThumbnail(item->thumbnail());
                _mediaToolTip->update();
                _mediaToolTip->setVisible(true);

            }
        }

        _hx = x;
    }
}


void
GUI_TLT_MediaManager::onPlayheadHoverLeave()
{
    // RLP_LOG_DEBUG("")

    _mediaToolTip->setVisible(false);
}


void
GUI_TLT_MediaManager::handlePlaylistChanged(QVariantMap evtInfo)
{
    QString selTrackUuidStr = evtInfo.value("track_uuid").toString();

    // RLP_LOG_DEBUG("")

    _currTrack = _view->controller()->session()->getTrackByUuid(selTrackUuidStr);

    _mediaToolTip->setTextTop(_currTrack->name());
    // RLP_LOG_DEBUG(selTrackUuidStr << "frame count:" << _currTrack->getMaxFrameNum())

    if (_currTrack != nullptr)
    {
        clearItems();

        _view->setInFrame(1);
        _view->setOutFrame(_currTrack->getMaxFrameNum());
        
        if (_mediaEnabled)
        {
            QVariantList nodeList = _currTrack->getNodeFrameList();

            bool isOdd = false;

            for (int i=0; i != nodeList.size(); ++i)
            {
                QVariantMap nodeInfo = nodeList.at(i).toMap();
                qlonglong frame = nodeInfo.value("frame").toLongLong();
                QString nodeUuidStr = nodeInfo.value("node_uuid").toString();

                DS_NodePtr node = _view->controller()->session()->getNodeByUuid(nodeUuidStr);
                if (node != nullptr)
                {
                    QVariantMap* nodeProps = node->getPropertiesPtr();
                    qlonglong length = nodeProps->value("media.frame_count").toLongLong();
                    qlonglong endFrame = frame + length;

                    // RLP_LOG_DEBUG(nodeUuidStr << "start:" << frame << "length:" << length);
                    
                    QVariantMap mediaInfo;
                    mediaInfo.insert("media.name", nodeProps->value("media.name"));
                    mediaInfo.insert("media.thumbnail.image", nodeProps->value("media.thumbnail.image"));

                    GUI_TLT_MediaItem* item = new GUI_TLT_MediaItem(_view, mediaInfo, frame, length, isOdd);

                    isOdd = !isOdd;
                    item->refresh();
                    _mediaItemMap.insert(nodeUuidStr, item);

                    for (qlonglong ifr = frame; ifr < endFrame; ++ifr)
                    {
                        _frameNodeMap.insert(ifr, nodeUuidStr);
                    }
                }
            }
        }
    }
}


void
GUI_TLT_MediaManager::handleSelectionUpdated(QVariantMap evtInfo)
{
    // RLP_LOG_DEBUG("")

    QString selMode = evtInfo.value("mode").toString();
    QVariantList selItems = evtInfo.value("sel_info").toList();
    
    bool doEnabled = true;
    if (selMode == "rem")
    {
        doEnabled = false;
    }

    for (int ri = 0; ri != selItems.size(); ++ri)
    {
        QString mediaUuidStr = selItems.at(ri).toMap().value("uuid").toString();
        if (_mediaItemMap.contains(mediaUuidStr))
        {
            _mediaItemMap.value(mediaUuidStr)->setEnabled(doEnabled);
            _mediaItemMap.value(mediaUuidStr)->update();
        }
    }

    if (selItems.size() == 0) // deselect everything
    {
        ItemIterator it;
        for (it = _mediaItemMap.begin(); it != _mediaItemMap.end(); ++it)
        {
            it.value()->setEnabled(true);
            it.value()->update();
        }

    }
}


void
GUI_TLT_MediaManager::onNoticeEvent(QString evtName, QVariantMap evtInfo)
{
    // RLP_LOG_DEBUG(evtName)

    if (evtName == "playlist.changed")
    {
        handlePlaylistChanged(evtInfo);
    }
    else if (evtName == "playlist.selection_updated")
    {
        handleSelectionUpdated(evtInfo);
    }
}