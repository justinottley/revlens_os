
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_FRAMEBASE_H
#define CORE_GUI_FRAMEBASE_H


#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/FocusGroup.h"

enum SplitOrientation {
    O_UNKNOWN,
    O_VERTICAL,
    O_HORIZONTAL
};

class GUI_BASE_API GUI_FrameBase : public GUI_ItemBase {
    Q_OBJECT

signals:
    void tabKeyPressed();

public:
    RLP_DEFINE_LOGGER

    GUI_FrameBase(
        GUI_ItemBase* parent=nullptr,
        QString name="",
        SplitOrientation o=O_UNKNOWN
    );

    virtual ~GUI_FrameBase();
    void onParentSizeChanged(qreal width, qreal height);

    virtual void hoverEnterEvent(QHoverEvent* event);
    virtual void hoverLeaveEvent(QHoverEvent* event);

    virtual void _setOrientation(SplitOrientation o);
    virtual SplitOrientation orientation() { return _orientation; }

public slots:

    int orientationInt() { return (int)_orientation; } // for easier py access
    void setOrientation(int orientInt) { return _setOrientation((SplitOrientation)orientInt); }

    void setResizeToParent(bool rtp) { _doResizeToParent = rtp; }

    virtual QString name() { return _name; }
    virtual void setName(QString name) { _name = name; }
    
    bool isFocused() { return _isFocused; }

    virtual void onTabKeyPressed(QKeyEvent* kevent);
    virtual GUI_FocusGroup* focusGroup() { return _focusGroup; }

protected:

    QString _name;
    SplitOrientation _orientation;
    bool _doResizeToParent;

    bool _isFocused;
    GUI_FocusGroup* _focusGroup;

};


#endif
