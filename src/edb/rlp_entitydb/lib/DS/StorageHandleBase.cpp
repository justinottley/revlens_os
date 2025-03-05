
#include "RlpEdb/DS/EdbObjectBase.h"
#include "RlpEdb/DS/StorageHandleBase.h"
#include "RlpEdb/DS/StorageManagerBase.h"

EdbDs_StorageHandleBase::EdbDs_StorageHandleBase(
    EdbDs_ObjectBase* eobj,
    EdbDs_StorageManagerBase* mgr):
    _eobj(eobj),
    _mgr(mgr)
{
}


/*
template<class T>
void
EdbDs_StorageHandleBase::flushOne(const char* methodName)
{
	T retval;
	int methodIndex = _eobj->metaObject()->indexOfMethod(methodName);
	QMetaMethod m = _eobj->metaObject()->method(methodIndex);
	
	m.invoke(_eobj,
	         Qt::DirectConnection,
	         Q_RETURN_ARG(T, retval));
	
	
}

*/