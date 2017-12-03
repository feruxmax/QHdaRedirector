#ifndef CODEC_H
#define CODEC_H
#include <stdlib.h>
#include <QVector>
#include <QStringList>
#include "node.h"
#include "hda.h"

template <typename T> struct pointer_values_equal
{
    int to_find;

    bool operator()(const T* other) const
    {
        return  *other == to_find;
    }
};

class Codec
{
    QString device;
    QString dev_init_config;
    QString dev_user_config;
    const char *hda_intel_conf;
    const char *snd_hda_intel_fw;

    int cardNo;
    int devNo;
    uint32_t vendor_id;
    uint32_t implementation_id;

    union NodeConf{
        PinConf descr;
        char byte[4];
    };

    void getConfigFromDevice();
    int findFreeAsNo(QVector<Node*>::iterator for_it);
    void makeAllNodesUnpassed();
    void getPinsConfig(QString &config_file);
public:
    Codec();
    Codec(const QString &devName);
    ~Codec();
    void getDefaultConfig();
    void applyPinConfSettings();

    void updatePaths();
    QStringList& getConsNames(PinCompex *pin);
    void lockPath(PinCompex *pin, int no);
    void unlockPath(PinCompex *pin);
    void applySettings();
    void writeSettings();
    void disableConfig();
    void generateAsSeqNumbers();

    QVector<Node*> nodes;
    QVector<Node*> activePinWidgets;
    QVector<Node*> audioOuts;
    QVector<AudioIn*> audioInputs;

};

#endif // CODEC_H
