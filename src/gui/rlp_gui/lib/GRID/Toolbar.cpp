
#include "RlpGui/GRID/Toolbar.h"
#include "RlpGui/BASE/Style.h"

RLP_SETUP_LOGGER(gui, GUI_GRID, Toolbar)

GUI_GRID_Toolbar::GUI_GRID_Toolbar(GUI_ItemBase* parent):
    GUI_ItemBase(parent)
{
    setHeight(30);

    _layout = new GUI_HLayout(this);

    _label = new GUI_Label(_layout, "");

    _colSelector = new GUI_IconButton("", this, 20);
    _colSelector->setPos(5, 5);
    _colSelector->setText("Fields");
    _colSelector->setOutlined(true);
    _colSelector->setSvgIconPath(":/feather/lightgrey/chevron-down.svg", 20);

    GUI_SvgIcon* ri = _colSelector->svgIcon();
    ri->setPos(ri->x(), 4);
    ri->setBgColour(QColor(160, 160, 160));


    _layout->addItem(_label);
    _layout->addSpacer(5); // Item(new GUI_Label(_layout, " "));
    // _layout->addItem(_colSelector);
}


QRectF
GUI_GRID_Toolbar::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_GRID_Toolbar::paint(GUI_Painter* painter)
{

}

