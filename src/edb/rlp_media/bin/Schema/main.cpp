

#include <iostream>

#include <QtCore/QDebug>

#include "RlpEdb/DS/DataSource.h"
#include "RlpEdb/DS/FieldType.h"
#include "RlpEdb/DS/EntityType.h"

#include "RlpEdb/STOR_SQL/StorageManager.h"

int main(int argc, char** argv)
{
    // std::cout << "hello world" << std::endl;
    STOR_SQL_StorageManager mgr("test.db");
    
    DS_DataSource dsrc("rlmedia", "media_container");
    dsrc.bindStorage(&mgr);
    
    // Root
    //
    DS_EntityType t_mediaContainer(
        "MediaContainer",
        &dsrc
    );
    
    DS_EntityType t_mediaHandle("MediaHandle", &dsrc);
    t_mediaHandle.addField(
        "type",
        "Media Handle Type",
        "str"
    );
    
    t_mediaHandle.addField(
        "description",
        "Description",
        "str"
    );
    
    t_mediaHandle.addField(
        "container",
        "Media Container Root",
        "entity"
    );
    
    
    // DS_EntityType video("Video", &dsrc);
    
    
    
    
    DS_EntityType t_streamBase("Stream", &dsrc);
    t_streamBase.addField(
        "codec_info",
        "Codec Info",
        "CodecInfo"
    );
    
    t_streamBase.addField(
        "codec_settings",
        "Codec Settings",
        "CodecSettings"
    );
    
    t_streamBase.addField(
        "handle",
        "Media Handle",
        "MediaHandle"
    );
    
    
    DS_EntityType t_videoStream(
        "VideoStream",
        &dsrc,
        &t_streamBase
    );
    
    t_videoStream.addField(
        "frame_width",
        "Frame Resolution Width",
        "int"
    );
    
    
    t_videoStream.addField(
        "frame_height",
        "Frame Resolution Height",
        "int"
    );
    
    
    
    // --------
    
    
    
    DS_Entity* mc = new DS_Entity(
        &t_mediaContainer,
        "010_010_anim_playblast_v001"
    );
    
    
    DS_Entity* mh = new DS_Entity(
        &t_mediaHandle,
        "av_track_main"
    );
    
    
    mh->setFieldValue("container", mc);
    
    
    DS_Entity* vs = new DS_Entity(
        &t_videoStream,
        "video_main"
    );
    
    vs->setFieldValue("handle", mh);
    vs->setFieldValue("frame_width", 1920);
    vs->setFieldValue("frame_height", 1080);
    
    
    /*

    // DS_FieldType tName("Name", stringAllowed, DS_FieldType::NVAL_SINGLE);
    DS_FieldType tName(
        "name",
        "Name",
        DS_FieldType::FT_TYPE_STR);

    QVariantList fieldData;
    QVariant v;
    v.setValue(tName),
    fieldData.append(v);
    
    
    
    std::cout << et.name().toStdString().c_str() << std::endl;
    std::cout << et.field("name").display_name().toStdString().c_str() << std::endl;
    
    
    */
    
    
    
    // dsrc.flush();
}