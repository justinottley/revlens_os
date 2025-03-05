//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DISP_GLPAINTENGINE_H
#define REVLENS_DISP_GLPAINTENGINE_H

#include "RlpRevlens/DISP/Global.h"

#include "RlpRevlens/DS/Buffer.h"
#include "RlpRevlens/DS/FrameBuffer.h"


class REVLENS_DISP_API DISP_GLPaintEngine : public QObject {
    Q_OBJECT
    
public:
    
    DISP_GLPaintEngine(QString name);
    
    virtual void initializeGLInternal(DS_BufferPtr buf) {}
    virtual void paintGLInternal(DS_BufferPtr buf) {}
    virtual void resizeGLInternal(int width, int height) {}
    
    virtual void setSize(int width, int height) {}
    
public slots:
    
    QString name() { return _name; }

    virtual QPointF getPanPercent() { return QPointF(0, 0); }
    virtual float getZoom() { return 1.0; }
    virtual void setScreenRect(QRect rect) { _srect = rect; }

protected:
    
    QString _name;
    QRect _srect;
};

#endif