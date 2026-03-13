

#include "RlpGui/GRID/FormatterBase.h"
#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/Cell.h"
#include "RlpGui/GRID/View.h"

#include "RlpGui/BASE/FrameOverlay.h"
#include "RlpGui/WIDGET/Label.h"

#include "RlpCore/UTIL/Convert.h"
#include "RlpCore/UTIL/Text.h"


RLP_SETUP_LOGGER(gui, GUI_GRID, FormatterBase)

GUI_GRID_FormatterBase::GUI_GRID_FormatterBase()
{
    _editIcon = new GUI_SvgIcon(":misc/edit.svg", nullptr, 20);
    _editIcon->setBgColour(QColor(200, 200, 200));
    _editIcon->setFgColour(QColor(240, 140, 140));

    _copyIcon = new GUI_SvgIcon(":feather/copy.svg", nullptr, 20);
    _copyIcon->setBgColour(QColor(200, 200, 200));
    _copyIcon->setFgColour(QColor(240, 140, 140));

}


void
GUI_GRID_FormatterBase::paintHeaderCell(GUI_GRID_Cell* cell, GUI_Painter* painter)
{
    QString val = cell->colType()->displayName();

    painter->setPen(Qt::black);
    painter->setBrush(QBrush(GUI_Style::BgLoMidGrey));
    painter->drawRect(cell->boundingRect());

    painter->setPen(Qt::white);
    painter->drawText(
        5,
        15,
        val
    );
}



void
GUI_GRID_FormatterBase::paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter)
{
}


GUI_ItemBase*
GUI_GRID_FormatterBase::makeDragItem(GUI_GRID_Cell* cell)
{
    // RLP_LOG_DEBUG("")

    GUI_GRID_View* view = cell->row()->view();
    QVariantList selValues = view->fullSelectionValues();

    QString val = getValue(cell);
    if (selValues.size() > 1)
    {
        val = QString("%1 items").arg(selValues.size());
    }

    QVariantMap mval = cell->row()->mimeData().toMap();
    mval.insert("selection", selValues);

    // LOG_Logging::pprint(mval);

    int tw = UTIL_Text::getWidth(val);

    GUI_FrameOverlay* di = new GUI_FrameOverlay(tw + 10, 30, nullptr);
    di->setColour(QColor(20, 20, 20)); // GUI_Style::BgDarkGrey);
    di->setOpacity(0.9);

    GUI_Label* l = new GUI_Label(di, val);

    QString mvalStr = UTIL_Convert::serializeToJson(mval);
    di->setMetadata("mimeData", QVariant(mvalStr).toByteArray());

    return di;

}