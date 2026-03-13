
#include "RlpEdb/MEDIA/Schema.h"

RLP_SETUP_LOGGER(edb, EdbMedia, Schema)


EdbMedia_Schema::EdbMedia_Schema(EdbDs_StorageManagerBase* storage):
    _storage(storage)
{
    _dsrc = new EdbDs_DataSource("rlmedia", "rlp_media_v1");
    _dsrc->bindStorage(_storage);

    initSchema();
    initData();
}


void
EdbMedia_Schema::initSchema()
{
    RLP_LOG_DEBUG("")

    _StreamType = new EdbDs_EntityType("Stream", _dsrc);
    _StreamType->addField("codec", "Codec", "str");
    _StreamType->addField("stream_type", "Stream Type", "str");

    _VideoStreamType = new EdbDs_EntityType("VideoStream", _dsrc, _StreamType);
    _VideoStreamType->addField("width", "width", "int");
    _VideoStreamType->addField("height", "height", "int");
    _VideoStreamType->addField("channel_count", "Channel Count", "int");
    _VideoStreamType->addField("pixel_size", "Pixel Size", "int");
    _VideoStreamType->addField("channel_order", "Channel Order", "int"); // enum
    _VideoStreamType->addField("colourspace",  "Colour Space", "int"); // enum
    _VideoStreamType->addField("start_frame", "Start Frame", "longint");
    _VideoStreamType->addField("end_frame", "End Frame", "longint");
    _VideoStreamType->addField("frame_rate", "Frame Rate", "float");

    _FrameInfoType = new EdbDs_EntityType("FrameInfo", _dsrc);
    _FrameInfoType->addField("stream", "Stream", "entity");
    _FrameInfoType->addField("frame_num", "Frame Number", "longint");
    _FrameInfoType->addField("data", "Frame Data", "binary");
    _FrameInfoType->addField("byte_count", "Byte Count", "int");

}


void
EdbMedia_Schema::initData()
{
    RLP_LOG_DEBUG("")
}