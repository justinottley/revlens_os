

#include <QtWidgets/QApplication>
// #include <QWidget>
// #include <QMainWindow>


#include "RlpCore/GUI/View.h"

#include "RlpCore/GUI/Window.h"

#ifdef SCAFFOLD_WASM

#include <QtCore/QtPlugin>
Q_IMPORT_PLUGIN(QWasmIntegrationPlugin)
Q_IMPORT_PLUGIN(QGifPlugin)
Q_IMPORT_PLUGIN(QICNSPlugin)
Q_IMPORT_PLUGIN(QICOPlugin)
Q_IMPORT_PLUGIN(QJpegPlugin)
Q_IMPORT_PLUGIN(QTgaPlugin)
Q_IMPORT_PLUGIN(QTiffPlugin)
Q_IMPORT_PLUGIN(QWbmpPlugin)
Q_IMPORT_PLUGIN(QWebpPlugin)

#endif


int main( int argc, char **argv )
{
    Q_INIT_RESOURCE(core_GUI_resources);
    
    QApplication qapp(argc, argv);

    GUI_Window v;
    v.resize(640, 400);

    // v.view()->show();

    
    // GUI_View v;
    // v.resize(640, 400);
    // v.move(20, 120);
    
    
    GUI_PanePtr rp = v.view()->rootPane();
    
    rp->setOrientation(O_VERTICAL);
    rp->split();

    rp->pane(0)->setOrientation(O_HORIZONTAL);
    
    rp->pane(0)->split();
    rp->pane(0)->split();

    rp->pane(0)->pane(0)->addTab("Media");

    rp->pane(0)->pane(1)->addTab("Viewer");
    



    //rp->pane(1)->setName("Outliner");
    //rp->pane(1)->setHeaderVisible(true);
    
    // rp->splitter(0)->setPos(0.2);
    

    
    
    // rp->pane(1)->addTab("Chat");
    // rp->pane(1)->addTab("Files");
    
    /*
    qInfo() << "Files tab: " << rp->pane(1)->tab(1)->height();
    
    GUI_Pane* tp = rp->pane(1)->tab(1)->pane();
    
    tp->setOrientation(
        GUI_Pane::O_HORIZONTAL
    );
    
    tp->split();
    tp->split();
    
    */
    
    // tp->splitter(0)->setPos(0.7);
    
    
    // tp->pane(1)->addTab("Images");
    
    
    // ->setActiveTab(0);
    
    // v.rootPane()->split();
    
    /*
    
    
    v.rootPane()->splitter(0)->setPos(0.2);
    
    v.rootPane()->pane(1)->setOrientation(O_VERTICAL);
    
    v.rootPane()->pane(1)->split();
    
    
    */
    
    
    // v.rootPane()->split();
    
    // v.rootPane()->splitter(1)->setPos(0.8);
    // v.rootPane()->pane(0)->setScaleFactor(0.2);
    // v.rootPane()->pane(1)->setScaleFactor(0.8);
    
    
    
    v.show();
    
    
    return qapp.exec();
}