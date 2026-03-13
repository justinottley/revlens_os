//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

//
// NOTE: UNUSED
//

#ifndef GUI_BASE_VARIANTHELPER_H
#define GUI_BASE_VARIANTHELPER_H

#include "RlpGui/BASE/Global.h"


class GUI_BASE_API GUI_VariantHelper : public QObject {
    Q_OBJECT


public:
    GUI_VariantHelper() {}

public slots:

    QVariant makeVariant(int dataIn)
    {
        return QVariant(dataIn);
    }

    void setData(QGraphicsItem* item, int key, QVariant data)
    {
        item->setData(key, data);
    }

};

class GUI_BASE_API  GUI_VariantHelperDecorator : public QObject {
    Q_OBJECT

public slots:

    GUI_VariantHelper* new_GUI_VariantHelper()
    {
        return new GUI_VariantHelper();
    }


};


#endif