#include "codec.h"
#include <QMap>
#include <QObject>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <qdebug.h>
#include <stdint.h>
#include <stdio.h>
#include <algorithm>

#define WRITE_VERB _IOWR('H', 0x11, hda_ioctl_command)

static inline int getBits(uint32_t val, uint32_t width, uint32_t start)
{
   int retval = 0;

   for(uint32_t i=start;i<start+width && i<sizeof(int)*8;i++){
       retval |= val & (1<<i);
   }

   return retval>>start;
}

static u_int32_t sendVerb(void *fd, uint32_t nid, uint32_t vb, uint32_t value)
{
    int fd_i = *((int*)fd);

    struct hda_ioctl_command {
        u_int32_t verb;
        u_int32_t retval;
    };

    hda_ioctl_command comm;

    comm.verb = ((nid)<<24 | (vb)<<8 | (value));
    if (ioctl(fd_i, WRITE_VERB, &comm) < 0){
        qDebug()<<QObject::tr("IOCTL error");
    }

    return comm.retval;
}

Codec::Codec():
    cardNo(0)
{
    dev_init_config = "123"; // "/sys/class/sound/hwC0D0/init_pin_configs";
    device = "/dev/snd/hwC0D0";
#ifndef WRITE_CONF_LOCAL
    hda_intel_conf = "/etc/modprobe.d/hda-intel.conf";
    snd_hda_intel_fw = "/lib/firmware/snd-hda-intel.fw";
#else
//    hda_intel_conf = "hda-intel.conf";
//    snd_hda_intel_fw = "snd-hda-intel.fw";
#endif
    activePinWidgets.resize(8);
    getConfigFromDevice();
    updatePaths();
}

Codec::Codec(const QString &devName)
{
    dev_init_config = "/sys/class/sound/" + devName + "/init_pin_configs";
    dev_user_config = "/sys/class/sound/" + devName + "/user_pin_configs";
    device = "/dev/snd/" + devName;
#ifndef WRITE_CONF_LOCAL
    hda_intel_conf = "/etc/modprobe.d/hda-intel.conf";
    snd_hda_intel_fw = "/lib/firmware/snd-hda-intel.fw";
#else
//    hda_intel_conf = "hda-intel.conf";
//    snd_hda_intel_fw = "snd-hda-intel.fw";
#endif
    QRegExp rx("(\\d+)");
    int pos;

    if( (pos = rx.indexIn(devName, 0) != -1)){
        cardNo = rx.cap(0).toInt();
        devNo = rx.cap(1).toInt();
    }

    activePinWidgets.resize(8);
    getConfigFromDevice();
    updatePaths();
}

Codec::~Codec()
{
    activePinWidgets.clear();
    audioOuts.clear();
    audioInputs.clear();
    for(int i=0;i<nodes.size();i++)
        nodes[i]->~Node();
}

void Codec::getPinsConfig(QString &config_file)
{
    char buf[256];
    int nid=-1, config;
    FILE *f;
    PinCompex *pin;

    f=fopen(config_file.toLatin1(),"r");

    if(f==NULL){
        qDebug()<<QObject::tr("read config error") << dev_init_config;
        return;
    }

    while(fgets(buf,256,f)!=NULL){
        if(sscanf(buf,"0x%x 0x%x",&nid, &config)==2){
            for(QVector<Node*>::iterator it = activePinWidgets.begin(); it != activePinWidgets.end(); it++){
                if( !(pin = dynamic_cast<PinCompex*>(*it))) continue; /** skip NULL pins */
                if(pin->NID == nid){
                    pin->setPinConf(config);
                    break;
                }
            }
        }
    }

    fclose(f);
}

void Codec::getDefaultConfig()
{
    getPinsConfig(dev_init_config);
}

void Codec::applyPinConfSettings()
{
    int fd;
    NodeConf conf;
    PinCompex *pin;

    fd = open(device.toLatin1(), O_RDWR);
    if (fd < 0) {
        qDebug()<< QObject::tr("error open device") << device;
        return;
    }

    for(QVector<Node*>::iterator it = activePinWidgets.begin(); it != activePinWidgets.end(); it++){
        if( !(pin = dynamic_cast<PinCompex*>(*it))) continue; /** skip NULL pins */
        if(pin->isLocked()){
            conf.descr = pin->confParams;
            sendVerb(&fd, pin->NID, VB_SET_PIN_DEF_CONF_BYTES_0, conf.byte[0]);
            sendVerb(&fd, pin->NID, VB_SET_PIN_DEF_CONF_BYTES_1, conf.byte[1]);
            sendVerb(&fd, pin->NID, VB_SET_PIN_DEF_CONF_BYTES_2, conf.byte[2]);
            sendVerb(&fd, pin->NID, VB_SET_PIN_DEF_CONF_BYTES_3, conf.byte[3]);
        }
    }
    ::close(fd);
}

void Codec::getConfigFromDevice()
{
    int fd, retval;
 
    retval = getBits(0x00fabcde, 8, 16);
    fd = open(device.toLatin1(), O_RDWR);
    if (fd < 0) {
        qDebug()<< QObject::tr("error open device") << device;
        return;
    }

    vendor_id = sendVerb(&fd, 0x00, VB_PARAMETERS, PAR_VENDOR_ID);


    retval = sendVerb(&fd, 0x00, VB_PARAMETERS, PAR_NODE_COUNT);
//    int numFunctGroups = getBits(retval, 8, 0);
    int firstFunctGroup = getBits(retval, 8, 16);
    implementation_id = sendVerb(&fd, firstFunctGroup, VB_GET_IMPLEMENTATION_ID, 0);
    retval = sendVerb(&fd, firstFunctGroup, VB_PARAMETERS, PAR_NODE_COUNT);
    int numWidgets =  getBits(retval, 8, 0);
    int nidOfFirstWidget = getBits(retval, 8, 16);

    //fill nodes from first function group
    for(int i=nidOfFirstWidget; i<numWidgets+nidOfFirstWidget; i++){
        retval = sendVerb(&fd, i, VB_PARAMETERS, PAR_AUDIO_WIDGET_CAP);
        WidgetType wt = (WidgetType)getBits(retval, 4, 20);
        Node *nd;
        switch(wt){
            case WT_AudioOut:
                nd = new AudioOut(i, wt); break;
            case WT_AudioMix:
                nd = new AudioMix(i, wt); break;
            case WT_AudioSel:
                nd = new AudioSel(i, wt);
                nd->selectedConNo = sendVerb(&fd, i, VB_GET_SELECT_CTRL, 0);
                break;
            case WT_PinCompex:
                nd = new PinCompex(i, wt);
                retval = sendVerb(&fd, i, VB_GET_CONFIG_DEFAULT, 0);
                ((PinCompex*)nd)->setPinConf(retval);
                nd->selectedConNo = sendVerb(&fd, i, VB_GET_SELECT_CTRL, 0);
                retval = sendVerb(&fd, i, VB_GET_PIN_WIDGET_CTRL, 0);
                ((PinCompex*)nd)->pinCtrl = (getBits(retval, 8, 0));

                // disable inout pins
                if(((PinCompex*)nd)->pinCtrl & PWCTRL_OUT)
                    ((PinCompex*)nd)->pinCtrl &= ~(PWCTRL_IN);
                else if(((PinCompex*)nd)->pinCtrl & PWCTRL_IN)
                    ((PinCompex*)nd)->pinCtrl &= ~(PWCTRL_OUT);
            break;
            case WT_AudioIn:
                nd = new AudioIn(i, wt);
                nd->selectedConNo = sendVerb(&fd, i, VB_GET_SELECT_CTRL, 0);
                break;
            default:
                nd = new Node(i, wt);
        }

        //fill connections
        retval = sendVerb(&fd, i, VB_PARAMETERS, PAR_CONNLIST_LEN);
        int numCons = getBits(retval, 6, 0);
        int longForm = getBits(retval, 1, 7);//if long form size of NID=16, in short - 8
        if(numCons>0){
            for(int j=0; j<numCons; j+=4){
                retval = sendVerb(&fd, i, VB_GET_CONN_LIST, j);
                for(int k=0; k < (4>>longForm); k++)
                    nd->addConn(getBits(retval, 8<<longForm, k*(8<<longForm)));
            }
        }
        nodes.append(nd);
    }

    ::close(fd);
    //fill active pin widgets
    int rearCount = 0, frontCount = 0;
    for(QVector<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++){
        if((*it)->type == WT_PinCompex){
            PinCompex* pin = (PinCompex*)(*it);
            if(pin->confParams.connectivity == PC_toJack &&
                    pin->confParams.conType == CT_1_8SM)
            {
                if((pin->confParams.loc == LOC_FrontExt || pin->confParams.loc == LOC_Left )&&
                        frontCount<2) //only 2 active front pin supported
                {
                    activePinWidgets[6+frontCount] = pin;
                    frontCount++;
                } else if(rearCount<6) //only 6 active rear pin supported
                {
                    activePinWidgets[rearCount] = pin;
                    rearCount++;
                }
            }
        } else if((*it)->type == WT_AudioOut){
            audioOuts.append(*it);
        } else if((*it)->type == WT_AudioIn){
            audioInputs.append((AudioIn*)*it);
        }
    }

    //fill connections
    for(QVector<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++){
        for(int i=0;i<(*it)->connNids.size();i++){
            pointer_values_equal<Node> fd = {(*it)->connNids[i]};
            QVector<Node*>::iterator nd_it = std::find_if(nodes.begin(), nodes.end(), fd);
            if(nd_it != nodes.end())
                (*it)->addConn(*nd_it);
        }
    }
    
    //setup pinConfig
    getPinsConfig(dev_user_config);
}

void Codec::updatePaths()
{
    PinCompex *pin;
    AudioIn *in;

    for(QVector<Node*>::iterator it1 = activePinWidgets.begin(); it1 != activePinWidgets.end(); it1++){
        if( !(pin = dynamic_cast<PinCompex*>(*it1))) continue; /** skip NULL pins */
        pin->paths.clear();
        pin->conAudioInsIdxs.clear();
        if( (in = dynamic_cast<AudioIn*>(pin->lockedToAudioIn))){
            pin->conAudioInsIdxs.append(in);
            continue;
        }
        for(QVector<Node*>::iterator it2 = audioOuts.begin(); it2 != audioOuts.end(); it2++){
            AudioOut *out = (AudioOut *)(*it2);
            makeAllNodesUnpassed();
            pin->findPath(out->NID);
        }
    }

    for(QVector<AudioIn*>::iterator it1 = audioInputs.begin(); it1 != audioInputs.end(); it1++){
        if( !(in = dynamic_cast<AudioIn*>(*it1))) continue;
        in->paths.clear();
        for(QVector<Node*>::iterator it2 = activePinWidgets.begin(); it2 != activePinWidgets.end(); it2++){
            if( !(pin = dynamic_cast<PinCompex*>(*it2))) continue; /** skip NULL pins */
            makeAllNodesUnpassed();
            if(in->findPath(pin->NID) && !pin->isLocked()){
                pin->conAudioInsIdxs.append(in);
            }
        }
    }
}

QStringList& Codec::getConsNames(PinCompex *pin)
{
    AudioOut *out;
    QStringList *slist = new QStringList();

    if(pin->pinCtrl & PWCTRL_OUT){
        for(QVector<QVector<Node*> >::iterator it = pin->paths.begin(); it != pin->paths.end(); it++){
            if( !(out = dynamic_cast<AudioOut*>(it->first()))) continue;
            slist->append(out->name());
        }
    }

    if(pin->pinCtrl & PWCTRL_IN){
        for(QVector<AudioIn*>::iterator it = pin->conAudioInsIdxs.begin();
            it != pin->conAudioInsIdxs.end(); it++)
        {
            slist->append((*it)->name());
        }
    }

    return *slist;
}

void Codec::lockPath(PinCompex *pin, int no)
{
    if(pin->pinCtrl & PWCTRL_OUT){
        pin->lockPath(no);
    } else if((pin->pinCtrl & PWCTRL_IN)){
        pin->conAudioInsIdxs[no]->lockPath(pin->NID);
        pin->lockedToAudioIn = pin->conAudioInsIdxs[no];
    }
}

void Codec::unlockPath(PinCompex *pin)
{
    AudioIn *in;

    if( (in = dynamic_cast<AudioIn*>(pin->lockedToAudioIn))){
       pin->lockedToAudioIn->unlockPath();
       pin->lockedToAudioIn = NULL;
    } else{
        pin->unlockPath();
    }
}

void Codec::applySettings()
{
    PinCompex *pin;
    int fd;

    fd = open(device.toLatin1(), O_RDWR);
    if (fd < 0) {
        qDebug()<< QObject::tr("error open device") << device;
        return;
    }

    for(QVector<Node*>::iterator it = activePinWidgets.begin(); it != activePinWidgets.end(); it++){
        if( !(pin = dynamic_cast<PinCompex*>(*it))) continue; /** skip NULL pins */
        pin->applySettings(&fd, sendVerb);
    }

    ::close(fd);
}

static u_int32_t printVerb(void *fd, uint32_t nid, uint32_t vb, uint32_t value)
{
    FILE *file = (FILE*)fd;
    u_int32_t retval;

    retval = fprintf(file, "0x%x 0x%x 0x%x\n", nid, vb, value);

    return retval;
}

void Codec::writeSettings()
{
    FILE *fd;

    fd = fopen(hda_intel_conf, "w");

    if(fd == NULL){
        qDebug()<< QObject::tr("error open file:") << hda_intel_conf;
        return;
    }

    fputs("options snd-hda-intel patch=snd-hda-intel.fw\n", fd);
    fclose(fd);

    fd = fopen(snd_hda_intel_fw, "w");

    if(fd == NULL){
        qDebug()<< QObject::tr("error open file:") << snd_hda_intel_fw;
        return;
    }

    PinCompex *pin;
    fputs("[codec]\n", fd);
    fprintf(fd, "0x%x 0x%x %d\n\n", vendor_id, implementation_id, devNo);

    fputs("[model]\n", fd);
    fputs("auto\n\n", fd);

    fputs("[pincfg]\n", fd);
    for(QVector<Node*>::iterator it = activePinWidgets.begin(); it != activePinWidgets.end(); it++){
        if( !(pin = dynamic_cast<PinCompex*>(*it))) continue; /** skip NULL pins */
        if(pin->isLocked()){
            fprintf(fd, "0x%x 0x%x\n", pin->NID, pin->getPinConf());
        }
    }

    fputs("\n[verb]\n", fd);
    for(QVector<Node*>::iterator it = activePinWidgets.begin(); it != activePinWidgets.end(); it++){
        if( !(pin = dynamic_cast<PinCompex*>(*it))) continue; /** skip NULL pins */
        if(pin->isLocked()){
            pin->applySettings(fd, printVerb);
        }
    }
    fclose(fd);
}

void Codec::disableConfig()
{
    FILE *fd;

    fd = fopen(hda_intel_conf, "w");

    if(fd == NULL){
        qDebug()<< QObject::tr("error open file:") << hda_intel_conf;
        return;
    }

    fputs("\n", fd);
    fclose(fd);
}

int Codec::findFreeAsNo(QVector<Node*>::iterator for_it)
{
    int as_no, retval=0;
    bool met;
    PinCompex *pin;

    for(as_no=1;as_no<15;as_no++){
        met = false;
        for(QVector<Node*>::iterator it = nodes.begin(); it != for_it; it++){
            if( !(pin = dynamic_cast<PinCompex*>(*it))) continue; /** only PinComplex */
            if(pin->confParams.as == as_no){
               met = true;
               break;
            }
        }

        for(QVector<Node*>::iterator it = for_it+1; it != nodes.end() && !met; it++){
            if( !(pin = dynamic_cast<PinCompex*>(*it))) continue; /** onlyPinComplex */
            if(pin->isLocked()) continue;

            if(pin->confParams.as == as_no){
               met = true;
               break;
            }
        }

        if(met == false){
            retval = as_no;
            break;
        }
    }

    return retval;
}

void Codec::generateAsSeqNumbers()
{
    PinCompex *pin, *pin2;
    AudioOut *out;
    int as_no, reques_as_no = 0x1;
    bool as_no_found;

    for(QVector<Node*>::iterator it = activePinWidgets.begin(); it != activePinWidgets.end(); it++){
        if( !(pin = dynamic_cast<PinCompex*>(*it))) continue; /** skip NULL pins */
        QVector<Node*>::iterator nd_it = nodes.begin()+(pin->NID - nodes.first()->NID);
        /** @fix works if nodes.first().NID+1 == (nodes.first()+1).NID */

        as_no = reques_as_no;
        if(pin->isLocked() && pin->pinCtrl & PWCTRL_OUT){
            if( !(out = dynamic_cast<AudioOut*>(pin->paths.first().first()))) continue;
            as_no_found = false;
            for(QVector<Node*>::iterator it2 = nodes.begin(); it2 != nd_it; it2++){
                if( !(pin2 = dynamic_cast<PinCompex*>(*it2))) continue; /** onlyPinComplex */
                if(pin2->confParams.as == reques_as_no && pin2->confParams.seq == out->curOut){
                    as_no = findFreeAsNo(nd_it);
                    as_no_found = true;
                    break;
                }
            }
            if(!as_no_found){
                for(QVector<Node*>::iterator it2 = nd_it+1; it2 != nodes.end(); it2++){
                    if( !(pin2 = dynamic_cast<PinCompex*>(*it2))) continue; /** onlyPinComplex */
                    if(pin2->confParams.as == reques_as_no &&
                            pin2->confParams.seq == out->curOut &&
                            !pin2->isLocked())
                    {
                        as_no = findFreeAsNo(nd_it);
                    }

                }
            }
            pin->confParams.as = as_no;
            pin->confParams.seq = out->curOut;
        } else if(pin->isLocked() && pin->pinCtrl & PWCTRL_IN){
            pin->confParams.as = findFreeAsNo(nd_it);
            pin->confParams.seq = 0x0;
        }
    }
}

void Codec::makeAllNodesUnpassed()
{
    for(QVector<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++){
        (*it)->passed = false;
    }
}
