
#ifndef GUI_BUTTON_BASE_H
#define GUI_BUTTON_BASE_H

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/MenuPaneBase.h"

class GUI_BASE_API GUI_ButtonBase : public GUI_ItemBase {
    Q_OBJECT

signals:
    void buttonPressed(QVariantMap metadata);
    void hoverEnter();
    void hoverLeave();

    void toggleChanged(bool toggle);

    void menuShown(QVariantMap md);
    void menuHidden(QVariantMap md);


public:
    RLP_DEFINE_LOGGER

    GUI_ButtonBase(QQuickItem* parent);

    virtual void hoverEnterEvent(QHoverEvent* event);
    virtual void hoverLeaveEvent(QHoverEvent* event);
    virtual void hoverMoveEvent(QHoverEvent* event);


public slots:

    virtual GUI_MenuPaneBase* menu() { return _menu; }

    virtual void setToggled(bool toggled) { _isToggled = toggled; }
    bool isToggled() { return _isToggled; }
    
    virtual void setOutlined(bool outlined) { _isOutlined = outlined; }
    bool isOutlined() { return _isOutlined; }

    virtual void toggleMenu() {}
    virtual void setShowMenuOnHover(bool smoh) { _showMenuOnHover = smoh; }
    
    virtual qreal size() const { return 0; }

protected:
    GUI_MenuPaneBase* _menu;
    bool _isToggled;
    bool _isOutlined;
    bool _showMenuOnHover;



};

#endif