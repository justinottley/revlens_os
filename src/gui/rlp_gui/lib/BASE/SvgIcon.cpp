
#include "RlpGui/BASE/SvgIcon.h"
#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/Scene.h"

#include "RlpCore/UTIL/AppGlobals.h"

#include <QtXml/QDomElement>

RLP_SETUP_LOGGER(gui, GUI, SvgIcon)


void SetAttrRecur(QDomElement &elem, QString strtagname, QString strattr,  QString strattrval)
{
    // if it has the tagname then overwritte desired attribute

    // if (elem.hasAttribute("style"))
    // {
    //     QString val = elem.attribute("style");
    //     QString strokeVal = "stroke:";
    //     strokeVal += strattrval;

    //     val = val.replace("stroke:#000000", strokeVal);
    //     // qInfo() << "SetAttrRecur(): setting style:"<< val;
    //     elem.setAttribute("style", val);

    // }

    if (elem.tagName().compare(strtagname) == 0)
    {
        elem.setAttribute(strattr, strattrval);
    }

    // loop all children
    for (int i = 0; i < elem.childNodes().count(); i++)
    {
        if (!elem.childNodes().at(i).isElement())
        {
            continue;
        }
        QDomElement docElem = elem.childNodes().at(i).toElement(); //<-- make const "variable"
        SetAttrRecur(docElem, strtagname, strattr, strattrval);
    }
}


QImage ChangeSVGColor(QString iconPath, int size, QString color, bool changeColour)
{
    // from https://stackoverflow.com/questions/15123544/change-the-color-of-an-svg-in-qt

    // open svg resource load contents to qbytearray
    QFile file(iconPath);
    if(!file.open(QIODevice::ReadOnly))
        return {};
    QByteArray baData = file.readAll();
    // load svg contents to xml document and edit contents
    QDomDocument doc;
    doc.setContent(baData);
    // recurivelly change color
    QDomElement docElem = doc.documentElement(); //<-- make const "variable"

    // NOTE: HARDCODED HEURISTIC

    if (changeColour)
    {
        if (iconPath.contains("feather"))
        {
            SetAttrRecur(docElem, "svg", "stroke", color);

        } else {
            SetAttrRecur(docElem, "path", "fill", color);
            SetAttrRecur(docElem, "polygon", "fill", color);

            // SetAttrRecur(docElem, "path", "stroke", color);
            // SetAttrRecur(docElem, "polyline", "stroke", color);
            // SetAttrRecur(docElem, "line", "stroke", color);
        }
    }
    
    
    
    // create svg renderer with edited contents

    // qInfo() << "ChangeSVGColor():" << QString(doc.toByteArray());

    QSvgRenderer svgRenderer(doc.toByteArray());

    QImage image(QSize(size, size), QImage::Format_ARGB32);

    // filename hash doesn't work for same image with different colors
    // image.setText("id", iconPath);

    // create pixmap target (could be a QImage)
    image.fill(Qt::transparent);

    // create painter to act over pixmap
    QPainter pixPainter(&image);

    // use renderer to render over painter which paints on pixmap
    svgRenderer.render(&pixPainter);

    return image;
}


QString toSvgCol(QColor col)
{
    QString result = QString("rgb(%1,%2,%3)").arg(col.red()).arg(col.green()).arg(col.blue());
    // qInfo() << "toSvgCol()" << col << result;

    return result;
}


GUI_SvgIcon::GUI_SvgIcon(
    QString filename,
    GUI_ItemBase* parent,
    qreal size,
    qreal padding,
    QColor bgCol,
    bool changeColour,
    GUI_ItemBase::PaintMode paintMode
):
    GUI_ItemBase(parent, paintMode),
    _filename(filename),
    _padding(padding),
    _xoff(0),
    _yoff(0),
    _inHover(false),
    _doOutlined(false),
    _outlineBgCol(GUI_Style::BgLightGrey)
{
    setSize(QSizeF(size + padding, size + padding));

    _bgCol = bgCol;
    _fgCol = Qt::white; // fgCol;

    _bgImage = ChangeSVGColor(_filename, size, toSvgCol(_bgCol), changeColour);
    _fgImage = ChangeSVGColor(_filename, size, toSvgCol(_fgCol), changeColour);

    setAcceptHoverEvents(true);
}


QImage
GUI_SvgIcon::loadSvgFromPath(QString iconPath, int size, QColor colour, bool changeColour)
{
    return ChangeSVGColor(iconPath, size, toSvgCol(colour), changeColour);
}


void
GUI_SvgIcon::setBgColour(QColor col)
{
    // RLP_LOG_DEBUG(col)

    _bgCol = col;
    _bgImage = ChangeSVGColor(_filename, width(), toSvgCol(_bgCol), true);
}

void
GUI_SvgIcon::setFgColour(QColor col)
{
    // RLP_LOG_DEBUG(col)

    _fgCol = col;
    _fgImage = ChangeSVGColor(_filename, width(), toSvgCol(_fgCol), true);
}


void
GUI_SvgIcon::setHover(bool hover)
{
    _inHover = hover;
}


void
GUI_SvgIcon::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    emit buttonPressed(_metadata);
}


void
GUI_SvgIcon::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG(_filename)

    _inHover = true;

    emit hoverChanged(true);

    update();
}


void
GUI_SvgIcon::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG(_filename)

    _inHover = false;

    emit hoverChanged(false);

    update();
}



QRectF
GUI_SvgIcon::boundingRect() const
{
    // padding, _padding, width() - _padding, height() - _padding);
    return QRectF(0, 0, width(), height());

}


void
GUI_SvgIcon::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG(painter->getPainterType())

    //painter->setBrush(QColor(200, 0, 0));
    //painter->drawRect(boundingRect());

     if (_doOutlined) {

        painter->setPen(_outlineBgCol); // Qt::black); // GUI_Style::BgLightGrey);
        painter->setBrush(_outlineBgCol);
        painter->drawRect(boundingRect());
    }

    if (_inHover)
    {
        painter->drawImage(_xoff, _yoff, &_fgImage);
    } else 
    {
        painter->setPen(_bgCol);
        painter->drawImage(_xoff, _yoff, &_bgImage);
    }

    // direct SVG render, dont do this
    // _renderer->render(painter, boundingRect());
}