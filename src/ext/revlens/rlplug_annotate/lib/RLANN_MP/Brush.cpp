
#include "RlpExtrevlens/RLANN_MP/Brush.h"
#include "RlpExtrevlens/RLANN_MP/Surface.h"
#include "RlpExtrevlens/RLANN_MP/Handler.h"

#include "RlpRevlens/DISP/GLView.h"

RLP_SETUP_EXT_LOGGER(RLANN_MP, Brush)

RLANN_MP_Brush::RLANN_MP_Brush(QString name):
    RLANN_DS_BrushBase(name),
    _doAntialiasing(true),
    _penCol(200, 20, 20),
    _penWidth(1)
{
    initBrush();

    // Set Default color to black
    setColor(QColor(0, 0, 0));
}


void
RLANN_MP_Brush::activate(DISP_GLView* view, RLANN_DS_DrawControllerBase* annCntrl)
{
    RLANN_DS_BrushBase::activate(view, annCntrl);

    // RLP_LOG_DEBUG("")
    // view->setCursor(Qt::CrossCursor);
}


void
RLANN_MP_Brush::initBrush()
{
    RLP_LOG_DEBUG("")

    _brush = mypaint_brush_new();
    mypaint_brush_from_defaults(_brush);

    setValue(MYPAINT_BRUSH_SETTING_COLOR_H, 0);
    setValue(MYPAINT_BRUSH_SETTING_COLOR_S, 0);
    setValue(MYPAINT_BRUSH_SETTING_COLOR_V, 0);
    setValue(MYPAINT_BRUSH_SETTING_SNAP_TO_PIXEL, 0.0);
    setValue(MYPAINT_BRUSH_SETTING_ANTI_ALIASING, 1.0);
    setValue(MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC, 2.0);
    setValue(MYPAINT_BRUSH_SETTING_DIRECTION_FILTER, 10.0);
    setValue(MYPAINT_BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS, 4.0);
//  
}


void
RLANN_MP_Brush::setColor(QColor ncolor)
{
    float h = ncolor.hue()/360.0;
    float s = ncolor.saturation()/255.0;
    float v = ncolor.value()/255.0;

    // Opacity is not handled here as it is defined by the brush settings.
    // If you wish to force opacity, use MPHandler::setBrushValue()
    //
//    float opacity = m_color.alpha()/255.0;
//    mypaint_brush_set_base_value(brush, MYPAINT_BRUSH_SETTING_OPAQUE, opacity);

    // RLP_LOG_DEBUG(ncolor << "hsv:" << h << s << v)

    setValue(MYPAINT_BRUSH_SETTING_COLOR_H, h);
    setValue(MYPAINT_BRUSH_SETTING_COLOR_S, s);
    setValue(MYPAINT_BRUSH_SETTING_COLOR_V, v);
}


float
RLANN_MP_Brush::getValue(MyPaintBrushSetting setting)
{
    return mypaint_brush_get_base_value(_brush, setting);
}


void RLANN_MP_Brush::setValue(MyPaintBrushSetting setting, float value)
{
    mypaint_brush_set_base_value(_brush, setting, value);
}


QVariantMap
RLANN_MP_Brush::settings()
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
RLANN_MP_Brush::setupFrame(RLANN_DS_AnnotationPtr ann)
{
    _currUuid = ann->uuid();
    RLANN_MP_Handler::instance()->initSurface(ann);
}


void
RLANN_MP_Brush::updateSettings(QVariantMap settings)
{
    if (settings.contains("brush_name") && settings.contains("brush_category"))
    {
        QString brushName = settings.value("brush_name").toString();
        QString brushCat = settings.value("brush_category").toString();
        setBrushByName(brushCat, brushName);
    }

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
        setValue(MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC, std::log(_penWidth));

        RLP_LOG_DEBUG("Set width:" << _penWidth)
    }

    setColor(_penCol);
    // RLP_LOG_DEBUG("pen width: " << _penWidth);
}


void
RLANN_MP_Brush::loadBrush(QByteArray brushData)
{
    RLP_LOG_DEBUG("")

    mypaint_brush_from_defaults(_brush);

    if (!mypaint_brush_from_string(_brush, brushData.constData()))
    {
        // Not able to load the selected brush. Let's execute some backup code...
        qDebug("Trouble when reading the selected brush !");
    }

    RLP_LOG_DEBUG("Incoming brush width:" << getValue(MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC))

    setColor(_penCol);
    setValue(MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC, std::log(_penWidth));
}


void
RLANN_MP_Brush::setBrushByPath(QString brushPath)
{
    RLP_LOG_DEBUG(brushPath)

    QFile f(brushPath);
    f.open(QIODevice::ReadOnly);

    QByteArray data = f.readAll();

    f.close();

    loadBrush(data);
}


void
RLANN_MP_Brush::setBrushByName(QString category, QString name)
{
    RLP_LOG_DEBUG(category << name)

    QString brushPath = ":brushes/%1/%2.myb";
    brushPath = brushPath.arg(category).arg(name);

    setBrushByPath(brushPath);
}


void
RLANN_MP_Brush::onBrushRequested(QVariantMap brushInfo)
{
    RLP_LOG_DEBUG(brushInfo);

    QString brushCat = brushInfo.value("category").toString();
    QString brushName = brushInfo.value("name").toString();

    setBrushByName(brushCat, brushName);
}


void
RLANN_MP_Brush::strokeBegin(QPointF& point, QVariantMap mdata)
{
    RLP_LOG_DEBUG("");

    _lastPoint = point;


    mypaint_brush_reset(_brush);
    mypaint_brush_new_stroke(_brush);

    if (mdata.contains("settings"))
    {
        updateSettings(mdata.value("settings").toMap());
    }

}


void
RLANN_MP_Brush::strokeTo(RLANN_DS_AnnotationPtr ann, const QPointF& endPoint, QVariantMap mdata)
{
    // RLP_LOG_DEBUG(mdata)
    // RLP_LOG_DEBUG(endPoint);
    
    // if (_doAntialiasing)
    // {
    //     painter->setRenderHint(QPainter::Antialiasing, true);
    // }

    float x = endPoint.x();
    float y = endPoint.y();
    float xtilt = 0.0;
    float ytilt = 0.0;
    float pressure = 1.0;

    if (mdata.contains("pressure"))
    {
        pressure = mdata.value("pressure").toFloat();
        pressure *= 2.0; //  1.4;
    }

    if (mdata.contains("xtilt"))
    {
        xtilt = mdata.value("xtilt").toFloat();
    }

    if (mdata.contains("ytilt"))
    {
        ytilt = mdata.value("ytilt").toFloat();
    }

    MyPaintSurface* mpsurf = (MyPaintSurface*)RLANN_MP_Handler::instance()->getSurface(_currUuid);

    float dtime = 1.0/10;

    mypaint_surface_begin_atomic(mpsurf);
    mypaint_brush_stroke_to(_brush, mpsurf, x, y, pressure, xtilt, ytilt, dtime);
    MyPaintRectangle roi;
    mypaint_surface_end_atomic(mpsurf, &roi);

    _lastPoint = endPoint;
}


void
RLANN_MP_Brush::paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata)
{
    if (_view != nullptr)
    {
        QPointF ccpos = _view->mapFromGlobal(QCursor::pos());

        painter->save();
        painter->setPen(_penCol);
        painter->setBrush(QBrush(Qt::transparent));

        painter->drawCircle(
            ccpos.x(),
            ccpos.y(),
            _penWidth
        );

        painter->restore();
    }
}