

#include "RlpCore/UTIL/Text.h"
#include <cmath>

RLP_SETUP_CORE_LOGGER(UTIL, Text)


int
UTIL_Text::getWidth(QString text, QString fontStr)
{
    
    QFont currFont = QApplication::font();
    if (fontStr.size() > 0)
    {
        RLP_LOG_DEBUG(text << fontStr)

        currFont = QFont();
        currFont.fromString(fontStr);
    }

    QFontMetrics fm(currFont);

    return fm.horizontalAdvance(text);
}


int
UTIL_Text::getTextWidthF(QString text, QFont f)
{
    QFontMetrics fm(f);
    return fm.horizontalAdvance(text);
}


QString
UTIL_Text::clipToWidth(QString text, int maxWidth)
{
    QFont currFont = QApplication::font();
    QFontMetrics fm(currFont);

    int avgWidth = fm.averageCharWidth();
    int numChars = text.length();

    int maxChars = floor((float)maxWidth / (float)avgWidth);

    QString result = text;
    if (numChars > maxChars)
    {
        result.truncate(maxChars);
    }

    // RLP_LOG_DEBUG("max chars:" << maxChars << " max width:" << maxWidth << "avg:" << avgWidth)


    return result;
}


QStringList
UTIL_Text::wrapToWidth(QString text, int maxLineWidth)
{
    // RLP_LOG_DEBUG(text << maxLineWidth)

    QFont currFont = QApplication::font();
    QFontMetrics fm(currFont);

    QStringList result;
    QString line;
    QStringList spacedList = text.split(" ");

    while (spacedList.size() > 0)
    {

        QString word = spacedList.at(0);
        QString lineWord = line + word;
        
        int wlen = fm.horizontalAdvance(word);
        int len = fm.horizontalAdvance(lineWord);

        if (wlen >= maxLineWidth)
        {
            result.append(line);
            spacedList.removeAt(0);
            line = "";
        }
        else if (len >= maxLineWidth)
        {
            result.append(line);
            line = "";
            
        } else {

            line += word;
            line += " ";

            spacedList.removeAt(0);
        }
    }

    result.append(line);

    return result;
}