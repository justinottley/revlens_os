//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//
// This is the same as the regular executable, just compiled to a different executable name
// for changing the startup sequence to bach
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

    GUI_Startup s;
    int result = s.start(argc, argv);

    #ifdef _WIN32
    timeEndPeriod(1);
    #endif
}