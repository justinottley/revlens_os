

#include "RlpCore/PY/Interp.h"
#include "RlpCore/DS/FileModelDataItem.h"

#include "RlpGui/GRID/CellFormatterFolder.h"

#include "RlpRevlens/GUI/ItemLoader.h"
#include "RlpRevlens/GUI/FileBrowser.h"


RLP_SETUP_LOGGER(prod, GUI, FileBrowser)



GUI_FileBrowser::GUI_FileBrowser(GUI_ItemBase* parent):
    GUI_ItemBase(parent)
{
    _layout = new GUI_VLayout(this);
    _layout->setHSpacing(15);

    _layout->addSpacer(20);

    GUI_HLayout* hl = new GUI_HLayout(this);
    hl->setSpacing(10);


    _pathEdit = new GUI_TextEdit(hl, 260, 30);

    connect(
        _pathEdit,
        &GUI_TextEdit::enterPressed,
        this,
        &GUI_FileBrowser::onPathEnterPressed
    );

    hl->addItem(_pathEdit);

    _setRootButton = new GUI_IconButton("", _layout, 20);
    _setRootButton->setText("Set Root");
    _setRootButton->setOutlined(true);

    _browseButton = new GUI_SvgButton(":misc/open.svg", this, 25);
    _browseButton->setToolTipText("Browse..");
    connect(
        _browseButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_FileBrowser::onBrowseRequested
    );

    connect(
        _setRootButton,
        &GUI_IconButton::buttonPressed,
        this,
        &GUI_FileBrowser::onSetRootPressed
    );

    hl->addItem(_browseButton);
    hl->addItem(_setRootButton);
    
    _layout->addItem(hl);

    _grid = new GUI_GRID_View(this);
    _grid->setAutoResize(false);
    _grid->colModel()->addFormatter("str", new GUI_GRID_CellFormatterFolder());

    QVariantMap md;
    md.insert("expander_visible", true);
    _grid->colModel()->addCol("name", "Name", "str", md, 380);
    _grid->updateHeader();

    GUI_HLayout* tlayout = _grid->toolbar()->layout();
    
    GUI_IconButton* loadButton = new GUI_IconButton("", _grid->toolbar(), 20);
    loadButton->setText("Load..");
    loadButton->setOutlined(true);
    
    // loadButton->setTextYOffset(0);
    tlayout->addSpacer(10);
    tlayout->addItem(loadButton);
    tlayout->addSpacer(10);
    tlayout->addItem(_grid->toolbar()->colSelector());

    connect(
        loadButton,
        &GUI_IconButton::buttonPressed,
        this,
        &GUI_FileBrowser::onLoadRequested
    );

    // _grid->toolbar()->setVisible(false);

    _layout->addItem(_grid);


    #ifdef SCAFFOLD_WASM
    setRoot("/wasm_fs_root/test_media");
    #elif SCAFFOLD_IOS
    QString appRoot = getenv("APP_ROOT");
    appRoot += "/Videos";
    setRoot(appRoot);
    #else
    QString homeDir = QDir::homePath();
    #ifdef __MACH__
    homeDir += "/Movies";
    #else
    homeDir += "/Videos";
    #endif

    setRoot(homeDir);
    #endif

    onParentSizeChanged(parent->width(), parent->height());
}


QQuickItem*
GUI_FileBrowser::create(GUI_ItemBase* parent, QVariantMap toolInfo)
{
    return new GUI_FileBrowser(parent);
}


void
GUI_FileBrowser::setRoot(QString rootDirName)
{
    // QString startDirName = 
    QDir startDir(rootDirName);

    _pathEdit->setText(rootDirName);

    QStringList direntries = startDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    RLP_LOG_DEBUG(direntries)
    
    CoreDs_ModelDataItemPtr result = CoreDs_FileModelDataItem::makeItem();

    for (int i = 0; i != direntries.size(); ++i)
    {
        CoreDs_ModelDataItemPtr rentry = CoreDs_FileModelDataItem::makeItem();
        CoreDs_ModelDataItemPtr clist = CoreDs_FileModelDataItem::makeItem();

        //CoreDs_FileModelDataItem centry;
        //centry.insert("name", "test");
        //clist->append(centry);

        rentry->insert("name", direntries.at(i));
        rentry->setMimeType("name", "text/dirname");
        rentry->insert("__rootDir__", rootDirName);
        rentry->insertItem("__children__", clist);
        
        result->append(rentry);
    }
    
    QStringList fentries = startDir.entryList(QDir::Files);

    for (int fi = 0; fi != fentries.size(); ++fi) {
        CoreDs_ModelDataItemPtr fentry = CoreDs_FileModelDataItem::makeItem();

        fentry->insert("__rootDir__", rootDirName);
        fentry->insert("name", fentries.at(fi));
        fentry->setMimeType("name", "text/filename");
        result->append(fentry);
    }

    _grid->setModelData(result);

}



void
GUI_FileBrowser::onSetRootPressed(QVariantMap /* metadata */)
{
    QString root = _pathEdit->text();
    RLP_LOG_DEBUG(root)

    setRoot(root);
}


void
GUI_FileBrowser::onPathEnterPressed(QVariantMap itemInfo)
{
    RLP_LOG_DEBUG(itemInfo)

    onSetRootPressed(itemInfo);
}


QString
GUI_FileBrowser::selectedPath()
{
    QString path = "";
    CoreDs_ModelDataItemPtr sel = _grid->selection();
    
    if (sel == nullptr)
    {
        RLP_LOG_WARN("Nothing selected")
        return path;
    }

    RLP_LOG_DEBUG(sel->values())

    QString root = sel->value("__rootDir__").toString();
    if (root == "") {
        root = _pathEdit->text();
    }
    QString fileName = sel->value("name").toString();
    path = root + "/" + fileName;

    return path;
}


void
GUI_FileBrowser::onBrowseRequested()
{
    QString dir = QFileDialog::getExistingDirectory(
        nullptr,
        tr("Open Directory"),
        QDir::homePath(), 
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir == "")
    {
        return;
    }


    _pathEdit->setText(dir);
    _pathEdit->textArea()->update();
    setRoot(dir);

}


void
GUI_FileBrowser::onLoadRequested(QVariantMap /* metadata */)
{
    RLP_LOG_DEBUG("")

    QString path = selectedPath();
    if (path == "")
    {
        return;
    }

    RLP_LOG_DEBUG(path)

    QVariantMap selData;
    selData.insert("local_path", path);
    selData.insert("title", path);

    QVariantList argList;
    argList.append(selData);

    RLP_LOG_DEBUG(argList)

    PY_Interp::call("revlens.gui.show_load_dialog", argList);
}



void
GUI_FileBrowser::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(width << height) //  << "grid pos:" << _grid->position().y())

    setWidth(width);
    setHeight(height);

    // _layout->setWidth(width);

    _grid->setWidth(width - 30);
    _grid->setHeight(height - 130);
}


QRectF
GUI_FileBrowser::boundingRect() const
{
    // return _grid->boundingRect();
    return parentItem()->boundingRect();
}


void
GUI_FileBrowser::paint(QPainter *painter)
{
}