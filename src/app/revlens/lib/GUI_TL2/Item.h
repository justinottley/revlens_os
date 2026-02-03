//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_TL2_ITEM_H
#define REVLENS_GUI_TL2_ITEM_H


#include "RlpRevlens/GUI_TL2/Global.h"

class GUI_TL2_View;
class GUI_TL2_Track;

class REVLENS_GUI_TL2_API GUI_TL2_Item : public GUI_ItemBase {
    Q_OBJECT

signals:
    void itemSelected(QVariantMap info);

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_Item(GUI_TL2_Track* track, QUuid uuid, qlonglong frameNum, qlonglong frameCount);


public slots:

    GUI_TL2_View* view();
    GUI_TL2_Track* track() { return _track; }

    virtual void refresh();
    virtual qreal itemWidth() const;

    virtual void setYPos(int ypos) { setPos(0, ypos); }
    virtual void clear() {}

    virtual void updateItem() { update(); }

    QUuid uuid() { return _uuid; }

    qlonglong frameCount();
    void setFrameCount(qlonglong frameCount);

    qlonglong startFrame() { return _startFrame; }
    void setStartFrame(qlonglong frameNum);

    qlonglong endFrame() { return _endFrame; }
    void setEndFrame(qlonglong endFrame);

    bool isSelected() { return _isSelected; }
    void setSelected(bool selected) { _isSelected = selected; }

    bool isEnabled() { return _isEnabled; }
    void setEnabled(bool isEnabled) { _isEnabled = isEnabled; updateItem(); }


protected:

    GUI_TL2_Track* _track;
    QUuid _uuid;

    qlonglong _startFrame;
    qlonglong _endFrame;

    bool _isSelected;
    bool _isEnabled;

};

#endif