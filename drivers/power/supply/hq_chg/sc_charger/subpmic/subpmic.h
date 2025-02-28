// SPDX-License-Identifier: GPL-2.0
/**
 * Copyright (c) 2022 - 2023 SOUTHCHIP Semiconductor Technology(Shanghai) Co., Ltd.
 */
#ifndef __LINUX_SUBPMIC_H__
#define __LINUX_SUBPMIC_H__

// Reg DEFINE
enum {
    SUBPMIC_REG_DEVICE_ID = 0X00,
    SUBPMIC_REG_HK_GEN_STATE,
    SUBPMIC_REG_HK_GEN_FLG,
    SUBPMIC_REG_HK_GEN_MASK,
    SUBPMIC_REG_VAC_VBUS_OVP,
    SUBPMIC_REG_TSBUS_FAULT,
    SUBPMIC_REG_TSBAT_FAULT,
    SUBPMIC_REG_HK_CTRL,
    SUBPMIC_REG_HK_INT_STAT = 0X09,
    SUBPMIC_REG_HK_INT_FLG,
    SUBPMIC_REG_HK_INT_MASK,
    SUBPMIC_REG_HK_FLT_STAT,
    SUBPMIC_REG_HK_FLT_FLG,
    SUBPMIC_REG_HK_FLT_MASK,
    SUBPMIC_REG_HK_ADC_CTRL,
    SUBPMIC_REG_HK_IBUS_ADC = 0X11,
    SUBPMIC_REG_HK_VBUS_ADC = 0X13,
    SUBPMIC_REG_HK_VAC_ADC = 0X15,
    SUBPMIC_REG_HK_VBATSNS_ADC = 0X17,
    SUBPMIC_REG_HK_VBAT_ADC = 0X19,
    SUBPMIC_REG_HK_IBAT_ADC = 0X1B,
    SUBPMIC_REG_HK_VSYS_ADC = 0X1D,
    SUBPMIC_REG_HK_TSBUS_ADC = 0X1F,
    SUBPMIC_REG_HK_TSBAT_ADC = 0X21,
    SUBPMIC_REG_HK_TDIE_ADC = 0X23,
    SUBPMIC_REG_HK_BATID_ADC = 0X25,
    /****** Buck charger Reg ******/
    SUBPMIC_REG_VSYS_MIN = 0x30,
    SUBPMIC_REG_VBAT,
    SUBPMIC_REG_ICHG_CC,
    SUBPMIC_REG_VINDPM,
    SUBPMIC_REG_IINDPM,
    SUBPMIC_REG_ICO_CTRL,
    SUBPMIC_REG_PRECHARGE_CTRL,
    SUBPMIC_REG_TERMINATION_CTRL,
    SUBPMIC_REG_RECHARGE_CTRL,
    SUBPMIC_REG_VBOOST_CTRL,
    SUBPMIC_REG_PROTECTION_DIS,
    SUBPMIC_REG_RESET_CTRL,
    SUBPMIC_REG_CHG_CTRL,
    SUBPMIC_REG_CHG_INT_STAT = 0x41,
    SUBPMIC_REG_CHG_INT_FLG = 0x44,
    SUBPMIC_REG_CHG_INT_MASK = 0x47,
    SUBPMIC_REG_CHG_FLT_STAT = 0x50,
    SUBPMIC_REG_CHG_FLT_FLG = 0x52,
    SUBPMIC_REG_CHG_FLT_MASK = 0x54,
    SUBPMIC_REG_JEITA_TEMP = 0x56,
    SUBPMIC_REG_BUCK_STAT,
    SUBPMIC_REG_BUCK_FLG,
    SUBPMIC_REG_BUCK_MASK,
    SUBPMIC_REG_Internal = 0x5D,
    /*******LED Reg *******/
    SUBPMIC_REG_LED_CTRL = 0x80,
    SUBPMIC_REG_FLED1_BR_CTR,
    SUBPMIC_REG_FLED2_BR_CTR,
    SUBPMIC_REG_FLED_TIMER,
    SUBPMIC_REG_TLED1_BR_CTR,
    SUBPMIC_REG_TLED2_BR_CTR,
    SUBPMIC_REG_LED_PRO,
    SUBPMIC_REG_LED_STATE,
    SUBPMIC_REG_LED_FLAG = 0x89,
    SUBPMIC_REG_LED_MASK,
    /****** DPDM Reg ******/
    SUBPMIC_REG_DPDM_EN = 0x90,
    SUBPMIC_REG_DPDM_CTRL,
    SUBPMIC_REG_DPDM_QC_CTRL,
    SUBPMIC_REG_DPDM_TFCP_CTRL,
    SUBPMIC_REG_DPDM_INT_FLAG,
    SUBPMIC_REG_DPDM_INT_MASK,
    SUBPMIC_REG_QC3_INT_FLAG,
    SUBPMIC_REG_QC3_INT_MASK,
    SUBPMIC_REG_DP_STAT,
    SUBPMIC_REG_DM_STAT,
    SUBPMIC_REG_DPDM_INTERNAL,
    SUBPMIC_REG_DPDM_CTRL_2 = 0x9D,
    SUBPMIC_REG_DPDM_NONSTD_STAT,
    SUBPMIC_REG_MAX,
};

/***************** hk **********************/
#define SUBPMIC_TDIE_ADC_DIS                BIT(0)
#define SUBPMIC_TSBAT_ADC_DIS               BIT(1)
#define SUBPMIC_TSBUS_ADC_DIS               BIT(2)
#define SUBPMIC_ADC_EN                      BIT(15)

static const int subpmic_chg_wd_time[] = {
    0, 500, 1000, 2000, 20000, 40000, 80000, 160000,
};

#define SUBPMIC_HK_V2X_UVLO_MASK             BIT(7)
#define SUBPMIC_HK_POR_MASK                  BIT(6)
#define SUBPMIC_HK_RESET_MASK                BIT(5)
#define SUBPMIC_HK_ADC_DONE_MASK             BIT(4)
#define SUBPMIC_HK_REGN_OK_MASK              BIT(3)
#define SUBPMIC_HK_VBAT_UVLO_MASK            BIT(2)
#define SUBPMIC_HK_VBUS_PRESENT_MASK         BIT(1)
#define SUBPMIC_HK_VAC_PRESENT_MASK          BIT(0)

/***************** charger **********************/
#define SUBPMIC_BUCK_TDIE_REG_MASK           BIT(22)
#define SUBPMIC_BUCK_TSBAT_COOL_MASK         BIT(21)
#define SUBPMIC_BUCK_TSBAT_WARM_MASK         BIT(20)
#define SUBPMIC_BUCK_ICO_MASK                BIT(18)
#define SUBPMIC_BUCK_IINDPM_MASK             BIT(17)
#define SUBPMIC_BUCK_VINDPM_MASK             BIT(16)
#define SUBPMIC_BUCK_CHG_MASK                BIT(13)
#define SUBPMIC_BUCK_BOOST_OK_MASK           BIT(12)
#define SUBPMIC_BUCK_VSYSMIN_MASK            BIT(11)
#define SUBPMIC_BUCK_QB_ON_MASK              BIT(10)
#define SUBPMIC_BUCK_BATFET_MASK             BIT(8)
#define SUBPMIC_BUCK_VSYS_SHORT_MASK         BIT(4)
#define SUBPMIC_BUCK_VSLEEP_BUCK_MASK        BIT(3)
#define SUBPMIC_BUCK_VBAT_DPL_MASK           BIT(2)
#define SUBPMIC_BUCK_VBAT_LOW_BOOST_MASK     BIT(1)
#define SUBPMIC_BUCK_VBUS_GOOD_MASK          BIT(0)


#define SUBPMIC_QC_EN                       BIT(0)
#define SUBPMIC_BUCK_FLAG_BOOST_GOOD        BIT(12)
#define SUBPMIC_BUCK_FLAG_VBUS_GOOD         BIT(0)
#define SUBPMIC_BUCK_GET_CHG_STATE(state)   ((state >> 13) & 0x7)

#define SUBPMIC_BUCK_VBAT_OFFSET            3840
#define SUBPMIC_BUCK_VBAT_STEP              8
#define SUBPMIC_BUCK_VBAT_MIN               3840
#define SUBPMIC_BUCK_VBAT_MAX               4856

#define SUBPMIC_BUCK_ICHG_OFFSET            0
#define SUBPMIC_BUCK_ICHG_STEP              50
#define SUBPMIC_BUCK_ICHG_MIN               0
#define SUBPMIC_BUCK_ICHG_MAX               3600

#define SUBPMIC_BUCK_IINDPM_OFFSET          100
#define SUBPMIC_BUCK_IINDPM_STEP            50
#define SUBPMIC_BUCK_IINDPM_MIN             100
#define SUBPMIC_BUCK_IINDPM_MAX             3250

#define SUBPMIC_BUCK_IINDPM_ICO_OFFSET      100
#define SUBPMIC_BUCK_IINDPM_ICO_STEP        50

#define SUBPMIC_BUCK_IPRECHG_OFFSET         50
#define SUBPMIC_BUCK_IPRECHG_STEP           50

#define SUBPMIC_BUCK_ITERM_OFFSET           100
#define SUBPMIC_BUCK_ITERM_STEP             50
#define SUBPMIC_BUCK_ITERM_MIN              100
#define SUBPMIC_BUCK_ITERM_MAX              1650


#define SUBPMIC_BUCK_OTG_VOLT_OFFSET        3900
#define SUBPMIC_BUCK_OTG_VOLT_STEP          100
#define SUBPMIC_BUCK_OTG_VOLT_MIN           3900
#define SUBPMIC_BUCK_OTG_VOLT_MAX           5800

#define SUBPMIC_BUCK_PRE_CURR_OFFSET        50
#define SUBPMIC_BUCK_PRE_CURR_STEP          50
#define SUBPMIC_BUCK_PRE_CURR_MIN           50
#define SUBPMIC_BUCK_PRE_CURR_MAX           800

static const int vsys_min[] = {
    2600, 2800, 3000, 3200, 3400, 3500, 3600, 3700,
};

static const int boost_curr[] = {
    500, 900, 1300, 1500, 2100, 2500, 2900, 3250,
};

static const int vindpm[] = {
    4000, 4100, 4200, 4300, 4400, 4500, 4600, 4700,
    7600, 8200, 8400, 8600, 10000, 10500, 10700,
};

static const int prechg_volt[] = {
    2900, 3000, 3100, 3200,
};

static const int rechg_volt[] = {
    100, 200, 300, 400,   /* below VBAT_REG */
};

/****************** led ****************************/
#define SUBPMIC_LED_FLAG_FLASH_DONE         (BIT(7) | BIT(6))
#define SUBPMIC_LED_CURR_STEP               12500
#define SUBPMIC_LED_CURR_OFFSET             25000

#define SUBPMIC_LED_FLASH_CURR_MIN      25
#define SUBPMIC_LED_FLASH_CURR_MAX      1500
#define SUBPMIC_LED_FLASH_CURR_OFFSET   25000
#define SUBPMIC_LED_FLASH_CURR_STEP     12500

#define SUBPMIC_LED_TORCH_CURR_MIN      25
#define SUBPMIC_LED_TORCH_CURR_MAX      500
#define SUBPMIC_LED_TORCH_CURR_OFFSET   25000
#define SUBPMIC_LED_TORCH_CURR_STEP     12500

#define SUBPMIC_LED_VBAT_MIN_OFFSET      2800
#define SUBPMIC_LED_VBAT_MIN_STEP        100
#define SUBPMIC_LED_VBAT_MIN_MIN         2800
#define SUBPMIC_LED_VBAT_MIN_MAX         3500

#define SUBPMIC_DISABLE_FLASH_TIMEOUT        -1
#define SUBPMIC_DEFAULT_FLASH_TIMEOUT        200

#define SUBPMIC_LED_FTIMEOUT1_MASK       BIT(7)
#define SUBPMIC_LED_FTIMEOUT2_MASK       BIT(6)
#define SUBPMIC_LED_OVP_MASK             BIT(5)
#define SUBPMIC_LED1_SHORT_MASK          BIT(4)
#define SUBPMIC_LED2_SHORT_MASK          BIT(3)
#define SUBPMIC_LED_TORCH_UVP_MASK       BIT(2)
#define SUBPMIC_LED_TORCH_OVP_MASK       BIT(1)
#define SUBPMIC_LED_VBAT_LOW_MASK        BIT(0)

static const int led_time[] = {
    10, 20, 30, 40, 50, 60, 70, 80, 90,
    100, 150, 200, 250, 300, 350, 400,
};

/****************** dpdm ****************************/
#define SUBPMIC_DPDM_FORCE_INDET            BIT(7)
#define SUBPMIC_DPDM_AUTO_INDET_EN          BIT(6)
#define SUBPMIC_DPDM_HVDCP_EN               BIT(5)

#define SUBPMIC_DPDM_BC12_DETECT_DONE       BIT(2)
#define SUBPMIC_DPDM_BC1_2_DONE_MASK        BIT(2)
#define SUBPMIC_DPDM_DP_OVP_MASK            BIT(1)
#define SUBPMIC_DPDM_DM_OVP_MASK            BIT(0)

#endif
