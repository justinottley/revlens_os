
#ifndef REVLENS_GUI_VIEW_TOOLBAR_MANAGER_H
#define REVLENS_GUI_VIEW_TOOLBAR_MANAGER_H

#include "RlpRevlens/GUI_VIEW/Global.h"


class GUI_VIEW_ToolbarManager : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_VIEW_ToolbarManager();

    static GUI_VIEW_ToolbarManager* instance() { return _instance; }
    static void setInstance(GUI_VIEW_ToolbarManager* instance) { _instance = instance; }


public slots:

    QVariantMap getViewerToolbarInfo() { return _toolbarInfoMap; }
    void registerViewerToolbar(QVariantMap toolbarInfo);

private:
    static GUI_VIEW_ToolbarManager* _instance;

    QVariantMap _toolbarInfoMap;

};
#endif
