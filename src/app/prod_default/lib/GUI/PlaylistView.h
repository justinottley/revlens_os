
#ifndef PROD_GUI_PLAYLIST_H
#define PROD_GUI_PLAYLIST_H

#include "RlpProd/GUI/Global.h"

#include "RlpGui/GRID/View.h"

class GUI_PlaylistView : public GUI_GRID_View {
    Q_OBJECT

signals:
    void requestAddToPlaylist(QVariantMap payload);

public:
    RLP_DEFINE_LOGGER

    GUI_PlaylistView(GUI_ItemBase* parent);

    void dragEnterEvent(QDragEnterEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dropEvent(QDropEvent *event);

    void onParentSizeChanged(qreal width, qreal height);

    void paint(GUI_Painter* painter);


public slots:

    static GUI_PlaylistView* new_GUI_PlaylistView(GUI_ItemBase* parent)
    {
        return new GUI_PlaylistView(parent);
    }


private:
    bool _highlightDrop;
};

#endif
