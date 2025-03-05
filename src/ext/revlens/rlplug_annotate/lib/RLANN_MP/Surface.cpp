
#include "mypaint-tiled-surface.h"
#include "mypaint-brush.h"
#include "mypaint-surface.h"

#include "RlpExtrevlens/RLANN_MP/Surface.h"


RLP_SETUP_LOGGER(ext, RLANN_MP, Surface)

static void cb_mypaint_on_tile_request_start(MyPaintTiledSurface *tiled_surface, MyPaintTileRequest* request)
{
    // qInfo() << "MP_Handler: cb_mypaint_on_tile_request_start()";

    RLANN_MP_Surface* h = (RLANN_MP_Surface*)tiled_surface; // RLANN_MP_Surface::instance();

    const int tx = request->tx;
    const int ty = request->ty;
    uint16_t* tile_pointer = NULL;

    if (tx >= h->tilesWidth() || ty >= h->tilesHeight() || tx < 0 || ty < 0)
    {
        // Give it a tile which we will ignore writes to
        qInfo() << "MP_Handler: cb_mypaint_on_tile_request_start(): NULL TILE" << tx << ty << "tiles:" << h->tilesWidth() << h->tilesHeight();
        tile_pointer = h->NULL_TILE;
    } else
    {
        RLANN_MP_Tile* tile = h->getOrInitTileFromIndex(QPoint(tx, ty));
        uint16_t* tbits = tile->bits();

        if (tbits) {

            // qInfo() << "MP_Handler: cb_mypaint_on_tile_request_start(): TILE OK" << tx << ty << "tiles:" << h->tilesWidth() << h->tilesHeight();

            tile_pointer = tbits;
        }
    }

    request->buffer = tile_pointer;
}


static void cb_mypaint_on_tile_request_end(MyPaintTiledSurface* tiled_surface, MyPaintTileRequest* request)
{
    // qInfo() << "MP_Handler: cb_mypaint_on_tile_request_end()";

    RLANN_MP_Surface* h = (RLANN_MP_Surface*)tiled_surface; // RLANN_MP_Surface::instance();

    const int tx = request->tx;
    const int ty = request->ty;

    RLANN_MP_Tile* tile = h->getOrInitTileFromIndex(QPoint(tx, ty));
    if (tile)
    {
        // qInfo() << "MP_Handler: cb_mypaint_on_tile_request_end() got tile, calling updateCache";
        tile->updateCache(h->imgBuf());
    }

    // tile->update();
}


// ------------------------

RLANN_MP_Surface::RLANN_MP_Surface(QImage* imgBuf):
    _imgBuf(imgBuf)
{
    RLP_LOG_DEBUG("Initializing mypaint tiled surface size" << imgBuf->width() << imgBuf->height())

    setSize(imgBuf->width(), imgBuf->height());

    // _mpSurface = (MyPaintTiledSurface*)malloc(sizeof(MyPaintTiledSurface));

    mypaint_tiled_surface_init(
        (MyPaintTiledSurface*)this,
        cb_mypaint_on_tile_request_start,
        cb_mypaint_on_tile_request_end
    );

    tile_size = MYPAINT_TILE_SIZE * MYPAINT_TILE_SIZE * 4 * sizeof(uint16_t);
    NULL_TILE = (uint16_t *)malloc(tile_size);
}


QPoint
RLANN_MP_Surface::tilePos(const QPoint& idx)
{
    return QPoint(
        MYPAINT_TILE_SIZE * idx.x(),
        MYPAINT_TILE_SIZE * idx.y()
    );
}

QPoint
RLANN_MP_Surface::tileIndexFromPos(const QPoint& pos)
{
    return QPoint(
        pos.x() / MYPAINT_TILE_SIZE,
        pos.y() / MYPAINT_TILE_SIZE
    );
}


void
RLANN_MP_Surface::setSize(int width, int height)
{
    // RLP_LOG_DEBUG(width << height)

    const int tiles_width = ceil((float)width / MYPAINT_TILE_SIZE);
    const int tiles_height = ceil((float)height / MYPAINT_TILE_SIZE);

    assert(MYPAINT_TILE_SIZE * tiles_width >= width);
    assert(MYPAINT_TILE_SIZE * tiles_height >= height);

    _tilesWidth = tiles_width;
    _tilesHeight = tiles_height;
}


RLANN_MP_Tile* 
RLANN_MP_Surface::getTileByPos(const QPoint& pos)
{
    return getOrInitTileFromIndex(tileIndexFromPos(pos));
}


RLANN_MP_Tile*
RLANN_MP_Surface::getOrInitTileFromIndex(const QPoint& idx)
{
    // RLP_LOG_DEBUG(idx)

    RLANN_MP_Tile* selectedTile = nullptr;

    // Which tile index is it ?
    if (((int)idx.x() < k_max) && ((int)idx.y() < k_max)) { // out of range ?

        // Ok, valid index. Does it exist already ?
        selectedTile = _tiles.value(idx, NULL);

        if (!selectedTile) {
            // Time to allocate it, update table and insert it as a QGraphicsItem in scene:
            selectedTile = new RLANN_MP_Tile();
            _tiles.insert(idx, selectedTile);

            QPoint nTilePos ( tilePos(idx) );


            selectedTile->setPos(nTilePos);

            // RLP_LOG_DEBUG("new tile:" << nTilePos.x() << " " << nTilePos.y())
        }

        // RLP_LOG_DEBUG("Tile OK:" << selectedTile)
    }

    return selectedTile;
}

