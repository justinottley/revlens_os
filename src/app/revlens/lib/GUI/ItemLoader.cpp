
#include "RlpRevlens/GUI/ItemLoader.h"
#include "RlpCore/PY/Interp.h"

RLP_SETUP_LOGGER(prod, GUI, ItemLoader)


GUI_ItemLoader::GUI_ItemLoader(GUI_ItemBase* parent):
    GUI_ItemBase(parent)
{
    setZValue(parent->z());

    RLP_LOG_DEBUG("parent zVal: " << parent->z())
    RLP_LOG_DEBUG("my zVal: " << z())

    _layout = new GUI_VLayout(this);

    _btnLayout = new GUI_HLayout(this, 5);
    _posLayout = new GUI_HLayout(this, 1);
    GUI_RadioButtonGroup* posg = new GUI_RadioButtonGroup(this, _posLayout);
    
    GUI_Label* trackLabel = new GUI_Label(this, "Track");
    _btnLayout->addItem(trackLabel);
    
    _trackListButton = new GUI_IconButton("", this, 20);
    _trackListButton->setOutlined(true);
    _trackListButton->setSvgIconPath(":/feather/lightgrey/chevron-down.svg", 20);

    GUI_SvgIcon* tri = _trackListButton->svgIcon();
    tri->setPos(tri->x(), 4);
    tri->setBgColour(QColor(160, 160, 160));

    GUI_MenuPane* tlMenu = qobject_cast<GUI_MenuPane*>(_trackListButton->menu());
    _btnLayout->addItem(_trackListButton);

    _posLabel = new GUI_RadioButton(this, "Pos", 20, 0);
    _posLayout->addItem(_posLabel);

    _posButton = new GUI_IconButton("", this, 20);
    _posButton->setOutlined(true);
    _posButton->setSvgIconPath(":/feather/lightgrey/chevron-down.svg", 20);
    
    GUI_SvgIcon* pri = _posButton->svgIcon();
    pri->setPos(pri->x(), 4);
    pri->setBgColour(QColor(160, 160, 160));

    GUI_MenuPane* pbMenu =  qobject_cast<GUI_MenuPane*>(_posButton->menu());
    pbMenu->addItem("Start");
    pbMenu->addItem("End");
    pbMenu->addItem("Curr Frame");

    _posButton->setText("End");


    _posLayout->addItem(_posButton);
    _posLayout->addSpacer(50);

    _frameLabel = new GUI_RadioButton(this, "Frame", 20, 0);
    _posLayout->addItem(_frameLabel);

    _posLabel->emitButtonPressed();

    _frameEdit = new GUI_TextEdit(this);
    _frameEdit->setHeight(60);
    _frameEdit->setWidth(80);
    _frameEdit->setText("1");

    _posLayout->addItem(_frameEdit);
    
    _layout->addItem(_btnLayout);
    _layout->addSpacer(5);
    _layout->addItem(_posLayout);

    _loadButton = new GUI_IconButton("", this, 20);
    _loadButton->setText("Load");
    _loadButton->setOutlined(true);

    _compareButton = new GUI_IconButton("", this, 20);
    _compareButton->setText("Compare");
    _compareButton->setOutlined(true),

    _compareMedia = new GUI_Label(this, "");

    GUI_HLayout* lBtnLayout = new GUI_HLayout(this);
    lBtnLayout->addSpacer(80);
    lBtnLayout->addItem(_loadButton);
    lBtnLayout->addSpacer(40);
    lBtnLayout->addItem(_compareButton);
    lBtnLayout->addItem(_compareMedia);

    _layout->addItem(lBtnLayout);


    connect(
        tlMenu,
        &GUI_MenuPane::menuItemSelected,
        this,
        &GUI_ItemLoader::onSelectedTrackChanged
    );

    connect(
        pbMenu,
        &GUI_MenuPane::menuItemSelected,
        this,
        &GUI_ItemLoader::onSelectedPosChanged
    );

    connect(
        _loadButton,
        &GUI_IconButton::buttonPressed,
        this,
        &GUI_ItemLoader::onLoadRequested
    );

    connect(
        _compareButton,
        &GUI_IconButton::buttonPressed,
        this,
        &GUI_ItemLoader::onCompareRequested
    );

    CNTRL_MainController* cntrl = qobject_cast<CNTRL_MainController*>(
        CNTRL_MainController::instance()
    );

    connect(
        cntrl->getMediaManager(),
        SIGNAL(itemSelected(QVariantMap)),
        this,
        SLOT(onItemSelectionChanged(QVariantMap))
    );

    initTrackList();

}


void
GUI_ItemLoader::onParentSizeChanged(qreal width, qreal height)
{
    RLP_LOG_DEBUG(width << height)

    setWidth(width);
    setHeight(height);
}


void
GUI_ItemLoader::initTrackList()
{
    // RLP_LOG_DEBUG("")

    CNTRL_MainController* cntrl = qobject_cast<CNTRL_MainController*>(
        CNTRL_MainController::instance()
    );

    DS_SessionPtr session = cntrl->session();
    QList<DS_TrackPtr> currTrackList = session->trackList();

    _trackListButton->menu()->clear();

    if (currTrackList.size() == 0)
    {
        return;
    }


    for (int i=0; i != currTrackList.size(); ++i)
    {
        DS_TrackPtr track = currTrackList.at(i);

        if (track->trackType() != TRACK_TYPE_MEDIA)
        {
            continue;
        }


        QString trackName = track->name();
        // trackListBtn->setText(trackName);

        RLP_LOG_DEBUG("ADDING TRACK:" << trackName)

        GUI_MenuItem* trackItem = _trackListButton->addAction(trackName);
        trackItem->insertDataValue("track_name", trackName);
        trackItem->insertDataValue("track_idx", track->index());
    }

    _trackListButton->setText(currTrackList.at(0)->name());

    _btnLayout->onItemSizeChanged();
}



void
GUI_ItemLoader::onSelectedTrackChanged(QVariantMap sigData)
{
    QString itemName = sigData.value("text").toString();
    QVariantMap itemData = sigData.value("data").toMap();
    RLP_LOG_DEBUG(itemName << " " << itemData)

    _trackListButton->setText(itemName);

    _currTrackInfo = itemData;
}


void
GUI_ItemLoader::onSelectedPosChanged(QVariantMap sigData)
{
    // RLP_LOG_DEBUG(itemName << " " << itemData)
    QString itemName = sigData.value("text").toString();

    _posButton->setText(itemName);
}


void
GUI_ItemLoader::onItemSelectionChanged(QVariantMap mediaInfoIn)
{
    RLP_LOG_DEBUG(mediaInfoIn)

    _currMediaSelectedInfo = mediaInfoIn;

    DS_NodePtr selNode = _currMediaSelectedInfo.value("node").value<DS_NodePtr>();
    if (selNode == nullptr)
    {
        return;
    }

    QString mediaName = selNode->getProperty<QString>("media.name");

    RLP_LOG_DEBUG("GOT MEDIA NAME: " << mediaName)
    _compareMedia->setText(mediaName);
}


void
GUI_ItemLoader::onLoadRequested(QVariantMap /* metadata */)
{
    RLP_LOG_DEBUG("")

    //QString path = _loadPath; // selectedPath();
    //if (path == "")
    //{
     //   return;
    //}

    // RLP_LOG_DEBUG(_loadItem)

    QVariantList mediaInputList;
    mediaInputList.append(_loadItem);
    
    int currTrackIdx = _currTrackInfo.value("track_idx").toInt();

    bool frameOk;
    bool doClear = true;

    CNTRL_MainController* cntrl = qobject_cast<CNTRL_MainController*>(
        CNTRL_MainController::instance()
    );
    
    int loadFrame = -1;
    if (_posLabel->isToggled())
    {
        QString posValue =_posButton->text();
        if (posValue == "Start")
        {
            loadFrame = 1;
            frameOk = true;
        }
        else if (posValue == "End")
        {
            DS_TrackPtr track = cntrl->session()->getTrackByIndex(currTrackIdx);
            QVariantList fl = track->getFrames();

            qlonglong maxFrame = 0;
            for (auto it : fl)
            {
                qlonglong nf = it.toLongLong();
                if (nf > maxFrame)
                {
                    maxFrame = nf;
                }
            }

            loadFrame = maxFrame + 1;
            frameOk = true;

            doClear = false;

        }
        else if (posValue == "Curr Frame")
        {
            loadFrame = cntrl->getVideoManager()->currentFrameNum();
            frameOk = true;
        }
    }
    else
    {
        loadFrame = _frameEdit->text().toInt(&frameOk);
        doClear = false;
    }

    if (!frameOk)
    {
        RLP_LOG_ERROR("Could not read input frame, aborting load")

        return;
    }

    QVariantList argList;
    argList.append(QVariant(mediaInputList));
    argList.append(currTrackIdx);
    argList.append(loadFrame);
    argList.append(doClear);

    PY_Interp::call("revlens.media.load_media_list", argList);

    emit loadComplete();
}


void
GUI_ItemLoader::onCompareRequested(QVariantMap /* metadata */)
{
    RLP_LOG_DEBUG("")

    //QString mediaInput = _loadPath; // selectedPath();
    //if (mediaInput == "")
    //{
    //    return;
    //}

    RLP_LOG_DEBUG(_currMediaSelectedInfo)

    if (! _currMediaSelectedInfo.contains("node"))
    {
        RLP_LOG_ERROR("Nothing selected, cannot run compare")
        return;
    }
    DS_NodePtr selNode = _currMediaSelectedInfo.value("node").value<DS_NodePtr>();
    if (selNode == nullptr)
    {
        return;
    }


    QVariantList argList;
    argList.append(_loadItem);
    argList.append(selNode->graph()->uuid().toString());

    // QVariantMap callInfo;
    // callInfo.insert("method", "revlens.media.append_to_composite");
    // callInfo.insert("args", argList); // append(QList) extends list

    RLP_LOG_DEBUG(argList)

    CNTRL_MainController* cntrl = qobject_cast<CNTRL_MainController*>(
        CNTRL_MainController::instance()
    );
    
    PY_Interp::call("revlens.media.append_to_composite", argList);

    emit loadComplete();

}


QRectF
GUI_ItemLoader::boundingRect() const
{
    return parentItem()->boundingRect();
}


void
GUI_ItemLoader::paint(QPainter *painter)
{
}