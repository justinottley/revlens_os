
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_TOOLINFO_H
#define CORE_GUI_TOOLINFO_H

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"

typedef QMap<QString, GUI_ItemBase*> ToolMap;
typedef QQuickItem* (*WidgetCreateFunc)(GUI_ItemBase*, QVariantMap);

typedef QMap<QString, WidgetCreateFunc> ToolCreateMap;

class GUI_BASE_API GUI_ToolInfo : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_ToolInfo(QVariantMap metadata=QVariantMap());

    void setCreateFunc(WidgetCreateFunc func) { _func = func; }
    WidgetCreateFunc func() { return _func; }

public slots:

    void setData(QVariantMap data) { _data = data; }
    QVariantMap data() { return _data; }


private:
    WidgetCreateFunc _func;
    QVariantMap _data;


};

#endif