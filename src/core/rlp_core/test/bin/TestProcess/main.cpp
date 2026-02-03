
#include "RlpCore/PROC/Process.h"

int main(int argc, char** argv)
{
    LOG_Logging::init();

    qInfo() << "Hello world";

    PROC_Process p;
    p.setCommand("./progtest");
    
    QCoreApplication qapp(argc, argv);

    p.start();


    qapp.exec();
}