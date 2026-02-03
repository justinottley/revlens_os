//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/GUI_TL2/TrackManager.h"

#include "RlpRevlens/GUI_TL2/View.h"
#include "RlpRevlens/GUI_TL2/Controller.h"

RLP_SETUP_LOGGER(revlens, GUI_TL2, TrackManager)

GUI_TL2_TrackManager::GUI_TL2_TrackManager(GUI_TL2_View* view):
    QObject(view),
    _view(view),
    _heightTrackSpacing(1)
{

    clearTracks();
    initPlugins();

    GUI_TL2_Controller* tlCntrl = GUI_TL2_Controller::instance();
    connect(
        tlCntrl,
        SIGNAL(activateTrackRequested(QString)),
        this,
        SLOT(activateTrack(QString))
    );

}


GUI_TL2_TrackManager::~GUI_TL2_TrackManager()
{
    RLP_LOG_DEBUG(this << "plugins:" << _trackPlugins.size())
    
    // clear() doesn't delete value pointer. need to do it manually
    //
    QStringList pluginNames = _trackPlugins.keys();
    for (int si=0; si != pluginNames.size(); ++si)
    {
        RLP_LOG_DEBUG("Destructing plugin:" << pluginNames.at(si))

        GUI_TL2_TrackFactoryPluginBase* p = _trackPlugins.take(pluginNames.at(si));
        delete p;
    }

    
    // clearTracks();

    RLP_LOG_DEBUG(this << "plugins:" << _trackPlugins.size())
}


void
GUI_TL2_TrackManager::clearTracks()
{
    RLP_LOG_DEBUG(this)

    TrackIterator it;
    for (it = _trackMap.begin(); it != _trackMap.end(); ++it) {

        if ((*it) != nullptr)
        {
            RLP_LOG_DEBUG("Removing Track " << (*it)->name());
            // (*it)->clearItems();
            (*it)->setParentItem(nullptr);
            _view->scene()->removeItem((*it));

            // TODO FIXME: track is not deleted. calling deleteLater()
            // triggers onTrackDeleted(). If onTrackDeleted() is called
            // we do not need to removeItem() or clear anything from internal
            // maps, since that routine handles it. Fix this?
            // 
            RLP_LOG_WARN("MEM LEAK: Track Deletion")
            // (*it)->deleteLater();
        }

    }

    _trackMap.clear();
    _trackUuidMap.clear();

    
}


void
GUI_TL2_TrackManager::initPlugins()
{
    RLP_LOG_DEBUG("")

    GUI_TL2_Controller* tlCntrl = GUI_TL2_Controller::instance();

    QStringList pluginList = tlCntrl->pluginNames();
    for (int pi=0; pi != pluginList.size(); ++pi)
    {
        QString pluginName = pluginList.at(pi);
        GUI_TL2_TrackFactoryPluginBase* plugin = tlCntrl->initPlugin(pluginName, this);

        _trackPlugins.insert(pluginName, plugin);

    }

}


void
GUI_TL2_TrackManager::initCurrSession()
{
    GUI_TL2_Controller* tlCntrl = GUI_TL2_Controller::instance();

    DS_SessionPtr session = tlCntrl->mainController()->session();

    RLP_LOG_DEBUG(session.get())

    for (int ti=0; ti != session->numTracks(); ++ti)
    {
        DS_TrackPtr track = session->getTrackByIndex(ti);
        GUI_TL2_Track* tlTrack = onTrackAdded(
            track->uuid().toString(),
            track->name(),
            track->trackTypeName(),
            false /* refresh, dont do it yet */
        );

        tlTrack->initCurrSession();
    }

    session->setFrameCount(session->frameCount());

}


bool
GUI_TL2_TrackManager::onTrackDeleted(QString trackUuidStr)
{
    // RLP_LOG_DEBUG(trackUuidStr);

    bool result = false;

    if (_trackUuidMap.contains(trackUuidStr)) {

        GUI_TL2_Track* track = _trackUuidMap.value(trackUuidStr);

        int trackIndex = -1;
        
        // Hmm.. get track index.. make sure track is in uuid map and index map
        TrackIterator it;
        for (it = _trackMap.begin(); it != _trackMap.end(); ++it) {

            RLP_LOG_DEBUG(it.key());
            RLP_LOG_DEBUG(it.value()->name());

            if ((*it)->name() == track->name()) {
                trackIndex = it.key();
                break;
            }
        }

        // TODO FIXME: if this is on, ensure things line up with clearTracks()
        track->clearItems();
        
        if (trackIndex != -1) {
            _trackMap.remove(trackIndex);
            updateTrackIndexes();
        }

        _trackUuidMap.remove(trackUuidStr);
        
        // TODO FIXME: if this is on, ensure things line up with clearTracks()
        _view->scene()->removeItem(track);

        result = true;

        refresh();
    }

    return result;
}


void
GUI_TL2_TrackManager::updateTrackIndexes()
{
    RLP_LOG_DEBUG("")

    int trackCount = _trackMap.size();
    QList<int> keys = _trackMap.keys();
    std::sort(keys.begin(), keys.end());

    // RLP_LOG_DEBUG("trackCount:" << trackCount)
    // RLP_LOG_DEBUG(keys)

    QHash<int, GUI_TL2_Track*> ntrackMap;

    int cNewIdx = trackCount;
    for (int nx = trackCount; nx > 0; --nx)
    {
        int cIdx = keys.at(nx - 1);
        // RLP_LOG_DEBUG("Reshuffling idx" << cIdx << "to" << cNewIdx);

        ntrackMap[cNewIdx] = _trackMap.value(cIdx);
        cNewIdx--;
    }

    _trackMap = ntrackMap;

    // RLP_LOG_DEBUG(_trackMap)

}


GUI_Scene*
GUI_TL2_TrackManager::scene()
{
    return _view->scene();
}


GUI_TL2_View*
GUI_TL2_TrackManager::view()
{
    return _view;
}


void
GUI_TL2_TrackManager::onSizeChanged(qreal width, qreal height)
{
    RLP_LOG_WARN("DEPRECATED?")
    refresh(height);
}


void
GUI_TL2_TrackManager::refresh(qreal iheight)
{
    // RLP_LOG_DEBUG("height: " << iheight << " max height: " << _view->timelineMaxTrackHeight())

    int numTracks = _trackMap.size();

    DS_SessionPtr session = _view->controller()->session();
    int numSTracks = session->numTracks();


    // Q_ASSERT(numTracks == numSTracks);
    if (numTracks != numSTracks)
    {
        RLP_LOG_DEBUG("GUI tracks: " << numTracks << " session tracks: " << numSTracks);
        RLP_LOG_WARN("skipping refresh")
        return;
    }

    QList<DS_TrackPtr> trackList = session->trackList();

    int currHeight = 0;
    int theight = _view->timelineHeight() + (_view->timelineMaxTrackHeight() * _view->yscrollPos());

    for (int x = 0; x != trackList.size(); ++ x) {
        GUI_TL2_Track* tlTrack = getTrackByUuid(trackList.at(x)->uuid().toString());
        currHeight += tlTrack->height();
        tlTrack->setYPos(theight - currHeight);
        currHeight += _heightTrackSpacing;
    }

    _view->setTimelineMaxTrackHeight(currHeight);
    _view->updateIfNecessary();
}


void
GUI_TL2_TrackManager::onSessionChanged(DS_SessionPtr session)
{
    // RLP_LOG_DEBUG(session.get())

    connect(
        session.get(),
        SIGNAL(trackAdded(QString, QString, QString)),
        this,
        SLOT(onTrackAdded(QString, QString, QString))
    );

    connect(
        session.get(),
        SIGNAL(trackDeleted(QString)),
        this,
        SLOT(onTrackDeleted(QString))
    );

    
    RLP_LOG_DEBUG("num. plugins: " << _trackPlugins.size());

    TrackPluginIterator it;
    for (it = _trackPlugins.begin(); it != _trackPlugins.end(); ++it) {
        (*it)->onSessionChanged(session);
    }
}


GUI_TL2_Track*
GUI_TL2_TrackManager::onTrackAdded(QString uuidStr,
                                   QString trackName,
                                   QString trackType,
                                   bool doRefresh)
{
    RLP_LOG_DEBUG(uuidStr << " " << trackName << " " << trackType);

    GUI_TL2_TrackFactoryPluginBase* trackPlugin = _trackPlugins.value(trackType);
    if (trackPlugin == nullptr) {
        RLP_LOG_ERROR("No Track plugin for track type \"" << trackType << "\", cannot add track!");
        return nullptr;
    }


    GUI_TL2_Track* track = trackPlugin->runCreateTrack(uuidStr, trackName);
    if (track == nullptr) {
        RLP_LOG_ERROR("INVALID TRACK OBJECT RETURN FROM PLUGIN: " << trackPlugin->name());
        return nullptr;
    }
    
    RLP_LOG_DEBUG("runCreateTrack() " << trackName << " - Done")

    int trackNum = _trackMap.size() + 1;
    if (trackName == "") {
        trackName = "ZTrack %1";
        trackName = trackName.arg(trackNum);
    }

    track->setName(trackName);

    if (_trackMap.contains(trackNum))
    {
        RLP_LOG_ERROR("track already exists at num" << trackNum)
    }


    _trackMap.insert(trackNum, track);
    _trackUuidMap.insert(uuidStr, track);

    trackPlugin->postCreateTrack(track);
    
    track->setParentItem(_view);

    connect(
        track,
        &GUI_TL2_Track::itemSelected,
        this,
        &GUI_TL2_TrackManager::onItemSelected
    );

    if (doRefresh)
    {
        refresh();
    }

    QVariantMap info;
    info.insert("view_uuid", _view->uuid());
    info.insert("track_uuid", uuidStr);
    info.insert("track_name", trackName);
    info.insert("track_type", trackType);

    emit trackAdded(info);

    return track;
}


GUI_TL2_Track*
GUI_TL2_TrackManager::getTrackByUuid(QString uuidStr)
{
    GUI_TL2_Track* result = nullptr;
    if (_trackUuidMap.contains(uuidStr)) {
        result = _trackUuidMap.value(uuidStr);
    }

    return result;
}


GUI_TL2_Track*
GUI_TL2_TrackManager::getTrackByIndex(int idx)
{
    GUI_TL2_Track* result = nullptr;
    if (_trackMap.contains(idx)) {
        result = _trackMap.value(idx);
    }

    return result;
}


QVariantList
GUI_TL2_TrackManager::trackIndexes()
{
    QList<int> tiList = _trackMap.keys();
    QVariantList result;

    for (int i = 0; i != tiList.size(); ++i)
    {
        result.append(tiList.at(i));
    }

    return result;
}


bool
GUI_TL2_TrackManager::activateTrack(QString uuidStr)
{
    // RLP_LOG_DEBUG(uuidStr);
    
    bool result = false;

    TrackIterator it;
    for (it = _trackMap.begin(); it != _trackMap.end(); ++it) {
        (*it)->setActive(false);
    }

    if (_trackUuidMap.contains(uuidStr)) {
        _trackUuidMap.value(uuidStr)->setActive(true);
        result = true;

        emit trackActivated(uuidStr);
    }

    return result;
}


QList<QString>
GUI_TL2_TrackManager::trackUuidList()
{
    return _trackUuidMap.keys();
}



void
GUI_TL2_TrackManager::onItemSelected(QVariantMap info)
{
    RLP_LOG_DEBUG(info);
    QString selectedUuid= info.value("media_uuid").toString();

    TrackIterator it;
    for (it = _trackMap.begin(); it != _trackMap.end(); ++it) {
        QMap<QUuid, GUI_TL2_Item*> items = (*it)->items();
        QMap<QUuid, GUI_TL2_Item*>::iterator iit;


        for (iit=items.begin(); iit!=items.end(); ++iit)
        {

            if ((*iit)->uuid().toString() != selectedUuid)
            {
                (*iit)->setSelected(false);
                (*iit)->updateItem();

            }
        }
    }

    emit itemSelected(info);
}