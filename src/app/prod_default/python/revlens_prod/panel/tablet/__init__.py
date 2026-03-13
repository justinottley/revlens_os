




def init_layout():

    import revlens
    from .main import TabletToolbarPanel

    rootPane = revlens.GUI.mainView().rootPane()
    rootPane.panePtr(1).splitterPtr(0).setActive(False) # iOS - Turn off ability to move this manually
    toolbarPane = rootPane.panePtr(0).body()
    return TabletToolbarPanel(toolbarPane)
