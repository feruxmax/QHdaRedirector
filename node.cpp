#include "node.h"

/*--------------------------- Node -------------------------------------------*/
Node::Node()
{
}

Node::Node(int nid, int ty)
    : selectedConNo(0),
      type((WidgetType)ty),
      NID(nid)
{
}

Node::~Node()
{
    ;
}

void Node::addConn(int nid)
{
    if(nid > 0)
        connNids.append(nid);
}

void Node::addConn(Node *nd)
{
    Con *con = new Con(nd);
    if(selectedConNo==connections.size())
        conSel = con;
    connections.append(*con);

}

int Node::findPath(int nid, QVector<QVector<Node*> > &paths)
{
    int retval = 0;

    if(!conSel->locked){
        for(QVector<Con>::iterator it=connections.begin(); it != connections.end(); it++){
            if(it->con->type == WT_PinCompex && it->con->NID != nid){
                continue;
            }

            retval = it->con->findPath(nid, paths);

            if(retval){
                paths.last().append(this);
                break;
            }
        }
    } else{
        retval = conSel->con->findPath(nid, paths);

        if(retval){
            paths.last().append(this);
        }
    }

    return retval;
}

void Node::makeLocked(int nid)
{
    for(QVector<Con>::iterator it=connections.begin(); it != connections.end(); it++){
       if(it->con->NID == nid){
           conSel = &(*it);
           it->locked = true;
       } else
           it->locked = false;
    }
}

void Node::makeUnlocked()
{
    for(QVector<Con>::iterator it=connections.begin(); it != connections.end(); it++){
        it->locked = false;
    }
}

void Node::applySettings(void*,
  u_int32_t(*)(void*, uint32_t, uint32_t, uint32_t))
{
    ;
}

/*--------------------------- AudioSel ---------------------------------------*/
void AudioSel::applySettings(void *fd,
  u_int32_t(*sendVerb)(void *fd, uint32_t nid, uint32_t vb, uint32_t value))
{
    if(isLocked()){
        sendVerb(fd, NID, VB_SET_SELECT_CTRL, connNids.indexOf(conSel->con->NID));
    }
}

bool AudioSel::isLocked()
{
    bool cond = false;

    for(QVector<Con>::iterator it=connections.begin(); it != connections.end(); it++){
        if(it->locked){ /** locked to AudioOut */
            cond = true;
            break;
        }
    }

    return cond;
}

/*--------------------------- AudioOut ---------------------------------------*/
int AudioOut::nextAuioOut = 0;

AudioOut::AudioOut(int nid, int ty):Node(nid, ty)
{
    curOut = nextAuioOut;
    if(curOut == 0)
        nextAuioOut=2;
    else if(curOut == 1)
        nextAuioOut=3;
    else if (curOut == 2)
        nextAuioOut = 1;
    else
        nextAuioOut++;

    switch(curOut){
    case 0:
        textNid.append("Front");
        break;
    case 1:
        textNid.append("Sub/Cent.");
        break;
    case 2:
        textNid.append("Rear");
        break;
    case 3:
        textNid.append("Suround");
        break;
    default:

        textNid = "0x" + textNid.setNum(nid, 16);

    }
}

AudioOut::~AudioOut()
{
    nextAuioOut--;
}

int AudioOut::findPath(int nid, QVector<QVector<Node*> > &paths)
{
    int retval = 0;

    if(nid == NID){
        QVector<Node*> *vec = new QVector<Node*>;
        vec->append(this);
        paths.append(*vec);
        retval = 1;
    }

    return retval;
}

void AudioOut::makeLocked(int)
{
    ;
}

const QString& AudioOut::name()
{
    return textNid;
}

/*--------------------------- PinCompex --------------------------------------*/
int PinCompex::findPath(int nid)
{
    int retval = 0;
    if(!conSel->locked){
        for(QVector<Con>::iterator it=connections.begin(); it != connections.end(); it++){
            retval = it->con->findPath(nid, paths);
            if(retval){
                paths.last().append(this);
            }
        }
    } else{
        retval = conSel->con->findPath(nid, paths);
        if(retval){
            paths.last().append(this);
        }
    }

    return retval;
}

int PinCompex::findPath(int nid, QVector<QVector<Node*> > &paths)
{
    int retval = 0;

    if(nid == NID){
        QVector<Node*> *vec = new QVector<Node*>;
        vec->append(this);
        paths.append(*vec);
        retval = 1;
    }

    return retval;
}

void PinCompex::setPinConf(uint32_t val)
{
   confParams = *((PinConf*)&val);
}

uint32_t PinCompex::getPinConf()
{
    return *((uint32_t*)&confParams);
}

void PinCompex::lockPath(int no)
{
    QVector<Node*> *path = &paths[no];

    for(int i = path->size()-1; i>0; i--){
        (*path)[i]->makeLocked((*path)[i-1]->NID);
    }
}

int PinCompex::getSelectedPathNo()
{
    bool ok;
    int retval = 0;
    if(pinCtrl & PWCTRL_OUT){
        for(QVector<QVector<Node*> >::iterator it = paths.begin(); it != paths.end(); it++){
            QVector<Node*> *path = &(*it);
            ok = true;
            for(int i = path->size()-1; i>0; i--){
                if((*path)[i-1] != (*path)[i]->conSel->con){
                    ok = false;
                    break;
                }
            }
            if(ok){
                retval = paths.indexOf(*path);
                break;
            }
        }
    } else if(pinCtrl & PWCTRL_IN){
        retval = 0;/** @fix path no to "input" supported yet */
    }

    return retval;
}

void PinCompex::unlockPath()
{
    QVector<Node*> *path = &paths.first();
    if(path){
        for(int i = path->size()-1; i>0; i--){
            (*path)[i]->makeUnlocked();
        }
    }
}

bool PinCompex::isLocked()
{
    AudioIn *in;
    bool cond = false;

    if( !(in = dynamic_cast<AudioIn*>(lockedToAudioIn))){
        for(QVector<Con>::iterator it=connections.begin(); it != connections.end(); it++){
            if(it->locked){ /** locked to AudioOut */
                cond = true;
                break;
            }
        }
    } else /** locked to AudioIn */
        cond = true;

    return cond;
}

void PinCompex::applySettings(void *fd,
  u_int32_t(*sendVerb)(void *fd, uint32_t nid, uint32_t vb, uint32_t value))
{
    AudioIn *in;

    if(isLocked()){
        sendVerb(fd, NID, VB_SET_SELECT_CTRL, connNids.indexOf(conSel->con->NID));
        if( !(in = dynamic_cast<AudioIn*>(lockedToAudioIn)))
            sendVerb(fd, NID, VB_SET_PIN_WIDGET_CTRL, PWCTRL_OUT);
        else
            sendVerb(fd, NID, VB_SET_PIN_WIDGET_CTRL, PWCTRL_IN);
    }
}

/*--------------------------- AudioIn --------------------------------------*/
int AudioIn::curAuioInputs = 0;

AudioIn::AudioIn(int nid, int ty):Node(nid, ty)
{
    curIn = curAuioInputs++;
    textNid = "Input" + textNid.setNum(curIn);
}

AudioIn::~AudioIn()
{
    curAuioInputs--;
}

const QString& AudioIn::name()
{
    return textNid;
}

int AudioIn::findPath(int nid)
{
    int retval = 0;
    if(!conSel->locked){
        for(QVector<Con>::iterator it=connections.begin(); it != connections.end(); it++){
            retval = it->con->findPath(nid, paths);
            if(retval){
                paths.last().append(this);
            }
        }
    } else{
        retval = conSel->con->findPath(nid, paths);
        if(retval){
            paths.last().append(this);
        }
    }

    return retval;
}

void AudioIn::lockPath(int nid)
{
    for(QVector<QVector<Node*> >::iterator it1=paths.begin(); it1!=paths.end();it1++){
        PinCompex *pin = dynamic_cast<PinCompex*> (it1->first());
        if(pin->NID == nid){
            QVector<Node*> *path = &(*it1);
            for(int i = path->size()-1; i>0; i--){
                (*path)[i]->makeLocked((*path)[i-1]->NID);
            }
            break;
        }
    }
}

void AudioIn::unlockPath()
{

    QVector<Node*> *path = &paths.first();
    for(int i = path->size()-1; i>0; i--){
        (*path)[i]->makeUnlocked();
    }

}
