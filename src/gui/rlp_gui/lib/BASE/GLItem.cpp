
#include "RlpGui/BASE/GLItem.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/BASE/FrameBase.h"

#include <QtQuick/QQuickWindow>
#include <QtGui/QOpenGLFunctions>

RLP_SETUP_LOGGER(gui, GUI, GLItem)

GUI_GLItem::GUI_GLItem(GUI_ItemBase* parent):
    GUI_ItemBase(parent)
{
    QQuickWindow* win = scene()->view();
    connect(win, &QQuickWindow::beforeSynchronizing, this, &GUI_GLItem::sync, Qt::DirectConnection);
    connect(win, &QQuickWindow::beforeRendering, this, &GUI_GLItem::initGL, Qt::DirectConnection);
    connect(win, &QQuickWindow::beforeRenderPassRecording, this, &GUI_GLItem::paintGL, Qt::DirectConnection);
}


void
GUI_GLItem::onParentVisibilityChanged(QString name, bool isVisible)
{
    // NOTE: stub; working solution in prod.GUI_ViewerPane that checks
    // current tab against tab metadata for this item
    //
    RLP_LOG_DEBUG(name << isVisible)

    if (isVisible)
    {
        qobject_cast<GUI_ItemBase*>(parentItem())->setPaintBackground(false);
    }
}


void
GUI_GLItem::sync()
{
    // RLP_LOG_DEBUG("")
}


void
GUI_GLItem::initGL()
{
    // NOTE: should return if not visible
    RLP_LOG_DEBUG(isVisible())
}


void
GUI_GLItem::paintGL()
{
    // NOTE: should return if not visible
    RLP_LOG_DEBUG(isVisible())

    scene()->view()->beginExternalCommands();

    glEnable(GL_SCISSOR_TEST);

    glScissor(0, 0, 100, 100);
    glViewport(0, 0, 100, 100);

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glClearColor(0.87, 0.07, 0.17, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene()->view()->endExternalCommands();
}


void
GUI_GLItem::paint(GUI_Painter* painter)
{
    RLP_LOG_DEBUG("")

    //painter->drawText(20, 20, "Hello World");
}