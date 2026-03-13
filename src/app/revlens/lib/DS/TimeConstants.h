//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_VIDEO_TIMECONSTANTS_H
#define REVLENS_VIDEO_TIMECONSTANTS_H

#define NANOSEC_SCALE 1000000000
#define MICROSEC_SCALE 1000000

#ifdef SCAFFOLD_WASM
#define MILLISEC_SCALE 1000000
#else
#define MILLISEC_SCALE 1000
#endif

// NOTE: Using a nanosleep implementation on Windows now at nanosec scale.
//       Uncomment for microsec scale (not that accurate)
//
#if 0 // _WIN32
    #define TIME_SEC_SCALE MICROSEC_SCALE
    #define DRIFT_THRESHOLD_1 1200
    #define DRIFT_THRESHOLD_2 3000
    #define DRIFT_THRESHOLD 1500
#else
    #define TIME_SEC_SCALE NANOSEC_SCALE
    #define DRIFT_THRESHOLD 10000
#endif


#endif