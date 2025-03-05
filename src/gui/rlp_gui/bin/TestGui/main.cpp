

#include "RlpCore/LOG/Logging.h"
#include "RlpCore/PY/Interp.h"
#include "RlpGui/PANE/View.h"
#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/FrameBase.h"
#include "RlpGui/WIDGET/Label.h"
#include "RlpGui/BASE/GLItem.h"

// #include "RlpCore/UTIL/StacktraceHandler.h"

// #include <QtWidgets/QLabel>
// #include <QtWidgets/QVBoxLayout>
// #include <QtOpenGLWidgets/QOpenGLWidget>

#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtQuick/QQuickWindow>

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

};



int main(int argc, char** argv)
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication qapp(argc, argv);

    LOG_Logging::init();
    GUI_Style::init();
    PY_Interp::init();

    Q_INIT_RESOURCE(libqnanopainterdata);

    // QQuickWindow* w = new QQuickWindow();
    // w->show();

    GUI_View* v = new GUI_View();
    v->resize(1800, 600);
    v->show();

    qInfo() << "Second";

    GUI_View* v2 = new GUI_View();
    v2->resize(800, 600);
    // v2->rootPane()->setOrientation(O_VERTICAL);
    // v2->rootPane()->split();
    // v2->show();

    GUI_GLItem* i = new GUI_GLItem(v2->rootPane()->body());
    i->setWidth(800);
    i->setHeight(600);

    v2->show();

    // GUI_NanoItem* i = new GUI_NanoItem(v2->rootPane()->body());

    // GUI_Label* l = new GUI_Label(v2->rootPane()->body(), "Hello World");

    // v2->rootPane()->update();
    /*
    QWidget* w = new QWidget();
    QLabel* l = new QLabel("Hello World", w);
    QHBoxLayout* lay = new QHBoxLayout();
    lay->addWidget(l);
    w->setLayout(lay);
    w->show();
    */


    /*
    QOpenGLWidget* w = new QOpenGLWidget();
    w->setWindowState(Qt::WindowFullScreen);

    QVBoxLayout* lay = new QVBoxLayout();
    QLabel* l = new QLabel("Test");
    lay->addWidget(l);

    w->setLayout(lay);

    //w->resize(800, 600);
    w->showFullScreen();
    */

    return qapp.exec();

}

