/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#ifndef __RSI_MGMT_H__
#define __RSI_MGMT_H__

#include <linux/sort.h>
#include "rsi_boot_params.h"
#include "rsi_main.h"

#define MAX_MGMT_PKT_SIZE   512
#define RSI_NEEDED_HEADROOM 80
#define RSI_RCV_BUFFER_LEN  2000

#define RSI_11B_MODE        0
#define RSI_11G_MODE        BIT(7)
#define RETRY_COUNT         15
#define RETRY_LONG          4
#define RETRY_SHORT         7
#define WMM_SHORT_SLOT_TIME 9
#ifdef CONFIG_STA_PLUS_AP
#define VAP_ID_OFFSET 15
#endif
#define SIFS_DURATION  16
#define SNIFFER_ENABLE 0x1
#define DSBL_TA_AGGR   0x1

#define KEY_TYPE_CLEAR   0
#define RSI_PAIRWISE_KEY 1
#define RSI_GROUP_KEY    2

#define RSI_MAC_SUB_LEN 3

#define NULL_SSID       2
#define INVALID_DATA    64
#define DWORD_ALIGNMENT 64

/* EPPROM_READ_ADDRESS */
#define WLAN_MAC_EEPROM_ADDR    40
#define WLAN_MAC_MAGIC_WORD_LEN 0x01
#define WLAN_HOST_MODE_LEN      0x04
#define WLAN_FW_VERSION_LEN     0x08
#define MAGIC_WORD              0x5A
#define WLAN_EEPROM_RFTYPE_ADDR 424

/*WOWLAN RESUME WAKEUP TYPES*/
#define UNICAST_MAGIC_PKT  BIT(0)
#define BROADCAST_MAGICPKT BIT(1)
#define EAPOL_PKT          BIT(2)
#define DISCONNECT_PKT     BIT(3)
#define HW_BMISS_PKT       BIT(4)
#define INSERT_SEQ_IN_FW   BIT(2)

#define RSI_BAND_CHECK 0x03

/* Receive Frame Types */
enum rx_cmd_type {
  CARD_READY_IND       = 0x0,
  TA_CONFIRM_TYPE      = 0x01,
  RX_DOT11_MGMT        = 0x02,
  RX_DOT11_DATA        = 0x03,
  TX_STATUS_IND        = 0x04,
  PS_NOTIFY_IND        = 0x05,
  SLEEP_NOTIFY_IND     = 0x06,
  DECRYPT_ERROR_IND    = 0x07,
  BEACON_EVENT_IND     = 0x08,
  DEBUG_IND            = 0x09,
  RX_MISC_IND          = 0xa,
  UNCONNECTED_PEER     = 0xb,
  HW_BMISS_EVENT       = 0xc,
  RATE_GC_TABLE_UPDATE = 0xd,
  RADAR_DETECTED       = 0x0e,
  TSF_SYNC_CONFIRM     = 0xc0,
  ANTENNA_SELECT       = 0xf,
  RADIO_MEAS_RPT       = 0x10,
  FW_ERROR_STATE_IND   = 0x1C,

};

#ifdef CONFIG_RSI_WOW
#define WOW_MAX_FILTERS_PER_LIST 16
#define WOW_PATTERN_SIZE         256
#endif
#define EAPOL4_CONFIRM   1
#define PROBEREQ_CONFIRM 2
#define NULLDATA_CONFIRM 3

#define NULLDATA_SUCCESS 0
#define NULLDATA_FAIL    1
#define MIN_BEACON_INTVL 56
#define MAX_BEACON_INTVL 1000

#define RSI_MAX_BGSCAN_CHANNEL_SUPPORTED 0x6F
#define RSI_MAX_BGSCAN_PROBE_REQ_LEN     0x71
#define HW_BMISS_THRESHOLD               20
#define DEFAULT_KEEP_ALIVE_PERIOD        90
#define CHANNEL_CHANGE_EVENT             BIT(0)

#define RSI_DELETE_PEER   0x0
#define RSI_ADD_PEER      0x1
#define START_AMPDU_AGGR  0x1
#define STOP_AMPDU_AGGR   0x0
#define INTERNAL_MGMT_PKT BIT(0)
#define ENABLE_RX_STBC    0x0100
#define ENCAP_OFFLOAD_EN  BIT(1)

#define PUT_BBP_RESET                0
#define BBP_REG_WRITE                0
#define RF_RESET_ENABLE              BIT(3)
#define RATE_INFO_ENABLE             BIT(0)
#define MORE_DATA_PRESENT            BIT(1)
#define RSI_BROADCAST_PKT            BIT(9)
#define RSI_DESC_11G_MODE            BIT(7)
#define RSI_DESC_REQUIRE_CFM_TO_HOST BIT(10)
#define ADD_DELTA_TSF_VAP_ID         BIT(11)
#define FETCH_RETRY_CNT_FRM_HST      BIT(12)
#define ENCAP_OFFLOAD_DATA_PKT       BIT(0)
#define QOS_SUPPORT                  BIT(12)
#define ENABLE_ENCRYPTION            BIT(15)
#define ENCAP_OFFLOAD_MGMT_PKT       BIT(15)
#define EAPOL_4_LEN                  113

/*Mac flags*/
#define QUEUE_TO_HEAD BIT(13)

#define UPPER_20_ENABLE         (0x2 << 12)
#define LOWER_20_ENABLE         (0x4 << 12)
#define FULL40M_ENABLE          0x6
#define RSI_UAPSD_WAKEUP_PERIOD 100
#define RSI_LMAC_CLOCK_80MHZ    0x1
#define RSI_ENABLE_40MHZ        (0x1 << 3)
#define ENABLE_SHORTGI_RATE     BIT(9)

#define RX_BA_INDICATION   1
#define RSI_TBL_SZ         40
#define MAX_RETRIES        8
#define RSI_IFTYPE_STATION 1

#define STD_RATE_MCS7 0x07
#define STD_RATE_MCS6 0x06
#define STD_RATE_MCS5 0x05
#define STD_RATE_MCS4 0x04
#define STD_RATE_MCS3 0x03
#define STD_RATE_MCS2 0x02
#define STD_RATE_MCS1 0x01
#define STD_RATE_MCS0 0x00
#define STD_RATE_54   0x6c
#define STD_RATE_48   0x60
#define STD_RATE_36   0x48
#define STD_RATE_24   0x30
#define STD_RATE_18   0x24
#define STD_RATE_12   0x18
#define STD_RATE_11   0x16
#define STD_RATE_09   0x12
#define STD_RATE_06   0x0C
#define STD_RATE_5_5  0x0B
#define STD_RATE_02   0x04
#define STD_RATE_01   0x02

#define RSI_RF_TYPE      1
#define RSI_RATE_00      0x00
#define RSI_RATE_1       0x0
#define RSI_RATE_2       0x2
#define RSI_RATE_5_5     0x4
#define RSI_RATE_11      0x6
#define RSI_RATE_6       0x8b
#define RSI_RATE_9       0x8f
#define RSI_RATE_12      0x8a
#define RSI_RATE_18      0x8e
#define RSI_RATE_24      0x89
#define RSI_RATE_36      0x8d
#define RSI_RATE_48      0x88
#define RSI_RATE_54      0x8c
#define RSI_RATE_MCS0    0x100
#define RSI_RATE_MCS1    0x101
#define RSI_RATE_MCS2    0x102
#define RSI_RATE_MCS3    0x103
#define RSI_RATE_MCS4    0x104
#define RSI_RATE_MCS5    0x105
#define RSI_RATE_MCS6    0x106
#define RSI_RATE_MCS7    0x107
#define RSI_RATE_MCS7_SG 0x307

#define _11G_RATE_MAP    (0xff << 4)
#define _11B_RATE_MAP    (0xf)
#define RSI_RATE_5_5_INX (BIT(2))
#define RSI_RATE_11_INX  (BIT(3))
#define RSI_RATE_9_INX   (BIT(5))
#define RSI_RATE_36_INX  (BIT(9))

#define MCS_RATE_MAP        0xff
#define _5G_LEGACY_RATE_MAP 0xff
#define _2G_LEGACY_RATE_MAP 0xfff
#define LEGACY_RATE         0
#define MCS_RATE            1
#define HE_MCS_RATE         2
#define HE_MCS_RATE_MAP     0xff
#define HE_DISABLE          0

#define BW_20MHZ 0
#define BW_40MHZ 1
#define BW_10MHZ 2
#define BW_5MHZ  3

#define EP_2GHZ_20MHZ 0
#define EP_2GHZ_40MHZ 1
#define EP_5GHZ_20MHZ 2
#define EP_5GHZ_40MHZ 3

#define SIFS_TX_11N_VALUE   580
#define SIFS_TX_11B_VALUE   346
#define SHORT_SLOT_VALUE    360
#define LONG_SLOT_VALUE     640
#define OFDM_ACK_TOUT_VALUE 2720
#define CCK_ACK_TOUT_VALUE  9440
#define LONG_PREAMBLE       0x0000
#define SHORT_PREAMBLE      0x0001

#define RSI_SUPP_FILTERS (FIF_ALLMULTI | FIF_PROBE_REQ | FIF_BCN_PRBRESP_PROMISC)

#define ANTENNA_SEL_INT 0x02 /* RF_OUT_2 / Integerated */
#define ANTENNA_SEL_UFL 0x03 /* RF_OUT_1 / U.FL */

/* Power save handshake types */
#define NO_HAND_SHAKE     0
#define GPIO_HAND_SHAKE   1
#define PACKET_HAND_SHAKE 2
#define TA_GPIO           0
#define ULP_GPIO          1
#define RF_POWER_3_3      1
#define RF_POWER_1_9      0
#define PAUSE_WIFI        1
#define RESUME_WIFI       2

/* Rx filter word definitions */
#define PROMISCOUS_MODE                     BIT(0)
#define ALLOW_DATA_ASSOC_PEER               BIT(1)
#define ALLOW_MGMT_ASSOC_PEER               BIT(2)
#define ALLOW_CTRL_ASSOC_PEER               BIT(3)
#define DISALLOW_BEACONS                    BIT(4)
#define ALLOW_CONN_PEER_MGMT_WHILE_BUF_FULL BIT(5)
#define DISALLOW_BROADCAST_DATA             BIT(6)

#define RSI_TXPOWER_MAX 20
#define RSI_TXPOWER_MIN -127

#define DEEP_SLEEP      1
#define CONNECTED_SLEEP 2

#define ULP_SLEEP_NOTIFY 6
#define ULP_SLEEP_ENTRY  1
#define ULP_TOKEN        cpu_to_le16(*(u16 *)&msg[12] >> 4)
#define SLEEP_REQUEST    1
#define WAKEUP_REQUEST   2

#define RSI_TXPOWER_MIN -127

#define BEACON_COUNTRY_IE 7

#define ENABLE_MAC_INFO BIT(0)
#define BROADCAST_IND   BIT(9)
#define CONTINUOUS_MODE BIT(10)

#define IEEE80211_MARKALL_UAPSD_QUEUES                                                                          \
  (IEEE80211_WMM_IE_STA_QOSINFO_AC_VO | IEEE80211_WMM_IE_STA_QOSINFO_AC_VI | IEEE80211_WMM_IE_STA_QOSINFO_AC_BE \
   | IEEE80211_WMM_IE_STA_QOSINFO_AC_BK)

/* Tx data frame format */
#define MAC_BBP_INFO BIT(0)
#define NO_ACK_IND   BIT(9)
#define QOS_EN       BIT(12)
/* frame type bit{11:10} */
#define NORMAL_FRAME            0x00
#define DTIM_BEACON_GATED_FRAME BIT(10)
#define BEACON_FRAME            BIT(11)
#define DTIM_BEACON             BIT(10) | BIT(11)
#define INSERT_TSF              BIT(15)
#define INSERT_SEQ_NO           BIT(2)

#ifdef CONFIG_PM
#define RSI_WOW_ANY        BIT(1)
#define RSI_WOW_GTK_REKEY  BIT(3)
#define RSI_WOW_MAGIC_PKT  BIT(4)
#define RSI_WOW_DISCONNECT BIT(5)
#endif
#ifndef CONFIG_STA_PLUS_AP
#define HOST_BG_SCAN_TRIG BIT(4)
#else
#define HOST_BG_SCAN_TRIG BIT(0)
#define BG_SCAN_TRIG      BIT(4)
#endif
#define TARGET_BOARD_CARACALLA   BIT(10)
#define RSI_TX_POWER_MODE_MASK   0x0F
#define RSI_RX_POWER_MODE_MASK   0xF0
#define TX_PWR_IN_SNIFFER_MODE   0
#define TX_AGGR_LIMIT_FOR_RS9116 16
#define TX_AGGR_LIMIT_FOR_11N    8
#define RX_AGGR_LIMIT_FOR_RS9116 8
#define TX_AGGR_LIMIT_FOR_RS9113 8
#define EN_ENHANCED_MAX_PSP      BIT(2)

enum opmode { UNKNOW_OPMODE = -1, AP_OPMODE = 0, STA_OPMODE = 1, P2P_GO_OPMODE = 2, P2P_CLIENT_OPMODE = 3 };

enum vap_status { VAP_ADD = 1, VAP_DELETE = 2, VAP_UPDATE = 3 };

enum peer_type { PEER_TYPE_AP, PEER_TYPE_STA, PEER_TYPE_P2P_GO, PEER_TYPE_P2P_CLIENT, PEER_TYPE_IBSS };

/*
 * Subtypes for RX_MISC_IND frame
 * Frame sub types from LMAC to Host
 */
enum rx_misc_ind_subtype { FW_UPGRADE_REQ };

extern struct ieee80211_rate rsi_rates[12];
extern u16 rsi_mcsrates[8];

enum sta_notify_events {
  STA_CONNECTED = 0,
  STA_DISCONNECTED,
  STA_TX_ADDBA_DONE,
  STA_TX_DELBA,
  STA_RX_ADDBA_DONE,
  STA_RX_DELBA
};

/* Send Frames Types */
enum cmd_frame_type {
  TX_DOT11_MGMT = 0,
  RESET_MAC_REQ,          /* 0x1 */
  RADIO_CAPABILITIES,     /* 0x2 */
  BB_PROG_VALUES_REQUEST, /* 0x3 */
  RF_PROG_VALUES_REQUEST, /* 0x4 */
  WAKEUP_SLEEP_REQUEST,   /* 0x5 */
  SCAN_REQUEST,           /* 0x6 */
  TSF_UPDATE,             /* 0x7 */
  PEER_NOTIFY,            /* 0x8 */
  BLOCK_HW_QUEUE,         /* 0x9 */
  SET_KEY_REQ,            /* 0xA */
  AUTO_RATE_IND,          /* 0xB */
  BOOTUP_PARAMS_REQUEST,  /* 0xC */
  VAP_CAPABILITIES,       /* 0xD */
  EEPROM_READ,            /* 0xE */
  EEPROM_WRITE,           /* 0xF */
  GPIO_PIN_CONFIG,        /* 0x10 */
  SET_RX_FILTER,          /* 0x11 */
  AMPDU_IND,              /* 0x12 */
  STATS_REQUEST,          /* 0x13 */
  BB_BUF_PROG_VALUES_REQ, /* 0x14 */
  BBP_PROG_IN_TA,         /* 0x15 */
  BG_SCAN_PARAMS,         /* 0x16 */
  BG_SCAN_PROBE_REQ,      /* 0x17 */
  CW_MODE_REQ,            /* 0x18 */
  PER_CMD_PKT,            /* 0x19 */
  DEV_SLEEP_REQUEST,      /* 0x1A */
  DEV_WAKEUP_CNF,         /* 0x1B */
  RF_LOOPBACK_REQ,        /* 0x1C */
  RF_LPBK_M3,             /* 0x1D */
  RF_RESET_FRAME,         /* 0x1E */
  SOC_REG_OPS,            /* 0x1F */
  ANT_SEL_FRAME,          /* 0x20 */
  CONFIRM,                /* 0x21 */
  WLAN_DE_REGISTER,       /* 0x22 */
  DEBUG_FRAME,            /* 0x23 */
  HW_BMISS_HANDLE,        /* 0x24 */
  MULTICAST_ENABLE,       /* 0x25 */
  TX_MISC_IND,            /* 0x26 */
  VAP_DYNAMIC_UPDATE,     /* 0x27 */
  COMMON_DEV_CONFIG,      /* 0x28 */
  RADIO_PARAMS_UPDATE,    /* 0x29 */
  RADAR_REQUEST,          /* 0x2A */
  WOWLAN_CONFIG_PARAMS,   /* 0x2B */
  IAP_CONFIG,             /* 0x2C */
  RADIO_MEASUREMENT_REQ,  /* 0x2D */
  FEATURES_ENABLE        = 0x33,
  DISABLE_PS_FROM_LMAC   = 0x35,
  WOWLAN_WAKEUP_REASON   = 0xc5, /* 0xC5 */
  WLAN_GAIN_TABLE_UPDATE = 0x45, /* 0x45 */
                                 //__9117_CODE_START
  REAL_TIME_STATS_REQ_FRAME  = 0x4A,
  REAL_TIME_STATS_RESP_FRAME = 0x2E,
  DIAG_TOOL_CMD              = 0x2F,
  HE_TB_MU_PARAM             = 0x49,
  //__9117_CODE_END
  FILTER_BCAST_FRAME_TYPE = 0x43,
  GET_RSSI_FRAME          = 0x4C
};

struct rsi_ulp_params {
  __le16 desc_word[8];
} __packed;

struct reg_class {
  u8 op_class;
  u8 bandwidth;
  u16 chans[20];
};

/* RSI Command packet formats */
struct rsi_mac_frame {
  __le16 desc_word[8];
} __packed;

struct rsi_boot_params_9116 {
  __le16 desc_word[8];
  struct bootup_params_9116 bootup_params;
} __packed;

struct rsi_boot_params {
  __le16 desc_word[8];
  struct bootup_params bootup_params;
} __packed;

struct bt_register_param {
  u16 desc_word[8];
  u8 params[9];
} __packed;

struct rsi_peer_notify {
  __le16 desc_word[8];
  u8 mac_addr[6];
  __le16 command;
  __le16 mpdu_density;
  __le16 reserved;
  __le32 sta_flags;
} __packed;

struct rsi_vap_caps {
  __le16 desc_word[8];
  u8 mac_addr[6];
  __le16 keep_alive_period;
  u8 bssid[6];
  __le16 reserved;
  __le32 flags;
  __le16 frag_threshold;
  __le16 rts_threshold;
  __le32 default_mgmt_rate;
  __le32 default_ctrl_rate;
  __le32 default_data_rate;
  __le16 beacon_interval;
  __le16 dtim_period;
  __le16 beacon_miss_threshold;
  //__9117_CODE_START
  __le16 txop_dur_rts_threshold;
  //__9117_CODE_END
} __packed;

struct rsi_dynamic_s {
  __le16 desc_word[8];

  struct framebody {
    __le16 data_rate;
    __le16 mgmt_rate;
    __le16 keep_alive_period;
  } frame_body;
} __packed;

#define RSI_MAX_BGS_PROBEREQ_LEN 120
struct rsi_bgscan_params {
  __le16 desc_word[8];
  __le16 bgscan_threshold;
  __le16 roam_threshold;
  __le16 bgscan_periodicity;
  u8 num_bg_channels;
  u8 two_probe;
  __le16 active_scan_duration;
  __le16 passive_scan_duration;
  __le16 channels2scan[MAX_BGSCAN_CHANNELS];
} __packed;

struct rsi_bgscan_probe {
  __le16 desc_word[8];
  __le16 mgmt_rate;
  __le16 flags;
  __le16 channel_num;
  __le16 channel_scan_time;
  __le16 probe_req_length;
} __packed;

struct rsi_set_key {
  __le16 desc_word[8];
  u8 key[4][32];
  u8 tx_mic_key[8];
  u8 rx_mic_key[8];
} __packed;

struct rsi_auto_rate {
  __le16 desc_word[8];
  __le16 failure_limit;
  __le16 initial_boundary;
  __le16 max_threshold_limt;
  __le16 num_supported_rates;
  __le16 aarf_rssi;
  __le16 moderate_rate_inx;
  __le16 collision_tolerance;
  __le16 supported_rates[40];
} __packed;
#define DISABLE_UPDATED_AUTORATE_PARAMS BIT(0)
#define DISABLE_AARF_RATE_BLOCKING      BIT(4)
//__9117_CODE_START
#define AX_RATE_VALID           BIT(10)
#define AX_RATE_MASK            0x7
#define MU_EDCA_EN              BIT(1)
#define BSS_COLOR_EN            BIT(2)
#define SPR_EN                  BIT(3)
#define MBSSID_MASK_EN          BIT(4)
#define SRG_VALID               BIT(5)
#define UORA_EN                 BIT(6)
#define BSR_EN                  BIT(6)
#define OBSS_PD_SR_EN           BIT(1)
#define SPR_VALUE               0x0
#define BSS_COLOR_VALUE         (adapter->ax_params.BSS_color)
#define EDCA_Q_EN               BIT(7)
#define MAX_TX_OP_RTS_TH        1023
#define MIN_TX_OP_RTS_TH        0
#define CONNECTION_TYPE         1 /* 11AX Mode */
#define EDCA_Q_SUSP             BIT(6)
#define ENABLE_ER_SU            BIT(14)
#define SU_PPDU                 0
#define ER_SU_PPDU              1
#define ERSU_HE_MIXED           0x1
#define ER_SU_ONLY              0x2
#define FIX_RATE_LESS_THAN_MCS3 ((common->fixed_rate & 0x7) <= 2)
#define SGI_RATE                ~(1 << 12)

typedef struct mu_edca_params_s {
  __le16 cw_max;
  unsigned char cw_min_lsb;
  unsigned char en_mu_edca_q;
  unsigned char cw_min_msb;
  unsigned char mu_edca_timer;
  __le16 mu_edca_aifsn;
} mu_edca_params_t;
//__9117_CODE_END

struct qos_params {
  __le16 cont_win_min_q;
  __le16 cont_win_max_q;
  __le16 aifsn_val_q;
  __le16 txop_q;
} __packed;

struct rsi_radio_caps {
  __le16 desc_word[8];
  struct qos_params qos_params[MAX_HW_QUEUES];
  u8 num_11n_rates;
  u8 num_11ac_rates;
  __le16 reserved; //! Added to support simulation compatiblity, can be removed
  __le16 sifs_tx_11n;
  __le16 sifs_tx_11b;
  __le16 slot_rx_11n;
  __le16 ofdm_ack_tout;
  __le16 cck_ack_tout;
  __le16 preamble_type;
  //__9117_CODE_START
  __le32 ax_feature_valids;
  mu_edca_params_t mu_edca_params[MAX_HW_QUEUES];
  __le16 bss_color_spr_value_mbssid_mask;
  __le16 trigger_response;
  __le16 spatial_reuse_enable;
  __le16 srg_obss_pd_value;
  __le32 srg_bss_color_bitmap[2];
  __le32 srg_partial_bssid_bitmap[2];
  __le16 ocw_range_value;
  __le16 reserved1;
  //__9117_CODE_END
} __packed;

struct rsi_ulp_gpio_vals {
  u8 motion_sensor_gpio_ulp_wakeup : 1;
  u8 sleep_ind_from_device : 1;
  u8 ulp_gpio_2 : 1;
  u8 push_button_ulp_wakeup : 1;
  u8 reserved : 4;
} __packed;

struct rsi_soc_gpio_vals {
  u32 pspi_csn_0 : 1;
  u32 pspi_csn_1 : 1;
  u32 host_wakeup_intr : 1;
  u32 pspi_data_0 : 1;
  u32 pspi_data_1 : 1;
  u32 pspi_data_2 : 1;
  u32 pspi_data_3 : 1;
  u32 i2c_scl : 1;
  u32 i2c_sda : 1;
  u32 uart1_rx : 1;
  u32 uart1_tx : 1;
  u32 uart1_rts_i2s_clk : 1;
  u32 uart1_cts_i2s_ws : 1;
  u32 dbg_uart_rx_i2s_din : 1;
  u32 dbg_uart_tx_i2s_dout : 1;
  u32 lp_wakeup_boot_bypass : 1;
  u32 led_0 : 1;
  u32 btcoex_wlan_active_ext_pa_ant_sel_A : 1;
  u32 btcoex_bt_priority_ext_pa_ant_sel_B : 1;
  u32 btcoex_bt_active_ext_pa_on_off : 1;
  u32 rf_reset : 1;
  u32 sleep_ind_from_device : 1;
} __packed;

struct rsi_config_vals {
  u16 desc_word[8];
  u8 lp_ps_handshake;
  u8 ulp_ps_handshake;
  u8 sleep_config_params; /* 0 for no handshake,
				 * 1 for GPIO based handshake,
				 * 2 packet handshake
				 */
  u8 unused_ulp_gpio;
  u32 unused_soc_gpio_bitmap;
  u8 ext_pa_or_bt_coex_en;
  u8 opermode;
  u8 wlan_rf_pwr_mode;
  u8 bt_rf_pwr_mode;
  u8 zigbee_rf_pwr_mode;
  u8 driver_mode;
  u8 region_code;
  u8 antenna_sel_val;
  u16 dev_peer_dist;
  u16 dev_bt_feature_bitmap;
  u8 uart_dbg;
  u8 xtal_good_time;
  u16 features_9116;
  u16 dev_ble_roles;
  u16 bt_bdr;
  u16 dev_anchor_point_gap;
  u8 reserved[2];
} __packed;

struct rsi_request_ps {
  u16 desc_word[8];
  struct ps_sleep_params ps_sleep;
  u8 ps_mimic_support;
  u8 ps_uapsd_acs;
  u8 ps_uapsd_wakeup_period;
  u8 reserved;
  u32 ps_listen_interval_duration;
  u32 ps_dtim_interval_duration;
  u16 ps_num_dtim_intervals;
} __packed;

struct rsi_wowlan_req {
  __le16 desc_word[8];
  u8 sourceid[ETH_ALEN];
  u16 wow_flags;
  u16 host_sleep_status;
} __packed;

struct rsi_bcn_meas_req {
  __le16 desc_word[8];
  u8 bssid[ETH_ALEN];
  u8 ssid[32];
  u8 rep_detail;
  u8 meas_mode;
  u8 bgscan_probe[0];
} __packed;

struct rsi_frame_meas_req {
  __le16 desc_word[8];
  u8 bssid[ETH_ALEN];
  u8 frm_req_type;
} __packed;

#define BBP_REMOVE_SOFT_RST_BEFORE_PROG BIT(0)
#define BBP_REMOVE_SOFT_RST_AFTER_PROG  BIT(1)
#define BBP_REG_READ                    BIT(2)

struct rsi_bb_prog_params {
  __le16 desc_word[8];
  struct bb_params_s {
    u16 reg_addr;
    u16 bb_prog_vals;
  } bb_params_req[100];
} __packed;

//__9117_CODE_START
struct rsi_rl_time_stats {
  __le16 desc_word[8];
} __packed;
//__9117_CODE_END
#define RSI_DUTY_CYCLING           BIT(0)
#define RSI_END_OF_FRAME           BIT(1)
#define RSI_SIFS_TX_ENABLE         BIT(2)
#define RSI_DPD                    BIT(3)
#define LMAC_BCON_DROP_EN          BIT(5)
#define DROP_BYTES_FEATURE         BIT(7)
#define LMAC_BCON_EN_DIS_THRESHOLD 4
struct rsi_wlan_9116_features {
  __le16 desc_word[8];
  u8 pll_mode;
  u8 rf_type;
  u8 wireless_mode;
  u8 enable_ppe;
  u8 afe_type;
  u8 reserved1;
  __le16 reserved2;
  __le32 feature_enable;
} __packed;

static inline u32 rsi_get_queueno(u8 *addr, u16 offset)
{
  return (le16_to_cpu(*(__le16 *)&addr[offset]) & 0x7000) >> 12;
}

static inline u32 rsi_get_length(u8 *addr, u16 offset)
{
  return (le16_to_cpu(*(__le16 *)&addr[offset])) & 0x0fff;
}

static inline u8 rsi_get_extended_desc(u8 *addr, u16 offset)
{
  return le16_to_cpu(*((__le16 *)&addr[offset + 4])) & 0x00ff;
}

static inline u8 rsi_get_rssi(u8 *addr)
{
  return *(u8 *)(addr + FRAME_DESC_SZ);
}

static inline u8 rsi_get_channel(u8 *addr)
{
  return *(char *)(addr + 15);
}

int rsi_send_internal_mgmt_frame(struct rsi_common *common, struct sk_buff *skb);
int rsi_load_bootup_params(struct rsi_common *common);
int rsi_load_radio_caps(struct rsi_common *common);
int rsi_send_w9116_features(struct rsi_common *common);
int rsi_mgmt_pkt_recv(struct rsi_common *common, u8 *msg);
int rsi_set_vap_capabilities(struct rsi_common *common, enum opmode mode, u8 *mac_addr, u8 vap_id, u8 vap_status);
int rsi_send_aggr_params_frame(struct rsi_common *common, u16 tid, u16 ssn, u8 buf_size, u8 event, u8 sta_id);
int rsi_send_vap_dynamic_update(struct rsi_common *common);
int rsi_send_block_unblock_frame(struct rsi_common *common, bool event);
//__9117_CODE_START
int rsi_real_time_stats_to_app(struct rsi_hw *adapter);
//__9117_CODE_END
#ifndef CONFIG_STA_PLUS_AP
int rsi_load_key(struct rsi_common *common, u8 *data, u16 key_len, u8 key_type, u8 key_id, u32 cipher, s16 sta_id);
int rsi_set_channel(struct rsi_common *common, struct ieee80211_channel *channel);
void rsi_inform_bss_status(struct rsi_common *common,
                           enum opmode opmode,
                           u8 status,
                           const u8 *bssid,
                           u8 qos_enable,
                           u16 aid,
                           struct ieee80211_sta *sta,
                           u16 sta_id,
                           u16 assoc_cap);
int rsi_send_sta_notify_frame(struct rsi_common *common,
                              enum opmode opmode,
                              u8 notify_event,
                              const unsigned char *bssid,
                              u8 qos_enable,
                              u16 aid,
                              u16 sta_id);
int rsi_send_beacon(struct rsi_common *common);
int rsi_send_auto_rate_request(struct rsi_common *common, struct ieee80211_sta *sta, u16 sta_id);
#else
int rsi_load_key(struct rsi_common *common,
                 u8 *data,
                 u16 key_len,
                 u8 key_type,
                 u8 key_id,
                 u32 cipher,
                 s16 sta_id,
                 struct ieee80211_vif *vif);
int rsi_set_channel(struct rsi_common *common, struct ieee80211_channel *channel, struct ieee80211_vif *vif);
int rsi_send_beacon(struct rsi_common *common, struct ieee80211_vif *vif);
void rsi_inform_bss_status(struct rsi_common *common,
                           enum opmode opmode,
                           u8 status,
                           const u8 *bssid,
                           u8 qos_enable,
                           u16 aid,
                           struct ieee80211_sta *sta,
                           u16 sta_id,
                           u16 assoc_cap,
                           struct ieee80211_vif *vif);
int rsi_send_sta_notify_frame(struct rsi_common *common,
                              enum opmode opmode,
                              u8 notify_event,
                              const unsigned char *bssid,
                              u8 qos_enable,
                              u16 aid,
                              u16 sta_id,
                              struct ieee80211_vif *vif);
int rsi_send_auto_rate_request(struct rsi_common *common,
                               struct ieee80211_sta *sta,
                               u16 sta_id,
                               struct ieee80211_vif *vif);
#endif
void rsi_indicate_pkt_to_os(struct rsi_common *common, struct sk_buff *skb);
int rsi_mac80211_attach(struct rsi_common *common);
int rsi_send_bgscan_params(struct rsi_common *common, int enable);
int rsi_send_bgscan_probe_req(struct rsi_common *common);
void rsi_indicate_tx_status(struct rsi_hw *common, struct sk_buff *skb, int status);
bool rsi_is_cipher_wep(struct rsi_common *common);
void rsi_core_qos_processor(struct rsi_common *common);
void rsi_core_xmit(struct rsi_common *common, struct sk_buff *skb);
int rsi_send_mgmt_pkt(struct rsi_common *common, struct sk_buff *skb);
int rsi_send_data_pkt(struct rsi_common *common, struct sk_buff *skb);
int rsi_send_pkt(struct rsi_common *common, struct sk_buff *skb);
int rsi_band_check(struct rsi_common *common, enum nl80211_band band);
int rsi_send_rx_filter_frame(struct rsi_common *common, u16 rx_filter_word);
int rsi_flash_read(struct rsi_hw *adapter);
int rsi_program_bb_rf(struct rsi_common *common);
int rsi_send_radio_params_update(struct rsi_common *common);
void init_bgscan_params(struct rsi_common *common);
int rsi_set_antenna(struct rsi_common *common, u8 antenna);
int rsi_hci_attach(struct rsi_common *common);
int rsi_handle_card_ready(struct rsi_common *common, u8 *msg);
int rsi_send_bt_reg_params(struct rsi_common *common);
void rsi_validate_bgscan_channels(struct rsi_hw *adapter, struct bgscan_config_params *params);
void update_bgscan_channel_for_acx_module(struct rsi_common *common);
int rsi_validate_debugfs_bgscan_channels(struct rsi_common *common);
int rsi_start_per_burst(struct rsi_hw *adapter);
void init_traffic_timer(struct rsi_hw *adapter, unsigned long timeout);
void check_traffic_pwr_save(struct rsi_hw *adapter);

int ieee80211_chan_to_bw(struct rsi_hw *adapter, u8 op_class, u8 channel_num);
int is_dfs_channel(struct rsi_hw *adapter, int channel);
int rsi_send_common_dev_params(struct rsi_common *common);
int rsi_bb_buffer_request_direct(struct rsi_hw *w_adapter, u16 *bb_buf_vals, u16 num_of_vals);
int rsi_mgmt_soc_reg_ops_req(struct rsi_hw *adapter, unsigned short *bb_prog_vals, unsigned short type);

#ifdef CONFIG_RSI_WOW
int rsi_send_wowlan_request(struct rsi_common *common, u16 flags, u16 sleep_status);
#endif
void rsi_scan_start(struct work_struct *data);
int rsi_send_probe_request(struct rsi_common *common, struct cfg80211_scan_request *scan_req, u8 n_ssid, u8 channel);
#ifdef CONFIG_CARACALLA_BOARD
void rsi_apply_carcalla_power_values(struct rsi_hw *adapter,
                                     struct ieee80211_vif *vif,
                                     struct ieee80211_channel *channel);
#endif
#ifdef CONFIG_RSI_11K
int rsi_get_channel_load_meas(struct rsi_common *common, struct rsi_meas_params params);
int rsi_get_frame_meas(struct rsi_common *common, struct rsi_frame_meas_params params);
int rsi_get_beacon_meas(struct rsi_common *common, struct rsi_beacon_meas_params params);
#endif
#endif
#ifdef CONFIG_STA_PLUS_AP
struct ieee80211_vif *rsi_get_sta_vif(struct rsi_hw *adapter);
struct ieee80211_vif *rsi_get_vif_using_vap_id(struct rsi_hw *adapter, u8 vap_id);
struct ieee80211_vif *rsi_get_first_valid_vif(struct rsi_hw *adapter);
#endif
