
#ifndef GUI_PYSBK_H
#define GUI_PYSBK_H

#include "RlpGui/SBKPYMODULE/Global.h"


class GUI_PYSBK_API PYSBK_Util {

public:
    static QWidget* getQWidget(PyObject* pyQWidget);

};


#endif