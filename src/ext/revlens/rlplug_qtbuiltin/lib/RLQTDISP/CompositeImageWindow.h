
#ifndef rlplug_QTBUILTIN_DISP_COMPOSITE_IMAGEWINDOW_H
#define rlplug_QTBUILTIN_DISP_COMPOSITE_IMAGEWINDOW_H


#include "RlpExtrevlens/RLQTDISP/Global.h"


#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/Scene.h"



class EXTREVLENS_RLQTDISP_API RLQTDISP_CompositeImageWindow : public GUI_ItemBase {

public:
    RLP_DEFINE_LOGGER

    RLQTDISP_CompositeImageWindow(GUI_ItemBase* parent, QString displayUuidStr, int width, int height, QVariantMap* metadata);

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


#endif