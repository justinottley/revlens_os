
#ifndef EXTREVLENS_RLANN_CNTRL_DRAW_COMMANDS_H
#define EXTREVLENS_RLANN_CNTRL_DRAW_COMMANDS_H

#include "RlpRevlens/CNTRL/Commands.h"


class RLANN_CNTRL_AnnotateCommand : public CNTRL_RevlensAppCommand {

public:
    RLANN_CNTRL_AnnotateCommand(QString name, CNTRL_MainController* cntrl):
        CNTRL_RevlensAppCommand(name, "annotate", cntrl)
    {
    }
};

#endif
