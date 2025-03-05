
#ifndef EXTREVLENS_RLQTDISP_FADEMESSAGE_PLUGIN_H
#define EXTREVLENS_RLQTDISP_FADEMESSAGE_PLUGIN_H

#include "RlpExtrevlens/RLQTDISP/Global.h"
#include "RlpExtrevlens/RLQTDISP/FadeMessage.h"

#include "RlpRevlens/DS/ControllerBase.h"
#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/DISP/GLView.h"

#include "RlpRevlens/DS/EventPlugin.h"

class EXTREVLENS_RLQTDISP_API RLQTDISP_FadeMessagePlugin : public DS_EventPlugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTDISP_FadeMessagePlugin();
    
    bool setMainController(CNTRL_MainController* controller);
    DS_EventPlugin* duplicate();

    void appendMessage(RLQTDISP_FadeMessage* m);
    void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& metadata);
    
private slots:
    
    void clearMessage();
    void updateDisplay();
    
public slots:
    
    void displayMessage(QString message, int duration=700, QString colour="");
    void setFontSize(int fontSize) { _fontSize = fontSize; }
    void setMessagePosition(int x, int y) { _messagePos = QPoint(x, y); }
    void setEnabled(bool enabled) { _enabled = enabled; }
    void setYPos(int ypos) { _ypos = ypos; }

public slots:

    void onPlayStateChanged(int playState);
    void onVolumeChanged(float vol);
    void onDisplayDeregistered(DISP_GLView* display);

private:
    
    int _fontSize;
    QPoint _messagePos;
    bool _enabled;
    
    int _ypos;

    QList<RLQTDISP_FadeMessage*> _messageQueue;
    
};

#endif