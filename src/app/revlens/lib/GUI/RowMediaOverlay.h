

#ifndef REVLENS_GUI_ROW_MEDIA_OVERLAY_H
#define REVLENS_GUI_ROW_MEDIA_OVERLAY_H

#include "RlpRevlens/GUI/Global.h"

#include "RlpGui/GRID/RowWidgets.h"
#include "RlpGui/BASE/SvgIcon.h"

class GUI_RowMediaOverlay : public GUI_GRID_RowWidget {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    static const int INSERT_POS_ABOVE;
    static const int INSERT_POS_BELOW;

    GUI_RowMediaOverlay(GUI_GRID_Row* row);

    void dragEnterEvent(QDragEnterEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent *event);

    void paint(GUI_Painter* painter);

private:
    void reset();
    void addToComposite(QDropEvent* event);
    void insertMedia(QDropEvent* event, int pos);

private:
    bool _validDragEnter;
    GUI_SvgIcon* _insertAboveButton;
    GUI_SvgIcon* _insertBelowButton;
    GUI_SvgIcon* _compositeButton;

    QFont _font;
    QString _text;

    bool _inInsertAbove;
    bool _inInsertBelow;
    bool _inComposite;

};


// ----

class GUI_RowMediaWidgetPlugin : public GUI_GRID_RowWidgetPlugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_RowMediaWidgetPlugin();

    void initRowWidget(GUI_GRID_Row* row, QVariantMap widgetInfo);

public slots:

    static GUI_RowMediaWidgetPlugin*
    new_GUI_RowMediaWidgetPlugin()
    {
        return new GUI_RowMediaWidgetPlugin();
    }

};


#endif
