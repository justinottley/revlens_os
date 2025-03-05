//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_TAB_H
#define CORE_GUI_TAB_H


#include "RlpGui/PANE/Global.h"
#include "RlpGui/BASE/FrameBase.h"

class GUI_Pane;

class GUI_Tab;
typedef std::shared_ptr<GUI_Tab> GUI_TabPtr;

class GUI_PANE_API GUI_Tab : public GUI_FrameBase {
    Q_OBJECT

signals:
    void visibilityChanged(QString tabName, bool isVisible);


public:
    RLP_DEFINE_LOGGER

    GUI_Tab(GUI_ItemBase* parent, QString name);
    ~GUI_Tab();

    SplitOrientation orientation();

    GUI_FrameBase* parentFrame();
    GUI_Pane* pane() { return _pane; }
    void registerPaneConnection(QMetaObject::Connection conn);
    void resetPaneConnections();

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);


public slots:

    QString name() { return _name; }

    void onSizeChangedInternal();
    void onParentSizeChanged(qreal nwidth, qreal nheight);

    void setTabVisible(bool visible);


private:
    GUI_Pane* _pane;
    QList<QMetaObject::Connection> _paneConnections;

};



#endif
