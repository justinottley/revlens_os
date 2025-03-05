//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#include <pybind11/pybind11.h>

#include <PythonQt/PythonQt.h>

#include "RlpExtrevlens/RLQPSD_MEDIA/Plugin.h"



bool
bootstrap()
{
    std::cout << "RLQPSD_MEDIA_PyModule::bootstrap()" << std::endl;
    
    PythonQtObjectPtr mainModule = PythonQt::self()->getMainModule();
    
    
    RLQPSD_MEDIA_Plugin* p = new RLQPSD_MEDIA_Plugin();
    mainModule.addObject("__RLQPSD_MEDIA__", p);
    mainModule.evalScript(QString("import rlplug_qpsd;rlplug_qpsd.RLQPSD_MEDIA = __RLQPSD_MEDIA__"));
    
    return true;
}


PYBIND11_MODULE(libRlpExtrevlensRLQPSD_PYMODULE, m) {
    m.doc() = "Revlens rlplug_qpsd python extension module"; // optional module docstring

    m.def("bootstrap", &bootstrap, "Bootstrap Python Bindings");
}