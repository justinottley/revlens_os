

#ifndef REVLENS_GUI_VIEWER_H
#define REVLENS_GUI_VIEWER_H

#include "RlpRevlens/GUI_VIEW/Global.h"
#include "RlpRevlens/GUI_VIEW/LoadOverlay.h"

#include "RlpGui/PANE/View.h"

#include "RlpRevlens/DISP/GLView.h"

typedef QMap<QString, GUI_ItemBase*>::iterator ToolbarMapIterator;

class GUI_VIEW_Filler : public GUI_ItemBase {

public:
    GUI_VIEW_Filler(GUI_ItemBase* parent):
        GUI_ItemBase(parent)
    {
    }

    void paint(GUI_Painter* painter)
    {
        QColor col = GUI_Style::PaneBg;
        QBrush b(col);
        painter->setPen(col);
        painter->setBrush(b);
        painter->drawRect(boundingRect());
    }
};


class GUI_VIEW_Pane : public GUI_ItemBase {
    Q_OBJECT

signals:
    void toolbarVisUpdated();

public:
    RLP_DEFINE_LOGGER

    GUI_VIEW_Pane(GUI_ItemBase* parent);
    ~GUI_VIEW_Pane();

    static QQuickItem* create(GUI_ItemBase* parent, QVariantMap toolInfo);

    // void onViewHoverEnter();
    // void onViewHoverLeave();
    // void onViewHoverMove();
    

    void dropEvent(QDropEvent* event);
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;

    QRectF boundingRect() const;

public slots:

    static GUI_VIEW_Pane*
    new_GUI_VIEW_Pane(GUI_ItemBase* parent)
    {
        return new GUI_VIEW_Pane(parent);
    }

    DISP_GLView* view() { return _view; }

    GUI_ItemBase* getToolbarByName(QString name) { return _toolbarMap.value(name, nullptr); }

    void setFillerHeight(int height);

    int toolbarYPosOffset() { return _tbYPosOffset; }
    int toolbarHeightOffset() { return _tbYHeightOffset; }

    void onViewContextMenuItemSelected(QVariantMap cmiInfo);
    void onParentSizeChanged(qreal width, qreal height);
    void onParentVisibilityChanged(QString name, bool isVisible);

    // void onViewerToolsToggled(QVariantMap md);

    // void onNoticeEvent(QString evtName, QVariantMap evtInfo);

    void setPresentationMode(bool pmode, bool showTimeline=true);


private:
    void initToolbars();
    void updateToolbarVis();

private:

    DISP_GLView* _view;
    GUI_VIEW_LoadOverlay* _loadOverlay;

    QMap<QString, GUI_ItemBase*> _toolbarMap;
    QList<GUI_ItemBase*> _visibleToolbars;

    bool _presentationMode;

    int _tbYPosOffset; // top toolbars
    int _tbYHeightOffset; // bottom toolbars

};


#endif
