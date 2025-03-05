
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_PYSCRIPTING_H
#define CORE_GUI_PYSCRIPTING_H


#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/VLayout.h"
#include "RlpGui/BASE/HLayout.h"

#include "RlpGui/WIDGET/TextEdit.h"
#include "RlpGui/WIDGET/IconButton.h"

#include "RlpGui/WIDGET/Global.h"

class GUI_WIDGET_API GUI_PyScripting : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_PyScripting(GUI_ItemBase* parent);

    QRectF boundingRect() const;

    static QQuickItem* create(GUI_ItemBase* parent, QVariantMap metadata);
    
public slots:

    void onSizeChanged(qreal width, qreal height);
    void onParentSizeChanged(qreal width, qreal height);
    void execText(QString text);
    void runRequested();
    void clearRequested();

    GUI_VLayout* layout() { return _layout; }


private:

    int _textWidgetPadding;

    GUI_VLayout* _layout;
    GUI_TextEdit* _textEdit;

    GUI_IconButton* _runButton;
    GUI_IconButton* _clearButton;
};

#endif