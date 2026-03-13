
#include "RlpRevlens/AUDIO/Waveform.h"


RLP_SETUP_LOGGER(revlens, AUDIO, Waveform)
RLP_SETUP_LOGGER(revlens, AUDIO, GenerateWaveformTask)

AUDIO_Waveform::AUDIO_Waveform(QByteArray* audioData, float frameRate, qlonglong frameCount):
    _audioData(audioData),
    _frameCount(frameCount),
    _audioFormat(DS_AudioFormat(frameRate))
{
}


QPixmap*
AUDIO_Waveform::drawWaveform(float pixelsPerFrame)
{
    // int samplesPerFrame = _audioFormat.samplesPerFrame();

    qint64 maxByteCount = _audioFormat.bytePositionForFrame(_frameCount);

    qint64 useByteCount = maxByteCount;
    if (_audioData->size() < useByteCount)
    {
        RLP_LOG_WARN("Not enough audio data for" << _frameCount << "frames")
        useByteCount = _audioData->size();
    }

    int samplesPerPixel = int(rint((float)_audioFormat.samplesPerFrame() / (float)pixelsPerFrame));
    int totalSamples = _audioFormat.samplesForBytes(useByteCount);

    int totalPixels = totalSamples / samplesPerPixel;

    // RLP_LOG_DEBUG(
    //     "Samples per pixel:" << samplesPerPixel <<
    //     "Total samples:" << totalSamples <<
    //     "Total pixels:" << totalPixels <<
    //     "Max bytes:" << useByteCount)


    qint64 atSample = 0;

    WaveformDataList wfdl;

    int16_t* bufData = (int16_t*)_audioData->data();

    for (int px=0; px != totalPixels; ++px)
    {
        // RLP_LOG_DEBUG(px)

        // char* bufData = (char*)_audioData->data()[atSample * 2];

        // char* bufData = (int16_t*)(_audioData->data())[atSample * 2];

        float valSumL = 0;
        float valSumR = 0;

        // RLP_LOG_DEBUG("test:" << px << bufData[0])

        for (int sx=0; sx != samplesPerPixel; ++sx)
        {
            // RLP_LOG_DEBUG(px << sx)

            int sampPos = (atSample + sx) * 2;
            // RLP_LOG_DEBUG(sampPos << "" << sampPos + 1 << totalSamples)
            float sampleValL = std::abs((float)bufData[sampPos] / 32767);

            // RLP_LOG_DEBUG(px << sx << sampleValL)
            float sampleValR = std::abs((float)bufData[sampPos + 1] / 32767);

            valSumL += sampleValL;
            valSumR += sampleValR;

        }

        float valAvgL = valSumL / samplesPerPixel;
        float valAvgR = valSumR / samplesPerPixel;


        atSample += samplesPerPixel;

        // RLP_LOG_DEBUG(px << valAvgL << valAvgR)

        wfdl.append({valAvgL, valAvgR});
    }


    int waveformMaxHeight = 20;
    QPixmap* pixmap = new QPixmap(totalPixels, waveformMaxHeight);
    pixmap->fill(Qt::transparent);

    int centerY = pixmap->height() / 2;

    QColor wColor(150, 150, 150, 200);
    QBrush lineBrush = QBrush(wColor, Qt::SolidPattern);
    QPen linePen = QPen(wColor, 1);

    QPainter tmpPainter(pixmap);
    tmpPainter.setPen(linePen);
    tmpPainter.setBrush(lineBrush);
    tmpPainter.setRenderHint(QPainter::Antialiasing, true);

    int lastXPos = 0;

    WaveformDataListIterator wfit;

    int xpos = 0;
    for (wfit = wfdl.begin(); wfit != wfdl.end(); ++wfit)
    {
        float valL = wfit->first;
        float valR = wfit->second;

        int waveformHeightL = int(rint(valL * waveformMaxHeight));
        QRect wrectL(xpos, centerY - waveformHeightL, 1, waveformHeightL);
        tmpPainter.drawRect(wrectL);

        int waveformHeightR = int(rint(valR * waveformMaxHeight));
        QRect wrectR(xpos, centerY, 1, waveformHeightR);
        tmpPainter.drawRect(wrectR);

        xpos++;
    }


    return pixmap;
}


//
// --------
//


AUDIO_GenerateWaveformTask::AUDIO_GenerateWaveformTask(
    DS_NodePtr node,
    float pixelsPerFrame):
        _node(node),
        _pixelsPerFrame(pixelsPerFrame)
{
}


void
AUDIO_GenerateWaveformTask::run()
{
    QVariantMap* nprops = _node->getPropertiesPtr();

    float nodeFrameRate = nprops->value("video.frame_rate").toFloat();
    qlonglong frameCount = nprops->value("media.frame_count").toLongLong();
    QByteArray* audioData = nprops->value("audio.data").value<QByteArray*>();

    // RLP_LOG_DEBUG("Audio data size:" << audioData->size())


    AUDIO_Waveform wf(audioData, nodeFrameRate, frameCount);
    QPixmap* wfPixmap = wf.drawWaveform(_pixelsPerFrame);
    QImage waveform = wfPixmap->toImage();
    delete wfPixmap;

    // RLP_LOG_DEBUG("GOT WAVEFORM:" << waveform.width() << waveform.height())

    QVariantMap props;
    props.insert("audio.waveform.image", waveform);

    QVariantMap flags;
    flags.insert("audio_waveform_ready", true);

    QVariantMap md;
    md.insert("props", props);
    md.insert("flags", flags);
    md.insert("evt_name", "audio_waveform");
    md.insert("evt_info", props);

    _node->emitDataReadyToGraph("update_properties", md);
}