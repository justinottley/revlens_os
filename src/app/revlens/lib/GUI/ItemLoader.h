
//
//  Copyright (c) 2014-2022 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_ITEM_LOADER_H
#define REVLENS_GUI_ITEM_LOADER_H

#include "RlpRevlens/GUI/Global.h"

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/HLayout.h"
#include "RlpGui/BASE/VLayout.h"
#include "RlpGui/MENU/MenuButton.h"
#include "RlpGui/WIDGET/Label.h"
#include "RlpGui/WIDGET/IconButton.h"
#include "RlpGui/WIDGET/TextEdit.h"
#include "RlpGui/WIDGET/RadioButton.h"
#include "RlpGui/WIDGET/RadioButtonGroup.h"

#include "RlpRevlens/DS/Track.h"
#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/MediaManager.h"


class REVLENS_GUI_API GUI_ItemLoader : public GUI_ItemBase {
    Q_OBJECT

signals:
    void loadComplete();

public:
    RLP_DEFINE_LOGGER

    GUI_ItemLoader(GUI_ItemBase* parent);

    void initTrackList();

    QRectF boundingRect() const;
    void paint(QPainter *painter);

    void onParentSizeChanged(qreal width, qreal height);

public slots:

    static GUI_ItemLoader* new_GUI_ItemLoader(GUI_ItemBase* parent) { return new GUI_ItemLoader(parent); }

    void onSelectedTrackChanged(QVariantMap sigData);
    void onSelectedPosChanged(QVariantMap sigData);
    void onItemSelectionChanged(QVariantMap mediaInfoIn);

    void onLoadRequested(QVariantMap metadata);
    void onCompareRequested(QVariantMap metadata);

    void setLoadPath(QVariant loadItem) { _loadItem = loadItem; }

private:

    GUI_VLayout* _layout;
    GUI_HLayout* _btnLayout;
    GUI_HLayout* _posLayout;

    GUI_IconButton* _loadButton;
    GUI_IconButton* _trackListButton;
    GUI_IconButton* _posButton;
    GUI_RadioButton* _posLabel;
    GUI_RadioButton* _frameLabel;

    GUI_TextEdit* _frameEdit;

    GUI_IconButton* _compareButton;
    GUI_Label* _compareMedia;

    QVariantMap _currTrackInfo;
    QVariantMap _currMediaSelectedInfo;

    QVariant _loadItem;

};

class REVLENS_GUI_API GUI_ItemLoaderDecorator : public QObject {
    Q_OBJECT

public slots:

    GUI_ItemLoader*
    new_GUI_ItemLoader(GUI_ItemBase* parent)
    {
        return new GUI_ItemLoader(parent);
    }
};


#endif