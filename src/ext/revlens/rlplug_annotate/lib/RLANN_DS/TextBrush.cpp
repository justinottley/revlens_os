
#include "RlpExtrevlens/RLANN_DS/TextBrush.h"

#include "RlpGui/BASE/Scene.h"
#include "RlpGui/PANE/View.h"
#include "RlpGui/PANE/PaneFrame.h"

#include "RlpRevlens/DISP/GLView.h"

RLP_SETUP_EXT_LOGGER(RLANN_DS, TextBrush)

RLANN_DS_TextBrush::RLANN_DS_TextBrush(QString name):
    RLANN_DS_BrushBase(name),
    _doAntialiasing(true),
    _penCol(200, 20, 20),
    _penWidth(2),
    _brush(Qt::transparent),
    _pf(nullptr),
    _te(nullptr)
{ 
}


QVariantMap
RLANN_DS_TextBrush::settings()
{
    QVariantMap result;
    QVariantMap col;
    col.insert("r", _penCol.red());
    col.insert("g", _penCol.green());
    col.insert("b", _penCol.blue());
    result.insert("col", col);
    result.insert("width", _penWidth);
    result.insert("name", _name);
    
    return result;

}


void
RLANN_DS_TextBrush::activate(DISP_GLView* view, RLANN_DS_DrawControllerBase* annCntrl)
{
    RLP_LOG_DEBUG("")

    view->setCursor(Qt::IBeamCursor);

    _view = view;
}


void
RLANN_DS_TextBrush::setupFrame(RLANN_DS_AnnotationPtr ann)
{
    RLP_LOG_DEBUG("")

    _ann = ann;
}


void
RLANN_DS_TextBrush::updateSettings(QVariantMap settings)
{
    if (settings.contains("col")) {
        QVariantMap col = settings.value("col").toMap();
        _penCol = QColor(
            col.value("b").toInt(),
            col.value("g").toInt(),
            col.value("r").toInt()
        );

    }
    
    if (settings.contains("width")) {
        _penWidth = settings.value("width").toInt();
    }
    
    // RLP_LOG_DEBUG("pen width: " << _penWidth);
}


void
RLANN_DS_TextBrush::strokeBegin(QPointF& point, QVariantMap settings)
{
    RLP_LOG_DEBUG("");

    if (_pf == nullptr)
    {
        _pf = qobject_cast<GUI_View*>(_view->scene()->view())->createFloatingPane(380, 200, false);
        _pf->setText("New Text Annotation");
        _pf->setTextBold(true);
        _pf->setCloseButtonVisible(false);
        _te = new GUI_TextEdit(_pf, 300, 120);
        _te->setPos(40, 30);

        GUI_IconButton* createButton = new GUI_IconButton("", _pf, 20);
        createButton->setPos(40, 140);
        createButton->setText("Create");
        createButton->setOutlined(true);

        GUI_IconButton* cancelButton = new GUI_IconButton("", _pf, 20);
        cancelButton->setPos(160, 140);
        cancelButton->setText("Cancel");
        cancelButton->setOutlined(true);

        connect(
            createButton,
            &GUI_IconButton::buttonPressed,

            this,
            &RLANN_DS_TextBrush::onCreateTextAnnotation
        );

        connect(
            cancelButton,
            &GUI_IconButton::buttonPressed,
            this,
            &RLANN_DS_TextBrush::onCloseTextPaneRequested
        );
    }

    _te->setText("");

    _pf->setVisible(true);
    QPointF ppos = _view->mapToGlobal(point);
    // _pf->setPos(ppos.x(), ppos.y());
    _lastPoint = point;
    updateSettings(settings);
}


void
RLANN_DS_TextBrush::strokeTo(QPainter* painter, const QPointF &endPoint)
{
    // RLP_LOG_DEBUG(_penWidth << " " << _penCol.red() << " " << _penCol.green() << " " << _penCol.blue());

    _lastPoint = endPoint;
}


void
RLANN_DS_TextBrush::onCreateTextAnnotation(QVariantMap md)
{
    RLP_LOG_DEBUG(md)

    _pf->setVisible(false);

    QString textval = _te->text();

    QPen pe(_penCol);
    QPainter p(_ann->getQImage());
    QFont f = p.font();
    f.setPixelSize(_penWidth * 2);
    p.setFont(f);
    p.setPen(pe);
    p.drawText(_lastPoint, textval);

    // For timeline display update
    _ann->emitStrokeEnd();
}


void
RLANN_DS_TextBrush::onCloseTextPaneRequested()
{
    _pf->setVisible(false);
}