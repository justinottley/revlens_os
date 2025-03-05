
#include <iostream>

#include "RlpCore/SEQ/Template.h"
#include "RlpCore/SEQ/Hint.h"
#include "RlpCore/SEQ/Init.h"

RLP_SETUP_CORE_LOGGER(SEQ, Init)

void
SEQ_Init::initAll()
{
    RLP_LOG_INFO("");
    
    SEQ_Template::initTemplateList();
    SEQ_FrameSignatureHint::initRegex();
}