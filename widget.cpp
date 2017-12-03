#include "widget.h"
#include "ui_widget.h"
#include <QDir>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    initDone(false)

{
    ui->setupUi(this);

    ui->cmb_device->addItems(getDevices());
    device = new Codec(ui->cmb_device->currentText());

    dev[0]=ui->cob_dev0;
    dev[1]=ui->cob_dev1;
    dev[2]=ui->cob_dev2;
    dev[3]=ui->cob_dev3;
    dev[4]=ui->cob_dev4;
    dev[5]=ui->cob_dev5;
    dev[6]=ui->cob_dev6;
    dev[7]=ui->cob_dev7;

    audio[0]=ui->cob_ch0;
    audio[1]=ui->cob_ch1;
    audio[2]=ui->cob_ch2;
    audio[3]=ui->cob_ch3;
    audio[4]=ui->cob_ch4;
    audio[5]=ui->cob_ch5;
    audio[6]=ui->cob_ch6;
    audio[7]=ui->cob_ch7;

    as[0]=ui->spinBox;
    as[1]=ui->spinBox_2;
    as[2]=ui->spinBox_3;
    as[3]=ui->spinBox_4;
    as[4]=ui->spinBox_5;
    as[5]=ui->spinBox_6;
    as[6]=ui->spinBox_7;
    as[7]=ui->spinBox_8;

    seq[0]=ui->spinBox_9;
    seq[1]=ui->spinBox_10;
    seq[2]=ui->spinBox_11;
    seq[3]=ui->spinBox_12;
    seq[4]=ui->spinBox_13;
    seq[5]=ui->spinBox_14;
    seq[6]=ui->spinBox_15;
    seq[7]=ui->spinBox_16;

    en[0]=ui->checkBox;
    en[1]=ui->checkBox_2;
    en[2]=ui->checkBox_3;
    en[3]=ui->checkBox_4;
    en[4]=ui->checkBox_5;
    en[5]=ui->checkBox_6;
    en[6]=ui->checkBox_7;
    en[7]=ui->checkBox_8;

    for(int i=0;i<8;i++){
        dev[i]->addItem(QPixmap(":/images/LineIn.png"),tr("Line-in"));
        dev[i]->addItem(QPixmap(":/images/Speakers.png"),tr("Speaker"));
        dev[i]->addItem(QPixmap(":/images/Headphones.png"),tr("HP Out"));
        dev[i]->addItem(QPixmap(":/images/Microphone.png"),tr("Mic"));
        //	dev[i]->addItem(QPixmap(":/images/LineOut.png"),tr("Line-out"));
        dev[i]->addItem(QPixmap(":/images/Question.png"),tr("Other"));
    }

    for(int i=0;i<9;i++)
        icons[i] = new QIcon();

    socPixmap[0] = new QPixmap(":/images/Disabled.png");
    socPixmap[1] = new QPixmap(":/images/Black.png");
    socPixmap[2] = new QPixmap(":/images/Grey.png");
    socPixmap[3] = new QPixmap(":/images/Blue.png");
    socPixmap[4] = new QPixmap(":/images/Green.png");
    socPixmap[5] = new QPixmap(":/images/Red.png");
    socPixmap[6] = new QPixmap(":/images/Orange.png");
    socPixmap[7] = new QPixmap(":/images/Yellow.png");
    socPixmap[8] = new QPixmap(":/images/Purple.png");
    socPixmap[9] = new QPixmap(":/images/Pink.png");
    socPixmap[10] = new QPixmap(":/images/Disabled.png");
    socPixmap[11] = new QPixmap(":/images/Disabled.png");
    socPixmap[12] = new QPixmap(":/images/Disabled.png");
    socPixmap[13] = new QPixmap(":/images/Disabled.png");
    socPixmap[14] = new QPixmap(":/images/White.png");
    socPixmap[15] = new QPixmap(":/images/Disabled.png");

    resetIf();
    setDefault();
    initDone = true;
}

Widget::~Widget()
{
    delete ui;
}

QStringList& Widget::getDevices()
{
    QDir dir("/dev/snd");
    QRegExp rx("(^hwC\\d+D\\d+$)");
    int pos = 0;
    QStringList *list = new QStringList;

    if(!dir.exists()) qDebug()<<"!!!";
    QStringList fileList = dir.entryList(QDir::System);

    for(QStringList::Iterator it = fileList.begin();
        it != fileList.end(); it++)
    {
        if( (pos = rx.indexIn(*it, 0)) != - 1){
            (*list)<< rx.cap(1);
        }
    }

    return *list;
}

int Widget::toDevId(int dev_no){
    switch(dev_no)
    {
    case 0:		//Line-out
    case 1:return 1;	//Speaker
    break;
    case 2:return 2;	//Speaker
    break;
    case 8:return 0;	//Line-in
    break;
    case 10:return 3;	//Mic
    break;
    default:return 4;	//Other
    }
}

int Widget::toDevNo(int dev_index){
    switch(dev_index)
    {
    case 0:return 8;	//Line-in
    break;
    case 1:return 0;	//Speaker
    break;
    case 2:return 2;	//Speaker
    break;
    case 3:return 10;	//Mic
    break;
    case 4:
    default:return 15;	//Other
    }

}

const char* Widget::locToStr(int loc)
{
    switch(loc){
    case LOC_RearExt:
        return "Rear";
    case LOC_FrontExt:
        return "Front";
    case LOC_Left:
        return "Left";
    case LOC_Right:
        return "Right";
    case LOC_Top:
        return "Top";
    case LOC_Bottom:
        return "Bottom";
    default:
        return "Other";
    }
}

void Widget::resetIf(){
    for(int i=0;i<8;i++){
        icons[i]->addPixmap(*socPixmap[15], QIcon::Normal, QIcon::On);
        en[i]->setIcon(*icons[i]);
        en[i]->setCheckable(false);
        audio[i]->setDisabled(true);
        dev[i]->setDisabled(true);
        as[i]->setDisabled(true);
        seq[i]->setDisabled(true);
    }
}

void Widget::changeState(int no)
{
    changePinsConfig();
    if(en[no]->checkState()==Qt::Checked){
        PinCompex *pin = dynamic_cast<PinCompex*> (device->activePinWidgets[no]);
        device->lockPath(pin, audio[no]->currentIndex());
        device->updatePaths();
        updateView();
        dev[no]->setDisabled(true);
        audio[no]->setDisabled(true);
        as[no]->setDisabled(true);
        seq[no]->setDisabled(true);
    } else{
        PinCompex *pin = dynamic_cast<PinCompex*> (device->activePinWidgets[no]);
        device->unlockPath(pin);
        device->updatePaths();
        updateView();
        dev[no]->setDisabled(false);
        audio[no]->setDisabled(false);
        as[no]->setDisabled(false);
        seq[no]->setDisabled(false);
    }
}

void Widget::updateView()
{
    PinCompex *pin;
    int k=0,j=0;

    for(QVector<Node*>::iterator it = device->activePinWidgets.begin();
        it != device->activePinWidgets.end(); it++)
    {
        if( !(pin = dynamic_cast<PinCompex*>(*it))) continue; /** skip NULL pins */
        if(pin->confParams.loc==LOC_RearExt && j<6){
            as[j]->setValue(pin->confParams.as);
            seq[j]->setValue(pin->confParams.seq);
            audio[j]->clear();
            audio[j]->addItems(device->getConsNames(pin));
            audio[j]->setCurrentIndex(pin->getSelectedPathNo());
            j++;
        } else if(pin->confParams.loc == LOC_FrontExt){
            as[6+k]->setValue(pin->confParams.as);
            seq[6+k]->setValue(pin->confParams.seq);
            audio[6+k]->clear();
            audio[6+k]->addItems(device->getConsNames(pin));
            audio[6+k]->setCurrentIndex(pin->getSelectedPathNo());
            k++;
        }
    }
}

void Widget::setDefault()
{
    icons[8]->addPixmap(*socPixmap[0], QIcon::Normal, QIcon::Off);
    QString nid;
    int k=0,j=0;
    PinCompex *pin;

    for(QVector<Node*>::iterator it = device->activePinWidgets.begin();
        it != device->activePinWidgets.end(); it++)
    {
        if( !(pin = dynamic_cast<PinCompex*>(*it))) continue; /** skip NULL pins */
        nid="0x" + nid.setNum(pin->NID, 16) +
                " " + locToStr(pin->confParams.loc) + " panel";
        if((pin->confParams.loc == LOC_FrontExt || pin->confParams.loc == LOC_Left) && k<2){
            icons[6+k]->addPixmap(*socPixmap[pin->confParams.color], QIcon::Normal, QIcon::On);
            as[6+k]->setValue(pin->confParams.as);
            as[6+k]->setEnabled(true);
            seq[6+k]->setValue(pin->confParams.seq);
            seq[6+k]->setEnabled(true);
            dev[6+k]->setCurrentIndex(toDevId(pin->confParams.dev));
            dev[6+k]->setEnabled(true);
            en[6+k]->setCheckable(true);
            en[6+k]->setToolTip(nid);
            en[6+k]->setChecked(false);
            en[6+k]->setIcon(*icons[6+k]);
            audio[6+k]->setEnabled(true);
            audio[6+k]->clear();
            audio[6+k]->addItems(device->getConsNames(pin));
            audio[6+k]->setCurrentIndex(pin->getSelectedPathNo());
            k++;
        } else if(j<6){
            icons[j]->addPixmap(*socPixmap[pin->confParams.color], QIcon::Normal, QIcon::On);
            as[j]->setValue(pin->confParams.as);
            as[j]->setEnabled(true);
            seq[j]->setValue(pin->confParams.seq); 
            seq[j]->setEnabled(true);
            dev[j]->setCurrentIndex(toDevId(pin->confParams.dev));
            dev[j]->setEnabled(true);
            en[j]->setIcon(*icons[j]);
            en[j]->setCheckable(true);
            en[j]->setChecked(false);
            en[j]->setToolTip(nid);
            audio[j]->setEnabled(true);
            audio[j]->clear();
            audio[j]->addItems(device->getConsNames(pin));
            audio[j]->setCurrentIndex(pin->getSelectedPathNo());
            j++;
        }
    }
}

void Widget::changePinCtrls(int pinNo)
{
    PinCompex *pin;
    int devNo = toDevNo(dev[pinNo]->currentIndex());

    if( (pin = dynamic_cast<PinCompex*>(device->activePinWidgets[pinNo]))){ /** skip NULL pins */
        pin->pinCtrl &= !(PWCTRL_OUT | PWCTRL_IN | PWCTRL_HP);

        if(DEV_LineOut <= devNo && devNo <= DEV_DigitalOtherOut)
              pin->pinCtrl |= PWCTRL_OUT;
        else if(DEV_LineIn <= devNo && devNo <= DEV_DigitalOtherIn)
            pin->pinCtrl |= PWCTRL_IN;

        if(devNo == DEV_HP_out)
            pin->pinCtrl |= PWCTRL_HP;

        audio[pinNo]->clear();
        audio[pinNo]->addItems(device->getConsNames(pin));
        audio[pinNo]->setCurrentIndex(pin->getSelectedPathNo());
    }
}

void Widget::changePinsConfig()
{
    PinCompex *pin;

    for(int j=0; j<device->activePinWidgets.size(); j++){
        if( !(pin = dynamic_cast<PinCompex*>(device->activePinWidgets[j]))) continue; /** skip NULL pins */
            pin->confParams.dev=toDevNo(dev[j]->currentIndex());
            pin->confParams.as=as[j]->value();
            pin->confParams.seq=seq[j]->value();
    }
}

void Widget::on_chb_asno_seqno_gen_auto_stateChanged(int)
{
    if(ui->chb_asno_seqno_gen_auto->checkState()==Qt::Checked){
        for(int i=0;i<8;i++){
            as[i]->setReadOnly(true);
            seq[i]->setReadOnly(true);
        }
    } else{
        for(int i=0;i<8;i++){
            as[i]->setReadOnly(false);
            seq[i]->setReadOnly(false);
        }
    }
}

void Widget::on_checkBox_stateChanged(int)
{
    changeState(0);
}

void Widget::on_checkBox_2_stateChanged(int)
{
    changeState(1);
}

void Widget::on_checkBox_3_stateChanged(int)
{
    changeState(2);
}

void Widget::on_checkBox_4_stateChanged(int)
{
   changeState(3);
}

void Widget::on_checkBox_5_stateChanged(int)
{
    changeState(4);
}

void Widget::on_checkBox_6_stateChanged(int)
{
    changeState(5);
}

void Widget::on_checkBox_7_stateChanged(int)
{
   changeState(6);
}

void Widget::on_checkBox_8_stateChanged(int)
{
    changeState(7);
}

void Widget::on_pushButton_2_clicked()
{
    this->close();
}

void Widget::on_bt_apply_clicked()
{
    if(ui->chb_asno_seqno_gen_auto->checkState()==Qt::Checked){
        device->generateAsSeqNumbers();
        updateView();
    }

    device->applySettings(); /** Must apply before pin default config */
    device->applyPinConfSettings();

}

void Widget::on_bt_write_settings_clicked()
{
    if(ui->chb_asno_seqno_gen_auto->checkState()==Qt::Checked){
        device->generateAsSeqNumbers();
        updateView();
    }

    device->writeSettings();
}

void Widget::on_cob_dev0_currentIndexChanged(int)
{
    if(initDone)
        changePinCtrls(0);
}

void Widget::on_cob_dev1_currentIndexChanged(int)
{
    if(initDone)
        changePinCtrls(1);
}

void Widget::on_cob_dev2_currentIndexChanged(int)
{
    if(initDone)
        changePinCtrls(2);
}

void Widget::on_cob_dev3_currentIndexChanged(int)
{
    if(initDone)
        changePinCtrls(3);
}

void Widget::on_cob_dev4_currentIndexChanged(int)
{
    if(initDone)
        changePinCtrls(4);
}

void Widget::on_cob_dev5_currentIndexChanged(int)
{
    if(initDone)
        changePinCtrls(5);
}

void Widget::on_cob_dev6_currentIndexChanged(int)
{
    if(initDone)
        changePinCtrls(6);
}

void Widget::on_cob_dev7_currentIndexChanged(int)
{
    if(initDone)
        changePinCtrls(7);
}


void Widget::on_bt_reset_to_def_clicked()
{
    device->getDefaultConfig();
    device->disableConfig();
    setDefault();
}

void Widget::on_bt_reread_settings_clicked()
{
    delete device;
    device = new Codec(ui->cmb_device->currentText());
    setDefault();
}

void Widget::on_cmb_device_currentIndexChanged(const QString &arg1)
{
    if(initDone){
        resetIf();
        delete device;
        device = new Codec(arg1);
        setDefault();
    }
}
