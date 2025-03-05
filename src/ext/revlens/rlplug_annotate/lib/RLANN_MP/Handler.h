
#ifndef EXTREVLENS_RLANN_MP_HANDLER_H
#define EXTREVLENS_RLANN_MP_HANDLER_H


#include "RlpExtrevlens/RLANN_MP/Global.h"

class RLANN_MP_Surface;

class EXTREVLENS_RLANN_MP_API RLANN_MP_Handler : public QObject {
    Q_OBJECT

signals:
    void brushRequested(QVariantMap brushInfo);

public:
    RLP_DEFINE_LOGGER

    static RLANN_MP_Handler* instance();

    RLANN_MP_Handler();

public slots:

    QStringList getBrushCategories() { return _brushCategories; }
    QStringList getBrushes(QString catName) { return _brushMap.value(catName); }

    void requestBrush(QString brushCat, QString brushName);

private:
    void initBrushes();

    static RLANN_MP_Handler* _instance;

    QHash<QString, QStringList> _brushMap;
    QStringList _brushCategories;
};

#endif
