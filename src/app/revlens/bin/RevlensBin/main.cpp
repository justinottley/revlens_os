//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#include <iostream>
#include <string>

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QApplication>

#include "RlpRevlens/GUI/Startup.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <timeapi.h>
#endif


int
main(int argc, char** argv)
{
    #ifdef _WIN32
    timeBeginPeriod(1);
    #endif

    // NEEDED FOR WASM MULTITHREADING
    //
    QApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication qapp(argc, argv);

    qInfo() << "qapp made";

    // qapp.setAttribute(Qt::AA_DisableHighDpiScaling);

    GUI_Startup s;
    s.start(argc, argv);

    #ifdef _WIN32
    timeEndPeriod(1);
    #endif

    return qapp.exec();
}