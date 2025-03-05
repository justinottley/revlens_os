#ifndef EXTREVLENS_RLANN_MP_TILE_H
#define EXTREVLENS_RLANN_MP_TILE_H

#include "RlpExtrevlens/RLANN_MP/Global.h"


#define CONV_16_8(x) ((x*255)/(1<<15))
#define CONV_8_16(x) ((x*(1<<15))/255)


class EXTREVLENS_RLANN_MP_API RLANN_MP_Tile : public QObject {
    Q_OBJECT

public:
    RLANN_MP_Tile();

    enum { k_tile_dim = 64 };
    enum { k_red = 0, k_green = 1, k_blue = 2, k_alpha =3 }; // Index to access RGBA values in myPaint

    uint16_t* bits();
    void updateCache(QImage* imgBuf);
    void clear();

    void setPos(QPoint pos) { _pos = pos; }


private:

    uint16_t  _pixels[k_tile_dim][k_tile_dim][4];
    QPoint _pos;

};

#endif