
#include "RlpExtrevlens/RLANN_GUI/BrushSizeButton.h"

#include "RlpGui/BASE/Style.h"
#include "RlpGui/MENU/MenuPane.h"

#include "RlpExtrevlens/RLANN_GUI/ColourButton.h"

RLP_SETUP_LOGGER(ext, RLANN_GUI, BrushSizeMenuItem)
RLP_SETUP_LOGGER(ext, RLANN_GUI, BrushSizeButton)

RLANN_GUI_BrushSizeMenuItem::RLANN_GUI_BrushSizeMenuItem(
    GUI_MenuPane* parent):
        GUI_ItemBase(parent),
        _width(300),
        _height(70)
{
    setWidth(_width);
    setHeight(_height);

    GUI_SvgButton* upButton = new GUI_SvgButton(
        ":feather/lightgrey/arrow-up.svg", this, 20);

    upButton->icon()->setBgColour(QColor(200, 200, 200));
    upButton->setPos(33, 5);


    GUI_SvgButton* downButton = new GUI_SvgButton(
        ":feather/lightgrey/arrow-down.svg", this, 20);
    
    downButton->icon()->setBgColour(QColor(200, 200, 200));
    downButton->setPos(33, 30);

    
    connect(
        upButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &RLANN_GUI_BrushSizeMenuItem::onUpPressed
    );

    connect(
        downButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &RLANN_GUI_BrushSizeMenuItem::onDownPressed
    );

    initColourButtons();
}


void
RLANN_GUI_BrushSizeMenuItem::initColourButtons()
{
    QList<QList<int> > colList_1 = { {255, 0, 0}, {255, 165, 0}, {255, 255, 0}, {255, 255, 165} };

    for (int i=0; i != colList_1.size(); ++i) {

        QList<int> ci = colList_1.at(i);
        RLANN_GUI_ColourButton* cb = new RLANN_GUI_ColourButton(
            QColor(ci[0], ci[1], ci[2]), this);
        
        // RLP_LOG_INFO(cb->color())
        cb->setPos(60 + (i * 25), 3);
        connect(
            cb,
            &RLANN_GUI_ColourButton::colourSelected,
            this,
            &RLANN_GUI_BrushSizeMenuItem::onColourSelected
        );
    }

    QList<QList<int> > colList_2 = { {0, 255, 0}, {0, 255, 165}, {0, 255, 255}, {165, 255, 255} };
    
    for (int i=0; i != colList_2.size(); ++i) {

        QList<int> ci = colList_2.at(i);
        RLANN_GUI_ColourButton* cb = new RLANN_GUI_ColourButton(
            QColor(ci[0], ci[1], ci[2]), this);

        // RLP_LOG_INFO(cb->color())

        cb->setPos(60 + (i * 25), 27);
        connect(
            cb,
            &RLANN_GUI_ColourButton::colourSelected,
            this,
            &RLANN_GUI_BrushSizeMenuItem::onColourSelected
        );
    }

    QList<QList<int> > colList_3 = { {255, 165, 255}, {255, 0, 255}, {165, 0, 255}, {0, 0, 255} };
    
    for (int i=0; i != colList_3.size(); ++i) {

        QList<int> ci = colList_3.at(i);
        RLANN_GUI_ColourButton* cb = new RLANN_GUI_ColourButton(
            QColor(ci[0], ci[1], ci[2]), this);
        
        // RLP_LOG_INFO(cb->color())

        cb->setPos(160 + (i * 25), 3);
        connect(
            cb,
            &RLANN_GUI_ColourButton::colourSelected,
            this,
            &RLANN_GUI_BrushSizeMenuItem::onColourSelected
        );
    }

    QList<QList<int> > colList_4 = { {255, 255, 255}, {160, 160, 160}, {90, 90, 90}, {20, 20, 20} };
    
    for (int i=0; i != colList_4.size(); ++i) {

        QList<int> ci = colList_4.at(i);
        RLANN_GUI_ColourButton* cb = new RLANN_GUI_ColourButton(
            QColor(ci[0], ci[1], ci[2]), this);

        // RLP_LOG_INFO(cb->color())

        cb->setPos(160 + (i * 25), 27);
        connect(
            cb,
            &RLANN_GUI_ColourButton::colourSelected,
            this,
            &RLANN_GUI_BrushSizeMenuItem::onColourSelected
        );
    }
}


void
RLANN_GUI_BrushSizeMenuItem::setValue(qreal value)
{
    if (value <= 0)
        return;

    _value = value;

    _text = "%1";
    _text = _text.arg(value);

    QVariantMap val;
    val.insert("val", value);

    emit valueChanged(val);

    update();
}


void
RLANN_GUI_BrushSizeMenuItem::onColourSelected(QColor col)
{
    RLP_LOG_DEBUG(col)
    
    QVariantMap colInfo;
    colInfo.insert("r", col.red());
    colInfo.insert("g", col.green());
    colInfo.insert("b", col.blue());

    emit colourSelected(colInfo);

}


void
RLANN_GUI_BrushSizeMenuItem::onUpPressed(QVariantMap /* metadata */)
{
    // RLP_LOG_DEBUG("")

    setValue(_value + 1);
}


void
RLANN_GUI_BrushSizeMenuItem::onDownPressed(QVariantMap /* metadata */)
{
    // RLP_LOG_DEBUG("")

    setValue(_value - 1);
}


QRectF
RLANN_GUI_BrushSizeMenuItem::boundingRect() const
{
    return QRectF(0, 0, _width - 1, _height - 1);
}


void
RLANN_GUI_BrushSizeMenuItem::paint(GUI_Painter* painter)
{
    // painter->setPen(Qt::black);
    // painter->setBrush(QBrush(GUI_Style::BgMidGrey));
    // painter->drawRect(boundingRect());
    painter->setPen(Qt::white);
    painter->drawText(10, 30, _text);
}


RLANN_GUI_BrushSizeButton::RLANN_GUI_BrushSizeButton(
    GUI_ItemBase* parent):
        GUI_SvgButton(
            ":/feather/lightgrey/circle.svg",
            parent,
            21,
            2
        ),
        _col(QColor(200, 20, 20))
{
    setToolTipText("Brush Size and Colour");

    // Not available with QQuickItem?
    // _icon->setFlag(QQuickItem::ItemStacksBehindParent, false);

    _sizeMenuItem = new RLANN_GUI_BrushSizeMenuItem(_menu);

    _menu->addItem(_sizeMenuItem);
    _menu->setWidth(_sizeMenuItem->width());
    _menu->setHeight(_sizeMenuItem->height());

    connect(
        this,
        &RLANN_GUI_BrushSizeButton::buttonPressed,
        this,
        &RLANN_GUI_BrushSizeButton::onButtonPressed
    );

    connect(
        _sizeMenuItem,
        &RLANN_GUI_BrushSizeMenuItem::colourSelected,
        this,
        &RLANN_GUI_BrushSizeButton::onColourSelected
    );

}


void
RLANN_GUI_BrushSizeButton::setValue(qreal value)
{
    // RLP_LOG_DEBUG(value)

    _sizeMenuItem->setValue(value);
}


void
RLANN_GUI_BrushSizeButton::onColourSelected(QVariantMap colInfo)
{
    // RLP_LOG_DEBUG(colInfo)

    QColor col = QColor(
        colInfo.value("r").toInt(), colInfo.value("g").toInt(), colInfo.value("b").toInt());
    _col = col;
    _menu->setVisible(false);

    update();
}


void
RLANN_GUI_BrushSizeButton::onButtonPressed(QVariantMap /* metadata */)
{
    RLP_LOG_DEBUG("")

    clearToolTip(); // if tooltip is open it occludes menu

    toggleMenu();
}


void
RLANN_GUI_BrushSizeButton::paint(GUI_Painter* painter)
{
    
    painter->setPen(Qt::black);
    painter->setBrush(_col);
    painter->drawRect(boundingRect());

    // if (_drawOutline) {

    //     QPen p(Qt::black);
    //     p.setWidth(1);

    //     painter->setPen(p);
    //     painter->setBrush(GUI_Style::BgMidGrey);
    //     painter->drawRect(boundingRect());
    // }

    // if (inHover()) {
    //     // painter->setBrush(GUI_Style::BgLightGrey);
    //     painter->setPen(Qt::black);
    //     painter->drawRect(boundingRect());
    // }

}


