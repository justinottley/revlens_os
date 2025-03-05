
#include "RlpRevlens/GUI_TL2/ToolBar.h"

#include "RlpGui/WIDGET/Label.h"

#include "RlpRevlens/GUI_TL2/TrackToggleButtonImage.h"
#include "RlpRevlens/GUI_TL2/Splitter.h"
#include "RlpRevlens/GUI_TL2/BookendDragButton.h"

#include "RlpRevlens/GUI_TL2/View.h"

#include "RlpCore/PY/Interp.h"

RLP_SETUP_LOGGER(revlens, GUI, TL2_ToolBar)

GUI_TL2_ToolBar::GUI_TL2_ToolBar(GUI_TL2_View* view):
    GUI_ItemBase(view),
    _view(view),
    _bgCol(35, 35, 35),
    _endFrameChanged(false)
{
    setHeight(20);
    setZValue(2);

    GUI_SvgButton* scrubButton = new GUI_SvgButton(":misc/move.svg", this, 20);
    scrubButton->setPos(10, 0);

    connect(
        scrubButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_TL2_ToolBar::onScrubModeSelected
    );


    _buttonMap.insert("scrub", scrubButton);

    GUI_SvgButton* editButton = new GUI_SvgButton(":misc/cursor.svg", this, 15);
    editButton->setPos(35, 3);

    connect(
        editButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_TL2_ToolBar::onEditModeSelected
    );
    _buttonMap.insert("edit", editButton);

    GUI_SvgButton* gearButton = new GUI_SvgButton(
        ":feather/lightgrey/settings.svg", this, 15, 0, GUI_Style::TrackHeaderFg
    );
    gearButton->connectToMenu();
    gearButton->setPos(60, 3);

    QVariantList args;
    QVariant v;
    v.setValue(gearButton->menu());
    args.append(v);

    PY_Interp::call("revlens.gui.timeline.startup_menu", args);

    _buttonMap.insert("settings", gearButton);


    _visStartFrame = new GUI_TextEdit(this, 100, 80);
    connect(
        _visStartFrame,
        &GUI_TextEdit::enterPressed,
        this,
        &GUI_TL2_ToolBar::setVisFrameRange
    );

    _visStartFrame->setPos(80, 0);
    _visStartFrame->setText("1");
    _visStartFrame->setTextColour(GUI_Style::BgGrey);
    _visStartFrame->setHScrollBarVisible(false);
    _visStartFrame->setOutlined(false);

    GUI_Label* frameDash = new GUI_Label(this, "-");
    frameDash->setPen(QPen(GUI_Style::BgGrey));
    frameDash->setPos(110, 0);

    _visEndFrame = new GUI_TextEdit(this, 100, 80);
    connect(
        _visEndFrame,
        &GUI_TextEdit::enterPressed,
        this,
        &GUI_TL2_ToolBar::setVisFrameRange
    );
    _visEndFrame->setPos(120, 0);
    _visEndFrame->setText(QString("%1").arg(_view->frameEnd()));
    _visEndFrame->setTextColour(GUI_Style::BgGrey);
    _visEndFrame->setHScrollBarVisible(false);
    _visEndFrame->setOutlined(false);

    setScrubMode();
}


void
GUI_TL2_ToolBar::refresh()
{
    setWidth(
        _view->timelineTrackHeaderWidth() +
        GUI_TL2_Splitter::BUTTON_WIDTH +
        GUI_TL2_BookendDragButton::BUTTON_WIDTH
        - 3
    );
}


void
GUI_TL2_ToolBar::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << " " << nheight << _view->timelineTrackHeaderWidth())

    setPos(2, nheight - height() - 10);

    refresh();
}


void
GUI_TL2_ToolBar::setScrubMode()
{
    RLP_LOG_DEBUG("")

    GUI_SvgButton* scrub = qobject_cast<GUI_SvgButton*>(_buttonMap.value("scrub"));
    scrub->icon()->setBgColour(QColor(230, 230, 230));
    scrub->icon()->update();

    GUI_SvgButton* edit = qobject_cast<GUI_SvgButton*>(_buttonMap.value("edit"));
    edit->icon()->setBgColour(GUI_Style::BgLightGrey);
    edit->icon()->update();

}

void
GUI_TL2_ToolBar::setEditMode()
{
    RLP_LOG_DEBUG("")

    GUI_SvgButton* scrub = qobject_cast<GUI_SvgButton*>(_buttonMap.value("scrub"));
    scrub->icon()->setBgColour(GUI_Style::BgLightGrey);
    scrub->icon()->update();

    GUI_SvgButton* edit = qobject_cast<GUI_SvgButton*>(_buttonMap.value("edit"));
    edit->icon()->setBgColour(QColor(230, 230, 230));
    edit->icon()->update();

}


void
GUI_TL2_ToolBar::setVisFrameRange(QVariantMap itemInfo)
{
    RLP_LOG_DEBUG("")

    QString sval = _visStartFrame->text();
    QString eval = _visEndFrame->text();

    qlonglong scount = _view->controller()->session()->frameCount();

    if (sval == "")
    {
        sval = "1";
        _visStartFrame->setText("1");
        update();
    }

    if (eval == "")
    {
        eval = QString("%1").arg(scount);
        _visEndFrame->setText(eval);
        _endFrameChanged = false;
        update();
    }

    bool sok = false;
    bool eok = false;
    int sival = sval.toInt(&sok);
    int eival = eval.toInt(&eok);

    

    RLP_LOG_DEBUG("session frame count:" << scount)

    if (eok && eival != scount)
    {
        _endFrameChanged = true;
    }

    if (sok && eok)
    {
        if (sival > eival)
        {
            RLP_LOG_WARN("Invalid frame range, resetting end frame")
            eival = sival + 1;
            _visEndFrame->setText(QString("%1").arg(eival));
            _visEndFrame->textArea()->update();
            _endFrameChanged = true;
            update();
        }


        _view->setFrameStart(sival);
        _view->setFrameEnd(eival);
        _view->update();

    }
}


void
GUI_TL2_ToolBar::onScrubModeSelected(QVariantMap mdata)
{
    RLP_LOG_DEBUG("")
    _view->setTimelineInputMode(GUI_TL2_View::TL_MODE_SCRUB);
}


void
GUI_TL2_ToolBar::onEditModeSelected(QVariantMap mdata)
{
    RLP_LOG_DEBUG("")

    int modeInt = (int)GUI_TL2_View::TL_MODE_EDIT;
    _view->setTimelineInputMode(modeInt);
}


void
GUI_TL2_ToolBar::reset()
{
    RLP_LOG_DEBUG("")

    qlonglong scount = _view->controller()->session()->frameCount();
    QString end = QString("%1").arg(scount);

    _visStartFrame->setText("1");
    _visStartFrame->textArea()->update();
    _visEndFrame->setText(end);
    _visEndFrame->textArea()->update();
}


QRectF
GUI_TL2_ToolBar::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_TL2_ToolBar::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG(boundingRect());

    painter->setPen(_bgCol);
    painter->setBrush(QBrush(_bgCol));

    //painter->setBrush(Qt::red);
    painter->drawRect(boundingRect());
}