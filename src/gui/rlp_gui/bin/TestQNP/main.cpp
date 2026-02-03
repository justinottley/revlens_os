
#include "RlpCore/LOG/Logging.h"
#include "RlpCore/PY/Interp.h"

#include "RlpCore/UTIL/StacktraceHandler.h"

#include "RlpGui/PANE/View.h"

#include "qnanowindow.h"
#include "qnanoquickitem.h"
#include "qnanoquickitempainter.h"

class HelloWindow : public QNanoWindow
{

public:
    HelloWindow()
    {
        setFillColor("#ffffff");
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




int main_window(int argc, char* argv[])
{
    UTIL_StacktraceHandler::installSignalHandler();

    LOG_Logging::init();

    // load res from libqnanopainter
    Q_INIT_RESOURCE(libqnanopainterdata);
    //Q_INIT_RESOURCE(gui_QTGUI_resources);

    QApplication qapp(argc, argv);

    HelloWindow window;
    window.resize(400, 400);
    window.show();

    return qapp.exec();

}



// HelloItemPainter contains the painting code
class HelloItemPainter : public QNanoQuickItemPainter
{

public:
    HelloItemPainter()
    {
    }

    void paint(QNanoPainter *p)
    {
        // Painting code is shared as it's identical in all hello* examples
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
class HelloItem : public QNanoQuickItem
{

public:
    HelloItem(QQuickItem *parent = nullptr)
        :  QNanoQuickItem(parent)
    {
    }

    // Reimplement
    QNanoQuickItemPainter *createItemPainter() const
    {
        // Create painter for this item
        return new HelloItemPainter();
    }
};


int main_item(int argc, char* argv[])
{
    UTIL_StacktraceHandler::installSignalHandler();

    LOG_Logging::init();

    // load res from libqnanopainter
    Q_INIT_RESOURCE(libqnanopainterdata);
    //Q_INIT_RESOURCE(gui_QTGUI_resources);

    QApplication qapp(argc, argv);

    PY_Interp::init();

    GUI_View* view = new GUI_View();
    view->resize(800, 600);

    GUI_Pane* rp = view->rootPane();

    HelloItem* hi = new HelloItem(rp->body());
    hi->setWidth(600);
    hi->setHeight(600);

    view->show();

    return qapp.exec();
}

int main(int argc, char* argv[])
{
    return main_item(argc, argv);
}