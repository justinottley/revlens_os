

#include "RlpEdb/GUI/GridView.h"
#include "RlpCore/DS/ModelDataItem.h"

RLP_SETUP_LOGGER(edb, EdbGui, GridView)

EdbGui_GridView::EdbGui_GridView(EdbCntrl_Query* qcntrl,GUI_ItemBase* parent):
    GUI_GRID_View(parent),
    _qcntrl(qcntrl)
{
}


void
EdbGui_GridView::initFromEntityType(QVariantMap entityInfo)
{

    QString etype = entityInfo.value("text").toString();
    RLP_LOG_INFO(etype)

    clearData();

    _header->clear();
    _colModel->clear();

    _toolbar->label()->setText(etype);

    GUI_MenuPane* csMenu = qobject_cast<GUI_MenuPane*>(_toolbar->colSelector()->menu());
    csMenu->clear();

    QVariantMap eFields = _qcntrl->getEntityTypeFields(etype);

    QVariantMap mdata;

    _colModel->addCol(
        "uuid", "UUID", "str", mdata, 80
    );

    _colModel->addCol(
        "name", "Name", "str", mdata, 80
    );

    _colModel->addCol(
        "create_date", "Date Created", "int", mdata, 100
    );

    QVariantMap::iterator it;
    for (it = eFields.begin(); it != eFields.end(); ++it) {

        QVariantMap eInfo = it.value().toMap();

        // qInfo() << it.value();
        
        QString fieldDisplayName = eInfo.value("display_name").toString();

        /*
        QVariantMap colInfo;
        
        colInfo.insert("col_name", it.key());
        colInfo.insert("col_type", );
        colInfo.insert("display_name", fieldDisplayName);
        colInfo.insert("width", 180);
        */
        // qInfo() << colInfo;

        RLP_LOG_DEBUG(fieldDisplayName)
        
        _colModel->addCol(
            it.key(),
            fieldDisplayName,
            eInfo.value("type_name").toString(),
            mdata,
            180
        );

    }

    _colModel->updateCols();
    _header->updateCells(_colModel->colNames());


    // Load all records
    // TEMP for testing / demo
    QVariantList allresults = _qcntrl->find(
        etype,
        QVariantList(),
        _colModel->colNames()
    );

    // RLP_LOG_DEBUG("")
    // RLP_LOG_DEBUG(" --  all results --")
    // RLP_LOG_DEBUG("")
    // for (int ri=0; ri != allresults.size(); ++ri) {
    //     RLP_LOG_DEBUG(allresults.at(ri))
    //     RLP_LOG_DEBUG("")
    // }
    //
    // RLP_LOG_DEBUG("")
    
    CoreDs_ModelDataItemPtr modelData = CoreDs_ModelDataItem::makeItem();

    // TEMP lookup name of all entity results
    //
    QVariantList uuidList;
    for (int i=0; i != allresults.size(); ++i) {

        CoreDs_ModelDataItemPtr row = CoreDs_ModelDataItem::makeItem();
        QVariantMap r = allresults.at(i).toMap();

        QVariantMap::iterator rit;
        for (rit = r.begin(); rit != r.end(); ++rit) {
            QVariant val = rit.value();

            // qInfo() << val;

            if (QString(val.typeName()) == "QUuid") {
                uuidList.push_back(val.toString());
            }

            row->insert(rit.key(), val);
        }

        modelData->append(row);

    }

    // qInfo() << "UUID LIST:";
    // qInfo() << uuidList;

    _entityNameMap.clear();
    _entityNameMap = _qcntrl->findByEntityUuid(uuidList);

    // qInfo() << _entityNameMap;

    setModelData(modelData);
    
    RLP_LOG_DEBUG("Done")
}