
//
// DEPRECATED
//

class EXTREVLENS_RLQTDISP_API RLQTDISP_CompositeTools : public GUI_ItemBase {
    Q_OBJECT

signals:
    void compositeModeChanged(QString mode);

public:
    RLP_DEFINE_LOGGER

    RLQTDISP_CompositeTools(DISP_GLView* parent);

    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);

public slots:

    void setEnabled(bool enabled);
    void onModeMenuShown(QVariantMap md);
    void onModeMenuItemSelected(QVariantMap itemData);

protected:
    GUI_IconButton* _modeButton;
    DISP_GLView* _display;
};
