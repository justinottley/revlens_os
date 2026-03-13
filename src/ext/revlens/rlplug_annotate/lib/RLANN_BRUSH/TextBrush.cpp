
#include "RlpExtrevlens/RLANN_BRUSH/TextBrush.h"

#include "RlpGui/BASE/Scene.h"
#include "RlpGui/PANE/View.h"
#include "RlpGui/PANE/PaneFrame.h"

#include "RlpRevlens/DISP/GLView.h"

RLP_SETUP_LOGGER(ext, RlpAnnBrush, Text)
RLP_SETUP_LOGGER(ext, RlpAnnBrush, TextAnnotationItem)
RLP_SETUP_LOGGER(ext, RlpAnnBrush, TextAnnotation)

const int RlpAnnBrush_Text::X_OFFSET = 15;
const int RlpAnnBrush_Text::Y_OFFSET = 20;


RlpAnnBrush_TextAnnotationItem::RlpAnnBrush_TextAnnotationItem(
    DISP_GLView* view,
    QString text,
    QPointF pos,
    QColor col,
    int penWidth,
    float imgRatio
):
    _text(text),
    _pos(pos),
    _col(QColor(col.blue(), col.green(), col.red())),
    _penWidth(penWidth)
{
    RLP_LOG_DEBUG(view)

    _tp = new RlpAnnBrush_TextPreview(view);

    _tp->setWidth(300);
    _tp->setImageRatio(imgRatio);
    _tp->setPenColor(col);
    _tp->setPenWidth(penWidth);
    _tp->setPos(pos.x() - RlpAnnBrush_Text::X_OFFSET, pos.y() - RlpAnnBrush_Text::Y_OFFSET);
    _tp->setHeight(_tp->textSize() + RlpAnnBrush_Text::X_OFFSET);

    connect(
        _tp,
        &RlpAnnBrush_TextPreview::enterPressed,
        this,
        &RlpAnnBrush_TextAnnotationItem::onPreviewEnterPressed
    );

}


void
RlpAnnBrush_TextAnnotationItem::onPreviewEnterPressed(QVariantMap md)
{
    RLP_LOG_DEBUG(md)

    _text = md.value("text.value").toString();
    emit textUpdated();
}


RlpAnnBrush_TextAnnotation::RlpAnnBrush_TextAnnotation(
    RLANN_DS_AnnotationPtr ann
):
    _ann(ann)
{
    _baseImage = ann->getQImage()->copy();
}


void
RlpAnnBrush_TextAnnotation::addTextItem(RlpAnnBrush_TextAnnotationItem* ann)
{
    _annotations.append(ann);
}


void
RlpAnnBrush_TextAnnotation::drawAnnotations(EditMode editMode)
{
    RLP_LOG_DEBUG("")

    QImage img = _baseImage.copy();

    if (editMode == EM_VIEW)
    {
        for (int i=0; i != _annotations.size(); ++i)
        {
            _annotations.at(i)->textPreview()->setVisible(false);
        }

        QPainter painter(&img); // _ann->getQImage());

        for (RlpAnnBrush_TextAnnotationItem* tann : _annotations)
        {
            drawAnnotation(&painter, tann);
        }

        _ann->loadFromImage(img);
        _ann->saveToHistory();

    } else
    {
        _ann->loadFromImage(img);

        for (int i=0; i != _annotations.size(); ++i)
        {
            _annotations.at(i)->textPreview()->setVisible(true);
        }
    }
}


void
RlpAnnBrush_TextAnnotation::drawAnnotation(QPainter* p, RlpAnnBrush_TextAnnotationItem* tann)
{
    RLP_LOG_DEBUG("")

    QString displayStr = _mdata.value("display.uuid").toString();
    QPointF displayPan = _mdata.value("display.pan").toPointF();
    QPointF previewPos = tann->pos();

    QPointF imgPoint = _ann->mapToImage(
        displayStr,
        QPointF(previewPos.x() + 10, (previewPos.y() + tann->textSize())),
        displayPan
    );

    QPen pe(tann->col());
    QFont f = p->font();
    f.setPixelSize(tann->penWidth() * 2);
    p->setFont(f);
    p->setPen(pe);
    p->drawText(imgPoint, tann->text());
}


void
RlpAnnBrush_TextAnnotation::onTextUpdated()
{
    RLP_LOG_DEBUG("")

    drawAnnotations(EM_VIEW);
}


RlpAnnBrush_Text::RlpAnnBrush_Text():
    RLANN_DS_BrushBase("Text"),
    _doAntialiasing(true),
    _penCol(200, 20, 20),
    _penWidth(2),
    _brush(Qt::transparent),
    _tp(nullptr)
{ 
}


RLANN_DS_BrushBase*
RlpAnnBrush_Text::clone()
{
    return new RlpAnnBrush_Text();
}


QVariantMap
RlpAnnBrush_Text::settings()
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
RlpAnnBrush_Text::activate(DISP_GLView* view, RLANN_DS_DrawControllerBase* annCntrl)
{
    RLP_LOG_DEBUG("")
    RLANN_DS_BrushBase::activate(view, annCntrl);

    view->setCursor(Qt::IBeamCursor);
}



void
RlpAnnBrush_Text::updateSettings(QVariantMap settings)
{
    RLP_LOG_DEBUG(settings)

    if (settings.contains("col"))
    {
        QVariantMap col = settings.value("col").toMap();
        _penCol = QColor(
            col.value("r").toInt(),
            col.value("g").toInt(),
            col.value("b").toInt()
        );

    }
    
    if (settings.contains("width"))
    {
        _penWidth = settings.value("width").toInt();
    }
    
    if (settings.contains("mode"))
    {
        if (_ann != nullptr)
        {
            QVariant v = _ann->property("tann");
            if (v.isValid())
            {
                RlpAnnBrush_TextAnnotation* tann = v.value<RlpAnnBrush_TextAnnotation*>();
                tann->drawAnnotations(
                    (RlpAnnBrush_TextAnnotation::EditMode)settings.value("mode").toInt());
            }
        }
    }

    // RLP_LOG_DEBUG("pen width: " << _penWidth);
}


void
RlpAnnBrush_Text::setupFrame(RLANN_DS_AnnotationPtr ann)
{
    RLP_LOG_DEBUG("")

    _ann = ann;

    QVariant v = ann->property("tann");

    RLP_LOG_DEBUG(v)

    if (!v.isValid())
    {
        RLP_LOG_DEBUG("Creating new text annotation")

        RlpAnnBrush_TextAnnotation* tann = new RlpAnnBrush_TextAnnotation(ann);

        QVariant v;
        v.setValue(tann);
        _ann->setProperty("tann", v);
    }
}


void
RlpAnnBrush_Text::strokeBegin(QPointF& point, QVariantMap mdata)
{
    RLP_LOG_DEBUG(point)

    // if (_tp == nullptr)
    // {
    //     _tp = new RlpAnnBrush_TextPreview(_view);
    //     connect(
    //         _tp,
    //         &RlpAnnBrush_TextPreview::enterPressed,
    //         this,
    //         &RlpAnnBrush_Text::onCreateTextAnnotation
    //     );
        
    //     _tp->setVisible(false);
    //     _tp->setWidth(300);
    //     _tp->setHeight(120);
    //     QPointF ppos = _view->mapToGlobal(point);
    // }

    if (mdata.contains("settings"))
    {
        updateSettings(mdata.value("settings").toMap());
    }

    if (_tp == nullptr)
    {
        RLP_LOG_DEBUG("NO CURRENT TEXT PREVIEW")

        QPointF ppos = mdata.value("event.pos").value<QPointF>();

        float imgRatio = 1.0;
        if (mdata.contains("image.ratio"))
        {
            imgRatio = mdata.value("image.ratio").toFloat();
        }

        RlpAnnBrush_TextAnnotationItem* ta = new RlpAnnBrush_TextAnnotationItem(
            _view,
            "",
            ppos,
            _penCol,
            _penWidth,
            imgRatio
        );

        RlpAnnBrush_TextAnnotation* tann = _ann->property("tann").value<RlpAnnBrush_TextAnnotation*>();

        connect(
            ta->textPreview(),
            &RlpAnnBrush_TextPreview::focusChanged,
            this,
            &RlpAnnBrush_Text::onTextAnnotationPreviewFocusChanged
        );

        connect(
            ta,
            &RlpAnnBrush_TextAnnotationItem::textUpdated,
            tann,
            &RlpAnnBrush_TextAnnotation::onTextUpdated
        );

        connect(
            ta,
            &RlpAnnBrush_TextAnnotationItem::textUpdated,
            this,
            &RlpAnnBrush_Text::onTextUpdated
        );

        tann->setMetadata(mdata);
        tann->addTextItem(ta);

        _tp = ta->textPreview();
    }


    // _tp->setImageRatio(imgRatio);
    // _tp->setPenSettings(settings());
    // _tp->setHeight(_tp->textSize() + X_OFFSET);

    // if (!_tp->isVisible())
    // {
    //     _pressPos = mdata.value("event.pos").value<QPointF>();
    //     _tp->setPosition(point);
    //     _tp->setPos(ppos.x() - X_OFFSET, ppos.y() - Y_OFFSET);
    //     _tp->setVisible(true);
    // }

    _pressPosG = mdata.value("event.gpos").value<QPointF>();

    _lastPoint = point;
    _mdata = mdata;

    
}


void
RlpAnnBrush_Text::strokeTo(RLANN_DS_AnnotationPtr ann, const QPointF &endPoint, QVariantMap mdata)
{
    // RLP_LOG_DEBUG(mdata)

    QPainter painter(ann->getQImage());

    if (_tp != nullptr)
    {
        QPointF pposG = mdata.value("event.gpos").value<QPointF>();
        QPointF pdiff = pposG - _pressPosG;

        _tp->setPos(_pressPos.x() + pdiff.x() - X_OFFSET, _pressPos.y() + pdiff.y() - Y_OFFSET);
        _tp->setPosition(endPoint);
    }
    _lastPoint = endPoint;
}


void
RlpAnnBrush_Text::strokeEnd(RLANN_DS_AnnotationPtr ann, QVariantMap mdata)
{
    // RLP_LOG_DEBUG(mdata)

    // if (_tp != nullptr)
    // {
    //     _pressPos = QPointF(_tp->pos().x() + X_OFFSET, _tp->pos().y() + Y_OFFSET);

    // }
}


void
RlpAnnBrush_Text::onTextAnnotationPreviewFocusChanged(RlpAnnBrush_TextPreview* tp, bool focus)
{
    RLP_LOG_DEBUG(focus)

    if (focus)
    {
        _tp = tp;
        _pressPos = QPointF(_tp->pos().x() + X_OFFSET, _tp->pos().y() + Y_OFFSET);
    } else
    {
        _tp = nullptr;
    }
}


void
RlpAnnBrush_Text::onTextUpdated()
{
    RLP_LOG_DEBUG("")

    QVariantMap binfo;
    binfo.insert("mode", RlpAnnBrush_TextAnnotation::EM_VIEW);

    if (_annCntrl != nullptr)
    {
        _annCntrl->emitBrushInfoChanged(binfo);
    }
    else
    {
        RLP_LOG_WARN("ANN CNTRL NOT SET???")
    }
    
}


/*
void
RlpAnnBrush_Text::onCreateTextAnnotation(QVariantMap md)
{
    RLP_LOG_DEBUG(md)

    QString textval = md.value("text.value").toString(); // _tp->text();
    int textSize = md.value("text.size").toInt();
    QPointF previewPos = md.value("preview.pos").toPointF();

    QString displayStr = _mdata.value("display.uuid").toString();
    QPointF displayPan = _mdata.value("display.pan").toPointF();

    QPointF imgPoint = _ann->mapToImage(
        displayStr,
        QPointF(previewPos.x() + 10, (previewPos.y() + textSize)),
        displayPan
    );

    QPen pe(_penCol);
    QPainter p(_ann->getQImage());
    QFont f = p.font();
    f.setPixelSize(_penWidth * 2);
    p.setFont(f);
    p.setPen(pe);
    p.drawText(imgPoint, textval);

    _ann->setProperty("text.value", textval);
    _ann->setProperty("text.pos", imgPoint);
    _ann->setProperty("text.size", textSize);
    _ann->setProperty("preview.pos", previewPos);

    // For timeline display update
    _ann->emitStrokeEnd();
}
*/
