
#include "RlpGui/PANE/Pane.h"
#include "RlpGui/PANE/Tab.h"


RLP_SETUP_LOGGER(core, GUI, Tab)

GUI_Tab::GUI_Tab(
    GUI_ItemBase* parent,
    QString name):
        GUI_FrameBase::GUI_FrameBase(parent, name)
{
    _pane = new GUI_Pane(this);

    setWidth(parent->width());
    setHeight(parent->height());
}


GUI_Tab::~GUI_Tab()
{
    // RLP_LOG_DEBUG("DESTRUCTOR")
    delete _pane;
}


GUI_FrameBase*
GUI_Tab::parentFrame()
{
    return dynamic_cast<GUI_FrameBase*>(parentItem());
}


void
GUI_Tab::registerPaneConnection(QMetaObject::Connection conn)
{
    _paneConnections.append(conn);
}


void
GUI_Tab::resetPaneConnections()
{
    for (int ci=0; ci!= _paneConnections.size(); ci++)
    {
        disconnect(_paneConnections.at(ci));
    }
}


void
GUI_Tab::onSizeChangedInternal()
{
    // RLP_LOG_DEBUG(size())

    _pane->setSize(size());
}


void
GUI_Tab::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    setWidth(nwidth);
    setHeight(nheight);

    _pane->setWidth(nwidth);
    _pane->setHeight(nheight);
    _pane->updatePanes();
}


void
GUI_Tab::setTabVisible(bool visible)
{
    setVisible(visible);

    emit visibilityChanged(_name, visible);
}


SplitOrientation
GUI_Tab::orientation()
{
    return parentFrame()->orientation();
}


QRectF
GUI_Tab::boundingRect() const
{
    return parentItem()->boundingRect();
}


void
GUI_Tab::paint(GUI_Painter* painter)
{

}