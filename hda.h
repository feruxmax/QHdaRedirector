#ifndef HDA_H
#define HDA_H


enum PortConnectivity{
    PC_toJack,
    PC_toNotPhis,
    PC_toFixed,
    PC_toBoth
};

enum ConnectionType{
    CT_Unknown,
    CT_1_8SM,
    CT_1_4SM,
    CT_1_ATAPI_internal,
    CT_RSA,
    CT_Optical,
    CT_OtherDigital,
    CT_OtherAnalog,
    CT_MulChAnalog,
    CT_XLR,
    CT_RJ11,
    CT_Comb,
    CT_Other
};

enum Location_e{
    LOC_RearExt = 0x01,
    LOC_FrontExt,
    LOC_Left,
    LOC_Right,
    LOC_Top,
    LOC_Bottom
//other not defined
};

enum DefaultDevice{
        DEV_LineOut,
        DEV_Speaker,
        DEV_HP_out,
        DEV_CD,
        DEV_SPDI_out,
        DEV_DigitalOtherOut,
        DEV_ModemLineSide,
        DEV_ModemHandsetSide,
        DEV_LineIn,
        DEV_AUX,
        DEV_MicIn,
        DEV_Telephony,
        DEV_SPDIF_in,
        DEV_DigitalOtherIn,
        DEV_Reserved,
        DEV_Other
};

enum Verbs_e{
    VB_PARAMETERS = 0xf00,
    VB_GET_SELECT_CTRL = 0xf01,
    VB_GET_CONN_LIST = 0xf02,
    VB_GET_PIN_WIDGET_CTRL = 0xf07,
    VB_GET_CONFIG_DEFAULT = 0xf1c,
    VB_GET_IMPLEMENTATION_ID = 0xf20,
    VB_SET_SELECT_CTRL = 0x701,
    VB_SET_PIN_WIDGET_CTRL = 0x707,
    VB_SET_PIN_DEF_CONF_BYTES_0 = 0x71c,
    VB_SET_PIN_DEF_CONF_BYTES_1,
    VB_SET_PIN_DEF_CONF_BYTES_2,
    VB_SET_PIN_DEF_CONF_BYTES_3
};

enum Parameter_e{
    PAR_VENDOR_ID = 0x00,
    PAR_SUBSYSTEM_ID = 0x01,
    PAR_NODE_COUNT = 0x04,
    PAR_AUDIO_WIDGET_CAP = 0x09,
    PAR_CONNLIST_LEN = 0x0e
};

enum PinWidgetCtrl_e{
    PWCTRL_IN = 0x20,
    PWCTRL_OUT= 0x40,
    PWCTRL_HP = 0x80
};
#endif // HDA_H
