
#ifndef REVLENS_GUI_TL2_TRACK_TOGGLEBUTTON_IMAGE_H
#define REVLENS_GUI_TL2_TRACK_TOGGLEBUTTON_IMAGE_H

#include "RlpRevlens/GUI_TL2/Global.h"

#include "RlpGui/BASE/SvgIcon.h"

#include "RlpRevlens/GUI_TL2/TrackToggleButton.h"

class GUI_TL2_Track;

class REVLENS_GUI_TL2_API GUI_TL2_TrackToggleButtonSvg : public GUI_TL2_TrackToggleButton {
    Q_OBJECT

public:

    GUI_TL2_TrackToggleButtonSvg(GUI_ItemBase* parent, QString imagePath);

    void paintOn(GUI_Painter* painter);
    void paintOff(GUI_Painter* painter);


public slots:

    void setBgColOn(QColor col) { _colBgOn = col; }

protected:
    QString _imagePath;

    GUI_SvgIcon* _svgIcon;
    // GUI_SvgIcon* _svgOff;


    QString _rootPath;

};

#endif
