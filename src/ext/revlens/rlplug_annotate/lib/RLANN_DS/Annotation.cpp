//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include <iostream>
#include <math.h>

#include "RlpExtrevlens/RLANN_DS/Annotation.h"

RLP_SETUP_EXT_LOGGER(RLANN_DS, Annotation)
RLP_SETUP_EXT_LOGGER(RLANN_DS, AnnotationDisplayWindow)

RLANN_DS_Annotation::RLANN_DS_Annotation(int width,
                                        int height,
                                        QString uuidStr,
                                        float opacity,
                                        QColor fillColor):
    RLANN_DS_QImageFrameBuffer(nullptr, width, height),
    _valid(false),
    _opacity(opacity),
    _fillColor(fillColor),
    _undoIndex(-1)
{
    _windowMap.clear();

    if (uuidStr == "") {
        // RLP_LOG_DEBUG("CREATING NEW ANNOTATION UUID");
        _uuid = QUuid::createUuid();
    } else {

        // RLP_LOG_DEBUG("INITIALIZING WITH UUID " << uuidStr);
        _uuid = QUuid(uuidStr);
    }
}


void
RLANN_DS_Annotation::reallocate()
{
    RLP_LOG_DEBUG("")

    _history.clear();
    RLANN_DS_QImageFrameBuffer::reallocate();

    saveToHistory();
}


QString
RLANN_DS_Annotation::diskCachePath()
{
    QString annDiskCachePath = _track->cacheDir().path() + "/" + fileName();
    return annDiskCachePath;
}


void
RLANN_DS_Annotation::setFrameRange(qlonglong startFrame, qlonglong endFrame)
{
    RLP_LOG_DEBUG(startFrame << " - " << endFrame);

    _startFrame = startFrame;
    _endFrame = endFrame;

}


void
RLANN_DS_Annotation::setOpacity(float opacity)
{
    // Clamp
    if (opacity > 1.0)
    {
        opacity = 1.0;
    } else if (opacity < 0)
    {
        opacity = 0;
    }
    
    _opacity = opacity;
}


void
RLANN_DS_Annotation::setUuid(QString annUuidStr)
{
    RLP_LOG_DEBUG("Updating UUID: " << _uuid.toString() << "to" << annUuidStr)

    _uuid = QUuid(annUuidStr);

    // Match UUID for sync
    //
    // if ((annUuidStr != "") && (_uuid.toString() != annUuidStr)) {
    //    RLP_LOG_DEBUG("Updating UUID to " << annUuidStr);
    //    
    // }
}


void
RLANN_DS_Annotation::setWindow(
    QString displayUuidStr,
    int width,
    int height,
    float displayZoom)
{
    // RLP_LOG_DEBUG(displayUuidStr)

    if (!_windowMap.contains(displayUuidStr))
    {
        // RLP_LOG_DEBUG(width << height)

        RLANN_DS_AnnotationDisplayWindow* adw = new RLANN_DS_AnnotationDisplayWindow(
            displayUuidStr,
            _width,
            _height
        );

        _windowMap.insert(displayUuidStr, adw);
    }


    _windowMap.value(displayUuidStr)->update(width, height, displayZoom);
}


QPointF
RLANN_DS_Annotation::mapToImage(QString displayUuidStr, const QPointF& inPoint, QPointF& panPercent)
{
    return _windowMap.value(displayUuidStr)->mapToImage(inPoint, panPercent);
}


void
RLANN_DS_Annotation::saveToHistory()
{
    // RLP_LOG_DEBUG("")

    QImage i = _image->copy();

    if (_undoIndex != -1)
    {
        int hsize = _history.size();
        for (int hi=_undoIndex + 1; hi < hsize; ++hi)
        {
            RLP_LOG_WARN("Clearing from undo history:" << hi)
            _history.removeLast();
        }
        _undoIndex = -1;
    }
    _history.push_back(i);
}


void
RLANN_DS_Annotation::setImageFromHistory(int idx)
{
    // RLP_LOG_DEBUG(idx);

    if ((idx < 0) || (idx >= _history.size()))
    {
        RLP_LOG_ERROR("invalid image history index")
        return;
    }

    QImage i = _history.at(idx);
    QImage* ni = new QImage(i.copy());

    delete _image;
    _image = ni;

    if (idx == (_history.size() - 1))
    {
        idx = -1;
    }

    _undoIndex = idx;

    emit annotationImageChanged(this);
}


bool
RLANN_DS_Annotation::undoStroke()
{
    // RLP_LOG_DEBUG("")

    int hIdx = _undoIndex;
    if (hIdx == -1)
    {
        hIdx = _history.size() - 1;
    }

    hIdx -= 1;

    setImageFromHistory(hIdx);

    return true;
}


bool
RLANN_DS_Annotation::redoStroke()
{
    RLP_LOG_DEBUG("")

    int hIdx = _undoIndex;
    if (hIdx == -1)
    {
        RLP_LOG_DEBUG("cannot redo, at top of history")
        return false;
    }

    hIdx += 1;

    setImageFromHistory(hIdx);

    return true;
}


void
RLANN_DS_Annotation::load(QString imagePath)
{
    loadFromFile(imagePath);

    emit strokeEnd();
}


void
RLANN_DS_Annotation::loadFromSessionData(QVariantMap annData)
{
    RLP_LOG_DEBUG("")

    QByteArray imgdata = QByteArray::fromBase64(annData.value("data").toString().toLatin1());
    if (imgdata.size() == 0)
    {
        RLP_LOG_ERROR("No image data, skipping")
        return;
    }

    loadFromData(imgdata);

    QVariantList undoHistory = annData.value("history").toList();

    RLP_LOG_DEBUG("Loading" << undoHistory.size() << "strokes into undo history")

    for (int hi=0; hi != undoHistory.size(); ++hi)
    {
        QByteArray imgData = QByteArray::fromBase64(undoHistory.at(hi).toString().toLatin1());
        QImage i = QImage::fromData(imgData, "PNG");
        _history.append(i);
    }

}


QByteArray
RLANN_DS_Annotation::getBase64Data(QImage* image)
{
    if (image == nullptr)
    {
        image = _image;
    }

    QByteArray ba;
    QBuffer rbuf(&ba);
    rbuf.open(QIODevice::WriteOnly);

    QByteArray result;
    if (image != nullptr)
    {
        image->save(&rbuf, "PNG", -1);

        rbuf.close();
        result = ba.toBase64();
    } else
    {
        RLP_LOG_ERROR("Invalid image")
    }


    return result;
}


bool
RLANN_DS_Annotation::save(QString path)
{
    RLP_LOG_DEBUG(path);

    return _image->save(path);
}


bool
RLANN_DS_Annotation::save(QIODevice* device, const char* format, int quality)
{
    return _image->save(device, format, quality);
}


bool
RLANN_DS_Annotation::offload()
{
    RLP_LOG_DEBUG("");

    if (!_image->isNull()) {
        
        QString cachePath = diskCachePath();
        save(cachePath);

        // delete _image;
        // _isAvailable = false;
        setDirty(false);

        return true;
    }

    return false;
}


bool
RLANN_DS_Annotation::reload()
{
    RLP_LOG_DEBUG("");

    QString cachePath = diskCachePath();
    return loadFromFile(cachePath);

    //_image = new QImage(diskCachePath(), QImage::Format_RGBA8888);

}


QJsonObject
RLANN_DS_Annotation::toJson(bool data)
{
    QJsonObject annFrameInfo;
    
    QString ann64data = "";
    if (data) {
        ann64data = QString(getBase64Data());
    }
    
    annFrameInfo.insert("data", ann64data);
    annFrameInfo.insert("width", getWidth());
    annFrameInfo.insert("height", getHeight());
    annFrameInfo.insert("uuid", _uuid.toString());

    QJsonArray history;
    for (int i=0; i != _history.size(); ++i)
    {
        QImage hi = _history.at(i);
        QString anndata = getBase64Data(&hi);
        history.append(anndata);
    }

    annFrameInfo.insert("history", history);
    return annFrameInfo;
}

// ----------------------------


RLANN_DS_AnnotationDisplayWindow::RLANN_DS_AnnotationDisplayWindow(
    QString displayUuidStr,
    int width,
    int height):
        _displayUuidStr(displayUuidStr),
        _width(width),
        _height(height),
        _windowWidth(0),
        _windowHeight(0),
        _displayZoom(1.0)
{
}


void
RLANN_DS_AnnotationDisplayWindow::update(int width, int height, float displayZoom)
{
    // RLP_LOG_DEBUG(width << height << displayZoom)

    _windowWidth = width;
    _windowHeight = height;
    
    _displayZoom = displayZoom;
    
    _nDestWidth = _width * displayZoom;
    _nDestHeight = _height * displayZoom;

    _nLeft = ((float)(_windowWidth - _nDestWidth)) / 2.0;
    _nTop = ((float)(_windowHeight - _nDestHeight)) / 2.0;

}


QPointF
RLANN_DS_AnnotationDisplayWindow::mapToImage(const QPointF& inPoint, QPointF& panPercent)
{
    float inX = inPoint.x();
    float inY = inPoint.y();


    // Adjust for pan
    //
    inX = inX - ((_windowWidth)* (panPercent.x() / 2.0));
    inY = inY - ((_windowHeight) * (panPercent.y() / -2.0));

    // Adjust for zoom
    //
    QPointF p(
        (inX / _displayZoom) - (_nLeft / _displayZoom),
        (inY / _displayZoom) - (_nTop / _displayZoom)
    );


    return p;
}
