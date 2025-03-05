
#include "RlpExtrevlens/RLQTDISP/FadeMessagePlugin.h"

#include "RlpExtrevlens/RLQTDISP/FadeMessage.h"

RLP_SETUP_EXT_LOGGER(RLQTDISP, FadeMessagePlugin)


RLQTDISP_FadeMessagePlugin::RLQTDISP_FadeMessagePlugin() :
    DS_EventPlugin(QString("FadeMessage")),
    _fontSize(20),
    _messagePos(QPoint(50, 50)),
    _enabled(true),
    _ypos(20)
{
    _messageQueue.clear();

    RLP_LOG_DEBUG("Initialized")
}


bool
RLQTDISP_FadeMessagePlugin::setMainController(CNTRL_MainController* controller)
{
    _controller = controller;

    connect(
        _controller->getVideoManager(),
        SIGNAL(displayDeregistered(DISP_GLView*)),
        this,
        SLOT(onDisplayDeregistered(DISP_GLView*))
    );

    return true;
}


DS_EventPlugin*
RLQTDISP_FadeMessagePlugin::duplicate()
{
    // RLP_LOG_DEBUG("")

    RLQTDISP_FadeMessagePlugin* childPlug = new RLQTDISP_FadeMessagePlugin();
    childPlug->setMainController(_controller);

    _childPlugins.append(childPlug);

    return childPlug;

}


void
RLQTDISP_FadeMessagePlugin::displayMessage(QString message, int duration, QString colour)
{
    if (!_enabled) {
        return;
    }

    // RLP_LOG_DEBUG(message << duration)

    RLQTDISP_FadeMessage* m = new RLQTDISP_FadeMessage(message, _fontSize, duration, colour);
    m->setYPos(_ypos);

    connect(
        m,
        &RLQTDISP_FadeMessage::valueChanged,
        this,
        &RLQTDISP_FadeMessagePlugin::updateDisplay
    );


    appendMessage(m);

    if (m->hasFade()) {
        m->start();    
    }


    // CNTRL_MainController* mc = dynamic_cast<CNTRL_MainController *>(DS_ControllerBase::instance());
    CNTRL_MainController* mc = CNTRL_MainController::instance();

    if (mc->getPlaybackState() == CNTRL_MainController::PAUSED)
    {
        mc->getVideoManager()->update();
    }
}


void
RLQTDISP_FadeMessagePlugin::paintGLPostDraw(GUI_Painter* painter, const QVariantMap& metadata)
{
    // RLP_LOG_DEBUG("fade message paint gl post draw")

    if (!_messageQueue.isEmpty())
    {
        // RLP_LOG_DEBUG("GOT MESSAGE");

        RLQTDISP_FadeMessage* m = _messageQueue.last();
        
        // int drawWidth = 20 + m->getTextWidth();
        /// int drawHeight = 10 + m->getTextHeight();
        
        // QRectF t(_messagePos.x(), _messagePos.y(), drawWidth, drawHeight);
        // m->getScene()->render(painter, t);

        m->renderMessage(painter);
    }
}



void
RLQTDISP_FadeMessagePlugin::appendMessage(RLQTDISP_FadeMessage* m)
{
    for (int ci=0; ci != _childPlugins.size(); ++ci)
    {
        RLQTDISP_FadeMessagePlugin* fmp = qobject_cast<RLQTDISP_FadeMessagePlugin *>(_childPlugins.at(ci));
        
        connect(m, SIGNAL(finished()), fmp, SLOT(clearMessage()));
    
        fmp->appendMessage(m);
    
    }

    _messageQueue.push_back(m);
}


void
RLQTDISP_FadeMessagePlugin::clearMessage()
{
    // RLP_LOG_DEBUG("");

    for (int ci=0; ci != _childPlugins.size(); ++ci)
    {
        qobject_cast<RLQTDISP_FadeMessagePlugin *>(_childPlugins.at(ci))->clearMessage();
    }

    _messageQueue.takeFirst();

    CNTRL_MainController* mc = CNTRL_MainController::instance();
    mc->getVideoManager()->update();
}


void
RLQTDISP_FadeMessagePlugin::onPlayStateChanged(int playstate)
{
    // RLP_LOG_DEBUG(playstate)

    QString msg;

    if (playstate == 0)
    {
        msg = "Play";
    } else
    {
        msg = "Pause";
    }

    displayMessage(msg);
}


void
RLQTDISP_FadeMessagePlugin::onVolumeChanged(float vol)
{
    RLP_LOG_DEBUG(vol);

    if (vol == 0)
    {
        displayMessage("Mute");

    } else
    {
        QString volChange("Set Volume: ");
        QString volStr;
        volStr.setNum(vol);
    
        volChange += volStr;
    
        displayMessage(volChange); 
    }
}


void
RLQTDISP_FadeMessagePlugin::onDisplayDeregistered(DISP_GLView* display)
{
    int cidx = -1;
    for (int ci=0; ci != _childPlugins.size(); ++ci)
    {
        if (_childPlugins.at(ci)->display() == display)
        {
            cidx = ci;
            break;
        }
    }

    if (cidx >= 0)
    {
        RLP_LOG_DEBUG("Reregistering child plugin at" << cidx)
        _childPlugins.takeAt(cidx);
    }
}


void
RLQTDISP_FadeMessagePlugin::updateDisplay()
{
    // RLP_LOG_DEBUG("UPDATE DISPLAY")
    
    CNTRL_MainController* mc = CNTRL_MainController::instance();
    
    if (mc->getPlaybackState() == CNTRL_MainController::PAUSED)
    {
        mc->getVideoManager()->update();
    } 

}