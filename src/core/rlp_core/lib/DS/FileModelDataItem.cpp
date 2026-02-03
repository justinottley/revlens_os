
#include "RlpCore/SEQ/Util.h"
#include "RlpCore/DS/FileModelDataItem.h"

RLP_SETUP_LOGGER(core, CoreDs, FileModelDataItem)

CoreDs_FileModelDataItem::CoreDs_FileModelDataItem()
{
}


CoreDs_ModelDataItemPtr
CoreDs_FileModelDataItem::makeItem()
{
    PY_Interp::acquireGIL();

    return CoreDs_ModelDataItemPtr(new CoreDs_FileModelDataItem());
}


void
CoreDs_FileModelDataItem::populate()
{
    QString rootDir = _data.value("__rootDir__").toString();
    QString dirName = _data.value("name").toString();

    QString dirEntry = rootDir + "/" + dirName;

    RLP_LOG_DEBUG(dirEntry)

    // QString startDirName = 
    QDir startDir(dirEntry);

    QStringList direntries = startDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    RLP_LOG_DEBUG(direntries)
    
    CoreDs_ModelDataItemPtr result = CoreDs_FileModelDataItem::makeItem();

    for (int i = 0; i != direntries.size(); ++i) {
        
        CoreDs_ModelDataItemPtr rentry = CoreDs_FileModelDataItem::makeItem();
        CoreDs_ModelDataItemPtr clist = CoreDs_FileModelDataItem::makeItem();
        
        rentry->insert("name", direntries.at(i));
        rentry->setMimeType("name", "text/dirname");
        rentry->insert("__rootDir__", dirEntry);
        rentry->insertItem("__children__", clist);
        
        result->append(rentry);
    }

    SequenceList seqList = SEQ_Util::getSequences(dirEntry);

    for (int si = 0; si != seqList.size(); ++si) {
        CoreDs_ModelDataItemPtr fentry = CoreDs_FileModelDataItem::makeItem();

        fentry->insert("__rootDir__", dirEntry);
        fentry->setMimeType("name", "text/filename");
        
        if (seqList.at(si)->size() > 1) {
            fentry->insert("name", seqList.at(si)->itemTemplateStr());

        } else {
            fentry->insert("name", seqList.at(si)->at(0)->rawItem());
        }

        result->append(fentry);
    }
    /*
    QStringList fentries = startDir.entryList(QDir::Files);

    for (int fi = 0; fi != fentries.size(); ++fi) {
        CoreDs_ModelDataItemPtr fentry = CoreDs_FileModelDataItem::makeItem();
        fentry->insert("name", fentries.at(fi));
        result->append(fentry);
    }
    */

    _items.insert("__children__", result);

    emit populated();
}

