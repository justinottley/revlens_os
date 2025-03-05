
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_FILE_BROWSER_H
#define REVLENS_GUI_FILE_BROWSER_H

#include "RlpCore/LOG/Logging.h"

#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/HLayout.h"
#include "RlpGui/BASE/VLayout.h"
#include "RlpGui/MENU/MenuButton.h"
#include "RlpGui/WIDGET/IconButton.h"
#include "RlpGui/PANE/SvgButton.h"
#include "RlpGui/WIDGET/TextEdit.h"
#include "RlpGui/WIDGET/RadioButton.h"
#include "RlpGui/WIDGET/RadioButtonGroup.h"

#include "RlpGui/GRID/CellFormatterStr.h"
#include "RlpGui/GRID/View.h"

#include "RlpRevlens/GUI/Global.h"


class REVLENS_GUI_API GUI_FileBrowser : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_FileBrowser(GUI_ItemBase* parent);

    QRectF boundingRect() const;
    void paint(QPainter *painter);

    static QQuickItem* create(GUI_ItemBase* parent, QVariantMap toolInfo);

    void setRoot(QString rootDirName);


public slots:

    static GUI_FileBrowser* new_GUI_FileBrowser(GUI_ItemBase* parent)
    {
        return new GUI_FileBrowser(parent);
    }

    QString selectedPath();

    void onPathEnterPressed(QVariantMap itemInfo);
    void onSetRootPressed(QVariantMap metadata);
    void onLoadRequested(QVariantMap metadata);
    void onParentSizeChanged(qreal width, qreal height);
    void onBrowseRequested();

private:

    GUI_TextEdit* _pathEdit;
    GUI_VLayout* _layout;

    GUI_IconButton* _setRootButton;
    GUI_SvgButton* _browseButton;
    GUI_GRID_View* _grid;

};

#endif

