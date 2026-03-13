
#include <QtCore/QUrl>

#include <QtWidgets/QApplication>

#include <QtWidgets/QOpenGLWidget>

#include <QtWebKitWidgets/QWebView>
#include <QtWebKitWidgets/QGraphicsWebView>

#include "RlpCore/LOG/Logging.h"
#include "RlpCore/UTIL/StacktraceHandler.h"

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>

#include "RlpGui/PANE/Window.h"
#include "RlpGui/PANE/ViewGV.h"
#include "RlpGui/BASE/Scene.h"

#include "RlpGui/WIDGET/WebViewItem.h"


class SceneItem : public QGraphicsItem {
    
public:
    SceneItem(QGraphicsObject* parent)
    {
        setAcceptHoverEvents(true);
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event)
    {
        qInfo() << "Hover Enter";
    }

    QRectF
    boundingRect() const
    {
        return QRectF(0, 0, 100, 100);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0)
    {
        painter->setBrush(QBrush(Qt::red));
        painter->setPen(Qt::red);
        painter->drawRect(boundingRect());
    }
};

int
main(int argc, char** argv)
{
    UTIL_StacktraceHandler::installSignalHandler();

    CoreLogLogging::init();

    QApplication qapp(argc, argv);
    // GUI_TestApplication qapp(argc, argv);

     // QGraphicsWebView* webView = new QGraphicsWebView();
    // webView->setInputMethodHints(0);
    
    
    // QGraphicsView* view = new QGraphicsView();
    // QGraphicsScene* scene = new QGraphicsScene();
    // GUI_Scene* scene = new GUI_Scene();

    
    GUI_View* view = new GUI_View();
    
    // view->scene()->addItem(new SceneItem(nullptr));
    // view->setScene(scene);
    view->setGeometry(QRect(0,0,500,500));

    view->resize(500, 500);
    // scene->addItem(webView);
    GUI_PanePtr rp = view->rootPane();

    rp->setOrientation(O_VERTICAL);

    rp->split();
    rp->splitter(0)->setPos(0.6);

    // QGraphicsWebView* webView = new QGraphicsWebView(rp->pane(1)->body());
    GUI_WebViewItem* webView = new GUI_WebViewItem(rp->pane(1)->body());
    webView->load(QUrl("https://google.ca"));

    view->show();
    
    
    // webView->load(QUrl("https://google.ca"));



    /*
    QWebView* wv = new QWebView();
    wv->load(QUrl("https://google.ca"));
    wv->show();
    */

    // qInfo() << "Hello World";

    return qapp.exec();
}