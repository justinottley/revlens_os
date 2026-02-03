//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_BASE_STYLE_H
#define GUI_BASE_STYLE_H

#include "RlpGui/BASE/Global.h"

#ifdef SCAFFOLD_IOS
static const int DEFAULT_ICON_SIZE = 20;
#else
static const int DEFAULT_ICON_SIZE = 15;
#endif

class GUI_BASE_API GUI_Style {

public:
    static QColor BgDarkGrey;
    static QColor BgLoMidGrey;
    static QColor BgMidGrey;
    static QColor BgLightGrey;
    static QColor BgGrey;

    static QColor BgRowEven;
    static QColor BgRowOdd;

    static QColor FgGrey;

    static QColor PaneBg;
    static QColor TabBg;

    static QColor IconBg;
    static QColor IconFg;

    static QColor TrackHeaderFg;
    static QColor TrackHeaderIconBg;
    static QColor TrackMediaBg;

    static QColor Highlight;

    static int FontPixelSizeDefault;

    static void init();

};

#endif