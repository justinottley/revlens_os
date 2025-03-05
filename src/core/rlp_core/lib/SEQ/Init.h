
#ifndef CORE_SEQ_INIT_H
#define CORE_SEQ_INIT_H

#include "RlpCore/SEQ/Global.h"
#include "RlpCore/LOG/Logging.h"


// Includes for convenience in clients
//
#include "RlpCore/SEQ/Sequence.h"
#include "RlpCore/SEQ/Item.h"
#include "RlpCore/SEQ/Template.h"
#include "RlpCore/SEQ/Util.h"

class CORE_SEQ_API SEQ_Init {


public:
    RLP_DEFINE_LOGGER
    
    static void initAll();
};


#endif
