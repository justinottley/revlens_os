
#include "RlpCore/PY/Interp.h"

#include <QtQuick/QQuickWindow>
#include <QtQuick/QQuickView>
#include <QtGui/QGuiApplication>

#ifdef _WIN32
#include "RlpCore/UTIL/WindowsStacktraceHandler.h"
#else
#include "RlpCore/UTIL/StacktraceHandler.h"
#endif



#include "RlpGui/BASE/Scene.h"
#include "RlpGui/PANE/Pane.h"
#include "RlpGui/PANE/View.h"

#include "RlpGui/MENU/MenuBar.h"

#include "RlpGui/BASE/HLayout.h"

#include "RlpGui/WIDGET/Label.h"
#include "RlpGui/WIDGET/TextEdit.h"
#include "RlpGui/WIDGET/PyScripting.h"
#include "RlpGui/WIDGET/RadioButton.h"
#include "RlpGui/WIDGET/RadioButtonGroup.h"

#include "RlpGui/GRID/View.h"

#include "RlpGui/BASE/GLItem.h"

// -- python binding

#include "RlpCore/QTCORE/QtCoreTypes.h"
#include "RlpGui/QTGUI/QtGuiTypes.h"

#include "RlpCore/CNTRL/pymodule.h"
#include "RlpGui/BASE/pymodule.h"
#include "RlpGui/PANE/pymodule.h"

// disable these for minimum build
#include "RlpGui/MENU/pymodule.h"
#include "RlpGui/GRID/pymodule.h"
#include "RlpGui/WIDGET/pymodule.h"



/*
// #include "RlpGui/TEST/ItemBase.h"

int main_1(int argc, char** argv)
{
    QGuiApplication app(argc, argv);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QQuickWindow w;
    

    // QQuickView view;
    // view.setResizeMode(QQuickView::SizeRootObjectToView);
    // view.setSource(QUrl("qrc:///scenegraph/openglunderqml/main.qml"));
    

    // GUI_QuickItemBase* item = new GUI_QuickItemBase(view.rootObject());
    GUI_QuickItemBase* item = new GUI_QuickItemBase(w.contentItem());

    item->setWidth(200);
    item->setHeight(200);
    item->setParent(w.contentItem());
    item->setParentItem(w.contentItem());


    w.show();

    return QGuiApplication::exec();
}
*/


int main_py(int argc, char** argv)
{
    qInfo() << "Starting up..";

    #ifdef SCAFFOLD_WASM
    setenv("PYTHONHOME", "/", 1);
    setenv("PYTHONPATH", "/wasm_fs_root/pylib:/lib", 1);
    #endif


    PY_Interp::init();

    // PY_Interp::eval("print('hello world');");

    qInfo() << "end";

    return 0;
}


int main_full(int argc, char** argv)
{
    qInfo() << "Starting up...";
  
    #ifdef SCAFFOLD_WASM
    setenv("PYTHONHOME", "/", 1);
    setenv("PYTHONPATH", "/wasm_fs_root/pylib:/lib", 1);
    #endif

    LOG_Logging::init();
    UTIL_StacktraceHandler::installSignalHandler();


    QGuiApplication app(argc, argv);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    Q_INIT_RESOURCE(core_CNTRL_resources);
    Q_INIT_RESOURCE(gui_BASE_resources);


    RlpCoreQTCOREmodule_bootstrap();
    RlpGuiQTGUImodule_bootstrap();

    RlpCoreCNTRLmodule_bootstrap();
    RlpGuiBASEmodule_bootstrap();
    RlpGuiPANEmodule_bootstrap();

    // disable these for minimum build
    //
    RlpGuiMENUmodule_bootstrap();
    RlpGuiGRIDmodule_bootstrap();
    RlpGuiWIDGETmodule_bootstrap();

    PY_Interp::init();


    // need this before VIEW is exported to register parent classes
    PY_Interp::eval("import RlpCoreQTCORE");
    PY_Interp::eval("import RlpGuiQTGUI");

    PY_Interp::eval("import RlpGuiBASEmodule");
    PY_Interp::eval("import RlpGuiPANEmodule");

    GUI_View* v = new GUI_View();

    PY_Interp::bindAppObject("RlpGuiPANEmodule", "VIEW",  v);

    // QString text = argv[1];

    //PY_Interp::evalFile(text);

    /*
    GUI_Scene* scene = new GUI_Scene(v->contentItem());

    v->setScene(scene);

    scene->setParent(v->contentItem());
    scene->setParentItem(v->contentItem());

    GUI_Pane* pane = new GUI_Pane(nullptr);
    pane->setName("ROOT");
    scene->addItem(pane);
    

    pane->initPane();
    */

    GUI_Pane* rp = v->rootPane();

    rp->setOrientation(O_VERTICAL);
    rp->split();
    rp->splitter(0)->setPosPixels(25);

    rp->pane(0)->setHeaderVisible(false);

    rp->splitter(0)->setActive(false);

    v->resize(800, 600);

    // GUI_Label* test = new GUI_Label(pane->body(), "Hello World");
    // GUI_TextEdit* te = new GUI_TextEdit(pane->body(), 400, 300);
    // GUI_PyScripting* te = new GUI_PyScripting(pane->body());
    // te->setPos(55, 5);
    
    v->registerTool("PyConsole", &GUI_PyScripting::create);


    /*

    // Test RadioButton / RadioButtonGroup
    //
    GUI_HLayout* hl = new GUI_HLayout(pane->body());
    GUI_RadioButtonGroup* rbg = new GUI_RadioButtonGroup(pane->body(), hl, "Button Group");

    GUI_RadioButton* re = new GUI_RadioButton(hl, "Hello World");
    hl->addItem(re);

    GUI_RadioButton* re1 = new GUI_RadioButton(hl, "Another");
    hl->addItem(re1);
    */

    // Test Grid
    /*
    GUI_GRID_View* grid = new GUI_GRID_View(pane->body());
    QVariantMap col;
    col.insert("col_name", "name");
    col.insert("col_type", "str");
    col.insert("display_name", "Name");
    col.insert("width", 100);
    
    QVariantMap col2;
    col2.insert("col_name", "length");
    col2.insert("col_type", "str");
    col2.insert("display_name", "Length");
    col2.insert("width", 70);
    
    QVariantMap col3;
    col3.insert("col_name", "status");
    col3.insert("col_type", "str");
    col3.insert("display_name", "Status");
    col3.insert("width", 60);
    
    grid->colModel()->addCol(col);
    grid->colModel()->addCol(col2);
    grid->colModel()->addCol(col3);

    CoreDs_ModelDataItemPtr md = CoreDs_ModelDataItem::makeItem();
    
    CoreDs_ModelDataItemPtr res = CoreDs_ModelDataItem::makeItem();
    res->insert("name", "100_010");
    res->insert("length", 72);
    res->insert("status", "ip");

    CoreDs_ModelDataItemPtr resw = CoreDs_ModelDataItem::makeItem();
    CoreDs_ModelDataItemPtr res2 = CoreDs_ModelDataItem::makeItem();
    res2->insert("name", "200_010");
    res2->insert("length", 172);
    res2->insert("status", "wtg");

    resw->append(res2);
    res->insertItem("__children__", resw);

    md->append(res);

    CoreDs_ModelDataItemPtr res1 = CoreDs_ModelDataItem::makeItem();
    res1->insert("name", "110_010");
    res1->insert("length", 32);
    res1->insert("status", "done");

    
    md->append(res1);

    grid->setModelData(md);
    
    grid->updateHeader();
    */


    // GUI_GLItem* item = new GUI_GLItem(pane->body());

    GUI_MenuBar* menu = new GUI_MenuBar(rp->pane(0)->body());
    GUI_MenuButton* m = menu->addMenu("File");
    m->addAction("Quit");

    GUI_MenuButton* m2 = menu->addMenu("Control");
    m2->addAction("Play");

    // GUI_Label* label = new GUI_Label(pane->body(), "Hello World");

    // pane->setOrientation(O_HORIZONTAL);
    // pane->split();
    // pane->splitter(0)->setPosPercent(0.5);

    
    v->show();

    return QGuiApplication::exec();
}


int main(int argc, char** argv)
{
    return main_full(argc, argv);
}
