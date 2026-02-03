
#include <QtCore/QUrl>

#include <QtWidgets/QApplication>

#include <QtWebKitWidgets/QWebView>
#include <QtWebKitWidgets/QGraphicsWebView>

#include "RlpCore/LOG/Logging.h"
#include "RlpCore/UTIL/StacktraceHandler.h"

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>

#include "RlpGui/PANE/Window.h"
#include "RlpGui/PANE/ViewGV.h"

#include "RlpGui/WIDGET/WebViewItem.h"


int
main(int argc, char** argv)
{
    UTIL_StacktraceHandler::installSignalHandler();

    CoreLogLogging::init();

    QApplication qapp(argc, argv);
    // GUI_TestApplication qapp(argc, argv);

    QGraphicsWebView* webView = new QGraphicsWebView();
    // webView->setInputMethodHints(0);
    
    
    QGraphicsView* view = new QGraphicsView();
    QGraphicsScene* scene = new QGraphicsScene();
    view->setScene(scene);
    view->setGeometry(QRect(0,0,500,500));

    scene->addItem(webView);

    view->show();
    
    

    /*
    GUI_Window w;
    GUI_View* v = new GUI_View();

    v->setInputMethodHints(0);
    w.setView(v);

    w.resize(800, 700);
    w.show();

    // webView->page()->settings()->setAttribute(QWebSettings::WebGLEnabled, true);
    v->scene()->addItem(webView);
    */

    
    webView->load(QUrl("https://google.ca"));

    

    /*
    QWebView* wv = new QWebView();
    wv->load(QUrl("https://google.ca"));
    wv->show();
    */

    // qInfo() << "Hello World";

    return qapp.exec();
}