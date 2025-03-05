

#include "RlpCore/PYMODULE/main.h"

CryptWrapper::CryptWrapper():
    _key("0xf03f2061eecb9b98")
{
}

void
init_stacktrace_handler()
{
    #ifndef _WIN32
    // UTIL_StacktraceHandler::installSignalHandler();
    #endif
}



void
RlpCorePYMODULE_bootstrap()
{
    CryptWrapper* c = new CryptWrapper();
    
    py::object m = py::module_::import("RlpCorePYMODULE");
    py::object pyobj = py::cast(c);
    py::setattr(m, "CRYPT", pyobj);

}


PYBIND11_MODULE(RlpCorePYMODULE, m) {

    m.doc() = "Revlens Platform Filesystem python extension module";
    m.def("bootstrap", &RlpCorePYMODULE_bootstrap, "Bootstrap Python Bindings");
    // m.def("init_pythonqt", &init_pythonqt, "Initialize PythonQt");
    // m.def("init_stacktrace_handler", &init_stacktrace_handler, "Initialize Stacktrace Handler");

    
    py::class_<CryptWrapper>(m, "CryptWrapper")
        .def("encrypt", &CryptWrapper::encrypt)
        .def("decrypt", &CryptWrapper::decrypt)
    ;
    

}