#ifndef EXTREVLENS_RLQTDISP_PREVIEW_VIEWBASE_H
#define EXTREVLENS_RLQTDISP_PREVIEW_VIEWBASE_H


#include "RlpExtrevlens/RLQTDISP/Global.h"

#include "RlpGui/BASE/ItemBase.h"

class EXTREVLENS_RLQTDISP_API RLQTDISP_PreviewViewBase : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTDISP_PreviewViewBase(GUI_ItemBase* parent);

    void setImage(QImage image);

protected:

    QImage* _image;
    bool _loading;
};

#endif

