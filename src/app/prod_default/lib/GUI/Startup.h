
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef PROD_GUI_STARTUP_H
#define PROD_GUI_STARTUP_H

#include "RlpProd/GUI/Global.h"


class CNTRL_MainController;
class GUI_View;

class PROD_GUI_API GUI_Startup : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    
    GUI_Startup();

    void pythonInit(GUI_View* view, CNTRL_MainController* cntrl);
    virtual void pythonInitPlugins() {}
    
    void pythonStartup();
    
    bool start(int argc, char** argv, bool showGui=true);


};

#endif
