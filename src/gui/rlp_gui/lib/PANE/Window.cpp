
#include "RlpGui/PANE/Window.h"

RLP_SETUP_LOGGER(gui, GUI, Window)

GUI_Window::GUI_Window():
    QOpenGLWindow()
{
    setView(new GUI_View());
}


void
GUI_Window::setView(GUI_View* view)
{
    RLP_LOG_DEBUG(view << view->viewType())

     _view = view;

    //#ifdef SCAFFOLD_WASM

    _view->setWindow(this);
    
    //#endif

}


void
GUI_Window::initializeGL()
{
    RLP_LOG_DEBUG("")

    _view->scene()->initializeGL();
}


void
GUI_Window::paintGL()
{
    // RLP_LOG_DEBUG("")

    QPainter painter;

    painter.begin(this);

    _view->paintInternal(&painter);

    painter.end();
}


void
GUI_Window::resizeEvent(QResizeEvent* event)
{
    RLP_LOG_DEBUG("")
    
    QOpenGLWindow::resizeEvent(event);    
    _view->resizeEvent(event);
}


bool
GUI_Window::event(QEvent *ev)
{
    // RLP_LOG_DEBUG("")
    // qInfo() << "Window: " << ev;

    QOpenGLWindow::event(ev);
    return _view->event(ev);

    //return result;

}