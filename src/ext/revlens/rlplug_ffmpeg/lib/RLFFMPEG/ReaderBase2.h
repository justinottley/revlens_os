//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLFFMPEG_READER_BASE_H
#define EXTREVLENS_RLFFMPEG_READER_BASE_H

#include "RlpExtrevlens/RLFFMPEG/Global.h"

extern "C" {

#include <libavformat/avformat.h>

}



#undef av_err2str
#define av_err2str(errnum) \
av_make_error_string((char*)__builtin_alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)

struct RLFFMPEG_PacketReadResult {
    uint64_t dts_result;
    uint64_t pts_result;
    int nb_samples;
    int return_status;
    AVFrame* av_frame;
};


#endif
