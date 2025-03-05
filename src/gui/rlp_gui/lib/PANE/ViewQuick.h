
#ifndef GUI_PANE_VIEWQUICK_H
#define GUI_PANE_VIEWQUICK_H

#include "RlpGui/PANE/Global.h"

#include "RlpGui/PANE/PaneFrame.h"

#include "RlpGui/BASE/ToolInfo.h"

#include <QtQuick/QQuickWindow>

class GUI_Scene;
class GUI_Pane;

class GUI_PANE_API GUI_View : public QQuickWindow {
    Q_OBJECT

signals:
    void toolAdded(QString toolName);
    void sizeChanged(qreal width, qreal height);

    void pyGuiReady();
    void selectionChanged(QVariantMap info);
    void frameReady();


public:
    RLP_DEFINE_LOGGER

    // TODO FIXME: should this come from somewhere else?
    enum FloatingPanePosition {
        FloatingPosNone,
        FloatingPosCenter
    };


    GUI_View();

    GUI_Scene* scene() { return _scene; }
    void setScene(GUI_Scene* scene);

    void resizeEvent(QResizeEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    void handleSceneGraphInitialized();
    void handleAfterAnimating();
    void handleBeforeFrameBegin();
    void handleAfterFrameEnd();


public slots:

    GUI_Pane* rootPane() { return _rootPane; }

    QStringList toolNames() { return _tools.keys(); }
    GUI_ToolInfo* tool(QString name) { return _tools.value(name); }
    bool hasTool(QString name) { return _tools.contains(name); }

    void registerTool(QString name, WidgetCreateFunc func, QVariantMap toolInfoIn=QVariantMap());
    void registerToolInternal(QString toolName, GUI_ToolInfo* tool);
    void registerToolGUI(QVariantMap toolInfoIn);

    void setTabHeadersVisible(bool visible);
    bool tabHeadersVisible() { return _tabHeadersVisible; }

    void setSplittersVisible(bool visible);
    bool splittersVisible() { return _splittersVisible; }

    void _emitSelectionChangedPrivate();
    void emitSelectionChanged(QVariantMap info);

    GUI_PaneFrame* createFloatingPane(int width, int height, bool obscuredModal=false);

    void resizeView(qreal width, qreal height) { resize(width, height); }
    void hideAllMenuPanes();
    qreal widthValue() { return width(); }
    qreal heightValue() { return height(); }

    QImage toImage() { return grabWindow(); }

    void setFocusItem(QQuickItem* item);
    QQuickItem* focusItem() { return _focusItem; }
    bool hasFocusItem() { return _focusItem != nullptr; }
    void clearFocusItem(QQuickItem* item);

    QPointF cursorPos() { return QCursor::pos(); }


public slots:
    void forceUpdate();


private:
    void initScene();

    QString _uuid;

    GUI_Scene* _scene;
    GUI_Pane* _rootPane;

    QMap<QString, GUI_ToolInfo*> _tools;

    bool _tabHeadersVisible;
    bool _splittersVisible;

    QVariantMap _selInfo;
    QQuickItem* _focusItem;

};

#endif