

#ifndef REVLENS_DISP_COMPOSITE_PLUGIN_H
#define REVLENS_DISP_COMPOSITE_PLUGIN_H

#include "RlpRevlens/DISP/Global.h"

#include "RlpRevlens/DS/Node.h"

class GUI_Painter;
class DISP_GLView;

class REVLENS_DISP_API DISP_CompositePlugin : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    DISP_CompositePlugin(QString name);

    virtual DISP_CompositePlugin* duplicate() { return nullptr; }

    virtual void setDisplay(DISP_GLView* display);

    virtual void layout(DS_BufferPtr buf, QVariantMap* displayMetadata) {}

    virtual void paintGLInternalRecursive(DS_BufferPtr buf, QVariantMap* bufMd, QVariantMap* displayMd, int recDepth) {}
    virtual void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& metadata) {}

    virtual void resizeGLInternal(int width, int height) {}

    // Callback to indicate to plugin that no composite operation
    // is available for this node / frame
    void unsetCompositeState() {}


public slots:

    QString name() { return _name; }
    void setEnabled(bool isEnabled);

protected:
    QString _name;
    bool _enabled;
    DISP_GLView* _display;

};

#endif