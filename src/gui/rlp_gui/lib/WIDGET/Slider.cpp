

#include "RlpGui/WIDGET/Slider.h"
#include "RlpGui/BASE/Style.h"

RLP_SETUP_LOGGER(gui, GUI, Slider)

GUI_Slider::GUI_Slider(GUI_ItemBase* parent, GUI_Slider::Orientation orientation, int length, int offset):
    GUI_ItemBase(parent),
    _orientation(orientation),
    _offset(offset),
    _length(length - (offset * 2)),
    _chover(false),
    _lhover(false),
    _lxpos(0),
    _mpos(0),
    _fgCol(QColor(250, 150, 150)),
    _bgCol(GUI_Style::BgDarkGrey)
{
    setAcceptHoverEvents(true);

    #ifdef SCAFFOLD_IOS
    _lhover = true;
    _chover = true;
    #endif

    if (orientation == GUI_Slider::O_SL_HORIZONTAL)
    {
        setWidth(length);
        setHeight(40);
    } else
    {
        setWidth(40);
        setHeight(length);
    }
    
    setValue(0);
}


int
GUI_Slider::getXYPos()
{
    // TODO FIXME: !! DRAGONS !!
    // this calculation seems to depend on how the widget is setup (parented)
    // Can this be improved to be more deterministic???

    int xypos = -1;
    if (_orientation == GUI_Slider::O_SL_HORIZONTAL)
        xypos = x();

    else
        xypos = y();

    return xypos;
}


float
GUI_Slider::getValue(int pos)
{
    float result = (float)(pos - getXYPos() - _offset) / (float)_length;
    if (result < 0)
        result = 0;

    if (result > 1)
        result = 1;

    if (_orientation == GUI_Slider::O_SL_VERTICAL)
        result = 1 - result;

    return result;
}


void
GUI_Slider::setValue(int pos)
{
    _pos = getValue(pos);
    _circpos = pos - getXYPos();
    if (_circpos < _offset)
    {
        _circpos = _offset;
    } else if (_circpos > (_length + _offset))
    {
        _circpos = _length + _offset;
    }

    // RLP_LOG_DEBUG("pos:" << pos << "_pos:" << _pos << "_circpos:" << _circpos << "xypos:" << getXYPos())
    update();
}


void
GUI_Slider::setSliderValue(float val)
{
    RLP_LOG_DEBUG(val)

    _pos = val;
    _circpos = (val * _length) + _offset;
}


void
GUI_Slider::mousePressEvent(QMouseEvent* event)
{
    event->accept();

    if (!_lhover)
    {
        return;
    }

    if (_orientation == GUI_Slider::O_SL_HORIZONTAL)
    {
        setValue(event->position().x() + x());
    }
    else if (_orientation == GUI_Slider::O_SL_VERTICAL)
    {
        setValue(event->position().y());
    }

    emit startMove(_pos);
}


void
GUI_Slider::mouseMoveEvent(QMouseEvent* event)
{
    event->accept();

    if (!_chover)
    {
        return;
    }

    if (_orientation == GUI_Slider::O_SL_HORIZONTAL)
    {
        if (event->position().x() >= INT_MAX)
        {
            // RLP_LOG_ERROR("invalid x pos")
            return;
        }
        setValue(event->position().x() + x());
    }
    else if (_orientation == GUI_Slider::O_SL_VERTICAL)
    {
        if (event->position().y() >= INT_MAX)
        {
            // RLP_LOG_ERROR("invalid x pos")
            return;
        }
        setValue(event->position().y());
    }

    emit moving(_pos);

    return;
}


void
GUI_Slider::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();

    if (_orientation == GUI_Slider::O_SL_HORIZONTAL)
    {
        setValue(event->position().x() + x());
    }
    else if (_orientation == GUI_Slider::O_SL_VERTICAL)
    {
        setValue(event->position().y());
    }

    emit endMove(_pos);
}


void
GUI_Slider::hoverMoveEvent(QHoverEvent* event)
{
    #ifdef SCAFFOLD_IOS
    return true;
    #endif

    // RLP_LOG_DEBUG(event)

    // _display->setCursor(Qt::CrossCursor);

    int xpos = event->position().x();
    int ypos = event->position().y();

    // RLP_LOG_DEBUG(ypos << _hmid)

    if (_orientation == GUI_Slider::O_SL_HORIZONTAL)
    {
        int hmid = height() / 2;

        if ((ypos > hmid - 10) && (ypos < hmid + 10) && 
            (xpos > _circpos - 10) && (xpos < _circpos + 10))
        {
            _chover = true;
            _lhover = false;
        } else
        {
            _chover = false;
        }

        bool lhover = _lhover;

        if (!_chover)
        {
            
            if ((ypos > hmid - 6) && (ypos < hmid + 6) &&
                (xpos > _offset) && (xpos < (_length + _offset)))
            {
                _lhover = true;
                // _chover = true;
                _lxpos = xpos;
                _mpos = getValue(xpos);

            } else
            {
                _lhover = false;
            }
        }
    } else
    {
        // vertical

        int hmid = width() / 2;

        if ((xpos > hmid - 10) && (xpos < hmid + 10) && 
            (ypos > _circpos - 10) && (ypos < _circpos + 10))
        {
            _chover = true;
            _lhover = false;
        } else
        {
            _chover = false;
        }

        bool lhover = _lhover;

        if (!_chover)
        {
            
            if ((xpos > hmid - 6) && (xpos < hmid + 6) &&
                (ypos > _offset) && (ypos < (_length + _offset)))
            {
                _lhover = true;
                // _chover = true;
                _lxpos = xpos;
                _mpos = getValue(ypos);

            } else
            {
                _lhover = false;
            }
        }
    }
    
    update();
}


void
GUI_Slider::paint(GUI_Painter* painter)
{
    // float pos = (float)_currframe / (float)_framecount;
    // int wmax = wx - 70;

    // RLP_LOG_DEBUG(_currframe << _framecount << pos << _circpos)

    painter->save();

    // int w = width();
    int midh = height() / 2;
    // int hmid = width() - 100;
    // _wmax = wmax;

    QPen bp(_bgCol);
    QBrush bb(_bgCol);

    //QRectF bgr(10, _hmid - 20, wx - 20, 40);

    painter->setPen(bp);
    painter->setBrush(bb);
    // painter->setOpacity(0.6);
    painter->drawRect(boundingRect());

    QPen p(QColor(220, 220, 220));
    // QRectF circ(_circpos, _hmid - 5, 10.0, 10.0);

    QBrush wb(QColor(220, 220, 220));

    p.setWidth(2);
    painter->setPen(p);
    painter->setBrush(wb);

    // painter->setOpacity(1.0);

    if (_lhover)
    {
        painter->setPen(_fgCol);
        painter->setBrush(QBrush(_fgCol));
    } else {
        painter->setPen(p);
        painter->setBrush(wb);
    }

    int lineXstart = _offset;
    int lineXend = width() - _offset;
    int lineYstart = midh;
    int lineYend = midh;

    if (_orientation == GUI_Slider::O_SL_VERTICAL)
    {
        lineXstart = width() / 2;
        lineYstart = _offset;
        lineXend = lineXstart;
        lineYend = height() - _offset;
    }

    // RLP_LOG_DEBUG("line:" << lineXstart << "-" << lineXend  << "offset:" << _offset)

    painter->drawLine(lineXstart, lineYstart, lineXend, lineYend);

    if (_chover)
    {
        painter->setPen(_fgCol);
        painter->setBrush(QBrush(_fgCol));
    } else {
        painter->setPen(p);
        painter->setBrush(wb);
    }

    int circXpos = _circpos;
    int circYpos = midh;
    int textXpos = _offset;
    int textYpos = midh + 17;

    if (_orientation == GUI_Slider::O_SL_VERTICAL)
    {
        circXpos = width() / 2;
        circYpos = _circpos;
        textXpos = 2;
        textYpos = 10;
    }

    // painter->drawEllipse(_circpos, midh, 5, 5);
    painter->drawEllipse(circXpos, circYpos, 5, 5);

    QString ctext;
    if (_lhover)
    {
        ctext = QString("%1").arg(_mpos);
    } else
    {
        ctext = QString("%1").arg(_pos); // hframe);
    }

    painter->drawText(textXpos, textYpos, ctext);
    painter->restore();
}