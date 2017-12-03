#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtGui>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

#include "codec.h"

namespace Ui {
    class Widget;
}

class Widget : public QWidget {
    Q_OBJECT
public:
    Widget(QWidget *parent = 0);
    ~Widget();

protected:


private:
    Ui::Widget *ui;
    bool initDone;
    Codec *device;

    QComboBox *dev[8];
    QComboBox *audio[8];
    QSpinBox *as[8];
    QSpinBox *seq[8];
    QCheckBox *en[8];
    QPixmap *socPixmap[16];
    QIcon *icons[9];

    void changeState(int no);
    int toDevId(int);
    int toDevNo(int);
    void updateView();
    void setDefault();
    void changePinsConfig();
    void changePinCtrls(int pinNo);
    QStringList& getDevices();
    void resetIf();
    const char *locToStr(int loc);

private slots:


private slots:
    void on_pushButton_2_clicked();
    void on_checkBox_stateChanged(int );
    void on_checkBox_2_stateChanged(int );
    void on_checkBox_3_stateChanged(int );
    void on_checkBox_4_stateChanged(int );
    void on_checkBox_5_stateChanged(int );
    void on_checkBox_6_stateChanged(int );
    void on_checkBox_7_stateChanged(int );
    void on_checkBox_8_stateChanged(int );


    void on_bt_apply_clicked();
    void on_bt_write_settings_clicked();
    void on_cob_dev0_currentIndexChanged(int);
    void on_cob_dev1_currentIndexChanged(int);
    void on_cob_dev2_currentIndexChanged(int);
    void on_cob_dev3_currentIndexChanged(int);
    void on_cob_dev4_currentIndexChanged(int);
    void on_cob_dev5_currentIndexChanged(int);
    void on_cob_dev6_currentIndexChanged(int);
    void on_cob_dev7_currentIndexChanged(int);
    void on_chb_asno_seqno_gen_auto_stateChanged(int);
    void on_bt_reset_to_def_clicked();
    void on_bt_reread_settings_clicked();
    void on_cmb_device_currentIndexChanged(const QString &arg1);
};

#endif // WIDGET_H
