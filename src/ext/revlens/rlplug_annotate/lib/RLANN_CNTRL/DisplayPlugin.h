

#include "RlpExtrevlens/RLANN_CNTRL/Global.h"

#include "RlpExtrevlens/RLANN_DS/Annotation.h"
#include "RlpExtrevlens/RLANN_DS/Track.h"

class RLANN_CNTRL_DrawController;

class EXTREVLENS_RLANN_CNTRL_API RLANN_CNTRL_DisplayPlugin : public DS_EventPlugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLANN_CNTRL_DisplayPlugin(RLANN_CNTRL_DrawController* annCntrl);
    ~RLANN_CNTRL_DisplayPlugin();

    bool setDisplay(DISP_GLView* display);

    void updateAnnDisplayWindow(RLANN_DS_AnnotationPtr ann);
    QPointF mapToDisplay(MousePos& annPos);

    bool hoverMoveEventInternal(QHoverEvent* event);
    bool mousePressEventInternal(QMouseEvent* event);
    bool mouseMoveEventInternal(QMouseEvent* event);
    bool mouseReleaseEventInternal(QMouseEvent* event);
    bool tabletEventInternal(QTabletEvent* event);

    void strokeTo(RLANN_DS_Track* track, qlonglong frame, const QPointF &endPoint, QVariantMap mdata=QVariantMap());
    bool strokeBegin(QPointF pos, QPointF globalpos);
    void strokeMove(QPointF pos, QPointF globalpos, QVariantMap mdata=QVariantMap());
    void strokeEnd(QPointF pos, QPointF globalpos);

    void resizeGLInternal(int width, int height);
    void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata);

public slots:

    // Display
    void onDisplayZoomEnd(float displayZoom);
    void onDisplayHoverEnter();


protected:
    QString _username;

    RLANN_CNTRL_DrawController* _annCntrl;
    bool _drawing;

    int _windowWidth;
    int _windowHeight;

    float _displayZoom;
    float _relZoom;

    float _zoom;
};

