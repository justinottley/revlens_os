
#ifndef EdbMedia_CONTROLLER_H
#define EdbMedia_CONTROLLER_H

#include "RlpEdb/MEDIA/Global.h"
#include "RlpEdb/MEDIA/Schema.h"
#include "RlpEdb/CNTRL/Query.h"

// #include "RlpCore/CRYPT/AES.h"

class CRYPT_AES { // Temporary if OpenSSL's AES is not available


public:
    CRYPT_AES() {}

    void initKey(QByteArray initKey, QByteArray iv) {}
    QString decryptToString(QString& msg) { return msg; }
    QByteArray decryptData(QByteArray& inData) { return inData; }

    QString encryptToString(QString& msg) { return msg; }
    QByteArray encryptData(QByteArray& inData) { return inData; }

};


class EDB_MEDIA_API EdbMedia_Controller : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbMedia_Controller(QString filePath, bool encryptFrames=false);


public slots:

    EdbMedia_Schema* schema() { return _schema; }
    CRYPT_AES* encryptionCntrl() { return _caes; }

    bool isValid();
    QString filePath() { return _filePath; }

    
    EdbDs_Entity* addVideoStream(QString name, QString uuid="");
    bool writeFrame(QString streamName, qlonglong frameNum, QByteArray data);

    QByteArray getFrameData(QUuid streamUuid, qlonglong frameNum);
    QByteArray getFrameData(QString streamName, qlonglong frameNum);
    QVariantList getAllFrameData(QString streamName);

    bool hasFrame(QString streamName, qlonglong frameNum);
    
    bool hasStreamByName(QString name) { return _streamMap.contains(name); }
    QStringList getAllStreams() { return _streamMap.keys(); }
    QVariantMap getVideoStreamInfo(QString streamName);

private:
    EdbCntrl_Query* _qcntrl;
    EdbMedia_Schema* _schema;
    CRYPT_AES* _caes;

    bool _encryptFrames;
    QString _filePath;
    QMap<QString, QUuid> _streamMap;

};

#endif
