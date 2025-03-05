
#include <QtCore/QCoreApplication>

#include "RlpCore/CNTRL/Auth.h"


int
main(int argc, char** argv)
{
    LOG_Logging::init();
    
    
    QCoreApplication app(argc, argv);

    CoreCntrl_Auth* login = new CoreCntrl_Auth();
    login->loginInit();

    app.exec();
    
    /*
     * python rlp_core.remoting.Client example:
     * 
     * c = Client('rlp', server='localhost:PORT')
     * c.handle({'test':'the'})
     */
}