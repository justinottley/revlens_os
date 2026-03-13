//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#include "RlpExtrevlens/RLQTDISP/FadeMessage.h"

RLP_SETUP_EXT_LOGGER(RLQTDISP, FadeMessage)


RLQTDISP_FadeMessage::RLQTDISP_FadeMessage(
        QString message,
        int fontSize,
        int duration,
        QString colour):
    QObject(),
    _message(message),
    _fontSize(fontSize),
    _colour(colour),
    _hasFade(true),
    _duration(duration),
    _ypos(20),
    _textWidth(50),
    _textHeight(20),
    _opacity(1.0),
    _msecInterval(40)
{
    
    QFont currFont = QApplication::font();
    
    currFont.setPixelSize(_fontSize);
    
    QFontMetrics fm(currFont);
    
    _textWidth = fm.horizontalAdvance(message);
    _textHeight = fm.height();

    _valStep = ((float)_msecInterval) / ((float)duration);

    // RLP_LOG_DEBUG("duration:" << duration << "valstep:" <<_valStep)

    _timer.setInterval(_msecInterval);
    connect(
        &_timer,
        &QTimer::timeout,
        this,
        &RLQTDISP_FadeMessage::onTimeout
    );


    // RLP_LOG_DEBUG(" TEXT WIDTH: " << _textWidth << std::endl;
    // RLP_LOG_DEBUG(" TEXT HEIGHT: " << _textHeight << std::endl;
    
    /*
    _scene = new QGraphicsScene();
    
    
    QGraphicsRectItem* bg = _scene->addRect(0, 0, _textWidth + 20, _textHeight + 10); // new QGraphicsRectItem();

    // TODO FIXME COLOR MAP?
    if (_colour == "red") {
        bg->setBrush(QColor(250, 90, 90));
    } else {
        bg->setBrush(QColor(128, 128, 128));
    }
    
    // bg->setPen(Qt::NoPen);
    
    
    QGraphicsSimpleTextItem* i = new QGraphicsSimpleTextItem(message);
    i->setPos(5, 5);
    i->setParentItem(bg);
    i->setFont(currFont);
    
    QGraphicsOpacityEffect* e = new QGraphicsOpacityEffect(_scene);
    e->setOpacity(1.0);
    
    bg->setGraphicsEffect(e);
    
    if (_duration <= 0) {
        _hasFade = false;
    }

    
    _anim = new QPropertyAnimation(e, "opacity", _scene);
    _anim->setDuration(_duration);
    _anim->setStartValue(1.0);
    _anim->setEndValue(0.0);
    */
}


void
RLQTDISP_FadeMessage::onTimeout()
{
    _opacity -= _valStep;
    if (_opacity <= 0)
    {
        _timer.stop();
        emit finished();

    } else
    {
        emit valueChanged();
    }
}


void
RLQTDISP_FadeMessage::renderMessage(GUI_Painter* painter)
{
    int xOff = 20;
    int yOff = _ypos;

    painter->save();
    painter->setOpacity(_opacity);

    QFont f = painter->font();
    f.setPixelSize(_fontSize);
    painter->setFont(f);

    QColor bgCol(GUI_Style::BgDarkGrey);

    if (_colour  == "red")
    {
        bgCol = QColor(250, 90, 90);
    }

    painter->setBrush(QBrush(bgCol));
    painter->drawRect(xOff, yOff, _textWidth + 25, _textHeight + 15);
    painter->setPen(QPen(Qt::white));
    
    painter->drawText(xOff + 10, yOff + 25, _message);
    painter->restore();

}