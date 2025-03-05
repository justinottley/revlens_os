
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/ToolInfo.h"
#include "RlpGui/PANE/Pane.h"
#include "RlpGui/PANE/PaneBody.h"
#include "RlpGui/PANE/View.h"

// #include "RlpGui/PANE/SvgButton.h"
// #include "RlpGui/PANE/MenuPane.h"

#include "RlpGui/BASE/MenuPaneBase.h"

RLP_SETUP_LOGGER(gui, GUI, Pane)

int GUI_Pane::gPaneCount = 0;

GUI_Pane::GUI_Pane(
    GUI_ItemBase* parent,
    QString name,
    SplitOrientation orientation):
        GUI_FrameBase::GUI_FrameBase(
            parent,
            name,
            orientation),
        _scaleFactor(1.0),
        _minWidth(0),
        _minHeight(0),
        _header(nullptr),
        _body(nullptr),
        _last(true),
        _single(true)
{
    setFlag(QQuickItem::ItemAcceptsDrops);

    //if (parent != nullptr) { // for root pane
    //    initPane();
    // }
    // RLP_LOG_DEBUG(parent)

    gPaneCount++;

    // setFillColor(GUI_Style::BgDarkGrey);
    // setOpacity(1.0);
}


GUI_Pane::~GUI_Pane()
{
    // RLP_LOG_DEBUG("DESTRUCTOR - TABS: " << _tabs.size() << "children:" << childItems().size())
    _tabs.clear();
}


void
GUI_Pane::initPane()
{
    // RLP_LOG_DEBUG("")

    _header = new GUI_PaneHeader(this);
    _body = new GUI_PaneBody(this);

    setHeaderVisible(true);
}


void
GUI_Pane::setHeaderVisible(bool isVisible, bool recursive)
{
    bool doVis = false;
    if (!recursive) // && (isVisible))
    {
        _header->setActive(isVisible);
        _header->setVisible(isVisible);
        doVis = true;
    
    } else if (_header->isActive())
    {
        _header->setVisible(isVisible);
        doVis = true;
    }

    if (doVis)
    {
        if (isVisible)
        {
            // Dont need this - _body->setHeight(height() - 25);
            _body->setPos(0, 25); // _header->height() + 1);
            

            for (int ti=0; ti != _tabs.size(); ++ti)
            {
                _tabs.at(ti)->pane()->setHeightOffset(20);
            }

        } else {
            // Dont need this - _body->setHeight(height() + 25);
            _body->setPos(0, 0);
            

            for (int ti=0; ti != _tabs.size(); ++ti)
            {
                _tabs.at(ti)->pane()->setHeightOffset(0);
            }

        }
    }


    if (recursive)
    {
        for (int pi = 0; pi != _panes.size(); ++pi)
        {
            _panes.at(pi)->setHeaderVisible(isVisible, recursive);
        }

    }
}


void
GUI_Pane::setSplittersVisible(bool isVisible, bool recursive)
{
    
    // bool doVis = false;

    if (!recursive) // && (isVisible))
    {
        for (int si=0; si != _splitters.size(); ++si)
        {
            _splitters.at(si)->setActive(isVisible);
            _splitters.at(si)->setVisible(isVisible);
        }
        
        // doVis = true;
    
    } else
    {
        for (int si=0; si != _splitters.size(); ++si)
        {
            if (_splitters.at(si)->isActive())
            {
                _splitters.at(si)->setVisible(isVisible);
            }
        }

        // doVis = true;
    }


    if (recursive)
    {
        for (int pi = 0; pi != _panes.size(); ++pi)
        {
            _panes.at(pi)->setSplittersVisible(isVisible, recursive);
        }
    }
}

/*

bool
GUI_Pane::setOrientation(SplitOrientation orientation)
{
    if (_panes.size() == 0) {
        _orientation = orientation;
        
        qInfo() << "setting orientation: " << (int)orientation;
        return true;
        
    }
    
    return false;
}

*/


void
GUI_Pane::onSizeChangedInternal()
{
    // RLP_LOG_DEBUG(_metadata)

    updatePanes();
}


void
GUI_Pane::setScaleFactor(float scaleFactor)
{
    // RLP_LOG_DEBUG(scaleFactor << " orientation: " << orientation())

    _scaleFactor = scaleFactor;
}


void
GUI_Pane::setBgColor(QColor col)
{
    _body->setBgColor(col);
}


GUI_PanePtr
GUI_Pane::pane(int i)
{
    if ((i >= 0) && (i < _panes.size())) {
        
        return _panes.at(i);
    }
    
    return nullptr;
}


GUI_Pane*
GUI_Pane::panePtr(int i)
{
    GUI_PanePtr r = pane(i);
    if (r != nullptr)
    {
        return r.get();
    }

    return nullptr;
}


GUI_PaneSplitterPtr
GUI_Pane::splitter(int i)
{
    return _splitters.at(i);
}


GUI_PaneSplitter*
GUI_Pane::splitterPtr(int i)
{
    GUI_PaneSplitterPtr sp = splitter(i);
    if (sp != nullptr)
    {
        return sp.get();
    }

    return nullptr;
}


bool
GUI_Pane::split(bool headerVisible)
{
    if (_orientation == O_UNKNOWN)
    {
        return false;
    }

    if (_panes.size() == 0)
    {

        GUI_PanePtr p(new GUI_Pane(_body));
        p->initPane();

        // Move any existing tabs into first pane
        //
        for (int i=0; i!=_tabs.size(); ++i) {

            GUI_TabPtr ct = tab(i);
            ct->setParentItem(p->body());
            p->_addTab(ct);
            _header->removeAt(0);
        }

        _tabs.clear();
        setHeaderVisible(false);

        p->setHeaderVisible(headerVisible);
        _panes.append(p);
        
        GUI_PanePtr p2(new GUI_Pane(_body));
        p2->initPane();
        p2->setHeaderVisible(true);
        
        _panes.append(p2);
        GUI_PaneSplitterPtr sp(new GUI_PaneSplitter(this, 0));
        
        
        _splitters.append(sp);
        
    } else {
        
        GUI_PanePtr p(new GUI_Pane(_body));
        _panes.append(p);
        
        
        // p->setBgColor(QColor(10, 10, 100));

        GUI_PaneSplitterPtr sp(new GUI_PaneSplitter(this, 1));
        
        
        _splitters.append(sp);
        
    }
    
    // _scaleFactor = -1;

    int paneCount = _panes.size();
    float scaleFactor = 1.0 / paneCount;

    RLP_LOG_VERBOSE(name() << " setting scaleFactor: " << scaleFactor)

    for (int i=0; i != paneCount; ++i) {    
        _panes.at(i)->setScaleFactor(scaleFactor);
        _panes.at(i)->setLast(false);
    }

    // Edge cases for drawing.....

    _panes.at(paneCount - 1)->setLast(true);

    if (paneCount == 1)
    {
        _panes.at(paneCount -1)->setSingle(true);
    }
    else if (_orientation == O_HORIZONTAL)
    {
        _panes.at(paneCount -1)->setSingle(false);
    }
        

    // Splitters

    for (int si=0; si != _splitters.size(); ++si) {
        _splitters.at(si)->setPos(scaleFactor * (si + 1), false);
    }

    updatePanes();

    return true;
}


bool
GUI_Pane::removePane(GUI_Pane* pane)
{
    if (pane->tabCount() > 0)
    {
        RLP_LOG_WARN("pane has " << pane->tabCount() << " tabs - ideally should have 0")
        // return false;
    }

    
    int fpane = -1;
    for (int pi = 0; pi != _panes.size(); ++pi) {
        if (_panes.at(pi).get() == pane) {

            RLP_LOG_INFO("FOUND PANE AT POS " << pi)
            fpane = pi;
            break;
        }
    }

    if (fpane >= 0) {
        _panes.removeAt(fpane);

        RLP_LOG_DEBUG("splitters:" << _splitters.size())

        int numSplitters = _splitters.size();
        if (numSplitters > 0)
        {
            if (fpane == numSplitters) {
                _splitters.removeAt(fpane - 1);
            } else {
                _splitters.removeAt(fpane);
            }
        }
        
        
        // delete pane;

        float scaleFactor = 1.0 / _panes.size();

        RLP_LOG_DEBUG("setting scaleFactor: " << scaleFactor << " new panecount: " << _panes.size())

        for (int i=0; i != _panes.size(); ++i) {
            _panes.at(i)->setScaleFactor(scaleFactor);
        }
        
        // Pull back pane start index on all splitters that come after
        //
        for (int si=fpane; si < _splitters.size(); ++si) {
            _splitters.at(si)->setPaneStartIndex(_splitters.at(si)->paneStartIndex() - 1);

        }

        for (int si=0; si != _splitters.size(); ++si) {
            _splitters.at(si)->setPos(scaleFactor * (si + 1), false);
        }

        updatePanes();

    }

    return true;
}


void
GUI_Pane::clear()
{
    // RLP_LOG_DEBUG("Tab count:" << tabCount())

    while (tabCount() > 0)
    // for (int ti=0; ti != tabCount(); ++ti)
    {
        GUI_TabPtr t = tab(0);
        onTabCloseRequested(t->name(), /*destruct=*/true);
    }

    for (int pi=0; pi != paneCount(); ++ pi)
    {
        GUI_PanePtr p = pane(pi);
        p->clear();
    }

    while (paneCount() > 0)
    {
        GUI_PanePtr p = pane(0);
        removePane(p.get());
    }
}


void
GUI_Pane::updatePanes(int currPaneIndex)
{
    // RLP_LOG_DEBUG(currPaneIndex << "parent:" << parentItem())

    int splitterSizeOffset = currPaneIndex * GUI_PaneSplitter::SIZE;
    QString parentClass = parentItem()->metaObject()->className();

    // RLP_LOG_DEBUG(name() << " - " << currPaneIndex << _scaleFactor << "parent:" << parentClass)

    if (parentClass == "GUI_Scene")
    {
        // RLP_LOG_DEBUG("ROOT PANE PARENT SCENE!!!")

        setWidth(parentItem()->width());
        setHeight(parentItem()->height());
    }

    if (parentClass == "GUI_PaneBody")
    {
        GUI_PaneBody* parent = static_cast<GUI_PaneBody*>(parentItem());
        if (parent != nullptr) {
            if (parent->orientation() == O_VERTICAL) {
            
                setWidth(parent->width());
                setHeight((parent->height() * _scaleFactor) - splitterSizeOffset);

                // RLP_LOG_DEBUG("SET HEIGHT!!! " << (parent->height() * _scaleFactor) - splitterSizeOffset)

            } else if (parent->orientation() == O_HORIZONTAL) {
                
                setHeight(parent->height());
                setWidth((parent->width() * _scaleFactor) - splitterSizeOffset);

            } else {

                // RLP_LOG_ERROR("no orientation - " << parent << " " << parent->name())

                setWidth(parent->width());
                setHeight(parent->height());
            }
        }
    }



    if ((_minWidth > 0) && (width() < _minWidth))
    {
        // RLP_LOG_WARN("Clamping to minWidth: " << _minWidth)

        setWidth(_minWidth);
    }

    if ((_minHeight > 0) && (height() < _minHeight))
    {
        // RLP_LOG_WARN("Clamping to minHeight: " << _minHeight)

        setHeight(_minHeight);
    }


    // qInfo() << "updatePanes(): width: " << _width << " height: " << _height;

    if (_panes.size() > 0) {

        _panes.at(0)->setPos(0, 0);
        _panes.at(0)->updatePanes();
    }
    
    if (_panes.size() > 1) {

        
        int aheight = 0;
        int awidth = 0;

        for (int i=1; i != _panes.size(); ++i) {
            
            awidth += _panes.at(i - 1)->width();
            aheight += _panes.at(i - 1)->height();

            // RLP_LOG_DEBUG("size so far:" << awidth  << aheight)

            if (_orientation == O_VERTICAL) {
                
                // RLP_LOG_DEBUG("!!!!SETPOS:  i: " << i << " - height: " << aheight)

                _panes.at(i)->setPos(0, (aheight + GUI_PaneSplitter::SIZE));

            } else if (_orientation == O_HORIZONTAL) {
                
                // RLP_LOG_DEBUG("  i: " << i << " - width: " << awidth)

                _panes.at(i)->setPos((awidth + GUI_PaneSplitter::SIZE), 0);
                // _panes.at(i)->setPos(awidth, 0);

            }

            _panes.at(i)->updatePanes(i);
        }
    }
    


    for (int i=0; i != _tabs.size(); ++i) {
        _tabs.at(i)->setSize(size());
    }
    
    for (int i=0; i != _splitters.size(); ++i) {
        _splitters.at(i)->updatePos();
    }

    if (_header != nullptr)
    {
        _header->updateTabHeaders();
    }

    emit sizeChanged(width(), height());
}


GUI_TabPtr
GUI_Pane::tab(int i)
{
    return _tabs.at(i);
    
}


GUI_TabPtr
GUI_Pane::addTab(QString tabName)
{
    GUI_TabPtr newTab(new GUI_Tab(_body, tabName));

    return _addTab(newTab);
}


GUI_TabPtr
GUI_Pane::_addTab(GUI_TabPtr newTab, int idx, bool tabHeaderVisible)
{
    Q_ASSERT(paneCount() == 0);

    newTab->setOrientation(_orientation);
    if (idx == -1)
    {
        _tabs.append(newTab);
    } else
    {
        RLP_LOG_DEBUG("Inserting new tab at index" << idx);
        _tabs.insert(idx, newTab);
    }

    GUI_TabHeader* newTabHeader = _header->addTabHeader(newTab->name(), idx);
    newTabHeader->setVisible(tabHeaderVisible);

    newTab->registerPaneConnection(
        connect(
            newTab.get(),
            &GUI_Tab::visibilityChanged,
            this,
            &GUI_Pane::onTabVisibilityChanged
        )
    );

    newTab->registerPaneConnection(
        connect(
            newTabHeader,
            &GUI_TabHeader::tabActivateRequested,
            this,
            &GUI_Pane::onTabActivateRequested
        )
    );

    newTab->registerPaneConnection(
        connect(
            newTabHeader,
            &GUI_TabHeader::closeTab,
            this,
            &GUI_Pane::onTabCloseRequested
        )
    );

    newTab->registerPaneConnection(
        connect(
            newTabHeader,
            &GUI_TabHeader::tabMoveRequested,
            this,
            &GUI_Pane::onTabMoveRequested
        )
    );

    setHeaderVisible(true);
    setActiveTab(_tabs.size() - 1);

    return newTab;
}


GUI_TabPtr
GUI_Pane::getTab(QString tabName)
{
    GUI_TabPtr result = nullptr;
    for (int ti = 0; ti != _tabs.size(); ++ti)
    {
        
        if (_tabs.at(ti)->name() == tabName)
        {
            result = _tabs.at(ti);
            break;
        }
    }

    return result;
}


int
GUI_Pane::getTabIndex(QString tabName)
{
    int result = -1;
    for (int ti = 0; ti != _tabs.size(); ++ti)
    {
        if (_tabs.at(ti)->name() == tabName)
        {
            result = ti;
            break;
        }
    }

    return result;
}


void
GUI_Pane::setActiveTab(int i)
{
    _activeTab = i;
    
    for (int ti = 0; ti != _tabs.size(); ++ti) {
        _tabs.at(ti)->setTabVisible(false);
        _header->tabHeader(ti)->setActive(false);
    }
    
    _tabs.at(i)->onParentSizeChanged(width(), height());
    _tabs.at(i)->setTabVisible(true);
    _header->tabHeader(i)->setActive(true);
    _header->tabHeader(i)->setVisible(true);
}


bool
GUI_Pane::onTabActivateRequested(QString tabName)
{
    RLP_LOG_INFO(tabName)

    bool result = false;
    int ti = getTabIndex(tabName);
    if (ti != -1)
    {
        setActiveTab(ti);
        result = true;
    }

    return result;
}


bool
GUI_Pane::onTabCloseRequested(QString tabName, bool destruct)
{
    RLP_LOG_DEBUG(tabName)

    bool result = false;

    int tabIndex = getTabIndex(tabName);
    if (tabIndex >= 0)
    {

        // TODO FIXME: DISABLED FOR WINDOWS STABILITY
        GUI_TabHeader* h =_header->tabHeader(tabIndex);
        RLP_LOG_DEBUG(_header->headerCount() << tabIndex)
        h->setVisible(false);
        h->setActive(false);

        // TODO FIXME: DISABLED FOR WINDOWS STABILITY
        if (destruct)
        {
            RLP_LOG_WARN("Destructing Tab at" << tabIndex)

            _header->removeAt(tabIndex);
            h->deleteLater();

            // emit tabCloseRequested(tabName, tabIndex);
            // GUI_TabPtr t = _tabs.at(tabIndex);
            // t->deleteLater();

            _tabs.removeAt(tabIndex);

            RLP_LOG_DEBUG("DONE")
            // return;
        }

        result = true;
    }

    _header->updateTabHeaders();

    if (_tabs.size() > 0)
    {
        setActiveTab(0);
    }

    return result;
}


void
GUI_Pane::onTabVisibilityChanged(QString tabName, bool visible)
{
    // RLP_LOG_DEBUG(tabName << visible)

    GUI_Scene* gscene = qobject_cast<GUI_Scene*>(scene());
    gscene->requestTabVisibilityChanged(tabName, visible);
}


void
GUI_Pane::onTabMoveRequested(QString toTabName, QString incomingTabName, QVariant tabWrapper)
{
    RLP_LOG_DEBUG("move" << incomingTabName << "To" << toTabName)

    int toIdx = getTabIndex(toTabName);

    GUI_TabPtr tab = tabWrapper.value<GUI_TabPtr>();
    if (tab->parentItem() == _body)
    {
        int incomingTabIdx = getTabIndex(incomingTabName);
        if (incomingTabIdx != -1)
        {
            GUI_TabPtr tab = _tabs.takeAt(incomingTabIdx);
            _tabs.insert(toIdx, tab);

            _header->moveTabHeader(incomingTabName, toTabName);
        } else
        {
            RLP_LOG_ERROR("Tab not found:" << incomingTabName)
        }
        
    }
    else
    {
        tab->resetPaneConnections();
        tab->setParentItem(_body);
        _addTab(tab, toIdx, /*tabHeaderVisible=*/ true);

        _header->updateTabHeaders();

    }
}


void
GUI_Pane::mousePressEvent(QMouseEvent *event)
{

    event->setAccepted(false);

    if ((parentItem() != nullptr) && (parentItem()->metaObject()->className() == "GUI_PaneBody"))
    {

        if (_header->actButton()->menu() != nullptr)
        {
            RLP_LOG_DEBUG("GOT MENU")

            event->setAccepted(true);
            
            _header->actButton()->menu()->setPos(event->pos());
            _header->actButton()->toggleMenu();


        } else
        {
            RLP_LOG_VERBOSE("Click away")
            qobject_cast<GUI_Scene*>(scene())->hideMenuPanes();
        }

    }

}


QRectF
GUI_Pane::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_Pane::paint(GUI_Painter* painter)
{
}