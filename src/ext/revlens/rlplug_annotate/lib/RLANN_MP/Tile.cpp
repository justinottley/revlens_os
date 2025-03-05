

#include "RlpExtrevlens/RLANN_MP/Tile.h"

RLANN_MP_Tile::RLANN_MP_Tile()
{
    clear(); // initialize tile to transparent
}


uint16_t*
RLANN_MP_Tile::bits()
{
    return (uint16_t*)_pixels;
}


void
RLANN_MP_Tile::updateCache(QImage* imgBuf)
{
    int bbl = imgBuf->bytesPerLine();
    int depth = imgBuf->depth() / 8;

    // QRgb* dst = (QRgb*)_cacheImg.bits();

    for (int y = 0 ; y < k_tile_dim ; y++) {
        for (int x = 0 ; x < k_tile_dim ; x++) {
            
            QRgb* dst = (QRgb*)(imgBuf->bits() + (((_pos.y() + y) * bbl) + ((_pos.x() + x) * depth)));
            uint16_t alpha = _pixels[y][x][k_alpha];

            if (alpha) {
                *dst = qRgba(
                    CONV_16_8(_pixels[y][x][k_blue]),
                    CONV_16_8(_pixels[y][x][k_green]),
                    CONV_16_8(_pixels[y][x][k_red]),
                    CONV_16_8(alpha)
                );
            }
         }
    }
}


void
RLANN_MP_Tile::clear()
{
    memset(_pixels, 0, sizeof(_pixels)); // Tile is transparent
}