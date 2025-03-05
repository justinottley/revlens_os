//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_SCENE_H
#define CORE_GUI_SCENE_H

#include "RlpCore/LOG/Logging.h"

#include "RlpGui/BASE/Global.h"


class GUI_MenuPaneBase;
class GUI_ViewBase;
class GUI_FrameBase;
class GUI_ItemBase;
class GUI_GLItem;
class GUI_ToolTip;

class GUI_BASE_API GUI_Scene : public QQuickItem {
    Q_OBJECT
    QML_ELEMENT


signals:
    void updateRequested();
    void menuPanesHidden();
    void menuBarMenuShown(QString menuName, int visCount);
    void menuBarMenuHidden(QString menuName, int visCount);

    void keyPressEventInternal(QKeyEvent* event);
    void keyReleaseEventInternal(QKeyEvent* event);
    void tabKeyPressed();

    void tabVisibilityChanged(QString tabName, bool visible);
    void paneMenuToggled(bool visible);
    void floatingPaneOpened();
    void floatingPaneClosed();

    void sizeChanged(qreal width, qreal height);


public:
    RLP_DEFINE_LOGGER

    GUI_Scene(QQuickItem* parent);

    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    bool event(QEvent* event); // need this for public

    void addMenuPane(GUI_ItemBase* pane);
    void removeMenuPane(GUI_ItemBase* pane);

    void addItem(GUI_ItemBase* item);
    void removeItem(GUI_ItemBase* item);
    void registerGLItem(GUI_GLItem* item);

    void initializeGL();
    void initToolTip(GUI_ItemBase* item);


public slots:

    void requestUpdate() { emit updateRequested(); }
    void requestTabVisibilityChanged(QString name, bool visible) { emit tabVisibilityChanged(name, visible); }
    void requestPaneMenuToggled(bool visible) { emit paneMenuToggled(visible); }
    void requestFloatingPaneOpened() { emit floatingPaneOpened(); }
    void requestFloatingPaneClosed() { emit floatingPaneClosed(); }
    void requestMenuBarMenuShown(QString menuName, int visCount) { emit menuBarMenuShown(menuName, visCount); }
    void requestMenuBarMenuHidden(QString menuName, int visCount) { emit menuBarMenuHidden(menuName, visCount); }
    
    void onSizeChangedInternal();

    void hideMenuPanes(GUI_ItemBase* skipPane=nullptr);
    
    // void setView(GUI_View* view) { _view = view; }
    // GUI_View* view() { return _view; }

    void setView(QQuickWindow* view) { _view = view; }
    QQuickWindow* view() { return _view; }


    void setFocusedPane(GUI_FrameBase* pane);

    // void paint(GUI_Painter* painter) {}

    GUI_ItemBase* dragItem() { return _dragItem; }

    void setDragItem(GUI_ItemBase* item);
    void clearDragItem();

    void showToolTip(QString text, int x, int y);
    void hideToolTip();

private:
    QList<GUI_ItemBase*> _menuPanes;
    
    QList<GUI_GLItem*> _glItems;
    QQuickWindow* _view; // GUI_View

    GUI_FrameBase* _focusedPane;

    GUI_ItemBase* _dragItem;
    GUI_ToolTip* _toolTip;
};

#endif
