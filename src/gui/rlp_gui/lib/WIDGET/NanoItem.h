
#ifndef GUI_NANOITEM_H
#define GUI_NANOITEM_H

#include "RlpGui/WIDGET/Global.h"

#include <qnanopainter.h>
#include <qnanoquickitem.h>
#include <qnanoquickitempainter.h>

class NanoItemPainter : public QNanoQuickItemPainter
{

public:
    NanoItemPainter()
    {
    }

    void paint(QNanoPainter *p)
    {
        float size = std::min(width(), height());
        QPointF center(width()/2, height()/2);
        // Hello text
        p->setTextAlign(QNanoPainter::ALIGN_CENTER);
        p->setTextBaseline(QNanoPainter::BASELINE_MIDDLE);
        QNanoFont font1(QNanoFont::DEFAULT_FONT_BOLD_ITALIC);
        font1.setPixelSize(size*0.08);
        p->setFont(font1);
        p->setFillStyle("#B0D040");
        p->fillText("HELLO", center.x(), center.y() - size*0.18);
    }
};

// HelloItem provides the API towards QML
class GUI_NanoItem : public QNanoQuickItem
{
    Q_OBJECT

public:
    GUI_NanoItem(QQuickItem *parent = nullptr)
        :  QNanoQuickItem(parent)
    {
    }

    // Reimplement
    QNanoQuickItemPainter *createItemPainter() const
    {
        // Create painter for this item
        return new NanoItemPainter();
    }

public slots:

    static GUI_NanoItem* new_GUI_NanoItem(QQuickItem* parent)
    {
        return new GUI_NanoItem(parent);
    }
};


#endif
