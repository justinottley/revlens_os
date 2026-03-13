
#include "RlpRevlens/GUI/StatusMessage.h"
#include "RlpGui/BASE/Style.h"

RLP_SETUP_LOGGER(revlens, GUI, StatusMessage)

GUI_StatusMessage::GUI_StatusMessage(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _message("")
{
    setWidth(260);
    setHeight(40);
}


void
GUI_StatusMessage::onStatusEvent(QString message)
{
    // RLP_LOG_DEBUG(message)
    _message = message;

    update();
}


void
GUI_StatusMessage::paint(GUI_Painter* painter)
{
    //painter->setPen(GUI_Style::BgMidGrey);
    //painter->drawRect(boundingRect());
    //painter->setPen(Qt::white);
    painter->drawText(2, 15, _message);
}