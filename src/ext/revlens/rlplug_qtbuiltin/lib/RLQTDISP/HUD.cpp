//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include <iostream>

#include "RlpExtrevlens/RLQTDISP/HUD.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/DISP/GLView.h"



RLP_SETUP_EXT_LOGGER(RLQTDISP, HUD)

RLQTDISP_HUD::RLQTDISP_HUD() :
    DS_EventPlugin(QString("HUD"), true),
    _lock(new QMutex()),
    _inPress(false)
{
    _hudConfig.clear();
}


DS_EventPlugin*
RLQTDISP_HUD::duplicate()
{
    // RLP_LOG_DEBUG("")

    RLQTDISP_HUD* childPlug = new RLQTDISP_HUD();
    childPlug->setMainController(_controller);
    childPlug->setHudConfig(_hudConfig);

    _childPlugins.append(childPlug);

    return childPlug;
}


bool
RLQTDISP_HUD::setMainController(CNTRL_MainController* controller)
{
    _controller = controller;

    connect(
        _controller->getVideoManager(),
        SIGNAL(displayDeregistered(DISP_GLView*)),
        this,
        SLOT(onDisplayDeregistered(DISP_GLView*))
    );

    return true;
}


void
RLQTDISP_HUD::onDisplayDeregistered(DISP_GLView* display)
{
    RLP_LOG_DEBUG(display)

    int cidx = -1;
    for (int ci=0; ci != _childPlugins.size(); ++ci)
    {
        if (_childPlugins.at(ci)->display() == display)
        {
            cidx = ci;
            break;
        }
    }

    if (cidx >= 0)
    {
        RLP_LOG_DEBUG("Reregistering child plugin at" << cidx)
        _childPlugins.takeAt(cidx);
    }
}


void
RLQTDISP_HUD::setHudConfig(QVariantMap hudConfig)
{
    QMutexLocker l(_lock);

    RLP_LOG_DEBUG("")

    _hudConfig = hudConfig;
}


void
RLQTDISP_HUD::setHudVisible(QString hudName, bool isVisible)
{
    RLP_LOG_DEBUG(hudName << isVisible)

    if (_childPlugins.size() > 0) // for multiple display support
    {
        for (int i=0; i != _childPlugins.size(); ++i)
        {
            qobject_cast<RLQTDISP_HUD*>(_childPlugins.at(i))->setHudVisible(hudName, isVisible);
        }
    }


    QMutexLocker l(_lock);

    QVariantMap hudInfo = getHudInfo(hudName);
    hudInfo.insert("visible", isVisible);

    _hudConfig.insert(hudName, hudInfo);

    if (_display != nullptr)
    {
        bool movable = hudInfo.value("movable").value<bool>();

        if (movable)
        {
            if (isVisible)
            {
                _currHud = hudName;
                _display->setSoloPluginByName(name());

            } else
            {
                _currHud.clear();
                _display->resetSoloPlugin(_name, true);

            }
        }

        if (_controller->getPlaybackState() == CNTRL_MainController::PAUSED)
        {
            _display->update();
        }
    }
}


bool
RLQTDISP_HUD::isHudVisible(QString hudName)
{
    QMutexLocker l(_lock);

    RLP_LOG_DEBUG(hudName)

    if (_hudConfig.contains(hudName))
    {

        QVariantMap hudInfo = _hudConfig.value(hudName).toMap();
        return hudInfo.value("visible").toBool();
    }

    return false;

}
bool
RLQTDISP_HUD::addHudConfig(QString hudName, QVariantMap hudConfig)
{
    QMutexLocker l(_lock);

    RLP_LOG_DEBUG(hudName)

    _hudConfig.insert(hudName, hudConfig);
    if (_childPlugins.size() > 0) // for multiple display support
    {
        for (int i=0; i != _childPlugins.size(); ++i)
        {
            qobject_cast<RLQTDISP_HUD*>(_childPlugins.at(i))->setHudConfig(_hudConfig);
        }
    }


    return true;
}


QString
RLQTDISP_HUD::getHudItemString(QVariantMap hudItem, const QVariantMap metadata)
{
    QString data_val = "";

    if (hudItem.contains("item.value"))
    {
        data_val = hudItem.value("item.value").value<QString>();
        return data_val;
    }

    QVariantMap hudMetadata = metadata;
    if (hudItem.contains("item.namespace"))
    {

        QString namespaceKey = hudItem.value("item.namespace").value<QString>();

        if (metadata.contains(namespaceKey))
        {
            hudMetadata = metadata.value(namespaceKey).value<QVariantMap>();            
        }
        
    }

    QString item_key = hudItem.value("item.key").value<QString>();
    QString item_type = hudItem.value("item.type").value<QString>();


    if (hudMetadata.contains(item_key))
    {

        if (item_type == "string")
        {
            data_val = hudMetadata.value(item_key).value<QString>();
        }

        else if (item_type == "float")
        {

            int float_precision = hudItem.value("item.precision").value<int>();

            float item_data_val = hudMetadata.value(item_key).value<float>();
            data_val.setNum(item_data_val, 'f', float_precision);
        }
        
        else if (item_type == "qlonglong")
        {

            qlonglong item_data_val = hudMetadata.value(item_key).value<qlonglong>();
            data_val.setNum(item_data_val);
        }
    } else
    {

        // BUILT IN KEYS
        if (item_key == QString("media.currframe"))
        {
            qlonglong currFrame = metadata.value("video.currframe").value<qlonglong>();
            
        }
    }


    if (hudItem.contains("item.value_suffix"))
    {
        QString val_suffix = hudItem.value("item.value_suffix").value<QString>();
        data_val += val_suffix;
    }

    return data_val;
}


//
// --------
//

QVariantMap
RLQTDISP_HUD::getHudInfo(QString hudName)
{
    return _hudConfig.value(hudName).toMap();
}


void
RLQTDISP_HUD::paintGLPostDraw(GUI_Painter* painter, const QVariantMap& metadata)
{
    if (!_enabled) {
        return;
    }

    if (_display == nullptr)
    {
        return;
    }

    if (_controller == nullptr)
    {
        RLP_LOG_WARN("controller is nullptr")
        return;
    }

    QMutexLocker l(_lock);

    // need to clip to display due to the way this is being drawn
    //
    QRegion displayWindow(
        0,
        0,
        metadata.value("window.width").toInt(),
        metadata.value("window.height").toInt()
    );


    painter->save();
    painter->setClipRegion(displayWindow);

    long currFrame = metadata.value("video.currframe").value<long>();

    DS_NodePtr currNode = _controller->session()->
                                       getNodeByFrame(currFrame);

    // need to make a copy to insert node metadata
    QVariantMap allMetadata = metadata;
    if (currNode != nullptr)
    {
        QVariantMap nodeMetadata = currNode->getProperties();
        allMetadata.insert(nodeMetadata);
    }

    // QVariantMap hudInfo = getHudInfo();

    QList<QString> hudNames = _hudConfig.keys();
    QList<QString>::iterator hudNameIt;

    for (hudNameIt = hudNames.begin(); hudNameIt != hudNames.end(); ++hudNameIt)
    {
        QVariantMap hudInfoData = _hudConfig.value(*hudNameIt).toMap();
        bool isVisible = hudInfoData.value("visible").toBool();;

        // RLP_LOG_DEBUG((*hudNameIt) << " visible: " << isVisible);

        if (!isVisible)
        {
            continue;
        }

        QVariantList currHud = hudInfoData.value("item_list").value<QVariantList>();

        painter->setOpacity(0.6);
        // painter->setPen(QColor(80, 80, 80));
        painter->setBrush(QBrush(QColor(40, 40, 40)));

        int rect_x = hudInfoData.value("rect.x").toInt();
        int rect_y = hudInfoData.value("rect.y").toInt();
        int rect_width = hudInfoData.value("rect.width").toInt();
        int rect_height = hudInfoData.value("rect.height").toInt();

        int pos_x_offset = hudInfoData.value("position.offset.x").toInt();
        int pos_y_offset = hudInfoData.value("position.offset.y").toInt();

        // RLP_LOG_DEBUG(hudInfoData)
        // RLP_LOG_DEBUG(rect_x << rect_y << rect_width << rect_height)

        painter->drawRect(
            rect_x + pos_x_offset,
            rect_y + pos_y_offset,
            rect_width,
            rect_height
        );


        // RLP_LOG_DEBUG("drawing hud text at " << pos_x << " " << pos_y << std::endl;

        // painter->setPen(QColor(200, 200, 200));
        painter->setBrush(QBrush(Qt::transparent));
        
        QVariantList::iterator i;
        for (i=currHud.begin(); i != currHud.end(); ++i)
        {

            QVariantMap hudItem = i->toMap();

            QString data_val = getHudItemString(hudItem, allMetadata);
            if (data_val == "")
            {
                continue;
            }


            // RLP_LOG_DEBUG(data_val.toStdString().c_str() << std::endl;
            QColor penColour(180, 180, 180);
            int font_size = 10;
            int pos_x = hudItem.value("position.x.value").value<int>() + pos_x_offset;
            int pos_y = hudItem.value("position.y.value").value<int>() + pos_y_offset;
            bool drawOutlined = false;
            bool dropShadow = false;

            if (hudItem.contains("position.x.relative_to"))
            {
                QString rel_val_x_key = hudItem.value("position.x.relative_to").value<QString>();
                if (allMetadata.contains(rel_val_x_key))
                {
                    int rel_val_x = metadata.value(rel_val_x_key).value<int>();
                    pos_x = rel_val_x + pos_x;
                }
            }


            if (hudItem.contains("position.y.relative_to"))
            {
                QString rel_val_y_key = hudItem.value("position.y.relative_to").value<QString>();
                if (allMetadata.contains(rel_val_y_key))
                {
                    int rel_val_y = metadata.value(rel_val_y_key).value<int>();
                    pos_y = rel_val_y + pos_y;
                }

            }


            if (hudItem.contains("font.size"))
            {
                font_size = hudItem.value("font.size").value<int>();
            }

            float opacity = 0.8;
            if (hudItem.contains("font.opacity"))
            {
                opacity = hudItem.value("font.opacity").value<float>();
            }

            if (hudItem.contains("font.colour"))
            {
                QVariantList colourInfo = hudItem.value("font.colour").value<QVariantList>();
                penColour = QColor(colourInfo[0].toInt(), colourInfo[1].toInt(), colourInfo[2].toInt());
            }

            if (hudItem.contains("text.outlined"))
            {
                drawOutlined = true;
            }

            if (hudItem.contains("text.dropshadow"))
            {
                dropShadow = true;
            }

            QFont f("Andale Mono");
            f.setStyleHint(QFont::Monospace);
            f.setPixelSize(font_size);

            // QPen p(QColor(255, 255, 255));
            // p.setBrush(penColour);
            // p.setWidth(3);
            painter->setOpacity(opacity);

            if (drawOutlined)
            {

                // painter->setPen(p);
                // painter->setFont(f);

                // QBrush textBrush(penColour);
                // QPainterPath ppath;
                // ppath.addText(pos_x, pos_y, f, data_val);

                // painter->strokePath(ppath, QPen(QColor(255,255,255)));
                // painter->fillPath(ppath, textBrush);
                // painter->setOpacity(opacity);
                
                // painter->drawPath(ppath);
                RLP_LOG_ERROR("Draw Outlined Broken!!")

            } else
            {

                // painter->setFont(f);

                if (dropShadow)
                {
                    QPen p(QColor(10, 10, 10));
                    painter->setPen(p);
                    painter->drawText(pos_x + 1, pos_y + 1, data_val);
                }

                QPen p(penColour);
                painter->setPen(p);
                
                painter->drawText(pos_x, pos_y, data_val);
            }
        }
    }

    painter->restore();

}


//
// ------
//

bool
RLQTDISP_HUD::mousePressEventInternal(QMouseEvent* event)
{
    RLP_LOG_DEBUG("");
    QMutexLocker l(_lock);

    _pressPos = event->position();
    _inPress = true;

    if (_currHud != "") {

        QVariantMap hudInfo = getHudInfo(_currHud);
        _pressPos -= QPointF(
            hudInfo.value("position.offset.x").toInt(),
            hudInfo.value("position.offset.y").toInt()
        );
    }
    
    return false;
}


bool
RLQTDISP_HUD::mouseMoveEventInternal(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("");

    if ((_inPress) && (_currHud != ""))
    {

        QVariantMap hudInfo = getHudInfo(_currHud);

        QPointF diffPos = event->pos() - _pressPos;

        hudInfo.insert("position.offset.x", diffPos.x());
        hudInfo.insert("position.offset.y", diffPos.y());

        _hudConfig.insert(_currHud, hudInfo);

        if (_controller->getPlaybackState() == CNTRL_MainController::PAUSED) {
            _display->update();
        }
    }

    return false;
}


bool
RLQTDISP_HUD::mouseReleaseEventInternal(QMouseEvent* event)
{
    RLP_LOG_DEBUG("");

    _inPress = false;
    _pressPos = QPoint(0, 0);

    return false;
}