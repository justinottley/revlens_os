#ifndef REVLENS_GUI_TL2_ToolBar_H
#define REVLENS_GUI_TL2_ToolBar_H

#include "RlpRevlens/GUI_TL2/Global.h"

#include "RlpGui/BASE/ButtonBase.h"
#include "RlpGui/BASE/Style.h"
#include "RlpGui/PANE/SvgButton.h"
#include "RlpGui/WIDGET/TextEdit.h"

#include "RlpRevlens/GUI_TL2/TrackToggleButton.h"



class GUI_TL2_View;

class REVLENS_GUI_TL2_API GUI_TL2_ToolBar : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_ToolBar(GUI_TL2_View* view);

    void onParentSizeChanged(qreal width, qreal height);

    void refresh();

    virtual QRectF boundingRect() const;
    virtual void paint(GUI_Painter* painter);

    GUI_TL2_View* view() { return _view; }
    

public slots:

    void onScrubModeSelected(QVariantMap mdata);
    void setScrubMode();

    void onEditModeSelected(QVariantMap mdata);
    void setEditMode();

    void setVisFrameRange(QVariantMap itemInfo);

    GUI_ButtonBase* button(QString name) { return _buttonMap.value(name); }
    GUI_TextEdit* visStartFrame() { return _visStartFrame; }
    GUI_TextEdit* visEndFrame() { return _visEndFrame; }
    bool endFrameChanged() { return _endFrameChanged; }

    void reset();


private:

    GUI_TL2_View* _view;

    QColor _bgCol;

    QMap<QString, GUI_ButtonBase*> _buttonMap;
    GUI_TextEdit* _visStartFrame;
    GUI_TextEdit* _visEndFrame;
    bool _endFrameChanged;
};

#endif

