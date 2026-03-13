//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//
/*
 * This class came about due to current Qt limitations with QOpenGLWidget
 * and WebAssembly.
 * As of Qt 5.14.1 (Apr 16 2020), QOpenGLWidget is not available for WebAssembly,
 * but it appears that QOpenGLWindow works. This class is now being used
 * to attempt the same thing that GUI_View (a subclass of QOpenGLWidget)
 * used to do, but also allow for deployment into WebAssembly.
 * 
 * https://bugreports.qt.io/browse/QTBUG-66944
 * 
 */


#ifndef CORE_GUI_WINDOW_H
#define CORE_GUI_WINDOW_H

#include <QtOpenGL/QOpenGLWindow>

#include "RlpGui/PANE/Global.h"
#include "RlpGui/PANE/View.h"

class GUI_PANE_API GUI_Window : public QOpenGLWindow {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_Window();

    void setView(GUI_View* view);

    void initializeGL();
    void paintGL();

    void resizeEvent(QResizeEvent* event);

    bool event(QEvent *ev);

    GUI_View* view() { return _view; }

private:
    GUI_View* _view;
};

#endif