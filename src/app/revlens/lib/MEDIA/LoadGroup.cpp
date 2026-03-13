
#include "RlpRevlens/MEDIA/LoadGroup.h"

RLP_SETUP_LOGGER(revlens, MEDIA, LoadGroup)

MEDIA_LoadGroup::MEDIA_LoadGroup(DS_ControllerPtr controller, DS_Track* track, int numItems):
    _controller(controller),
    _uuid(QUuid::createUuid()),
    _track(track),
    _trackIdx(track->index()),
    _numItems(numItems),
    _completed(0),
    _thumbCompleted(0)
{
}


void
MEDIA_LoadGroup::onMediaDataReady(QString dataType, QVariantMap data)
{
    // RLP_LOG_DEBUG(dataType)

    if (dataType == "video_source")
    {
        _completed += 1;

        QString msg = QString("Loading: %1 / %2").arg(_completed).arg(_numItems);
        // RLP_LOG_DEBUG(msg)
        _controller->emitStatusEvent(msg);
    }

    else if (dataType == "thumbnail_ready")
    {
        if (data.value("image_type").toInt() == 1) // IMAGE_TYPE_FINAL
        {
            _thumbCompleted += 1;
        }
    }


    if ((_completed >= _numItems) || (_thumbCompleted ==_numItems))
    {
        _track->setLoading(false);

        QVariantMap md;
        md.insert("track_idx", _trackIdx);
        md.insert("num_items", _numItems);
        md.insert("load_uuid", _uuid);
        md.insert("data_type", dataType);

        _controller->emitStatusEvent("");
        _controller->emitNoticeEvent("load_complete", md);
        emit complete(_uuid, dataType);
    }
}