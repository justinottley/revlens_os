

from rlp import QtCore, QtGui


import rlp.gui as RlpGui

class GUI_ButtonGroup(RlpGui.GUI_ItemBase):

    MODE_HORIZONTAL = 0
    MODE_VERTICAL = 1

    def __init__(self, parent, mode=MODE_HORIZONTAL):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.buttonPressed = QtCore.PY_Signal(self)

        self.buttons = []
        self.views = {}
        self.mode = mode

        self._colFgActive = QtGui.QColor(110, 110, 150)
        self._colBgActive = QtGui.QColor(90, 90, 130)

        # self._colFgInactive = QtGui.QColor(200, 200, 200)
        # self._colBgInactive = QtGui.QColor(160, 160, 160)
        self._colFgInactive = QtGui.QColor(100, 100, 100)
        self._colBgInactive = QtGui.QColor(50, 50, 50)


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

    def buttonWidth(self):
        result = 0
        for btn in self.buttons:
            result += btn.width()

        return result


    def setSizeButtonsToParent(self, toParent):
        self._sizeButtonsToParent = toParent


    def addButton(self, title, imagePath='', iconSize=20, iconHeight=30, iconWidthPadding=10, padding=4, view=None, offset=0):

        # if imagePath.endswith('.svg'):
        #     button = RlpGui.GUI_SvgButton(imagePath, self, iconSize)
        #     button.setText(title)
        #     button.setOverlayText(title)

        # else:
        button = RlpGui.GUI_IconButton(imagePath, self, iconSize, padding)
        button.setText(title)
        button.setHeight(iconHeight)
        button.setTextHOffset(5)
        button.setWidth(button.width() + iconWidthPadding)
        button.setOutlined(True)

        button.setMetadata('title', title)
        bsvg = button.svgIcon()
        if bsvg:
            bsvg.setMetadata('title', title)

        button.buttonPressed.connect(self.onButtonPressed)

        self.buttons.append(button)
        self.layout.addItem(button, offset)

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
            fgCol = self._colFgActive 
            bgCol = self._colBgActive

        else:
            fgCol = self._colFgInactive
            bgCol = self._colBgInactive

        button.setOutlinedBgColour(fgCol)
        button.setHoverColour(fgCol)
        button.icon().setFgColour(fgCol)
        button.icon().setBgColour(bgCol)

        button.icon().update()


    def onButtonPressed(self, md):

        print(md)
        if not md:
            print('Warning - invalid button press')
            return

        title = md['title']
        prevView = self.__currentViewName
        self.__currentViewName = title

        pressedButton = None
        for button in self.buttons:
            if button.metadataValue('title') == title:

                # make sure button metadata is reemitted
                md.update(button.metadata())
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
            viewObj.setVisible(True)

            if hasattr(viewObj, 'onParentSizeChangedItem'):
                viewObj.onParentSizeChangedItem(self._w, self._h)

            else:
                viewObj.setWidth(self._w)
                viewObj.setHeight(self._h)
                
            viewObj.update()

        md['obj'] = pressedButton
        md['prev_view'] = prevView
        self.buttonPressed.emit(md)


    def onParentSizeChangedItem(self, width, height):

        if self._sizeButtonsToParent:

            buttonWidth = (width - 10) / len(self.buttons)
            for button in self.buttons:
                button.setWidth(buttonWidth)

        self._w = width
        self._h = height

        for viewObj in self.views.values():
            viewObj.onParentSizeChangedItem(width, height)
            # viewObj.setWidth(width)
            # viewObj.setHeight(height)

