//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/DS/ControllerBase.h"
#include "RlpRevlens/DS/DataEvent.h"


DS_ControllerBase::DS_ControllerBase():
    CoreCntrl_ControllerBase()
{

}


bool
DS_ControllerBase::event(QEvent* event)
{
    if ((int)event->type() == DS_DataEventType) {

        DS_DataEvent* evt = dynamic_cast<DS_DataEvent*>(event);
        emit dataReady(evt->info());

    }
    return QObject::event(event);
}