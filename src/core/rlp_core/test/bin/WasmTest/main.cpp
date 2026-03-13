//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#include <iostream>
#include <string>

#include <QtCore/QDebug>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>


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


// #include <QtCore/QThread>
// #include <QtCore/QThreadPool>
// #include <QtCore/QRunnable>

// #include "RlpCore/DS/Test.h"

// #include "RlpCore/GUI/View.h"
// #include "RlpCore/GUI/Window.h"

// #include <QtWebEngine/QtWebEngine>

int
main(int argc, char** argv)
{
    // QtWebEngine::initialize();

    // NEEDED FOR WASM MULTITHREADING
    //
    QApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);
    QApplication::setAttribute( Qt::AA_ShareOpenGLContexts );

    QApplication app(argc, argv);

    

    
    // QThreadPool::globalInstance()->setMaxThreadCount(4);

    qInfo() << "Testing123";

    // TestWidget t;
    // t.resize(640, 480);
    // t.show();

    /*
    GUI_Window v;
    v.resize(640, 400);

    GUI_PanePtr rp = v.view()->rootPane();
    
    rp->setOrientation(O_VERTICAL);
    rp->split();

    rp->pane(0)->setOrientation(O_HORIZONTAL);
    
    rp->pane(0)->split();
    rp->pane(0)->split();

    rp->pane(0)->pane(0)->addTab("Media");

    rp->pane(0)->pane(1)->addTab("Viewer");

    
    v.show();
    */


    // QThreadPool::globalInstance()->start(new DS_TestRunnable());


    /*
    QWidget* w = new QWidget(); // indow();
    
    QHBoxLayout* l = new QHBoxLayout();
    QLabel* lb = new QLabel("Hello world");

    l->addWidget(lb);

    w->setLayout(l);
    // w->centralWidget()->setLayout(l);
    
    w->resize(400, 300);
    
    w->show();
    */

    /*
    GUI_View v;
    v.resize(640, 400);
    v.move(20, 120);
    
    
    
    */


    //rp->pane(1)->setName("Outliner");
    
    return app.exec();

    // return 0;
}