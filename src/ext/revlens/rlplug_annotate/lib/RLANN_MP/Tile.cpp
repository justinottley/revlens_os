

#include "RlpExtrevlens/RLANN_MP/Tile.h"

RLP_SETUP_LOGGER(ext, RLANN_MP, Tile)

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
RLANN_MP_Tile::updateImageFromTile(QImage* imgBuf)
{

    int bbl = imgBuf->bytesPerLine();
    int depth = imgBuf->depth() / 8;
    int iw = imgBuf->width();
    int ih = imgBuf->height();

    // QRgb* dst = (QRgb*)_cacheImg.bits();

    for (int y = 0 ; y < k_tile_dim ; y++)
    {
        if ((_pos.y() + y) >= ih)
        {
            continue;
        }

        for (int x = 0 ; x < k_tile_dim ; x++)
        {
            if ((_pos.x() + x) >= iw)
            {
                continue;
            }

            QRgb* dst = (QRgb*)(imgBuf->bits() + (((_pos.y() + y) * bbl) + ((_pos.x() + x) * depth)));
            uint16_t alpha = _pixels[y][x][k_alpha];

            if (alpha)
            {
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
RLANN_MP_Tile::updateTileFromImage(QImage* img, int alphaValue)
{
    // RLP_LOG_DEBUG(_pos << img->width() << img->height())

    int bbl = img->bytesPerLine();
    int depth = img->depth() / 8;
    int iw = img->width();
    int ih = img->height();

    for (int y = 0; y < k_tile_dim; y++)
    {
        if ((_pos.y() + y) >= ih)
        {
            continue;
        }

        for (int x = 0; x < k_tile_dim; x++)
        {
            if ((_pos.x() + x) >= iw)
            {
                continue;
            }

            QRgb* dst = (QRgb*)(img->bits() + (((_pos.y() + y) * bbl) + ((_pos.x() + x) * depth)));

            _pixels[y][x][k_red] = CONV_8_16(qBlue(*dst));
            _pixels[y][x][k_green] = CONV_8_16(qGreen(*dst));
            _pixels[y][x][k_blue] = CONV_8_16(qRed(*dst));
            _pixels[y][x][k_alpha] = alphaValue;  //  1; // CONV_8_16(qAlpha(*dst));

         }
    }
}


void
RLANN_MP_Tile::clear()
{
    memset(_pixels, 0, sizeof(_pixels)); // Tile is transparent
}