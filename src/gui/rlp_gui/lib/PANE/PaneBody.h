
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_PANEBODY_H
#define CORE_GUI_PANEBODY_H

#include "RlpCore/LOG/Logging.h"

#include "RlpGui/PANE/Global.h"
#include "RlpGui/BASE/FrameBase.h"
#include "RlpGui/BASE/ItemBase.h"

class GUI_Pane;

class GUI_PANE_API GUI_PaneBody : public GUI_ItemBase {
    Q_OBJECT


public:
    RLP_DEFINE_LOGGER

    GUI_PaneBody(GUI_ItemBase* pane);

    GUI_Pane* pane() const;

    void onParentSizeChanged(qreal width, qreal height);

    void setBgColor(QColor col);


    SplitOrientation orientation();
    
    QString name();

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);


private:

    QColor _bgColor;
    
    int _paneIdx;
};

#endif

