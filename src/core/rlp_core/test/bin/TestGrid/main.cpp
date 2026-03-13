
#include "RlpCore/UTIL/StacktraceHandler.h"

#include "RlpCore/DS/ModelDataItem.h"
#include "RlpCore/GUI_GRID/View.h"


#include <QtWidgets/QApplication>
// #include <QWidget>
// #include <QMainWindow>

#include "RlpCore/GUI/View.h"
#include "RlpCore/GUI_GRID/View.h"
#include "RlpCore/GUI/MenuButton.h"
#include "RlpCore/GUI/HLayout.h"

int main( int argc, char **argv )
{
    UTIL_StacktraceHandler::installSignalHandler();

    QApplication qapp(argc, argv);

    GUI_View v;
    v.resize(640, 600);
    v.move(20, 120);
    
    GUI_PanePtr rp = v.rootPane();
    
    rp->setOrientation(O_VERTICAL);
    
    rp->split();

    /*
    
    rp->setOrientation(O_VERTICAL);
    rp->split();
    rp->splitter(0)->setPos(0.3);
    
    rp->pane(1)->addTab("Shot");
    
    */

    /*
    */

    GUI_HLayout* hl = new GUI_HLayout(rp->pane(0)->body());

    GUI_MenuButton* btn = new GUI_MenuButton(hl);
    btn->setText("Entities");
    hl->addItem(btn);

    btn->menu()->addItem("Project");
    
    GUI_MenuButton* btns = new GUI_MenuButton(hl);
    btns->setText("Something");
    hl->addItem(btns);

    GUI_GRID_View* grid = new GUI_GRID_View(
       // rp->pane(1)->tab(0)->pane()
       rp->pane(1)->body()
    );

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



    // QVariantList md;
    CoreDs_ModelDataItemPtr md = CoreDs_ModelDataItem::makeItem();

    CoreDs_ModelDataItemPtr res = CoreDs_ModelDataItem::makeItem();
    res->insert("name", "100_010");
    res->insert("length", 72);
    res->insert("status", "ip");
    
    md->append(res);
    
    CoreDs_ModelDataItemPtr res2 = CoreDs_ModelDataItem::makeItem();
    res2->insert("name", "100_020");
    res2->insert("length", 34);
    res2->insert("status", "wts");
    
    md->append(res2);
    
    
    
    grid->setModelData(md);
    
    grid->updateHeader();

    v.show();
    
    qInfo() << v.boundingRect();
    
    return qapp.exec();
}