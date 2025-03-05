//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#include "RlpRevlens/MEDIA/Factory.h"
#include "RlpRevlens/AUDIO/Generator.h"
#include "RlpRevlens/AUDIO/Cache.h"
#include "RlpRevlens/AUDIO/WaveformCache.h"

RLP_SETUP_LOGGER(revlens, AUDIO, WaveformCache)
RLP_SETUP_LOGGER(revlens, AUDIO, GenerateWaveformCacheTask)

void
AUDIO_WaveformCache::clear()
{
    // RLP_LOG_DEBUG("");
    _waveformCache.clear();
}


void
AUDIO_WaveformCache::initAudioWaveformData(QList<long> &audioList, float pixelsPerFrame)
{
    // RLP_LOG_DEBUG(audioList.size() << "ppf:" << pixelsPerFrame)

    // AUDIO_Cache* audioCache = _controller->getAudioManager()->cache();

    QList<long>::iterator it;

    // float pixelsPerFrame = std::fmax(getPixelsPerFrame(), 1);

    // RLP_LOG_DEBUG("starting pixels per frame: " << pixelsPerFrame );

    for (it = audioList.begin(); it != audioList.end(); ++it) {

        long videoFrame = (*it);

        if (_waveformCache.contains(videoFrame)) {
            RLP_LOG_DEBUG("SKIPPING WAVEFORM CACHE: " << videoFrame );
            continue;
        }


        if (!_audioCache->videoFrameIndex()->contains(videoFrame)) {
            RLP_LOG_ERROR("not in videoFrameIndex: " << videoFrame );
            continue;
        }

        long bufferIndex = _audioCache->videoFrameIndex()->value(videoFrame);

        // RLP_LOG_DEBUG("vf: " << videoFrame << " buffer index: " << bufferIndex );

        if ((!_audioCache->index()->contains(bufferIndex)) ||
            (!_audioCache->indexVF()->contains(bufferIndex))) {

            RLP_LOG_WARN("index " << bufferIndex << " not found in cache" );
            continue;
        }

        // RLP_LOG_DEBUG("indexvf: " << waveformCache->indexVF()->value(bufferIndex) );



        // Gather all the samples for this frame
        //
        WaveformDataList wfdRaw;

        while (_audioCache->indexVF()->value(bufferIndex) == videoFrame) {

            DS_AudioBufferPtr buf = _audioCache->index()->value(bufferIndex);
            int16_t* bufData = (int16_t*)buf->data();

            for (int x=0; x != buf->samples(); ++x) {

                float sampleValL = std::abs((float)bufData[(x * 2)] / 32767);
                float sampleValR = std::abs((float)bufData[(x * 2) + 1] / 32767);

                QPair<float, float> p(sampleValL, sampleValR);
                wfdRaw.push_back(p);
            }

            bufferIndex += 1;
        }


        // Figure out how many samples corresponds to 1 pixel step
        //
        int wfdSize = wfdRaw.size();
        int samplesForPixel = floor(float(wfdSize) / pixelsPerFrame);

        // RLP_LOG_DEBUG(videoFrame << " pixelsPerFrame: " << pixelsPerFrame << " samplesForPixel: " << samplesForPixel << " wfd size: " << wfdSize );

        WaveformDataList* wfd = new WaveformDataList();

        int xsamplesForFrame = 0;
        for (int x=0; x < wfdSize; x += samplesForPixel) {


            float stepSumL = 0.0;
            float stepSumR = 0.0;

            int sCountL = 0;
            int sCountR = 0;

            for (int sx=0; sx < samplesForPixel; ++sx) {

                if ((x + sx) >= wfdSize) {
                    break;
                }

                // RLP_LOG_DEBUG(f << " " << x + sx << " " << wfd.size() );

                float valL = wfdRaw[x + sx].first;
                if (valL != 0) {
                    stepSumL += valL;
                    sCountL += 1;
                }

                float valR = wfdRaw[x + sx].second;
                if (valR != 0) {
                    stepSumR += valR;
                    sCountR += 1;
                }
            }

            float stepAvgL = 0.0;
            if (sCountL > 0) {
                stepAvgL = stepSumL / (float)sCountL;
                // stepAvgL *= 4.0;
            }

            float stepAvgR = 0.0;
            if (sCountR > 0) {
                stepAvgR = stepSumR / (float)sCountR;
                // stepAvgR *= 4.0;
            }

            QPair<float, float> p(stepAvgL, stepAvgR);

            wfd->push_back(p);


            xsamplesForFrame++;
        }

        // RLP_LOG_DEBUG("xsamplesForFrame: " << xsamplesForFrame << " wfd size: " << wfd->size() );

        _waveformCache[videoFrame] = wfd;

    }

}


void
AUDIO_WaveformCache::drawWaveform(QPixmap* pixmap, long startFrame, long numFrames, float pixelsPerFrame)
{
    // RLP_LOG_DEBUG(startFrame << numFrames << pixelsPerFrame)

    // int trackHeight = 20;
    // int yposOffset = 1;

    int waveformMaxHeight = pixmap->height(); // (trackHeight / 2) + 10;
    int centerY = pixmap->height() / 2; // yposOffset + (trackHeight / 2) - 15;

    QColor wColor(150, 150, 150, 200);
    //QColor wColor(150, 0, 0, 255);
    QBrush lineBrush = QBrush(wColor, Qt::SolidPattern);
    QPen linePen = QPen(wColor, 1);

    QPainter tmpPainter(pixmap);
    tmpPainter.setPen(linePen);
    tmpPainter.setBrush(lineBrush);
    tmpPainter.setRenderHint(QPainter::Antialiasing, true);

    int lastXPos = 0;
    float ppf = std::fmax(1, pixelsPerFrame);

    for (int f=startFrame; f != (startFrame + numFrames); ++f) {


        // int xpos = int(rint(ppf * //getXPosAtFrame(f);

        //if (xpos < 0) {
        //    continue;
        //}
        //if (xpos < currPanXPos ) {
        //    continue;


        //if ((xpos != 0) && (xpos == lastXPos)) {
        //    continue;
        //}

        //if (_waveformDisplayCache.contains(f)) {
        //    continue;
        // }



        if (!(_waveformCache.contains(f))) {
            continue;

        }

        WaveformDataList* wfd = _waveformCache.value(f);
        WaveformDataListIterator wfdit;

        // int xpos = (f - startFrame) * ppf;
        int xpos = (f - 1) * ppf;

        // RLP_LOG_DEBUG("AUDIO_WaveformCache::drawWaveform(): " << f << " startF: " << startFrame << " xpos: " << xpos << " " );


        QPainterPath path;

        int xstep = 0;
        for (wfdit = wfd->begin(); wfdit != wfd->end(); ++wfdit) {

            float valL = (*wfdit).first;
            float valR = (*wfdit).second;

            // RLP_LOG_DEBUG(valL << " " << valR );

            int waveformHeightL = int(rint(valL * waveformMaxHeight));
            QRect wrectL(xpos + xstep, centerY - waveformHeightL, 1, waveformHeightL);
            path.addRect(wrectL);

            int waveformHeightR = int(rint(valR * waveformMaxHeight));
            QRect wrectR(xpos + xstep, centerY, 1, waveformHeightR);
            path.addRect(wrectR);

            // RLP_LOG_DEBUG(waveformHeightR << " " << waveformHeightR );

            xstep++;
        }


        tmpPainter.drawPath(path);



        // _waveformDisplayCache[f] = true;


        lastXPos = xpos;
    }


    // QString filename("/tmp/waveform.png");
    // filename += QString("%1").arg(xposOffset);
    // filename += ".png";


    // RLP_LOG_DEBUG("SAVING " << filename.toStdString().c_str() );

    // QImage img = pixmap->toImage();
    // img.save(filename);


}


AUDIO_WaveformCacheController::AUDIO_WaveformCacheController(QString sessionName):
    _session(DS_SessionPtr(new DS_Session(sessionName))),
    _playbackMode(new DS_PlaybackMode())
{
    _playbackMode->setLoopMode(DS_PlaybackMode::NONE);
}



AUDIO_GenerateWaveformCacheTask::AUDIO_GenerateWaveformCacheTask(DS_Node* node):
    _node(node)
{
}


void
AUDIO_GenerateWaveformCacheTask::runGenerateWaveform(DS_ControllerPtr cacheController, QVariantMap* nodeProperties, DS_NodePtr audioReader)
{
    RLP_LOG_DEBUG("");



    // _audioGenerator = new AUDIO_Generator(AUDIO_Generator::makeAudioFormat(), _cacheController);
    AUDIO_Generator audioGenerator(AUDIO_Generator::makeAudioFormat(), cacheController);
    audioGenerator.cache()->clear();

    AUDIO_WaveformCache waveformCache(audioGenerator.cache());

    // _waveformCache = new AUDIO_WaveformCache(_audioGenerator->cache());

    // Create a new audio reader for caching the waveform image
    //


    // Heuristic
    // float ppf = 0.5;
    float ppf = int(rint(2000.0 / (float)cacheController->session()->frameCount()));
    if (ppf < 1.0)
    {
        ppf = 1.0;
    }

    QPixmap* waveform = new QPixmap(
        cacheController->session()->frameCount() * ppf,
        20);

    waveform->fill(Qt::transparent);


    float frameRate = nodeProperties->value("video.frame_rate").toFloat();
    RLP_LOG_DEBUG("video frame rate: " << frameRate
              << " frame count: " <<  cacheController->session()->frameCount() );

    audioGenerator.setTargetFrameRate(frameRate, false);


    //
    // Read audio data
    //
    while (audioGenerator.readAudioOnce(audioReader)) {
    }


    RLP_LOG_DEBUG("Building waveform..")

    // Build waveform
    //
    long totalFrames = cacheController->session()->frameCount();
    QList<long> l;
    for (long fnum=1; fnum <= totalFrames; ++fnum) {
        l.append(fnum);
    }

    waveformCache.initAudioWaveformData(l, ppf);
    waveformCache.drawWaveform(waveform, 1, totalFrames, ppf);

    // Clear audio data, we dont need it anymore
    //
    waveformCache.clear();
    audioGenerator.cache()->clear();


    cacheController->session()->clear();

    DS_Track* track = nodeProperties->value("session.track").value<DS_Track*>();

    // NOTE: There may not be a track available yet to put the waveform image under, so we just use the tempdir right now
    //
    QString waveformPath = QDir::tempPath() + "/" + nodeProperties->value("graph.uuid").value<QUuid>().toString() + "_waveform.png";
    nodeProperties->insert("audio.waveform.path", waveformPath);

    // waveform->save("/tmp/waveform.png");
    // QString filename("/tmp/waveform.png");
    // RLP_LOG_DEBUG("SAVING " << filename.toStdString().c_str() );

    QImage img = waveform->toImage();
    img.save(waveformPath);


    // Emit event to UI
    //
    QVariantMap wfinfo;
    // QVariant wf;
    // wf.setValue(*waveform);

    wfinfo.insert("waveform.path", waveformPath);;
    wfinfo.insert("data_type", "audio_waveform");
    wfinfo.insert("graph.uuid", nodeProperties->value("graph.uuid"));

    _node->emitDataReadyToGraph(wfinfo);
}


void
AUDIO_GenerateWaveformCacheTask::run()
{
    // RLP_LOG_DEBUG("");

    QVariantMap* nodeProperties = _node->getPropertiesPtr();

    MEDIA_Factory* f = MEDIA_Factory::instance();
    DS_NodePtr caAudioReader = f->createAudioReader(nodeProperties); // _node->getPropertiesPtr());

    // RLP_LOG_DEBUG("created node " << caAudioReader->uuid().toString());

    if (caAudioReader == nullptr)
    {
        RLP_LOG_WARN("NO AUDIO READER, ABORTING" );
        return;
    }

    long frameCount = caAudioReader->getProperty<long>("media.frame_count");


    // DS_ControllerPtr cacheController = _node->cacheController();
    DS_ControllerPtr cacheController = DS_ControllerPtr(new AUDIO_WaveformCacheController(_node->uuid().toString()));

    // NOTE: adding track skipped
    // DS_TrackPtr strack = cacheController->session()->addTrack();

    // NOTE: if the audio reader node is added to the track (and hence session) it looks like events
    // are attempted to be emitted by the graph to the node after it is deleted (later) which causes
    // a crash. We avoid this by not inserting the node into the track
    // strack->insertNode(caAudioReader, 1);

    cacheController->session()->setFrameCount(frameCount);

    cacheController->playbackMode()->setInFrame(1);
    cacheController->playbackMode()->setOutFrame(cacheController->session()->frameCount());

    runGenerateWaveform(cacheController, nodeProperties, caAudioReader);
}