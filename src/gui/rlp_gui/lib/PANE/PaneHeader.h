
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_PANEHEADER_H
#define CORE_GUI_PANEHEADER_H

#include "RlpGui/BASE/ButtonBase.h"
#include "RlpGui/PANE/SvgButton.h"
#include "RlpGui/PANE/Global.h"
#include "RlpGui/PANE/TabHeader.h"

class GUI_Pane;

class GUI_PANE_API GUI_PaneHeader : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_PaneHeader(GUI_ItemBase* pane);
    
    GUI_TabHeader* addTabHeader(QString tabName, int idx=-1);
    void moveTabHeader(QString tabName, QString moveBeforeName);

    GUI_Pane* pane() const;
    GUI_ButtonBase* actButton() { return _actButton; }

    void removeAt(int i);

    float height() { return _height; }
    void setBgColor(QColor col);
    
    void updateTabHeaders();
    
    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

public slots:

    GUI_TabHeader* tabHeader(int i) { return _tabHeaders.at(i); }
    int headerCount() { return _tabHeaders.size(); }
    void setActive(bool active) { _active = active; }
    bool isActive() { return _active; }
    int tabIndex(QString tabName);

    void onParentSizeChanged(qreal width, qreal height);
    void onActionPressed(QVariantMap metadata);
    QQuickItem* onActionItemSelected(QVariantMap sigData);
    void onToolAdded(QString toolName);
    
    void initActButtonToolsPrivate();

    void onPanRight(QVariantMap md);
    void onPanLeft(QVariantMap md);

private:
    void initActButton();


private:
    
    float _height;
    QColor _bgColor;

    QList<GUI_TabHeader*> _tabHeaders;

    GUI_SvgButton* _actButton;
    GUI_SvgButton* _panLeftButton;
    GUI_SvgButton* _panRightButton;

    qreal _xoffset;
    int _xpan;

    bool _active;
};

#endif
