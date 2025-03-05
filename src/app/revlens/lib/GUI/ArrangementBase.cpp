
#include "RlpRevlens/GUI/ArrangementBase.h"
#include "RlpCore/PY/Interp.h"

RLP_SETUP_LOGGER(prod, GUI, ArrangementBase)

void
GUI_ArrangementBase::init(GUI_PanePtr parent, GUI_View* view, CoreCntrl_ControllerBase* cntrl)
{
    // RLP_LOG_DEBUG("")
}


bool
GUI_ArrangementBase::_loadArrangement(GUI_Pane* rp, QVariantMap paneInfoIn)
{
    // RLP_LOG_DEBUG(paneInfoIn)

    GUI_Scene* pscene = qobject_cast<GUI_Scene*>(rp->scene());
    GUI_View* pview = qobject_cast<GUI_View*>(pscene->view());

    // RLP_LOG_DEBUG(pview->toolNames())

    QVariantList tabList = paneInfoIn.value("tabs").toList();
    for (int ti=0; ti!=tabList.size(); ++ti)
    {
        QVariantMap tabInfo = tabList.at(ti).toMap();

        QString toolName = tabInfo.value("name").toString();
        if (pview->hasTool(toolName))
        {
            GUI_ToolInfo* toolInfo = pview->tool(toolName);
            // RLP_LOG_DEBUG("Loading Tool:" << toolName << toolInfo->data())

            QVariantMap sigInfo;
            sigInfo.insert("text", toolName);
            sigInfo.insert("data", toolInfo->data());

            rp->header()->onActionItemSelected(sigInfo);

        } else
        {
            RLP_LOG_ERROR("TOOL NOT FOUND:" << toolName)
        }


    }

    if (paneInfoIn.contains("activeTab"))
    {
        int activeTabIdx = paneInfoIn.value("activeTab").toInt();
        GUI_TabHeader* th = rp->header()->tabHeader(activeTabIdx);
        th->tabActivateRequested(th->name());
    }

    if (!paneInfoIn.contains("orientation"))
    {
        RLP_LOG_DEBUG("SKIPPING, no orientation")
        return true;
    }


    int oInt = paneInfoIn.value("orientation").toInt();
    QVariantList paneList = paneInfoIn.value("panes").toList();

    rp->setOrientation(oInt);
    rp->split(true);

    if (paneInfoIn.contains("posPixels"))
    {
        int posPixels = paneInfoIn.value("posPixels").toInt();
        rp->splitter(0)->setPosPixels(posPixels);
        rp->splitter(0)->setActive(false);
    } else {
        float pos = paneInfoIn.value("pos").toFloat();
        rp->splitter(0)->setPosPercent(pos);
    
    }
    rp->updatePanes();

    for (int i=0; i!=paneList.size(); ++i)
    {
        QVariantMap paneInfo = paneList.at(i).toMap();

        GUI_Pane* subpane = rp->panePtr(i);

        // LOG_Logging::pprint(paneInfo);
        _loadArrangement(subpane, paneInfo);

    }

    if (paneInfoIn.contains("tabHeaderVisibility"))
    {
        bool tabvis = paneInfoIn.value("tabHeaderVisibility").toBool();
        pview->setTabHeadersVisible(false);
    }

    return true;
}


bool
GUI_ArrangementBase::loadArrangement(GUI_Pane* rp, QVariantMap paneInfoIn)
{
    if (paneInfoIn.contains("startup.pre"))
    {
        RLP_LOG_DEBUG("Executing Arrangement Pre startup")
        QString startupCode = paneInfoIn.value("startup.pre").toString();
        PY_Interp::eval(startupCode);
    }


    bool result = _loadArrangement(rp, paneInfoIn);

    if (paneInfoIn.contains("startup.post"))
    {
        RLP_LOG_DEBUG("Executing Arrangement post startup")
        QString startupCode = paneInfoIn.value("startup.post").toString();
        PY_Interp::eval(startupCode);
    } else {
        RLP_LOG_DEBUG("NO STARTUP CODE")
        PY_Interp::acquireGIL();

    }

    emit arrangementLoaded();

    return result;
}