

#include "RlpExtrevlens/RLANN_MP/Handler.h"


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