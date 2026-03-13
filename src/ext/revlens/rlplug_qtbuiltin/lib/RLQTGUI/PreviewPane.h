

#ifndef EXTREVLENS_RLQTGUI_PREVIEWPANE_H
#define EXTREVLENS_RLQTGUI_PREVIEWPANE_H

#include "RlpExtrevlens/RLQTGUI/Global.h"
#include "RlpExtrevlens/RLQTCNTRL/PreviewController.h"
#include "RlpExtrevlens/RLQTDISP/PreviewViewBase.h"

#include "RlpRevlens/GUI/ViewerLoadOverlay.h"

#include "RlpGui/BASE/ItemBase.h"

class RLQTGUI_PreviewPane : public RLQTDISP_PreviewViewBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTGUI_PreviewPane(GUI_ItemBase* parent);

    void mousePressEvent(QMouseEvent* event);

    void dropEvent(QDropEvent* event);
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;

    void onParentSizeChanged(qreal width, qreal height);

    void paint(GUI_Painter* painter);

public slots:

    static RLQTGUI_PreviewPane* new_RLQTGUI_PreviewPane(GUI_ItemBase* parent)
    {
        return new RLQTGUI_PreviewPane(parent);
    }


protected:
    RLQTCNTRL_PreviewController* _cntrl;
    GUI_ViewerLoadOverlay* _loadOverlay;

    QString _currPath;

};

#endif
