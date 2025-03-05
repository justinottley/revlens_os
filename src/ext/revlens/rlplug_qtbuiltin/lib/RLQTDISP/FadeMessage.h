//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLQTDISP_FADEMESSAGE_H
#define EXTREVLENS_RLQTDISP_FADEMESSAGE_H

#include "RlpExtrevlens/RLQTDISP/Global.h"


/*
class RLQTDISP_FadeMessagePropertyAnimation : public QPropertyAnimation {
    
    RLQTDISP_FadeMessagePropertyAnimation();
    
    void updateCurrentTime(int currentTime);
    
};

*/

class EXTREVLENS_RLQTDISP_API RLQTDISP_FadeMessage : public QObject {
    Q_OBJECT
    
signals:
    void finished();
    void valueChanged();


public:
    RLP_DEFINE_LOGGER

    RLQTDISP_FadeMessage(QString message, int fontSize, int duration, QString colour="");
    void renderMessage(GUI_Painter* painter);


public slots:
    
    QString getMessage() { return _message; }
    int getFontSize() { return _fontSize; }
    int getTextWidth() { return _textWidth; }
    int getTextHeight() { return _textHeight; }
    int getDuration() { return _duration; }

    int getYPos() { return _ypos; }
    void setYPos(int ypos) { _ypos = ypos; }

    bool hasFade() { return _hasFade; }

    qreal value() { return _opacity; }

    void start() { _timer.start(); }


private slots:
    void onTimeout();

private:
    
    QString _message;
    int _fontSize;
    QString _colour;
    bool _hasFade;
    int _duration;
    int _ypos;

    int _textWidth;
    int _textHeight;
    
    qreal _opacity;

    int _msecInterval;
    qreal _valStep;

    QTimer _timer;

};



#endif