
#ifndef EXTREVLENS_RLQTGUI_TILECOMPOSITE_PLUGIN_H
#define EXTREVLENS_RLQTGUI_TILECOMPOSITE_PLUGIN_H

#include "RlpExtrevlens/RLQTGUI/Global.h"

#include "RlpRevlens/DISP/CompositePlugin.h"

#include "RlpRevlens/DISP/GLView.h"

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/Scene.h"


class EXTREVLENS_RLQTGUI_API RLQTGUI_CompositeImageWindow : public GUI_ItemBase {

public:
    RLP_DEFINE_LOGGER

    RLQTGUI_CompositeImageWindow(GUI_ItemBase* parent, QString displayUuidStr, int width, int height, QVariantMap* metadata);

    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    void refresh(int width, int height, float displayZoom, QVariantMap* fbufMetadata);

    void setEnabled(bool enabled) { _enabled = enabled; }

    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);

protected:

    QString _displayUuidStr;

    // Fixed framebuffer size
    int _fbufWidth;
    int _fbufHeight;

    // Coordinate system / rescaling
    //
    
    int _windowWidth;
    int _windowHeight;

    float _displayZoom;

    // zoom adjusted
    //
    float _nDestWidth;
    float _nDestHeight;
    float _nTop;
    float _nLeft;

    bool _inHover;
    bool _enabled;
    QVariantMap* _metadata;

};


class RLQTGUI_TileCompositePlugin : public DISP_CompositePlugin {
    Q_OBJECT

signals:
    // void imageWindowNeeded(QVariantMap* fbufmdata);
    void imageWindowNeeded(GUI_ItemBase* parent, QVariantMap* fbufMetadata);

public:
    RLP_DEFINE_LOGGER

    RLQTGUI_TileCompositePlugin();

    DISP_CompositePlugin* duplicate()
    {
        return new RLQTGUI_TileCompositePlugin();
    }


    // TODO FIXME: first pass layout system, will need expanding
    bool isBufferEnabled(int bufferIdx, QVariantMap* displayMetadata);
    void layout(DS_BufferPtr destFrame, QVariantMap* displayMetadata);

    // void readVideo(DS_Node* node, DS_TimePtr timeInfo, DS_BufferPtr destFrame);

    void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& metadata);
    void resizeGLInternal(int width, int height) {}

    void unsetCompositeState() {}


private:
    // void recalculateImageRects(DS_BufferPtr fbuf, qreal wx, qreal wy, qreal zoom);
    void recalculateImageRects(const QVariantMap& paintMetadata, DS_FrameBufferPtr fbuf);


private slots:
    void onImageWindowNeeded(GUI_ItemBase* parent, QVariantMap* fbufmdata);

private:

    QMap<int, RLQTGUI_CompositeImageWindow*> _imgMap;

};

#endif
