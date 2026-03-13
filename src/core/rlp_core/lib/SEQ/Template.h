
#ifndef CORE_SEQ_TEMPLATE_H
#define CORE_SEQ_TEMPLATE_H

#include "RlpCore/SEQ/Global.h"

class SEQ_Template;
typedef std::shared_ptr<SEQ_Template> SEQ_TemplatePtr;

class CORE_SEQ_API SEQ_Template {
    
public:

    RLP_DEFINE_LOGGER
    
    
    static QStringList TEMPLATE_SEQ_LIST;
    static QString FSTRING;
    
    static void initTemplateList();
    
public:
    
    SEQ_Template(QString templateStr);
    SEQ_Template(QString prefix, QString suffix);
    
    static SEQ_TemplatePtr makeTemplate(QString templateStr);
    static SEQ_TemplatePtr makeTemplate(QString prefix, QString suffix);
    
    /*
     * template string formatted with input sentinel fstring, or default
     * fstring if none supplied
     */
    QString format(QString fstr = nullptr);
    
    QString prefix() { return _prefix; }
    QString suffix() { return _suffix; }
    
    // QString rawTemplateStr() { return _rawTemplateStr; }
    
private:
    
    void parseTemplateString(QString& templateStr);
    
private:
    
    
    QString _prefix; // part before sentinel
    QString _suffix; // part after sentinel
    QString _srcSentinel; // source sentinel string that was found - "{f}", "#", etc
    
    QString _template;
};

#endif
