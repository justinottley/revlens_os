//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#include <iostream>
#include <string>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

// #include "RlpCore/GUI/View.h"
// #include "RlpCore/GUI/Window.h"

#include "RlpCore/PY/Interp.h"

#ifdef SCAFFOLD_WASM_TARGET

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


int
main(int argc, char** argv)
{
    setenv("PYTHONHOME", "/", 1);

    QApplication app(argc, argv);

    UTIL_Py::init(PythonQt::IgnoreSiteModule | PythonQt::RedirectStdOut)

    // PythonQt::init(
    // PythonQt_QtAll::init();


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

    
}