#ifndef NODE_H
#define NODE_H
#include <stdint.h>
#include <QVector>
#include <QString>
#include "hda.h"

enum WidgetType{
    WT_AudioOut,
    WT_AudioIn,
    WT_AudioMix,
    WT_AudioSel,
    WT_PinCompex,
    WT_PowerWgt,
    WT_VolumeKnobWgt,
    WT_BeepGenWgt = 0x7,
    WT_VenDefined = 0xf
};

struct PinConf{
    unsigned int seq:4;
    unsigned int as:4;
    unsigned int misc:4;
    unsigned int color:4;
    unsigned int conType:4;
    unsigned int dev:4;
    unsigned int loc:6;
    unsigned int connectivity:2;
};

struct Con;
class AudioIn;

class Node
{

public:
    QVector<int> connNids;
    int selectedConNo;

    Node();
    virtual ~Node();
    bool operator==(int nid) const {return nid == NID;}
    Node(int nid, int ty);
    void addConn(int nid);
    void addConn(Node *nd);
    virtual int findPath(int nid, QVector<QVector<Node*> > &paths);
    virtual void makeLocked(int);
    void makeUnlocked();
    virtual void applySettings(void *fd,
       u_int32_t(*sendVerb)(void *fd, uint32_t nid, uint32_t vb, uint32_t value));

    WidgetType type;
    int NID;
    Con *conSel;
    bool passed;

protected:
    QVector<Con> connections;
};


struct Con{
    Node *con;
    bool locked;
    Con(){;}
    Con(Node *nd){con = nd; locked = false;}
};

class AudioOut : public Node{
    static int nextAuioOut;
    QString textNid;
public:
    AudioOut(){;}
    ~AudioOut();
    AudioOut(int nid, int ty);
    virtual int findPath(int nid, QVector<QVector<Node*> > &paths);
    const QString& name();
    int getNo(){return curOut;}
    void makeLocked(int);

    int curOut;
};


class AudioMix : public Node{

public:
    AudioMix(){;}
    AudioMix(int nid, int ty):Node(nid, ty){;}

};

class AudioSel : public Node{
    bool isLocked();
public:
    AudioSel(){;}
    AudioSel(int nid, int ty):Node(nid, ty){;}
    virtual void applySettings(void *fd,
      u_int32_t(*sendVerb)(void *fd, uint32_t nid, uint32_t vb, uint32_t value));

};

class PinCompex : public Node{

public:
    PinCompex(){;}
    PinCompex(int nid, int ty):Node(nid, ty){lockedToAudioIn = NULL;}
    int findPath(int nid);
    virtual int findPath(int nid, QVector<QVector<Node*> > &paths);
    void setPinConf(uint32_t val);
    uint32_t getPinConf();
    void lockPath(int no);
    int getSelectedPathNo();
    void unlockPath();
    bool isLocked();
    virtual void applySettings(void *fd,
      u_int32_t(*sendVerb)(void *fd, uint32_t nid, uint32_t vb, uint32_t value));

    QVector<QVector<Node*> > paths;
    QVector<AudioIn*> conAudioInsIdxs;
    PinConf confParams;
    uint8_t pinCtrl;

    AudioIn* lockedToAudioIn;
};

class AudioIn : public Node{
    static int curAuioInputs;
    QString textNid;

public:
    int curIn;
    AudioIn(){;}
    AudioIn(int nid, int ty);
    ~AudioIn();
    int findPath(int nid);
    const QString& name();
    void lockPath(int nid);
    void unlockPath();

    QVector<QVector<Node*> > paths;
};
#endif // NODE_H
