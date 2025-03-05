//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include <iostream>

#include "RlpCore/PY/Interp.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/DISP/GLView.h"

#include "RlpExtrevlens/RLQTCNTRL/Plugin.h"

RLP_SETUP_EXT_LOGGER(RLQTCNTRL, Plugin)

RLQTCNTRL_Plugin::RLQTCNTRL_Plugin() :
    DS_EventPlugin(QString("KeyBindings"), true)
{
    RLP_LOG_DEBUG("initialized");
}


DS_EventPlugin*
RLQTCNTRL_Plugin::duplicate()
{
    // RLP_LOG_DEBUG("")

    return new RLQTCNTRL_Plugin();
}


bool
RLQTCNTRL_Plugin::keyPressEventInternal(QKeyEvent* event)
{
   // RLP_LOG_DEBUG("");
   
   //
   // Upcall into python - revlens.keyboard.handle_key_press_event()
   //
   
   // PythonQtObjectPtr mainModule = PythonQt::self()->getMainModule();
   
   // mainModule.evalScript("import revlens.keyboard\n");
   
   
   // PythonQtObjectPtr tag = mainModule.evalScript("revlens.keyboard\n", Py_eval_input);
   // Q_ASSERT(!tag.isNull());
   
   QVariantList argList; // = QVariantList();
   QVariantMap keyEventMap;
   keyEventMap.insert("key", event->key());
   keyEventMap.insert("modifiers", (int)event->modifiers());
   keyEventMap.insert("text", event->text());

   argList.append(keyEventMap);

   // tag.call("handle_key_press_event", argList);
   QVariant result = PY_Interp::call("revlens.keyboard.handle_key_press_event", argList);
   
   return true;
   
}


bool
RLQTCNTRL_Plugin::keyReleaseEventInternal(QKeyEvent* event)
{
    // RLP_LOG_DEBUG("RLQTCNTRL_Plugin::keyReleaseEvent() with python" << std::endl;
    return false;
}


bool
RLQTCNTRL_Plugin::mousePressEventInternal(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    if (event->button() == Qt::RightButton)
    {

        // RLP_LOG_DEBUG("right click");

        GUI_MenuPane* contextMenu = _display->contextMenu();

        QPointF spos = _display->mapToScene(event->position());
        QPointF dpos = _display->position(); 
        
        contextMenu->setPos(
            spos.x() + dpos.x(),
            spos.y() + dpos.y()
        );

        contextMenu->setVisible(!contextMenu->isVisible());

        return true;
    }

    return false;
}