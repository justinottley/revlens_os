
//
// DEPRECATED
//

RLP_SETUP_EXT_LOGGER(RLQTDISP, CompositeTools)


RLQTDISP_CompositeTools::RLQTDISP_CompositeTools(DISP_GLView* parent):
    GUI_ItemBase(parent),
    _display(parent)
{
    setOpacity(0.75);

    _modeButton = new GUI_IconButton(":misc/layer.png", this, 15);

    connect(
        _modeButton,
        &GUI_IconButton::menuShown,
        this,
        &RLQTDISP_CompositeTools::onModeMenuShown
    );

    //_modeButton->setColour(QColor(120, 120, 120));
    _modeButton->addAction("Tile");
    _modeButton->addAction("Wipe");
    _modeButton->addAction("Blend");

    _modeButton->setText("Tile");
    _modeButton->setTextHOffset(4);
    _modeButton->setTextYOffset(2);
    _modeButton->setOutlined(true);

    connect(
        qobject_cast<GUI_MenuPane*>(_modeButton->menu()),
        &GUI_MenuPane::menuItemSelected,
        this,
        &RLQTDISP_CompositeTools::onModeMenuItemSelected
    );
}


void
RLQTDISP_CompositeTools::setEnabled(bool enabled)
{
    RLP_LOG_DEBUG(enabled)

    _modeButton->setVisible(enabled);
}


void
RLQTDISP_CompositeTools::onModeMenuShown(QVariantMap md)
{
    // RLP_LOG_DEBUG(md)
    QPointF mbpos = mapToScene(_modeButton->pos());
    _modeButton->menu()->setPos(
        QPointF(
            mbpos.x(),
            _modeButton->menu()->pos().y()
        )
    );
}


void
RLQTDISP_CompositeTools::onModeMenuItemSelected(QVariantMap itemInfo)
{
    QString itemName = itemInfo.value("text").toString();

    // RLP_LOG_DEBUG(itemName)

    _modeButton->setText(itemName);

    DS_FrameBufferPtr fbuf = _display->frameBuffer();
    QVariantMap* bm = fbuf->getBufferMetadata();

    if (!bm->contains("node.composite.idx"))
    {
        return;
    }    

    QString nodeUuidStr = bm->value("graph.uuid").toString();

    RLP_LOG_DEBUG("Switching Composite Mode to:" << itemName << "for node:" << nodeUuidStr)

    CNTRL_MainController* controller = CNTRL_MainController::instance();
    DS_NodePtr node = controller->session()->getNodeByUuid(nodeUuidStr);
    node->setProperty("display.composite_mode", itemName);


    // need to eject everything from the lookahead to recache the metadata
    // into the framebuffers
    CNTRL_Video* cv = controller->getVideoManager();

    qlonglong cframe = cv->currentFrameNum();

    RLP_LOG_DEBUG("Current frame:" << cframe)

    cv->clearAll();
    cv->updateToFrame(cframe, true);

    emit compositeModeChanged(itemName);
}


QRectF
RLQTDISP_CompositeTools::boundingRect() const
{
    return _modeButton->boundingRect();
}


void
RLQTDISP_CompositeTools::paint(GUI_Painter* painter)
{
}

