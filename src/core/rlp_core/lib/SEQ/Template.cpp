
#include <iostream>

#include "RlpCore/SEQ/Template.h"

QString SEQ_Template::FSTRING = "{f}";
QStringList SEQ_Template::TEMPLATE_SEQ_LIST;

RLP_SETUP_LOGGER(core, SEQ, Template)

SEQ_Template::SEQ_Template(QString templateStr):
    _srcSentinel("")
{
    // called in SEQ_Util::initAll();
    // SEQ_Template::initTemplateList();
    
    parseTemplateString(templateStr);
}


SEQ_Template::SEQ_Template(QString prefix, QString suffix):
    _prefix(prefix),
    _suffix(suffix),
    _srcSentinel("")
{
    // called in SEQ_Util::initAll()
    // initTemplateList();
    
    _template = _prefix + SEQ_Template::FSTRING + _suffix;
}


SEQ_TemplatePtr
SEQ_Template::makeTemplate(QString templateStr)
{
    return SEQ_TemplatePtr(new SEQ_Template(templateStr));
}


SEQ_TemplatePtr
SEQ_Template::makeTemplate(QString prefix, QString suffix)
{
    return SEQ_TemplatePtr(new SEQ_Template(prefix, suffix));
}


void
SEQ_Template::initTemplateList()
{
    RLP_LOG_DEBUG("initializing supported template string list");
    
    SEQ_Template::TEMPLATE_SEQ_LIST.clear();
    
    SEQ_Template::TEMPLATE_SEQ_LIST
        << "{f}"
        << "####"
        << "#"
        << "%04d";
    
}


void
SEQ_Template::parseTemplateString(QString& templateStr)
{
    // RLP_LOG_DEBUG(templateStr);

    QStringList::iterator it;

    for (it = SEQ_Template::TEMPLATE_SEQ_LIST.begin();
         it != SEQ_Template::TEMPLATE_SEQ_LIST.end();
         ++it)
    {

        int idx = templateStr.indexOf(*it);
        if (idx != -1)
        {
            
            // RLP_LOG_DEBUG("FOUND template string sentinel: " << (*it))
            
            _prefix = templateStr.mid(0, idx);
            _suffix = templateStr.mid(idx + (*it).size(), templateStr.size());
            _srcSentinel = (*it);
            
            // save the default template
            _template = _prefix + SEQ_Template::FSTRING + _suffix;
        }
     }
}


QString
SEQ_Template::format(QString fstr)
{
    // std::cout << "SEQ_Template::format()" << std::endl;
    
    if (fstr == nullptr)
    {
        return _template;
    }

    return _prefix + fstr + _suffix;
}