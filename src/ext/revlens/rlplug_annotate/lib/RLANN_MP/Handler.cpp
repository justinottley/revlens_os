

#include "RlpExtrevlens/RLANN_MP/Handler.h"
#include "RlpExtrevlens/RLANN_MP/Surface.h"

#include "mypaint-tiled-surface.h"

RLP_SETUP_LOGGER(ext, RLANN_MP, Handler)

RLANN_MP_Handler* RLANN_MP_Handler::_instance;

RLANN_MP_Handler*
RLANN_MP_Handler::instance()
{
    if (RLANN_MP_Handler::_instance == nullptr)
    {
        RLANN_MP_Handler::_instance = new RLANN_MP_Handler();
    }

    return RLANN_MP_Handler::_instance;
}


RLANN_MP_Handler::RLANN_MP_Handler()
{
    initBrushes();
}


void
RLANN_MP_Handler::initSurface(RLANN_DS_AnnotationPtr ann)
{
    QUuid annUuid = ann->uuid();
    if (!_surfMap.contains(annUuid))
    {
        // RLP_LOG_DEBUG("Creating new Surface for" << annUuid << "history size:" << ann->historySize())

        int aHistSize = ann->historySize();
        if (aHistSize > 1)
        {
            RLANN_MP_Surface* s = new RLANN_MP_Surface(ann->getQImage(), ann->imageFromHistory(1));
            _surfMap.insert(annUuid, s);

        } else if (aHistSize > 0)
        {
            RLP_LOG_WARN("Initializing Surface with blank background")

            RLANN_MP_Surface* s = new RLANN_MP_Surface(ann->getQImage(), ann->imageFromHistory(0));
            _surfMap.insert(annUuid, s);
        } else
        {
            RLP_LOG_ERROR("BACKGROUND IMAGE NOT AVAILABLE, CANNOT CREATE SURFACE! Annotation History size:" << ann->historySize())
        }
        
    }
}


RLANN_MP_Surface*
RLANN_MP_Handler::getSurface(QUuid annUuid)
{
    RLANN_MP_Surface* result = nullptr;
    if (_surfMap.contains(annUuid))
    {
        result = _surfMap.value(annUuid);
    } else
    {
        RLP_LOG_WARN("No surface for" << annUuid)
    }

    return result;
}


void
RLANN_MP_Handler::initBrushes()
{
    RLP_LOG_DEBUG("")

    QStringList result;

    QFile bf(":brushes/brushes.conf");
    bf.open(QIODevice::ReadOnly);

    while (!bf.atEnd())
    {
        QString line(bf.readLine().trimmed());
        if (line.isEmpty() || line.startsWith("#")) continue;

        if (line.startsWith("Group:"))
        {
            QString groupName = line.section(':', 1).trimmed();
            _brushCategories.append(groupName);
            RLP_LOG_DEBUG(groupName)
            continue;
        }


        QStringList lineParts = line.split('/');
        if (lineParts.size() != 2)
        {
            continue;
        }


        QString groupName = lineParts[0];
        QString brushName = lineParts[1];

        // RLP_LOG_DEBUG(groupName << brushName)

        if (!_brushMap.contains(groupName))
        {
            _brushMap.insert(groupName, QStringList());
        }

        QStringList brushList = _brushMap.value(groupName);
        brushList.append(brushName);

        _brushMap.insert(groupName, brushList);
    }
}


void
RLANN_MP_Handler::requestBrush(QString brushCat, QString brushName)
{
    QVariantMap info;
    info.insert("category", brushCat);
    info.insert("name", brushName);

    emit brushRequested(info);
}