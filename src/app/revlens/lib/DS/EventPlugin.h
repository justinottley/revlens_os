//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_CONTROL_PLUGIN_H
#define REVLENS_DS_CONTROL_PLUGIN_H

#include "RlpRevlens/DS/Global.h"
#include "RlpRevlens/DS/Plugin.h"

class GUI_Painter;

class REVLENS_DS_API DS_EventPlugin : public DS_Plugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    DS_EventPlugin(QString name, bool enabled=true);

    virtual ~DS_EventPlugin();

    virtual DS_EventPlugin* duplicate() { return nullptr; }
    virtual bool canDuplicate() { return true; }

    virtual bool keyPressEventInternal(QKeyEvent* event) { return false; }
    virtual bool keyReleaseEventInternal(QKeyEvent* event) { return false; }
    
    virtual bool mousePressEventInternal(QMouseEvent* event) { return false; }
    virtual bool mouseMoveEventInternal(QMouseEvent* event) { return false; }
    virtual bool mouseReleaseEventInternal(QMouseEvent* event) { return false; }
    virtual bool hoverMoveEventInternal(QHoverEvent* event) { return false; }

    virtual bool enterEventInternal(QMouseEvent* event) { return false; }
    virtual bool leaveEventInernal(QMouseEvent* event) { return false; }

    virtual void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& metadata) {}
    virtual void resizeGLInternal(int width, int height) {}

public slots:
    QString name() { return _name; } // redefined for binding

    virtual void setEnabled(bool enabled);
    virtual QString getSettingsUI() { return QString(""); } // redefined for binding

    DS_EventPlugin* asEventClass();

    void setPref(QString name, QVariant val) { setProperty(name.toUtf8().constData(), val); }

protected:

    QList<DS_EventPlugin*> _childPlugins;

};

Q_DECLARE_METATYPE(DS_EventPlugin *)


#endif