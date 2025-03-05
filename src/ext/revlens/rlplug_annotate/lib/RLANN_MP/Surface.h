
#ifndef EXTREVLENS_RLANN_MP_SURFACE_H
#define EXTREVLENS_RLANN_MP_SURFACE_H

#include "RlpExtrevlens/RLANN_MP/Global.h"
#include "RlpExtrevlens/RLANN_MP/Tile.h"


// struct _MyPaintTiledSurface; // forward declaration

#include "mypaint-tiled-surface.h"

class EXTREVLENS_RLANN_MP_API RLANN_MP_Surface : public MyPaintTiledSurface {

public:
    RLP_DEFINE_LOGGER

    RLANN_MP_Surface(QImage* imgBuf);

    QImage* imgBuf() { return _imgBuf; }

    // -------------

    // uint16_t *TILE_BUFFER; // Stores tiles in a linear chunk of memory (16bpc RGBA)
    uint16_t *NULL_TILE; // Single tile that we hand out and ignore writes to
    // const size_t TILE_SIZE;

    enum { k_center = 50, k_max = 2*k_center};

    int tilesWidth() { return _tilesWidth; }
    int tilesHeight() { return _tilesHeight; }


    // _MyPaintTiledSurface* surface() { return _mpSurface; }
    void setSize(int width, int height);

    RLANN_MP_Tile* getTileByPos(const QPoint& pos);
    RLANN_MP_Tile* getOrInitTileFromIndex(const QPoint& idx);

    QPoint tilePos(const QPoint& idx);

    QPoint tileIndexFromPos(const QPoint& pos);


private:
    

    // _MyPaintTiledSurface* _mpSurface;

    int _width;
    int _height;

    int _tilesWidth;
    int _tilesHeight;

    QHash<QPoint, RLANN_MP_Tile*> _tiles;

    QImage* _imgBuf;

};

#endif