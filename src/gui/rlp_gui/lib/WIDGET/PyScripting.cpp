
#include "RlpCore/PY/Interp.h"
#include "RlpGui/WIDGET/PyScripting.h"


RLP_SETUP_LOGGER(core, GUI, PyScripting)

GUI_PyScripting::GUI_PyScripting(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _textWidgetPadding(30)
{
    RLP_LOG_DEBUG(parent->width() << parent->height())
    setWidth(parent->width() - (_textWidgetPadding * 2)); // pwidth);
    setHeight(parent->height());

    _layout = new GUI_VLayout(this);

    _textEdit = new GUI_TextEdit(_layout, width(), 200);

    GUI_HLayout* buttonLayout = new GUI_HLayout(this);
    buttonLayout->setSpacing(20);

    _runButton = new GUI_IconButton("", buttonLayout);
    _runButton->setText("Run");
    _runButton->setOutlined(true);
    
    _clearButton = new GUI_IconButton("", buttonLayout);
    _clearButton->setText("Clear");
    _clearButton->setOutlined(true);

    buttonLayout->addItem(_runButton);
    buttonLayout->addItem(_clearButton);

    _layout->addItem(_textEdit, _textWidgetPadding);
    _layout->addSpacer(20);
    _layout->addItem(buttonLayout, _textWidgetPadding);

    connect(
        parent,
        SIGNAL(sizeChanged(qreal, qreal)),
        this,
        SLOT(onSizeChanged(qreal, qreal))
    );

    connect(
        _runButton,
        &GUI_IconButton::buttonPressed,
        this,
        &GUI_PyScripting::runRequested
    );

    connect(
        _clearButton,
        &GUI_IconButton::buttonPressed,
        this,
        &GUI_PyScripting::clearRequested
    );
}


QQuickItem*
GUI_PyScripting::create(GUI_ItemBase* parent, QVariantMap metadata)
{
    return new GUI_PyScripting(parent);
}


QRectF
GUI_PyScripting::boundingRect() const
{
    return QRectF(0, 0, 800, 600);
}


void
GUI_PyScripting::onSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(width << " " << height)

    //_textEdit->setWidth(width - (_textWidgetPadding * 2));

}


void
GUI_PyScripting::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(width << height)

    qreal nwidth = width - (_textWidgetPadding * 2);
    _textEdit->setWidth(nwidth);
    _textEdit->setHeight(height - 150);

    setWidth(nwidth);
    setHeight(height);
}


void
GUI_PyScripting::execText(QString text)
{
    RLP_LOG_DEBUG(text)
    
    PY_Interp::eval(text);
}


void
GUI_PyScripting::runRequested()
{
    RLP_LOG_DEBUG("")

    execText(_textEdit->text());
    _textEdit->clear();
}


void
GUI_PyScripting::clearRequested()
{
    RLP_LOG_DEBUG("")

    _textEdit->clear();
}