
#include "RlpCore/DS/ModelDataItem.h"

#include "RlpCore/PY/Interp.h"
#include "RlpCore/PY/PyQVariantMap.h"

#include <pybind11/eval.h>

RLP_SETUP_LOGGER(core, CoreDs, ModelDataItem)

CoreDs_ModelDataItem::CoreDs_ModelDataItem():
    _populated(false),
    _pycallback(py::none())
{
}

CoreDs_ModelDataItem::~CoreDs_ModelDataItem()
{
    if (!_pycallback.is(py::none()))
    {
        PY_Interp::acquireGIL();
    }

    _items.clear();
    _itemList.clear();
    _data.clear();
}


CoreDs_ModelDataItemPtr
CoreDs_ModelDataItem::makeItem()
{
    PY_Interp::acquireGIL();

    return CoreDs_ModelDataItemPtr(new CoreDs_ModelDataItem());
}


CoreDs_ModelDataItemPtr
CoreDs_ModelDataItem::at(int i)
{
    // RLP_LOG_DEBUG(i)

    return _itemList.at(i);
}


CoreDs_ModelDataItemPtr
CoreDs_ModelDataItem::item(QString key)
{
    return _items.value(key);
}


bool
CoreDs_ModelDataItem::contains(QString key)
{
    return _data.contains(key);
}


bool
CoreDs_ModelDataItem::containsItem(QString key)
{
    return _items.contains(key);
}


QVariant
CoreDs_ModelDataItem::value(QString key)
{
    return _data.value(key);
}


QVariantMap
CoreDs_ModelDataItem::values()
{
    return _data;
}


int
CoreDs_ModelDataItem::size()
{
    return _itemList.size();
}


void
CoreDs_ModelDataItem::append(CoreDs_ModelDataItemPtr i)
{
    _itemList.append(i);
}


void
CoreDs_ModelDataItem::clear()
{
    _items.clear();
    _itemList.clear();
    _data.clear();
}


void
CoreDs_ModelDataItem::insert(QString key, QVariant value)
{
    _data.insert(key, value);
}


void
CoreDs_ModelDataItem::setMimeType(QString key, QString mtype)
{
    _mimeTypes.insert(key, mtype);
}


bool
CoreDs_ModelDataItem::hasMimeType(QString key)
{
    // RLP_LOG_DEBUG(key)

    return _mimeTypes.contains(key);
}


QString
CoreDs_ModelDataItem::mimeType(QString key)
{
    return _mimeTypes.value(key).toString();
}


void
CoreDs_ModelDataItem::insertItem(QString key, CoreDs_ModelDataItemPtr item)
{
    _items.insert(key, item);
}


void
CoreDs_ModelDataItem::setPopulated()
{
    emit populated();
}


void
CoreDs_ModelDataItem::populate()
{
    RLP_LOG_DEBUG("")

    if (_populated) {
        return;
    }

    PY_Interp::acquireGIL();

    if (!_pycallback.is(py::none()))
    {
        // RLP_LOG_DEBUG("PYTHON POPULATE CALLBACK")

        // py::object cb = py::eval(_populatePyCallback.toLocal8Bit().constData());
        // py::dict pydata =  PyQVariantMap(_data).toDict();
        py::object me = py::cast(this);

        _pycallback(me);

    } else {
        emit populated();
    }
    
    // Subclasses should implement this
    
}