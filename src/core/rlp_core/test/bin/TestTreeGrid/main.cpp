
#include <QtWidgets/QApplication>

#include "RlpCore/DS/ModelDataItem.h"

#include "RlpCore/GUI/View.h"
#include "RlpCore/GUI_GRID/View.h"


int main(int argc, char** argv)
{
    QApplication qapp(argc, argv);

    GUI_View v;
    v.resize(340, 400);
    v.move(20, 120);
    
    GUI_PanePtr rootPane = v.rootPane();

    GUI_GRID_View* grid = new GUI_GRID_View(rootPane.get());
    grid->toolbar()->setVisible(false);
    grid->header()->setVisible(false);

    grid->colModel()->addCol(
        "name",
        "Name",
        "str"
    );


    DS_DictModelDataItem data;
    data.insert("name", "test");

    QVariantList child1List;
    
    QVariantMap child;
    child.insert("name", "child");
    child.insert("__children__", child1List);

    QVariantList childList;
    childList.append(child);

    data.insert("__children__", childList);

    

    QVariantMap data1;
    data1.insert("name", "another");
    
    DS_ListModelDataItem dataList;

    dataList.append(data);
    dataList.append(data1);

    grid->setModelData(&dataList);
    v.show();

    return qapp.exec();

}