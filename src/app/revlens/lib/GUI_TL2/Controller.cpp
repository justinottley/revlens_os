
#include "RlpRevlens/GUI_TL2/Controller.h"

#include "RlpRevlens/GUI_TL2/MediaTrackFactoryPlugin.h"

RLP_SETUP_LOGGER(revlens, GUI_TL2, Controller)

GUI_TL2_Controller* GUI_TL2_Controller::_instance;

GUI_TL2_Controller::GUI_TL2_Controller(CNTRL_MainController* mc):
    CoreCntrl_ControllerBase(),
    _mc(mc)
{
    registerTrackFactoryPlugin(
        QString("Media"),
        GUI_TL2_MediaTrackFactoryPlugin::create
    );
}


QList<GUI_TL2_View*>
GUI_TL2_Controller::views()
{
    return _viewMap.values();
}


bool
GUI_TL2_Controller::registerTrackFactoryPlugin(QString tpName, TrackFactoryCreateFunc tfunc)
{
    RLP_LOG_DEBUG("Track Type: " << tpName);

    _trackPlugins.insert(tpName, tfunc);

    return true;
}


bool
GUI_TL2_Controller::registerTimelineView(GUI_TL2_View* view)
{
    if (_viewMap.empty())
    {
        _focusedView = view;
    }

    QString viewUuid = view->uuid();

    RLP_LOG_DEBUG(viewUuid << view)

    _viewMap.insert(viewUuid, view);

    emit timelineViewCreated(viewUuid);

    return true;
}


bool
GUI_TL2_Controller::deregisterTimelineView(GUI_TL2_View* view)
{
    QString viewUuid = view->uuid();
    if (_viewMap.contains(viewUuid))
    {
        RLP_LOG_DEBUG(viewUuid)

        _viewMap.remove(viewUuid);
        return true;
    }

    return false;
}


GUI_TL2_TrackFactoryPluginBase*
GUI_TL2_Controller::initPlugin(QString name, GUI_TL2_TrackManager* tmgr)
{
    RLP_LOG_DEBUG(name)

    GUI_TL2_TrackFactoryPluginBase* result = nullptr;
    if (_trackPlugins.contains(name))
    {
        TrackFactoryCreateFunc func = _trackPlugins.value(name);
        result = func(tmgr);
    }

    return result;
}


void
GUI_TL2_Controller::onActiveTrackChanged(QString uuidStr)
{
    // RLP_LOG_DEBUG(uuidStr)

    emit activateTrackRequested(uuidStr);
}