
#include "RlpCore/LOG/Logging.h"

#include "RlpCore/PY/Interp.h"
#include "RlpCore/UTIL/StacktraceHandler.h"

// #include "RlpGui/PANE/Window.h"
#include "RlpGui/PANE/View.h"

#include "RlpGui/MENU/MenuButton.h"
#include "RlpGui/BASE/HLayout.h"

#include "RlpGui/GRID/View.h"

#include "RlpEdb/GUI/GridView.h"

#include "RlpEdb/CNTRL/Query.h"


#include <QtQuick/QQuickWindow>

int main( int argc, char **argv )
{
    UTIL_StacktraceHandler::installSignalHandler();

    LOG_Logging::init();


    QApplication qapp(argc, argv);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    PY_Interp::init();

    GUI_View* v = new GUI_View();
    v->resize(800, 600);

    // GUI_Window w;
    // w.setView(v); // w.view(); // GUI_View v;    
    // w.resize(640, 600);
    // w.move(20, 120);
    
    GUI_Pane* rp = v->rootPane();
    
    rp->setOrientation(O_VERTICAL);
    
    rp->split();
    rp->splitter(0)->setPosPercent(0.1);
    /*
    
    rp->setOrientation(O_VERTICAL);
    rp->split();
    rp->splitter(0)->setPos(0.3);
    
    rp->pane(1)->addTab("Shot");
    
    */

    qDebug() << "loading commandline optoins";

    QString dbPath = argv[1]; // production.db // "/home/justinottley//.config/rlp/rfs/rlp_test/fs.db"; // 
    QString dbExt = "rfs"; // argv[2]; // pdb_test
    QString dbType = "rfs"; // argv[3]; // production_db

    qDebug() << dbPath << " " <<  dbExt << " " << dbType;

    EdbCntrl_Query* qcntrl = new EdbCntrl_Query(dbPath, dbExt, dbType); // "production.db", "pdb_test", "production_db");


    GUI_HLayout* hl = new GUI_HLayout(rp->pane(0)->body());

    GUI_MenuButton* btn = new GUI_MenuButton(hl);
    btn->setText("Entities");
    hl->addItem(btn);


    EdbGui_GridView* grid = new EdbGui_GridView(qcntrl, rp->pane(1)->body());
    grid->setPos(10, 15);

    QObject::connect(
        btn->menu(),
        &GUI_MenuPane::menuItemSelected,
        grid,
        &EdbGui_GridView::initFromEntityType
    );


    //
    // Data
    //

    QVariantList allEntityTypes = qcntrl->getEntityTypes();
    for (int i=0; i != allEntityTypes.size(); ++i)
    {
        btn->menu()->addItem(allEntityTypes.at(i).toMap().value("name").toString());
    }

    v->show();

    return qapp.exec();
}