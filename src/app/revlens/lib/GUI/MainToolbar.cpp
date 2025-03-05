
#include "RlpRevlens/GUI/MainToolbar.h"

#include "RlpGui/MENU/MenuButton.h"
#include "RlpGui/BASE/HLayout.h"

RLP_SETUP_LOGGER(prod, GUI, MainToolbar)

GUI_MainToolbar::GUI_MainToolbar(GUI_PanePtr parent)
{
    GUI_HLayout* layout = new GUI_HLayout(parent->body());

    GUI_MenuButton* fileMenu = new GUI_MenuButton(layout);
    fileMenu->setText("File");

    layout->addItem(fileMenu);
}