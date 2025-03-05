
#include "RlpGui/PANE/PaneBody.h"

#include "RlpGui/BASE/Style.h"
#include "RlpGui/PANE/Pane.h"



RLP_SETUP_LOGGER(gui, GUI, PaneBody)

GUI_PaneBody::GUI_PaneBody(GUI_ItemBase* pane):
    GUI_ItemBase::GUI_ItemBase(pane),
    _bgColor(GUI_Style::PaneBg)
{
    //RLP_LOG_DEBUG(_rcol << _gcol << _bcol)
    // setSize(QSizeF(20, 20));
    //setFixedSize(false);

    _paneIdx = GUI_Pane::gPaneCount;
}


GUI_Pane*
GUI_PaneBody::pane() const
{
    return dynamic_cast<GUI_Pane*>(parentItem());
}


SplitOrientation
GUI_PaneBody::orientation()
{
    return pane()->orientation();
}


void
GUI_PaneBody::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(metaObject()->className() << width << height)

    setSize(QSizeF(width, height));
}


QString
GUI_PaneBody::name()
{
    return pane()->name();
}


void
GUI_PaneBody::setBgColor(QColor col)
{
    _bgColor = col;
}


QRectF
GUI_PaneBody::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}



void
GUI_PaneBody::paint(GUI_Painter* painter)
{
    if (pane()->paneCount() > 0)
    {
        return;
    }

    if (!_paintBg)
    {
        return;
    }


    painter->setPen(_bgColor);
    painter->setBrush(_bgColor);
    painter->drawRect(boundingRect());
}