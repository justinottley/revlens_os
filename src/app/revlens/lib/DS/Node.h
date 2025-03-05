//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_NODE_H
#define REVLENS_DS_NODE_H

#include "RlpCore/LOG/Logging.h"

#include "RlpRevlens/DS/Global.h"
#include "RlpRevlens/DS/Time.h"
#include "RlpRevlens/DS/FrameBuffer.h"
#include "RlpRevlens/DS/Graph.h"

class DS_Node;
typedef std::shared_ptr<DS_Node> DS_NodePtr;
typedef std::vector<DS_NodePtr>::iterator NodeInputListIterator;

Q_DECLARE_METATYPE(DS_NodePtr)


class REVLENS_DS_API DS_Node : public QObject {
    Q_OBJECT

public:
    enum NodeDataType {
        DATA_VIDEO,
        DATA_AUDIO,
        DATA_TEXT,
        DATA_LOAD,
        DATA_UNKNOWN
    };


public:
    RLP_DEFINE_LOGGER

    DS_Node(QString name);
    DS_Node(QString name, QVariantMap* properties);
    DS_Node(QString name, QVariantMap* properties, QString uuid);

    virtual ~DS_Node();

    virtual void readVideo(
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame,
        bool postUpdate = true) {}

    virtual void readAudio(
        DS_TimePtr timeInfo,
        DS_BufferPtr destBuffer) {}

    virtual void scheduleReadTask(
        DS_TimePtr timeInfo,
        DS_BufferPtr destFrame,
        NodeDataType dataType,
        bool optional = false) {}

    virtual void scheduleReleaseTask(
        qlonglong frameNum
    ) {}

    virtual void initLocalProperties();

    template <class T>
    void setProperty(const char* key, T value) {
        _properties->insert(key, value);
    }

    template<class T>
    T getProperty(const char* key) {

        if (hasProperty(key, false)) {
            return _properties->value(key).value<T>();

        } else {

            // try inputs
            for (int i=0; i != _inputs.size(); ++i) {

                if (_inputs[i] != nullptr) {
                    if (_inputs[i]->hasProperty(key)) { // this input node and all of its inputs
                        return _inputs[i]->getProperty<T>(key);
                    }
                }
            }
        }

        return T();
    }


    template<class T>
    T getProperty(const char* key, T defaultValue) {
        T result = getProperty<T>(key);
        if (result) {
            return result;
        }

        return defaultValue;
    }


    bool hasProperty(const char* key, bool inputs = true) {

        bool result = false;

        if ((_properties == nullptr || _properties == NULL)) {
            return false;
        }

        if (_properties->contains(key)) {
            result = true;

        } else if (inputs) {

            // try inputs
            for (int i=0; i != _inputs.size(); ++i) {

                if (_inputs[i] != nullptr) {
                    result = _inputs[i]->hasProperty(key);
                    if (result) {
                        break;
                    }
                }
            }
        }

        return result;
    }

    void removeProperty(const char* key) {
        if (_properties->find(key) != _properties->end()) {
            _properties->erase(_properties->find(key));
        }
    }

    QVariantMap* getPropertiesPtr() { return _properties; }


signals:
    void dataReady(QVariantMap info);

public slots:

    const QString name() { return _name; }

    virtual void onGraphDataReady(QVariantMap data);
    void emitDataReadyToGraph(QVariantMap data);

    QVariantMap getProperties() {
        if (_properties != NULL) {
            return (*_properties);
        }

        return QVariantMap();
    }

    // for python binding
    void insertProperty(QString key, QVariant value)
    {
        _properties->insert(key, value);
    }

    virtual void addInput(DS_NodePtr node) {
        _inputs.push_back(node);
    }

    DS_NodePtr input(int idx);
    int inputCount() { return _inputs.size(); }

    QUuid uuid() { return _uuid; }
    DS_Graph* graph();

    virtual void enable() {}

    virtual void disable() {}

    DS_TimePtr makeTime(long frameNum);

    QJsonObject toJson();


protected:

    const QString _name;
    const QUuid _uuid;

    std::vector<DS_NodePtr> _inputs;
    // std::vector<DS_NodePlugPtr> _outputs;

    QVariantMap* _properties;
};


class REVLENS_DS_API DS_NodePtrDecorator : public QObject {
    Q_OBJECT

public slots:

    DS_NodePtr* new_DS_NodePtr(QString name) { return new DS_NodePtr(new DS_Node(name)); }

    DS_NodePtr* new_DS_NodePtr(QString name, QVariantMap* properties) {
        return new DS_NodePtr(new DS_Node(name, properties));
    }

    bool isValid(DS_NodePtr* node) { return ((*node) != nullptr); }

    QVariantMap getProperties(DS_NodePtr* node) { return (*node)->getProperties(); }

    bool hasProperty(DS_NodePtr* node, const char* key, bool inputs = true)
    {
        return (*node)->hasProperty(key, inputs);
    }
    void removeProperty(DS_NodePtr* node, const char* key) { return (*node)->removeProperty(key); }

    void setProperty(DS_NodePtr* node, QString key, QVariant value)
    {
        (*node)->getPropertiesPtr()->insert(key, value);
    }

    QJsonObject toJson(DS_NodePtr* node) { return (*node)->toJson(); }

    DS_Graph* graph(DS_NodePtr* node) { return (*node)->graph(); }

    DS_Node* get(DS_NodePtr* node) { return (*node).get(); }

    void addInput(DS_NodePtr* node, DS_NodePtr nodeIn) {
        (*node)->addInput(nodeIn);
    }

};


#endif
