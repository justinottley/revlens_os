

#include <pybind11/pybind11.h>

#include <PythonQt/PythonQt.h>

#include "RlpEdb/CNTRL/Query.h"
#include "RlpEdb/STOR_SQL/Connection.h"


RLP_SETUP_MODULE_LOGGER(edb, DB_PYMODULE)

void
bootstrap()
{
    RLP_LOG_DEBUG("")

    PythonQt::self()->registerClass(&EdbDs_EntityType::staticMetaObject, "rlp_entitydb");
    PythonQt::self()->registerClass(&EdbDs_FieldType::staticMetaObject, "rlp_entitydb");
    PythonQt::self()->registerClass(&EdbStorSql_Connection::staticMetaObject, "rlp_entitydb");

    PythonQt::self()->registerClass(&EdbCntrl_Query::staticMetaObject, "rlp_entitydb");
    PythonQt::self()->addDecorators(new EdbCntrl_QueryDecorator());

}



PYBIND11_MODULE(libRlpEdbDB_PYMODULE, m) {
    m.doc() = "Revlens Platform Entity Database Engine python extension module";
    m.def("bootstrap", &bootstrap, "Bootstrap python bindings");
}