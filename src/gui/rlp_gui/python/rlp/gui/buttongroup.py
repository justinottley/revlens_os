

from rlp import QtCore, QtGui


import rlp.gui as RlpGui

class GUI_ButtonGroup(RlpGui.GUI_ItemBase):

    MODE_HORIZONTAL = 0
    MODE_VERTICAL = 1

    def __init__(self, parent, mode=MODE_HORIZONTAL):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.buttonPressed = QtCore.PySignal()

        self.buttons = []
        self.views = {}
        self.mode = mode

        if mode == GUI_ButtonGroup.MODE_HORIZONTAL:
            self.layout = RlpGui.GUI_HLayout(self)
            self.setHeight(20)

        elif mode == GUI_ButtonGroup.MODE_VERTICAL:
            self.layout = RlpGui.GUI_VLayout(self)
            self.setWidth(30)

        self._sizeButtonsToParent = False

        self._w = parent.width()
        self._h = parent.height()

        self.__currentViewName = None
        
        self.onParentSizeChangedItem(parent.width(), parent.height())


    @property
    def currentViewName(self):
        return self.__currentViewName

    @property
    def currentView(self):
        return self.views.get(self.currentViewName)

    def setSizeButtonsToParent(self, toParent):
        self._sizeButtonsToParent = toParent


    def addButton(self, title, imagePath='', iconSize=20, view=None):

        if imagePath.endswith('.svg'):
            button = RlpGui.GUI_SvgButton(imagePath, self, iconSize)
            button.setText(title)

        else:
            button = RlpGui.GUI_IconButton(imagePath, self, iconSize, 4)
            button.setText(title)
            button.setOutlined(True)

        button.setMetadata("title", title)
        button.buttonPressed.connect(self.onButtonPressed)

        self.buttons.append(button)
        self.layout.addItem(button, 0)

        if view:
            self.views[title] = view
            view.setWidth(0)
            view.setHeight(0)
            view.hideItem()

        if self.mode == self.MODE_VERTICAL:
            self.setHeight(self.layout.itemHeight() + 10)

        return button


    def selectButton(self, title):
        self.__currentViewName = title
        self.onButtonPressed({'title':title})


    def setView(self, title, view):
        self.views[title] = view


    def setButtonActiveState(self, button, is_active):

        if is_active:
            fgCol = QtGui.QColor(80, 80, 120)
            bgCol = QtGui.QColor(60, 60, 100)

        else:
            fgCol = QtGui.QColor(80, 80, 80)
            bgCol = QtGui.QColor(60, 60, 60)

        button.setOutlinedBgColour(bgCol)
        button.setHoverColour(fgCol)
        button.update()
        button.icon().update()


    def onButtonPressed(self, md):

        title = md['title']
        prevView = self.__currentViewName
        self.__currentViewName = title

        pressedButton = None
        for button in self.buttons:
            if button.text() == title:
                pressedButton = button
                self.setButtonActiveState(button, True)

            else:
                self.setButtonActiveState(button, False)

        for viewName, viewObj in self.views.items():
            if viewName == title:
                continue
            
            viewObj.setWidth(0)
            viewObj.setHeight(0)
            viewObj.hideItem()

        if title in self.views:
            # print('Showing: {} {}x{}'.format(title, self._w, self._h))
            viewObj = self.views[title]
            viewObj.setWidth(self._w)
            viewObj.setHeight(self._h)
            viewObj.showItem()

        md['obj'] = pressedButton
        md['prev_view'] = prevView
        self.buttonPressed.emit(md)


    def onParentSizeChangedItem(self, width, height):

        if not self._sizeButtonsToParent:
            return

        buttonWidth = (width - 10) / len(self.buttons)
        for button in self.buttons:
            button.setWidth(buttonWidth)

        self._w = width
        self._h = height

        for viewObj in self.views.items():
            viewObj.setWidth(width)
            viewObj.setHeight(height)

