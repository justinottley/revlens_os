//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_PANE_H
#define CORE_GUI_PANE_H


#include "RlpGui/PANE/Global.h"
#include "RlpGui/BASE/FrameBase.h"
#include "RlpGui/PANE/Tab.h"
#include "RlpGui/PANE/PaneSplitter.h"
#include "RlpGui/PANE/PaneHeader.h"
#include "RlpGui/PANE/PaneBody.h"

class GUI_View;

class GUI_Pane;
typedef std::shared_ptr<GUI_Pane> GUI_PanePtr;

class GUI_PANE_API GUI_Pane : public GUI_FrameBase {
    Q_OBJECT

signals:

    void tabVisibilityChanged(QString name, bool visible);
    void tabCloseRequested(QString tabName, int tabIndex);

public:
    RLP_DEFINE_LOGGER

    static int gPaneCount;

    GUI_Pane(
        GUI_ItemBase* parent=nullptr,
        QString name="",
        SplitOrientation orientation=O_UNKNOWN
    );
    
    ~GUI_Pane();

    void initPane();

    GUI_TabPtr _addTab(GUI_TabPtr tab, int idx=-1, bool tabHeaderVisible=false);

    QRectF boundingRect() const;
    
    void mousePressEvent(QMouseEvent *event);

    void paint(GUI_Painter* painter);

    // Child Panes
    //
    GUI_PanePtr pane(int i);


public slots:

    static GUI_Pane*
    new_GUI_Pane(GUI_ItemBase* parent, QString name, int orientation)
    {
        GUI_Pane* pane = new GUI_Pane(parent, name, (SplitOrientation)orientation);
        pane->initPane();

        return pane;
    }

    // void setSize(qreal width, qreal height);
    void onSizeChangedInternal();
    void setScaleFactor(float scaleFactor);
    void setBgColor(QColor col);

    void setHeaderVisible(bool isVisible, bool recursive=false);
    void setSplittersVisible(bool isVisible, bool recursive=true);

    void setMinWidth(int minWidth) { _minWidth = minWidth; }
    void setMinHeight(int minHeight) { _minHeight = minHeight; }

    float scaleFactor() { return _scaleFactor; }


    bool split(bool headerVisible=true);

    // Child Panes
    //
    GUI_Pane* panePtr(int i);

    int paneCount() { return _panes.size(); }
    void updatePanes(int currPaneIndex=0);
    bool removePane(GUI_Pane* pane);

    GUI_PaneSplitterPtr splitter(int i);
    GUI_PaneSplitter* splitterPtr(int i); // for python
    int splitterCount() { return _splitters.size(); }

    void clear();

    // Tabs
    //
    GUI_TabPtr tab(int i);
    int tabCount() { return _tabs.size(); }
    GUI_TabPtr addTab(QString tabName);
    bool removeTab(QString tabName) { return onTabCloseRequested(tabName, /*destruct=*/ true); }
    GUI_TabPtr getTab(QString tabName);
    int getTabIndex(QString tabName);

    void setActiveTab(int i);
    int activeTabIndex() { return _activeTab; }


    GUI_PaneBody* body() { return _body; }
    GUI_PaneHeader* header() { return _header; }


public slots:

    bool onTabActivateRequested(QString tabName);
    bool onTabCloseRequested(QString tabName, bool destruct=false);
    void onTabVisibilityChanged(QString tabName, bool visible);
    void onTabMoveRequested(QString tabName, QString moveName, QVariant tabWrapper);

    void setLast(bool isLast) { _last = isLast; }
    bool isLast() { return _last; }

    void setSingle(bool single) { _single = single; }
    bool isSingle() { return _single; }


private:

    QList<GUI_PanePtr> _panes;
    QList<GUI_PaneSplitterPtr> _splitters;

    QList<GUI_TabPtr> _tabs;
    int _activeTab;

    GUI_PaneHeader* _header;
    GUI_PaneBody* _body;

    float _scaleFactor; // percentage width / height of parent

    int _minWidth;
    int _minHeight;

    bool _last; // grumble
    bool _single; // only pane child

};


class GUI_PANE_API GUI_PanePtrDecorator : public QObject {
    Q_OBJECT

public slots:

    GUI_PaneBody* body(GUI_PanePtr* panePtr) { return (*panePtr)->body(); }
    GUI_PaneHeader* header(GUI_PanePtr* panePtr) { return (*panePtr)->header(); }
    GUI_PaneSplitter* splitter(GUI_PanePtr* panePtr, int i) { return (*panePtr)->splitter(i).get(); }
    GUI_Pane* pane(GUI_PanePtr* panePtr, int i) { return (*panePtr)->pane(i).get(); }

    int orientationInt(GUI_PanePtr* panePtr) { return (*panePtr)->orientationInt(); }

    int paneCount(GUI_PanePtr* panePtr) { return (*panePtr)->paneCount(); }
    int tabCount(GUI_PanePtr* panePtr) { return (*panePtr)->tabCount(); }
    int splitterCount(GUI_PanePtr* panePtr) { return (*panePtr)->splitterCount(); }

    void clear(GUI_PanePtr* panePtr) { return (*panePtr)->clear(); }

    void setHeaderVisible(GUI_PanePtr* panePtr, bool isVisible, bool recursive=false) { (*panePtr)->setHeaderVisible(isVisible, recursive); }
};

#endif
