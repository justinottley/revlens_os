
#include "RlpRevlens/GUI_TL2/SyncHandler.h"

#include "RlpRevlens/GUI_TL2/View.h"


RLP_SETUP_LOGGER(revlens, GUI_TL2, SyncHandler)

GUI_TL2_SyncHandler::GUI_TL2_SyncHandler(GUI_TL2_View* view):
    _view(view),
    _controller(view->controller())
{
}


void
GUI_TL2_SyncHandler::onUpdateToFrame(qlonglong frame)
{
    // RLP_LOG_DEBUG(frame);

    QVariantMap syncInfo;
    syncInfo.insert("frame", frame);
    _controller->emitSyncAction("onUpdateToFrame", syncInfo);
}


void
GUI_TL2_SyncHandler::onScrubStart(qlonglong frame)
{
    // RLP_LOG_DEBUG(frame)

    QVariantMap syncInfo;
    syncInfo.insert("frame", frame);
    _controller->emitSyncAction("onScrubStart", syncInfo);
}


void
GUI_TL2_SyncHandler::onScrubEnd(qlonglong frame)
{
    // RLP_LOG_DEBUG(frame);

    QVariantMap syncInfo;
    syncInfo.insert("frame", frame);
    _controller->emitSyncAction("onScrubEnd", syncInfo);
}