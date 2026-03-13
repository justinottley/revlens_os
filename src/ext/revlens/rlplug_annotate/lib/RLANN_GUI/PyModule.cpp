
#include <PythonQt/PythonQt.h>

#include "RlpExtrevlens/RLANN_DS/Track.h"
#include "RlpExtrevlens/RLANN_DS/BrushBase.h"
#include "RlpExtrevlens/RLANN_DS/Annotation.h"
#include "RlpExtrevlens/RLANN_CNTRL/DrawController.h"
#include "RlpExtrevlens/RLANN_MEDIA/MediaPlugin.h"
#include "RlpExtrevlens/RLANN_MEDIA/NodeFactoryPlugin.h"
#include "RlpExtrevlens/RLANN_GUI/PyModule.h"
#include "RlpExtrevlens/RLANN_GUI/AnnotationTrackFactoryPlugin.h"
#include "RlpExtrevlens/RLANN_GUI/BrushSizeButton.h"

RLP_SETUP_EXT_LOGGER(RLANN_GUI, PyModule)

RLANN_GUI_PyModule::RLANN_GUI_PyModule()
{
}


void
RLANN_GUI_PyModule::bootstrap()
{
    RLP_LOG_DEBUG("");

    PythonQt::self()->registerClass(&RLANN_DS_Track::staticMetaObject, "rlplug_annotate");
    PythonQt::self()->registerClass(&RLANN_DS_BrushBase::staticMetaObject, "rlplug_annotate");
    PythonQt::self()->registerClass(&RLANN_GUI_AnnotationTrackFactoryPlugin::staticMetaObject, "rlplug_annotate");

    PythonQt::self()->registerClass(&RLANN_GUI_BrushSizeButton::staticMetaObject, "rlplug_annotate");
    PythonQt::self()->addDecorators(new RLANN_GUI_BrushSizeButtonDecorator());

    PythonQt::self()->registerClass(&RLANN_GUI_BrushSizeMenuItem::staticMetaObject, "rlplug_annotate");
    
    PythonQtObjectPtr mainModule = PythonQt::self()->getMainModule();
    
    qRegisterMetaType<RLANN_DS_AnnotationPtr>("RLANN_DS_AnnotationPtr");
    PythonQt::self()->registerCPPClass("RLANN_DS_AnnotationPtr", NULL, "revlens");
    PythonQt::self()->addDecorators(new RLANN_DS_AnnotationPtrDecorator());
    
    RLANN_CNTRL_DrawController* plugCntrl = new RLANN_CNTRL_DrawController();
    mainModule.addObject("__RLANN_OBJ_DRAW_CONTROLLER__", plugCntrl);
    mainModule.evalScript(QString("import rlplug_annotate;rlplug_annotate.RLANN_CNTRL_CONTROLLER = __RLANN_OBJ_DRAW_CONTROLLER__"));
    
    RLANN_MEDIA_Plugin* mediaPlugin = new RLANN_MEDIA_Plugin(plugCntrl);
    mainModule.addObject("__RLANN_OBJ_DRAW_MEDIA_PLUGIN__", mediaPlugin);
    mainModule.evalScript(QString("import rlplug_annotate;rlplug_annotate.RLANN_CNTRL_MEDIA = __RLANN_OBJ_DRAW_MEDIA_PLUGIN__"));
    
    RLANN_MEDIA_NodeFactoryPlugin* nodeFactoryPlugin = new RLANN_MEDIA_NodeFactoryPlugin(plugCntrl, mediaPlugin);
    mainModule.addObject("__RLANN_OBJ_DRAW_NODEFACTORY_PLUGIN__", nodeFactoryPlugin);
    mainModule.evalScript(QString("import rlplug_annotate;rlplug_annotate.RLANN_CNTRL_NODEFACTORY  = __RLANN_OBJ_DRAW_NODEFACTORY_PLUGIN__"));

    
    RLANN_GUI_AnnotationTrackFactoryPlugin* trackFactory = new RLANN_GUI_AnnotationTrackFactoryPlugin(plugCntrl);
    mainModule.addObject("__RLANN_OBJ_TRACK_FACTORY__", trackFactory);
    mainModule.evalScript(QString("import rlplug_annotate;rlplug_annotate.RLANN_GUI_TRACK_FACTORY = __RLANN_OBJ_TRACK_FACTORY__"));
}