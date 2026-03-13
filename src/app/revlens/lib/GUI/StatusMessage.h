
#ifndef REVLENS_GUI_STATUS_MESSAGE_H
#define REVLENS_GUI_STATUS_MESSAGE_H

#include "RlpRevlens/GUI/Global.h"
#include "RlpGui/BASE/ItemBase.h"

class GUI_StatusMessage : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_StatusMessage(GUI_ItemBase* parent);

    void paint(GUI_Painter* painter);

public slots:
    void onStatusEvent(QString message);


private:
    QString _message;

};

#endif
