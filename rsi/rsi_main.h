/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#ifndef __RSI_MAIN_H__
#define __RSI_MAIN_H__

#include <linux/string.h>
#include <linux/skbuff.h>
#include <net/mac80211.h>
#include <linux/etherdevice.h>
#include <linux/version.h>
#include <linux/cdev.h>

struct rsi_hw;

#include "rsi_ps.h"

#define DRV_VER "SiWT917.2.10.0.5"
#define ERR_ZONE        BIT(0)  /* Error Msgs		*/
#define INFO_ZONE       BIT(1)  /* Generic Debug Msgs	*/
#define INIT_ZONE       BIT(2)  /* Driver Init Msgs	*/
#define MGMT_TX_ZONE    BIT(3)  /* TX Mgmt Path Msgs	*/
#define MGMT_RX_ZONE    BIT(4)  /* RX Mgmt Path Msgs	*/
#define DATA_TX_ZONE    BIT(5)  /* TX Data Path Msgs	*/
#define DATA_RX_ZONE    BIT(6)  /* RX Data Path Msgs	*/
#define FSM_ZONE        BIT(7)  /* State Machine Msgs	*/
#define ISR_ZONE        BIT(8)  /* Interrupt Msgs	*/
#define INT_MGMT_ZONE   BIT(9)  /* Internal mgmt Msgs	*/
#define MGMT_DEBUG_ZONE BIT(10) /* ON-AIR Mgmt          */

#define FSM_FW_NOT_LOADED          0
#define FSM_CARD_NOT_READY         1
#define FSM_COMMON_DEV_PARAMS_SENT 2
#define FSM_BOOT_PARAMS_SENT       3
#define FSM_EEPROM_READ_MAC_ADDR   4
#define FSM_EEPROM_READ_RF_TYPE    5
#define FSM_RESET_MAC_SENT         6
#define FSM_RADIO_CAPS_SENT        7
#define FSM_BB_RF_PROG_SENT        8
#define FSM_MAC_INIT_DONE          9
#define FSM_SCAN_CFM               15

/* Auto Channel Selection defines*/
#define MAX_NUM_CHANS         39
#define ACS_ENABLE            1
#define ACS_DISABLE           0
#define TIMER_ENABLE          BIT(8)
#define ACS_TIMEOUT_TYPE      15
#define ACTIVE_SCAN_DURATION  65
#define PASSIVE_SCAN_DURATION (HZ / 9)
#define ACS_TIMEOUT_TIME      (PASSIVE_SCAN_DURATION - 10)

/* Antenna Diversity */
#define MAX_SCAN_PER_ANTENNA 2

extern u16 rsi_zone_enabled;
extern __printf(2, 3) void rsi_dbg(u32 zone, const char *fmt, ...);
void rsi_hex_dump(u32 zone, char *msg_str, const u8 *msg, u32 len);

#ifndef CONFIG_STA_PLUS_AP
#define RSI_MAX_VIFS    3
#define NUM_EDCA_QUEUES 4
#else
#ifdef CONFIG_RSI_P2P
#define RSI_MAX_VIFS 3
#else
#define RSI_MAX_VIFS 2
#endif
#define NUM_EDCA_QUEUES 8
#endif

#define IEEE80211_ADDR_LEN     6
#define FRAME_DESC_SZ          16
#define MIN_802_11_HDR_LEN     24
#define MIN_802_11_HDR_LEN_MFP 32
#define MGMT_FRAME_PROTECTION  BIT(15)
#define FLAGS                  5

#define DATA_QUEUE_WATER_MARK       400
#define MIN_DATA_QUEUE_WATER_MARK   300
#define BK_DATA_QUEUE_WATER_MARK    600
#define BE_DATA_QUEUE_WATER_MARK    3200
#define VI_DATA_QUEUE_WATER_MARK    3900
#define VO_DATA_QUEUE_WATER_MARK    4500
#define MULTICAST_WATER_MARK        200
#define MAC_80211_HDR_FRAME_CONTROL 0
#define WME_NUM_AC                  4
#ifndef CONFIG_STA_PLUS_AP
#define NUM_SOFT_QUEUES 6
#else
#define STA_PEER            0
#define NUM_STA_DATA_QUEUES 4
#define NUM_SOFT_QUEUES     10
#endif
#define MAX_HW_QUEUES          12
#define INVALID_QUEUE          0xff
#define MAX_CONTINUOUS_VO_PKTS 8
#define MAX_CONTINUOUS_VI_PKTS 4
#define MGMT_HW_Q \
  10 /* Queue No 10 is used for
					    * MGMT_QUEUE in Device FW,
					    *  Hence this is Reserved
					    */
#define BROADCAST_HW_Q 9
#define BEACON_HW_Q    11

/* Queue information */
#define RSI_COEX_Q           0x0
#define RSI_ZIGB_Q           0x1
#define RSI_BT_Q             0x2
#define RSI_WLAN_Q           0x3
#define RSI_WIFI_MGMT_Q      0x4
#define RSI_WIFI_DATA_Q      0x5
#define RSI_BT_MGMT_Q        0x6
#define RSI_BT_DATA_Q        0x7
#define IEEE80211_MGMT_FRAME 0x00
#define IEEE80211_CTL_FRAME  0x04

#define RSI_MAX_ASSOC_STAS      32
#define RSI_MAX_COEX_ASSOC_STAS 4
#define IEEE80211_QOS_TID       0x0f
#define IEEE80211_NONQOS_TID    16

#define MGMT_SW_Q 8

#define DEV_MODEL_9116        (adapter->device_model >= RSI_DEV_9116)
#define COMMON_DEV_MODEL_9116 (common->priv->device_model >= RSI_DEV_9116)

//__9117_CODE_START
#if defined(CONFIG_RSI_11K) && defined(RSI_DEBUG_RRM)
#define MAX_DEBUGFS_ENTRIES_917 13
#else
#define MAX_DEBUGFS_ENTRIES_917 10
#endif
//__9117_CODE_END

#if defined(CONFIG_RSI_11K) && defined(RSI_DEBUG_RRM)
#define MAX_DEBUGFS_ENTRIES 10
#else
#define MAX_DEBUGFS_ENTRIES 7
#endif

#define MAX_BGSCAN_CHANNELS   38
#define MAX_BG_CHAN_FROM_USER 24
#define DFS_CHANNEL           BIT(15)

#define TID_TO_WME_AC(_tid) (((_tid) == 0 || (_tid) == 3) ? BE_Q : ((_tid) < 3) ? BK_Q : ((_tid) < 6) ? VI_Q : VO_Q)

#define WME_AC(_q)                  \
  (((_q) == BK_Q) ? IEEE80211_AC_BK \
                  : ((_q) == BE_Q) ? IEEE80211_AC_BE : ((_q) == VI_Q) ? IEEE80211_AC_VI : IEEE80211_AC_VO)

/* WoWLAN flags */
#define RSI_WOW_ENABLED       BIT(0)
#define RSI_WOW_NO_CONNECTION BIT(1)

#define MAX_REG_COUNTRIES       30
#define NL80211_DFS_WORLD       4
#define KEYID_BITMASK(key_info) ((key_info & 0xC0) >> 6)
#define USB_FULL_SPEED_SIZE     64

/* NL SOCKET PER COMMANDS */
enum per_commands {
  PER_RECEIVE      = 2,
  PER_RECV_STOP    = 6,
  PER_TRANSMIT     = 1,
  PER_PACKET       = 8,
  BT_PER_TRANSMIT  = 4,
  PER_BLE_TRANSMIT = 19
};
#define WLAN_PACKET 1
#define BT_PACKET   2
#define BLE_PACKET  3

#define PER_CONT_MODE          1
#define PER_BURST_MODE         0
#define PER_AGGR_LIMIT_PER_PKT 1792
#define FSM_AMPDU_IND_SENT     14
#define CONTINUOUS_RUNNING     1
#define BURST_RUNNING          2
#define ACX_MODULE             4
#define MAX_CHAN_FOR_ACX       13

#define OP_WLAN_STA_MODE   BIT(0)
#define OP_WLAN_AP_MODE    BIT(1)
#define OP_BT_CLASSIC_MODE BIT(2)
#define OP_BT_LE_MODE      BIT(3)
#define OP_ZIGBEE_MODE     BIT(4)
#define OP_BT_DUAL_MODE    (BIT(2) | BIT(3))

//############################## EFUSE MAP Defines ########################
#define EFUSE_MAP_VERSION_OFFSET   28
#define EXT_RF_CHIP_VERSION_OFFSET EFUSE_MAP_VERSION_OFFSET + 1
#define MODULE_VERSION_OFFSET      EXT_RF_CHIP_VERSION_OFFSET + 1
#define MODULE_PART_NO_OFFSET      MODULE_VERSION_OFFSET + 1
#define RESERVERD_FIELD_OFFSET     MODULE_PART_NO_OFFSET + 2
#define MFG_SW_VERSION_OFFSET      RESERVERD_FIELD_OFFSET + 1
#define MODULE_TYPE_OFFSET         MFG_SW_VERSION_OFFSET + 1
#define CHIP_VERSION_OFFSET        MODULE_TYPE_OFFSET + 1
#define HW_CONFIG_BITMAP_OFFSET    CHIP_VERSION_OFFSET + 1
#define MANF_SW_SUB_VERSION_OFFSET HW_CONFIG_BITMAP_OFFSET + 2
#define CHIP_ID_NO_OFFSET          MANF_SW_SUB_VERSION_OFFSET + 1
#define SDB_MODE_OFFSET            CHIP_ID_NO_OFFSET + 4

struct rsi_buff_rw {
  __le16 desc_word[8];
  struct {
    u16 bb_buf_vals[100];
  } bb_buf_req; //: optimise this as some sets may not have the max vals
} __packed;

struct master_params_s {
  unsigned int address;
  unsigned short no_of_bytes;
  unsigned int data[32];
  unsigned int read_write;
};

struct efuse_map_s {
  u16 efuse_map_version;
  u16 module_version;
  u16 module_part_no;
  u16 mfg_sw_version;
  u16 module_type;
  u16 chip_version;
  u16 m4sb_cert_en;
  u16 mfg_sw_subversion;
  u32 chip_id_no;
  u16 sdb_mode;
};

struct lmac_version_info {
  u8 build_num;
  u8 patch_num;
  u8 minor_id;
  u8 major_id;
  u8 Reserved;
  u8 cust_id;
  u8 rom_ver;
  u8 chip_id;
} __packed;

#define RCV_BUFF_LEN 2100

struct version_info {
  u16 major;
  u16 minor;
  u16 security_version;
  u16 build_id;
  u16 patch_id;
  u16 chip_id;
  u8 release_num;
  u8 customer_id;
  u8 patch_num;
  union {
    struct {
      u8 fw_ver[8];
    } info;
  } ver;
} __packed;

struct skb_info {
  s8 rssi;
  u32 flags;
  u16 channel;
  s8 tid;
  s8 sta_id;
  u8 internal_hdr_size;
  struct ieee80211_sta *sta;
};

#ifndef CONFIG_STA_PLUS_AP
enum edca_queue { BK_Q = 0, BE_Q, VI_Q, VO_Q, MGMT_SOFT_Q, MGMT_BEACON_Q };
#else
enum edca_queue { BK_Q = 0, BE_Q, VI_Q, VO_Q, BK_Q_AP, BE_Q_AP, VI_Q_AP, VO_Q_AP, MGMT_SOFT_Q, MGMT_BEACON_Q };
#endif

struct security_info {
  u32 ptk_cipher;
  u32 gtk_cipher;
};

struct wmm_qinfo {
  s32 weight;
  s32 wme_params;
  s32 pkt_contended;
  s32 txop;
};

struct transmit_q_stats {
  u32 total_tx_pkt_send[NUM_EDCA_QUEUES + 2];
  u32 total_tx_pkt_freed[NUM_EDCA_QUEUES + 2];
};

struct vif_priv {
  bool is_ht;
  bool sgi;
  u16 seq_start;
  u8 vap_id;
  struct ieee80211_key_conf *key;
  u8 rx_bcmc_pn[IEEE80211_CCMP_PN_LEN];
  u8 rx_bcmc_pn_prev[IEEE80211_CCMP_PN_LEN];
  u8 prev_keyid;
  bool rx_pn_valid;
//__9117_CODE_START
#ifdef CONFIG_TWT_SUPPORT
  u16 seq_num;
#endif
  //__9117_CODE_END
};

struct rsi_event {
  atomic_t event_condition;
  wait_queue_head_t event_queue;
};

enum { ZB_DEVICE_NOT_READY = 0, ZB_DEVICE_READY };

struct rsi_thread {
  void (*thread_function)(void *);
  struct completion completion;
  struct task_struct *task;
  struct rsi_event event;
  atomic_t thread_done;
};

struct cqm_info {
  s8 last_cqm_event_rssi;
  int rssi_thold;
  u32 rssi_hyst;
};

struct bgscan_config_params {
  u16 bgscan_threshold;
  u16 roam_threshold;
  u16 bgscan_periodicity;
  u8 num_user_channels;
  u8 num_bg_channels;
  u8 debugfs_bg_channels;
  u8 two_probe;
  u16 active_scan_duration;
  u16 passive_scan_duration;
  u16 user_channels[MAX_BGSCAN_CHANNELS];
  u16 debugfs_channels[MAX_BG_CHAN_FROM_USER];
  u16 channels2scan[MAX_BGSCAN_CHANNELS];
};

#ifdef RSI_DEBUG_RRM
struct rsi_chload_meas_req_params {
  u8 macid[ETH_ALEN];
  u8 regulatory_class;
  u8 channel_num;
  u16 rand_interval;
  u16 meas_duration;
  u8 meas_req_mode;
  u8 meas_type;
};

struct rsi_frame_meas_req_params {
  u8 destid[ETH_ALEN];
  u8 regulatory_class;
  u8 channel_num;
  u16 rand_interval;
  u16 meas_duration;
  u8 meas_req_mode;
  u8 meas_type;
  u8 frame_req_type;
  u8 macid[ETH_ALEN];
};

struct rsi_beacon_meas_req_params {
  u8 destid[ETH_ALEN];
  u8 regulatory_class;
  u8 channel_num;
  u16 rand_interval;
  u16 meas_duration;
  u8 meas_req_mode;
  u8 meas_type;
  u8 meas_mode;
  u8 bssid[ETH_ALEN];
  char str[32];
};
#endif

#ifdef CONFIG_RSI_11K
struct rsi_meas_params {
  u8 dialog_token;
  u8 channel_num;
  u8 meas_req_mode;
  u8 meas_type;
  u16 meas_duration;
  u16 rand_interval;
  u8 channel_width;
  u8 regulatory_class;
};

struct rsi_frame_meas_params {
  struct rsi_meas_params mp;
  u8 frame_req_type;
  u8 mac_addr[ETH_ALEN];
};

struct rsi_beacon_meas_params {
  struct rsi_meas_params mp;
  u8 meas_mode;
  u8 mac_addr[ETH_ALEN];
  u8 ssid_ie[32 + 2];
  u8 bcn_rpt_info[64];
  u8 rpt_detail;
};
#endif

struct rsi_9116_features {
  u8 pll_mode;
  u8 rf_type;
  u8 wireless_mode;
  u8 afe_type;
  u8 enable_ppe;
  u8 dpd;
  u32 sifs_tx_enable;
  u32 ps_options;
};

struct xtended_desc {
  u8 confirm_frame_type;
  u8 retry_cnt;
  u16 reserved;
};

struct rsi_sta {
  struct ieee80211_sta *sta;
  s16 sta_id;
  u16 seq_no[IEEE80211_NUM_ACS];
  u16 seq_start[IEEE80211_NUM_ACS];
  bool start_tx_aggr[IEEE80211_NUM_TIDS];
  u16 min_supported_rate;
  struct sk_buff *sta_skb;
};

struct rsi_hw;

#ifdef CONFIG_STA_PLUS_AP
enum vap_modes { NO_VAP = 0, AP_ALONE = 1 << 0, STA_ALONE = 1 << 1, CONCURRENT = AP_ALONE | STA_ALONE };
#endif

struct rsi_common {
  struct rsi_hw *priv;
#ifndef CONFIG_STA_PLUS_AP
  struct vif_priv vif_info[RSI_MAX_VIFS];
#endif

  char driver_ver[48];
  struct version_info lmac_ver;

  struct rsi_thread tx_thread;
#ifdef CONFIG_SDIO_INTR_POLL
  struct rsi_thread sdio_intr_poll_thread;
#endif
  struct sk_buff_head tx_queue[NUM_EDCA_QUEUES + 2];

  /* Mutex declaration */
  struct mutex mutex;
  struct mutex pslock;
  /* Mutex used between tx/rx threads */
  struct mutex tx_lock;
  struct mutex rx_lock;
  struct mutex bgscan_lock;
  u8 endpoint;

  /* Channel/band related */
  u8 band;
  u8 num_supp_bands;
  u8 channel_width;

  u16 rts_threshold;
  //__9117_CODE_START
  u16 txop_dur_rts_threshold;
  //__9117_CODE_END
  u16 frag_threshold;
  u16 bitrate_mask[2];
  u32 fixedrate_mask[2];
  bool update_country;

  u8 rf_reset;
  struct transmit_q_stats tx_stats;
  struct security_info secinfo;
  struct wmm_qinfo tx_qinfo[NUM_EDCA_QUEUES];
  struct ieee80211_tx_queue_params edca_params[NUM_EDCA_QUEUES];
  u8 mac_addr[IEEE80211_ADDR_LEN];

  /* state related */
  u32 fsm_state;
  u8 bt_fsm_state;
  u8 zb_fsm_state;
  bool init_done;
  u8 bb_rf_prog_count;
  bool iface_down;

  /* Generic */
  u8 channel;
  u8 *saved_rx_data_pkt;
  u8 mac_id;
  u8 radio_id;
  u16 rate_pwr[20];
  u16 min_rate;

  /* WMM algo related */
  u8 selected_qnum;
  u32 pkt_cnt;
  u8 min_weight;

  /* bgscan related */
  struct cqm_info cqm_info;
  struct bgscan_config_params bgscan_info;
  int bgscan_en;
  u8 start_bgscan;
  u8 bgscan_probe_req[1500];
  int bgscan_probe_req_len;
  u16 bgscan_seq_ctrl;
  u8 mac80211_cur_channel;
  bool hw_data_qs_blocked;
  u8 driver_mode;
  u8 coex_mode;
  u16 oper_mode;
  u8 ta_aggr;
  u8 skip_fw_load;
  u8 lp_ps_handshake_mode;
  u8 ulp_ps_handshake_mode;
  u16 ulp_token;
  bool sleep_entry_received;
  bool ulp_sleep_ack_sent;
  bool sleep_ind_gpio_sel;
  u8 ulp_gpio_read;
  u8 ulp_gpio_write;
  u8 uapsd_bitmap;
  u8 rf_power_val;
  u8 device_gpio_type;
  u16 country_code;
  u8 wlan_rf_power_mode;
  u8 bt_rf_power_mode;
  u8 obm_ant_sel_val;
  u8 antenna_diversity;
  u16 rf_pwr_mode;
  char antenna_gain[2];
  u8 host_wakeup_intr_enable;
  u8 host_wakeup_intr_active_high;
  int tx_power;
  u8 ant_in_use;
  bool suspend_in_prog;
  bool rx_in_prog;
  bool hibernate_resume;
  bool reinit_hw;
  struct completion wlan_init_completion;
  struct rsi_event rsi_bt_per_event;
  bool debugfs_bgscan;
  bool debugfs_bgscan_en;
  bool bgscan_in_prog;
  bool debugfs_stop_bgscan;
  bool send_initial_bgscan_chan;
#ifdef CONFIG_RSI_WOW
  u8 wow_flags;
#endif

#if defined(CONFIG_RSI_BT_ALONE) || defined(CONFIG_RSI_COEX_MODE) || defined(CONFIG_RSI_BT_ANDROID)
  void *hci_adapter;
#endif

#ifdef CONFIG_RSI_COEX_MODE
  void *coex_cb;
#endif

  /* AP mode related */
  u8 beacon_enabled;
  u16 beacon_interval;
  u16 beacon_cnt;
  u8 dtim_cnt;
  u16 bc_mc_seqno;
#ifndef CONFIG_STA_PLUS_AP
  struct rsi_sta stations[RSI_MAX_ASSOC_STAS + 1];
#else
  struct rsi_sta stations[RSI_MAX_ASSOC_STAS];
  enum vap_modes vap_mode;
#endif
  int num_stations;
  int max_stations;
  struct ieee80211_channel *ap_channel;
  struct ieee80211_key_conf *key;
  u8 eapol4_confirm;

  /* Wi-Fi direct mode related */
  bool p2p_enabled;
  struct timer_list roc_timer;
  struct ieee80211_vif *roc_vif;
  int last_vap_type;
  u8 last_vap_addr[6];
  u8 last_vap_id;

  struct semaphore tx_bus_lock;

  struct semaphore tx_access_lock;
#define MAX_IDS   3
#define WLAN_ID   0
#define BT_ZB_ID  1
#define COMMON_ID 2
  struct wireless_techs {
    bool tx_intention;
    u8 wait_for_tx_access;
    wait_queue_head_t tx_access_event;
  } techs[MAX_IDS];
  bool common_hal_tx_access;

  struct cfg80211_scan_request *scan_request;
  struct ieee80211_vif *scan_vif;
  bool scan_in_prog;
  struct workqueue_struct *scan_workqueue;
  struct work_struct scan_work;
  struct rsi_event chan_set_event;
  struct rsi_event probe_cfm_event;
  struct rsi_event chan_change_event;
  struct rsi_event cancel_hw_scan_event;
  struct rsi_event cfm_sleep_exit;
#ifdef CONFIG_RSI_BT_ANDROID
  struct rsi_event rsi_btchr_read_wait;
#endif
  struct timer_list scan_timer;
  bool hw_scan_cancel;
  struct timer_list suspend_timer;
  struct rsi_event mgmt_cfm_event;
  void *zb_adapter;

  /* 11k related */
#ifdef RSI_DEBUG_RRM
  struct rsi_chload_meas_req_params rrm_chload_params;
  struct rsi_frame_meas_req_params rrm_frame_params;
  struct rsi_beacon_meas_req_params rrm_beacon_params;
#endif
#ifdef CONFIG_RSI_11K
  u8 num_pend_rrm_reqs;
  struct sk_buff_head rrm_queue;
  struct sk_buff *rrm_pending_frame;
  struct rsi_meas_params chload_meas;
  struct rsi_frame_meas_params frame_meas;
  struct rsi_beacon_meas_params beacon_meas;
#endif
  struct rsi_9116_features w9116_features;
#ifdef CONFIG_RSI_MULTI_MODE
  u16 dev_oper_mode[6];
#else
  u16 dev_oper_mode;
#endif
#ifdef CONFIG_RSI_BT_ANDROID
  int rsi_skb_queue_front;
  int rsi_skb_queue_rear;
#define QUEUE_SIZE 500
  struct sk_buff *rsi_skb_queue[QUEUE_SIZE];
  dev_t bt_devid;              /* bt char device number */
  struct cdev bt_char_dev;     /* bt character device structure */
  struct class *bt_char_class; /* device class for usb char driver */
#endif
  /* 9116 related */
  u16 peer_dist;
  u16 bt_feature_bitmap;
  u8 uart_debug;
  u16 ext_opt;
  u8 host_intf_on_demand;
  u8 crystal_as_sleep_clk;
  u16 feature_bitmap_9116;
  u16 ble_roles;
  bool three_wire_coex;
  u16 bt_bdr_mode;
  u16 anchor_point_gap;
  u8 bt_rf_type;
  u8 ble_tx_pwr_inx;
  u8 ble_pwr_save_options;
  u32 ble_vendor_feature_bitmap;
  u8 bt_rf_tx_power_mode;
  u8 bt_rf_rx_power_mode;
  u8 rsi_scan_count;
  bool hwscan_en;
  u32 wlan_pwrsave_options;
  bool enable_40mhz;
  bool enabled_uapsd;
  u8 max_sp_len;
  u8 bgscan_ssid[32];
  u8 bgscan_ssid_len;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0))
  u8 hw_scan_count;
  u8 user_channels_count;
  u16 user_channels_list[MAX_BGSCAN_CHANNELS];
#endif
  u8 use_protection;
  bool peer_notify_state;
  u8 sta_bssid[ETH_ALEN];
  u8 fixed_rate_en;
  u16 fixed_rate;
  u16 hw_bmiss_threshold;
  u16 keep_alive_period;
  //__9117_CODE_START
  u8 gaurd_interval;
  u8 nominal_pe;
  u8 dcm_enable;
  u8 ldpc_enable;
  bool enable_ng_cb;
  u8 value_of_ng_cb;
#ifdef CONFIG_UORA_SUPPORT
  bool enable_uora;
  u8 uora_ocw_min;
  u8 uora_ocw_max;
#endif
  u8 trigger_resp_ind;
  bool bsr_support;
  u8 config_er_su;
  bool enable_er_su;
  u8 ssid[32];
  u8 ssid_len;
  //__9117_CODE_END
  u8 xtal_good_time;
  struct efuse_map_s efuse_map;
  //enhanced max psp related
  bool disable_ps_from_lmac;
  u16 rx_data_inactive_interval;
  u8 minimum_basic_rate;
  bool default_deep_sleep_enable;
  u8 pta_config;
  bool enable_encap_offload;
};

enum host_intf { RSI_HOST_INTF_SDIO = 0, RSI_HOST_INTF_USB };

enum rsi_dev_model {
  RSI_DEV_9113 = 0,
  RSI_DEV_9116,
  //__9117_CODE_START
  RSI_DEV_9117,
  RSI_DEV_917
  //__9117_CODE_END
};

struct eepromrw_info {
  u32 offset;
  u32 length;
  u8 write;
  u16 eeprom_erase;
  u8 data[480];
};

struct eeprom_read {
  u16 length;
  u16 off_set;
};

//__9117_CODE_START
/* Real time stats*/

struct tx_status_info {
  uint16_t status_valid : 1;
  uint16_t success_valid : 1;
  uint16_t type_of_failure : 2;
  uint16_t response_type : 5;
  uint16_t last_pkt : 1;
  uint16_t rts_selfcts_status : 1;
  uint16_t underun : 1;
  uint16_t tx_qdma_idle : 1;
  uint16_t cca_not_sensed : 1;
  uint16_t rx_disabled : 1;
  uint16_t desc_prog_error : 1;
  uint16_t tack_received : 1;
  uint16_t ndp_11ah_received : 1;
  uint16_t tp_ppdu_txed : 1;
  uint16_t num_data_pkts_txed : 7;
  uint16_t all_ack_count : 1;
  uint16_t reserved : 5;
} __packed;

typedef struct {
  unsigned short tx_stat_type : 4;
  unsigned short tx_stat_len : 12;
  unsigned char tx_que_no;
  unsigned char reserved1;
  unsigned int tx_real_timestamp;
  struct tx_status_info status_info;
  unsigned int tx_timestamp;
  unsigned long long ba_bitmap;
  unsigned short ba_seq_no;
  unsigned short pkt_start_seq_no;
  unsigned long long pkt_seq_no_bitmap;
  unsigned short tx_pkt_type;
  unsigned char tx_pe_applied;
  unsigned char tx_retry_count;
  unsigned short tx_power;
} __attribute__((packed)) real_time_tx_stats;

struct rx_status_info {
  uint16_t mpdu_hdr_len : 6;
  uint16_t pkt_type_is_data : 1;
  uint16_t reserved1 : 7;
  uint16_t ps_poll_received : 1;
  uint16_t mic_computed : 1;
  uint16_t ppe_stats_valid : 1;
  uint16_t reserved2 : 2;
  uint16_t TA_Match : 1;
  uint16_t first_MPDU : 1;
  uint16_t firt_msdu : 1;
  uint16_t last_msdu : 1;
  uint16_t A_MPDU : 1;
  uint16_t A_MSDU : 1;
  uint16_t cipher_suite_mismatch : 1;
  uint16_t icv_error : 1;
  uint16_t mic_error : 1;
  uint16_t crc_error : 1;
  uint16_t aborted : 1;
  uint16_t pkt_copmleted : 1;
} __packed;

struct rx_pkt_type {
  uint16_t pkt_rate : 7;
  uint16_t pkt_mode : 2;
  uint16_t stbc_sgi : 3;
  uint16_t bandwidth : 2;
  uint16_t mode_11ax : 1;
  uint16_t mode_11ac : 1;
  uint16_t pkt_length : 16;
} __packed;

typedef struct {
  unsigned short rx_stat_type_len : 4;
  unsigned short rx_stat_len : 12;
  unsigned char rx_pe_applied;
  unsigned char reserved2;
  unsigned int rx_real_timestamp;
  unsigned int rx_timestamp;
  struct rx_status_info status_info;
  struct rx_pkt_type pkt_type;
  unsigned int rssi;
} __attribute__((packed)) real_time_rx_stats;

#define SET_BMISS_THRESHOLD 14
#define KEEP_ALIVE_PERIOD   15
#define REAL_TIME_STATS     10
#define TX_STAT_LENGTH      (sizeof(real_time_tx_stats))
#define RX_STAT_LENGTH      (sizeof(real_time_rx_stats))
#define TX_STATS            1
#define RX_STATS            2
//__9117_CODE_END

typedef struct {
  //! no. of tx pkts
  unsigned short tx_pkts;
  //! no. of rx pkts
  unsigned short rx_pkts;
  //! no. of tx retries
  unsigned short tx_retries;
  //! no. of pkts that pass crc
  unsigned short crc_pass;
  //! no. of pkts failing crc chk
  unsigned short crc_fail;
  //! no. of times cca got stuck
  unsigned short cca_stk;
  //! no of times cca didn't get stuck
  unsigned short cca_not_stk;
  //! no. of pkt aborts
  unsigned short pkt_abort;
  //! no. of false rx starts
  unsigned short fls_rx_start;
  //! cca idle time
  unsigned short cca_idle;
  //! no. of greenfield pkts
  unsigned short gf_pkts;
  //! no. of high throughput pkts
  unsigned short ht_pkts;
  //! no. of legacy pkts
  unsigned short leg_pkt;
  //!  add description
  unsigned short leg_prty_fails;
  //! no. of ht pkts failing crc chk
  unsigned short ht_crc_fails;
  //!  add description
  unsigned short sp_rejected;
  //!  add description
  unsigned short lp_rejected;
  //! Channel 1 signal power
  unsigned short ch1_sig_pow;
  //! channel 1 noise power
  unsigned short ch1_noise_pow;
  //! channel 2 signal power
  unsigned short ch2_sig_pow;
  //! channel 2 noise power
  unsigned short ch2_noise_pow;
  //! channel 3 signal power
  unsigned short ch3_sig_pow;
  //! channel 3 noise power
  unsigned short ch3_noise_pow;
  //! no. of rx retries
  unsigned short rx_retries;
  //! rssi value
  unsigned short bea_avg_rssi;
  //! cal_rssi
  unsigned short cal_rssi;
  //! lna_gain bb_gain
  unsigned short lna_bb_gain;
  //! avg_val
  unsigned short avg_val;
  //! xretries pkts dropped
  unsigned short xretries;
  //! consecutive pkts dropped
  unsigned short max_cons_pkts_dropped;
  //!
  unsigned short false_under_sat;
  //!BSS MATCHED BROADCAST PKT STATS
  unsigned short bss_broadcast_pkts;
  //!BSS MATCHED MULTICAST PKT STATS
  unsigned short bss_multicast_pkts;
  //!BSS and DA MATCHED MULTICAST PKT STATS
  unsigned short bss_da_matched_multicast_pkts;
  unsigned int eof_pkt_drop_count;
  unsigned int mask_pkt_drop_count;
  unsigned int ack_sent;
  //!No.of pkts rcvd with 48M
  unsigned short pkt_rcvd_with_48M;
  //!No.of pkts rcvd with 24M
  unsigned short pkt_rcvd_with_24M;
  //!No.of pkts rcvd with 12M
  unsigned short pkt_rcvd_with_12M;
  //!No.of pkts rcvd with 6M
  unsigned short pkt_rcvd_with_6M;
  //!No.of pkts rcvd with 54M
  unsigned short pkt_rcvd_with_54M;
  //!No.of pkts rcvd with 36M
  unsigned short pkt_rcvd_with_36M;
  //!No.of pkts rcvd with 18M
  unsigned short pkt_rcvd_with_18M;
  //!No.of pkts rcvd with 9M
  unsigned short pkt_rcvd_with_9M;
  //!No.of pkts rcvd with 11M
  unsigned short pkt_rcvd_with_11M;
  //!No.of pkts rcvd with 5.5M
  unsigned short pkt_rcvd_with_5M;
  //!No.of pkts rcvd with 2M
  unsigned short pkt_rcvd_with_2M;
  //!No.of pkts rcvd with 1M
  unsigned short pkt_rcvd_with_1M;
  //!No.of pkts rcvd with mcs0
  unsigned short pkt_rcvd_with_mcs0;
  //!No.of pkts rcvd with mcs1
  unsigned short pkt_rcvd_with_mcs1;
  //!No.of pkts rcvd with mcs2
  unsigned short pkt_rcvd_with_mcs2;
  //!No.of pkts rcvd with mcs3
  unsigned short pkt_rcvd_with_mcs3;
  //!No.of pkts rcvd with mcs4
  unsigned short pkt_rcvd_with_mcs4;
  //!No.of pkts rcvd with mcs5
  unsigned short pkt_rcvd_with_mcs5;
  //!No.of pkts rcvd with mcs6
  unsigned short pkt_rcvd_with_mcs6;
  //!No.of pkts rcvd with mcs7
  unsigned short pkt_rcvd_with_mcs7;
  //__9117_CODE_START
  //!No.of pkts rcvd with 11ax SU PPDU Type
  unsigned short pkt_count_HE_SU_PPDU;
  //!No.of pkts rcvd with ER SU PPDU Type
  unsigned short pkt_count_HE_ER_SU_PPDU;
  //!No.of pkts rcvd with TB PPDU Type
  unsigned short pkt_count_HE_TRIG_PPDU;
  //!No.of pkts rcvd with MU PPDU Type
  unsigned short pkt_count_HE_MU_PPDU;
  //! Sum of all the pkts rcvd with SU, TB, MU PPDUs
  unsigned short pkt_count_11AC;
  //!sum of all the pkts transmitted
  unsigned short tx_done;
  //!No.of HE_MU_MIMO pkts rcvd
  unsigned short he_mu_mimo_pkt_cnt;
  //!No.of HE_MU_OFDMA pkts rcvd
  unsigned short he_mu_ofdma_pkt_cnt;
  //__9117_CODE_END
  /* Channel Utilization stats */
  unsigned int utilization;
  unsigned int rssi_utilization;
  unsigned int tot_bytes;
  unsigned int rssi_bytes;
  unsigned int interval_duration;
  unsigned int false_cca_avg_rssi;
  unsigned int max_cca_avg_rssi;
  unsigned int cca_duration;
  unsigned int ed_duration;
  unsigned short int noise_rssi;
  int stop_per;

} per_stats;

//__9117_CODE_START
struct he_conf {
  unsigned char coding_type;
  unsigned char nominal_pe;
  unsigned char UL_DL;
  unsigned char he_ppdu_type;
  unsigned char beam_change;
  unsigned char BW;
  unsigned char STBC;
  unsigned char TX_BF;
  unsigned char GI_LTF;
  unsigned char DCM;
  unsigned char NSTS_MIDAMBLE;
  unsigned char spatial_reuse;
  unsigned char BSS_color;
  unsigned short HE_SIGA2_RESERVED;
  unsigned char RU_ALLOCATION;
  unsigned char N_HELTF_TOT;
  unsigned char SIGB_DCM;
  unsigned char SIGB_MCS;
  unsigned short USER_STA_ID;
  unsigned char USER_IDX;
  unsigned char SIGB_COMPRESSION_FIELD;
};
//__9117_CODE_END

typedef struct per_packet_s {
  unsigned char enable;
  unsigned int length;
  unsigned char insert_seq;
  unsigned char packet[1536];
} per_packet;

//__9117_CODE_START
#define HE_TB_PPDU       2
#define HE_MU_PPDU       3
#define MCS_AX_RATE_MASK 0x7
struct he_tb_mu_params_t {
  __le16 desc_word[8];
  unsigned int SPATIAL_REUSE_2 : 4;
  unsigned int SPATIAL_REUSE_3 : 4;
  unsigned int SPATIAL_REUSE_4 : 4;
  unsigned int N_HELTF_TOT : 3;
  unsigned int RU_ALLOCATION_TB : 8;
  unsigned int HE_SIGA2_RESERVED : 9;
  unsigned int SIGB_MCS : 3;
  unsigned int SIGB_DCM : 1;
  unsigned int HE_LTF_SYM : 3;
  unsigned int SIGB_COMPRESSION_FIELD : 1;
  unsigned int reserved : 24;
  unsigned int RU_ALLOCATION_MU;
  unsigned int reserved1 : 1;
  unsigned int USER_STA_ID : 11;
  unsigned int USER_NSTS : 3;
  unsigned int USER_TxBF : 1;
  unsigned int USER_MCS : 4;
  unsigned int USER_DCM : 1;
  unsigned int USER_CODING_TYPE : 1;
  unsigned int reserved2 : 4;
  unsigned int USER_IDX : 6;
};
//__9117_CODE_END

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0))
#define NUM_NL80211_BANDS 3
#endif
//__9117_CODE_START
#define RATE_OFFSET_NOMINAL_PE  5
#define RATE_OFFSET_GI_LTF      11
#define RATE_OFFSET_DCM         13
#define RATE_OFFSET_CODING_TYPE 9
struct rsi_ax_params {
  bool _11ax_enabled;
  u8 bss_color;
  bool spatial_reuse_en;
  bool UL_DL;
  u8 coding_type;
  u8 nominal_pe;
  u8 spatial_reuse_enable;
  u8 he_ppdu_type;
  u8 beam_change;
  u8 BW;
  u8 STBC;
  u8 Tx_BF;
  u8 GI_LTF;
  u8 DCM;
  u8 NSTS_MIDAMBLE;
  u8 spatial_reuse;
  u8 BSS_color;
  u16 ax_rate;
  u8 mbssid_mask_value;
  u8 connected_nt_profile_index;
  u8 profile_periodicity;
  u8 trasmitter_bssid[6];
  u8 ipps_valid;
  u8 obss_pd_min;
  u8 obss_pd_max;
  u8 edca_susp;
};

/* Individual TWT related */
#ifdef CONFIG_TWT_SUPPORT
#define TWT_CONFIG_CMD         11
#define TWT_STATUS             13
#define RESCHEDULE_TWT         17
#define WLAN_CATEGORY_S1G      22
#define WLAN_S1G_TWT_SETUP     6
#define WLAN_S1G_TWT_TEARDOWN  7
#define TWT_INFORMATION_ACTION 11
#define WLAN_EID_S1G_TWT       216
#define TWT_CONFIG             75
#define NEXT_TWT_SUBFIELD_SIZE 3 //Next twt subfield size value of 3 indicates the size of next twt to be 64 bits
#define TWT_INFO_FRAME_CONFIRM 0x4

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(5, 14, 21))
enum ieee80211_twt_setup_cmd {
  TWT_SETUP_CMD_REQUEST,
  TWT_SETUP_CMD_SUGGEST,
  TWT_SETUP_CMD_DEMAND,
  TWT_SETUP_CMD_GROUPING,
  TWT_SETUP_CMD_ACCEPT,
  TWT_SETUP_CMD_ALTERNATE,
  TWT_SETUP_CMD_DICTATE,
  TWT_SETUP_CMD_REJECT,
};
#endif

//TWT STATUS CODES
#define TWT_DEVICE_NOT_IN_CONNECTED_STATE   0
#define TWT_SETUP_ERR_SESSION_ACTIVE        1
#define TWT_TEARDOWN_ERR_FLOWID_NOT_MATCHED 2
#define TWT_TEARDOWN_ERR_NOACTIVE_SESS      3
#define TWT_SETUP_SESSION_IN_PROGRESS       4
#define TWT_SESSION_SUCC                    5
#define TWT_UNSOL_SESSION_SUCC              6
#define TWT_SETUP_AP_REJECTED               7
#define TWT_SETUP_RSP_OUTOF_TOL             8
#define TWT_SETUP_RSP_NOT_MATCHED           9
#define TWT_SETUP_UNSUPPORTED_RSP           10
#define TWT_TEARDOWN_SUCC                   11
#define TWT_AP_TEARDOWN_SUCC                12
#define TWT_SETUP_FAIL_MAX_RETRIES_REACHED  13
#define TWT_INACTIVE_DUETO_ROAMING          14
#define TWT_INACTIVE_DUETO_DISCONNECT       15
#define TWT_INACTIVE_NO_AP_SUPPORT          16
#define TWT_RESCHEDULE_SUCC                 17
#define TWT_RESCHEDULE_FAIL                 18

typedef struct rsi_twt_config_s {
  u8 wake_duration;
  u8 wake_duration_tol;
  u8 wake_int_exp;
  u8 wake_int_exp_tol;
  u16 wake_int_mantissa;
  u16 wake_int_mantissa_tol;
  u8 implicit_twt;
  u8 un_announced_twt;
  u8 triggered_twt;
  u8 negotiation_type;
  u8 twt_channel;
  u8 twt_protection;
  u8 twt_flow_id;
  u8 restrict_tx_outside_tsp;
  u8 twt_retry_limit;
  u8 twt_retry_interval;
  u8 req_type;
  u8 twt_enable;
  u8 twt_wake_duration_unit;
} rsi_twt_config_t;

typedef struct twt_element_s {
  u8 eid;
  u8 elength;
  u8 control_ndp_pi : 1;
  u8 control_resp_pm_mode : 1;
  u8 control_negotiation_type : 2;
  u8 control_twt_info_frame_dis : 1;
  u8 control_wake_duration_unit : 1;
  u8 control_resereved : 2;
  u16 req_type_twt_req : 1;
  u16 req_type_twt_setup_command : 3;
  u16 req_type_trigger : 1;
  u16 req_type_implicit_twt : 1;
  u16 req_type_flow_type : 1;
  u16 req_type_twt_flow_id : 3;
  u16 req_type_twt_wi_exp : 5;
  u16 req_type_twt_protection : 1;
  u32 target_wake_time[2];
  u8 nom_min_wake_duration;
  u16 wake_interval_mantisa;
  u8 twt_channel;
} __attribute__((packed)) twt_element_t;

typedef struct twt_setup_frame_s {
  u8 action_category;
  u8 action_type;
  u8 dialog_token;
  twt_element_t twt_element;
} __attribute__((packed)) twt_setup_frame_t;

typedef struct twt_teardown_frame_t {
  u8 action_category;
  u8 action_type;
  u8 flow_id : 3;
  u8 reserved : 2;
  u8 negotiation_type : 2;
  u8 teardown_all : 1;
} __attribute__((packed)) twt_teardown_frame_t;

typedef struct twt_session_config_s {
  u16 frame_desc[8];
  u16 twt_enable : 1;
  u16 twt_negotiation_type : 1;
  u16 reserved : 14;
  u16 twt_req : 1;
  u16 twt_setup_command : 3;
  u16 twt_trigger : 1;
  u16 twt_implicit : 1;
  u16 twt_flow_type : 1;
  u16 twt_flow_id : 3;
  u16 twt_wake_inter_expo : 5;
  u16 twt_protection : 1;
  u32 target_wake_time[2];
  u32 twt_nom_wake_dur;
  u32 twt_wake_interval[2];
} __attribute__((packed)) twt_session_config_t;

typedef struct twt_status_resp {
  uint8_t twt_flow_id;
  uint8_t wake_duration;
  uint8_t wake_duration_unit;
  uint8_t implicit_twt;
  uint8_t triggered_twt;
  uint8_t un_announced_twt;
  uint8_t wake_int_exp;
  uint16_t wake_int_mantissa;
  uint8_t req_type;
} rsi_twt_status_resp;

typedef enum {
  SUSPEND_INDEFINITELY,
  SUSPEND_FOR_DURATION,
  RESUME_IMMEDIATELY,
} wifi_reschedule_twt_action_t;

typedef struct {
  uint8_t flow_id;
  wifi_reschedule_twt_action_t twt_action;
  uint16_t reserved1;
  uint8_t reserved2;
  uint64_t suspend_duration;
} wifi_reschedule_twt_config_t;

typedef struct twt_info_frame_s {
  uint8_t action_category;
  uint8_t action_type;
  uint8_t flow_id : 3;
  uint8_t response_requested : 1;
  uint8_t next_twt_request : 1;
  uint8_t next_twt_subfield_size : 2;
  uint8_t all_twt : 1;
  uint64_t suspend_duration;
} __attribute__((packed)) twt_info_frame_t;

#endif
//__9117_CODE_END
struct rsi_per_params {
  unsigned short enable;
  signed short power;
  unsigned int rate;
  unsigned short pkt_length;
  unsigned short mode;
  unsigned short channel;
  unsigned short rate_flags;
  unsigned short per_ch_bw;
  unsigned short aggr_enable;
  unsigned short aggr_count;
  unsigned short no_of_pkts;
  unsigned int delay;
  unsigned char ctry_region;
  unsigned char enable_11j;
  //__9117_CODE_START
  uint8_t enable_11ax;

  struct he_conf heconf;
  //__9117_CODE_END
};

/* bb read/write related */
#define RSI_SET_BB_RF     5
#define BB_READ_REQ       0x0
#define BB_WRITE_REQ      0x1
#define MASTER_READ       21
#define MASTER_WRITE      22
#define MASTER_OPS        0xA1
#define BUF_READ_REQ      0x6
#define BUF_WRITE_REQ     0x7
#define WLAN_9116_FEATURE 68

typedef struct bb_rf_params_s {
  unsigned short Data[1024];
  unsigned short no_of_fields;
  unsigned short no_of_values;
  unsigned char value;
  unsigned char soft_reset;
  unsigned char protocol_id;
} bb_rf_params_t;

typedef struct bb_rf_params_bt_s {
  unsigned char value;
  unsigned char no_of_fields;
  unsigned char no_of_values;
  unsigned char soft_reset;
  unsigned short Data[128];
} bb_rf_params_bt_t;

struct rsi_hw {
  struct rsi_common *priv;
  enum rsi_dev_model device_model;
  struct ieee80211_hw *hw;
  struct ieee80211_vif *vifs[RSI_MAX_VIFS];
#ifdef CONFIG_STA_PLUS_AP
  u8 vif_bit_map;
#endif
  struct ieee80211_tx_queue_params edca_params[NUM_EDCA_QUEUES];

  struct ieee80211_supported_band sbands[NUM_NL80211_BANDS];

  struct device *device;
  int sc_nvifs;
  enum host_intf rsi_host_intf;
  enum ps_state ps_state;
  bool usb_in_deep_ps;
  bool usb_intf_in_suspend;
  struct usb_interface *usb_iface;
  struct rsi_ps_info ps_info;
  bool user_ps_en;
  spinlock_t ps_lock;
  u32 isr_pending;
  u32 usb_buffer_status_reg;
#ifdef CONFIG_RSI_DEBUGFS
  struct rsi_debugfs *dfsentry;
  //__9117_CODE_START
  struct rsi_debugfs_917 *dfsentry_917;
  //__9117_CODE_END
  u8 num_debugfs_entries;
#endif

  char *fw_file_name;
  struct timer_list bl_cmd_timer;
  struct timer_list traffic_timer;
//__9117_CODE_START
#ifdef CONFIG_TWT_SUPPORT
  struct timer_list twt_setup_timer;
#endif
  //__9117_CODE_END
  u8 blcmd_timer_expired;
  u32 flash_capacity;
  u32 tx_blk_size;
  atomic_t tx_pending_urbs;
  u32 common_hal_fsm;
  u8 eeprom_init;
  struct eepromrw_info eeprom;
  u32 interrupt_status;

  u8 dfs_region;
  char country[2];
  bool peer_notify;
  void *rsi_dev;

  struct rsi_host_intf_ops *host_intf_ops;
  int (*check_hw_queue_status)(struct rsi_hw *adapter, u8 q_num);
  int (*rx_urb_submit)(struct rsi_hw *adapter, u8 ep_num);
  int (*determine_event_timeout)(struct rsi_hw *adapter);
  void (*process_isr_hci)(struct rsi_hw *adapter);
  int (*check_intr_status_reg)(struct rsi_hw *adapter);
  u8 rrm_state;
  u8 rrm_enq_state;
#ifdef CONFIG_RSI_MULTI_MODE
  int drv_instance_index;
#endif
  u8 auto_chan_sel;
  u8 idx;
  struct survey_info rsi_survey[MAX_NUM_CHANS];
  u8 n_channels;
  //__9117_CODE_START
  struct rsi_ax_params ax_params;
  u32 rate_field_params;
  //__9117_CODE_END
  //PER PARAMS
  struct ieee80211_channel channel;
  u16 per_rate_flag;
  bool per_stop_bit;
  u8 tx_running;
  u16 no_of_per_fragments;
  u32 total_per_pkt_sent;
  struct rsi_per_params per_params;
  per_packet per_packet;
  u16 ch_util_start_flag;
  u16 stats_interval;
  u8 false_cca_rssi_threshold;
  u8 recv_stop;
  u8 rx_stats_inprog;
  u8 disable_programming;
  int wlan_nl_pid;
  int bt_nl_pid;
  struct sock *nl_sk;
  per_stats sta_info;
  //__9117_CODE_START
  real_time_tx_stats tx_stats_info;
  real_time_rx_stats rx_stats_info;
  u8 stat_type;
  u16 next_stat;
  //__9117_CODE_END
  bool usb_full_speed;
  bb_rf_params_t bb_rf_params;
  bb_rf_params_t bb_rf_read;
  unsigned char bb_rf_rw;
  unsigned char soft_reset;
  u8 read_cmd;
  s16 rx_rssi;
  unsigned long prev_rssi_fetch_time;
  u8 chip_rev;
  unsigned long var_interval;
  unsigned long old_jiffies;
  unsigned long due_time;
  bool wifi_paused;
  //__9117_CODE_START
#ifdef CONFIG_TWT_SUPPORT
  bool ap_support_twt;
  u8 twt_session_active;
  u8 twt_active_session_flow_id;
  u8 twt_session_in_progress;
  u8 twt_retry_count_limit;
  u8 twt_demand_resp_retry_limit;
  u8 twt_current_status;
  rsi_twt_config_t rsi_twt_config;
  rsi_twt_status_resp twt_status_to_app;
  wifi_reschedule_twt_config_t reschedule_twt_config;
  u8 ap_twt_info_frame_support;
  bool twt_rescheduling_in_progress;
#endif
  //__9117_CODE_END
  struct master_params_s master_ops;
};

struct acs_stats_s {
  u16 chan_busy_time;
  u8 noise_floor_rssi;
};

void rsi_print_version(struct rsi_common *common);
struct rsi_host_intf_ops {
  int (*read_pkt)(struct rsi_hw *adapter, u8 *pkt, u32 len);
  int (*write_pkt)(struct rsi_hw *adapter, u8 *pkt, u32 len);
  int (*master_access_msword)(struct rsi_hw *adapter, u16 ms_word);
  int (*read_reg_multiple)(struct rsi_hw *adapter, u32 addr, u8 *data, u16 count);
  int (*reg_read)(struct rsi_hw *adapter, u32 addr, u8 *data);
  int (*write_reg_multiple)(struct rsi_hw *adapter, u32 addr, u8 *data, u16 count);
  int (*master_reg_read)(struct rsi_hw *adapter, u32 addr, u32 *read_buf, u16 size);
  int (*master_reg_write)(struct rsi_hw *adapter, unsigned long addr, unsigned long data, u16 size);
  int (*load_data_master_write)(struct rsi_hw *adapter, u32 addr, u32 instructions_size, u16 block_size, u8 *fw);
  int (*ta_reset_ops)(struct rsi_hw *adapter);
  int (*rsi_check_bus_status)(struct rsi_hw *adapter);
  int (*check_hw_queue_status)(struct rsi_hw *adapter, u8 q_num);
  int (*reinit_device)(struct rsi_hw *adapter);
};

struct rsi_proto_ops;

enum host_intf rsi_get_host_intf(void *priv);
void rsi_set_zb_context(void *priv, void *zb_context);
void *rsi_get_zb_context(void *priv);
struct rsi_proto_ops {
  int (*coex_send_pkt)(void *priv, struct sk_buff *skb, u8 hal_queue);
  enum host_intf (*get_host_intf)(void *priv);
  void (*set_zb_context)(void *priv, void *context);
  void *(*get_zb_context)(void *priv);
  struct rsi_mod_ops *zb_ops;
};
struct rsi_mod_ops {
  int (*attach)(void *priv, struct rsi_proto_ops *ops);
  void (*detach)(void *priv);
  int (*recv_pkt)(void *priv, u8 *msg);
};

struct rsi_nl_desc {
  __le16 desc_word[8];
} __packed;

int nl_sk_init(struct rsi_hw *adapter);
void nl_sk_exit(struct sock *nl_sk);
#define NETLINK_USER 31
int rsi_send_rx_stats_cmd(struct rsi_hw *adapter, struct nlmsghdr *nlh);
int rsi_process_rx_stats(struct rsi_hw *adapter);
#define RSI_EFUSE_MAP 80
int rsi_copy_efuse_content(struct rsi_hw *adapter);
int rsi_process_efuse_content(struct rsi_common *common, struct efuse_map_s);
#if defined(CONFIG_RSI_COEX_MODE) || defined(CONFIG_RSI_BT_ALONE)
#define BT_E2E_STAT       0x13
#define BT_BLE_GAIN_TABLE 0x06
int rsi_bt_e2e_stats(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload_len, u16 cmd);
int rsi_bt_ble_update_gain_table(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload_len, u16 cmd);
int rsi_bt_transmit_cmd(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload_len, u16 cmd);
int rsi_bt_per_stats(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload_len, u16 cmd);
#endif
#define UPDATE_WLAN_GAIN_TABLE 78
#define FILTER_BCAST           79
#define GET_RSSI               81
int rsi_update_wlan_gain_table(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload_len);
int rsi_transmit_stats_cmd(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload);
int rsi_set_bb_rf_values(struct rsi_hw *adapter);
int rsi_mgmt_send_bb_prog_frames(struct rsi_hw *adapter, unsigned short *bb_prog_vals, unsigned short num_of_vals);
int rsi_do_master_ops(struct rsi_hw *w_adapter, u16 type);
int rsi_bb_prog_data_to_app(struct rsi_hw *adapter);
int send_rssi_to_app(struct rsi_hw *adapter);
int send_filter_broadcast_frame_to_fw(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload_len);
int send_get_rssi_frame_to_fw(struct rsi_hw *adapter);
//__9117_CODE_START
#define DIAG_TOOL_UPDATE 86
int diag_tool_update_dpd_fw(struct rsi_hw *adapter,
                            struct nlmsghdr *nlh,
                            uint16_t dpd_cmd,
                            uint16_t dpd_param_name,
                            uint16_t dpd_param_val);
int send_diag_tool_response_to_app(struct rsi_hw *adapter, int data);
int send_bmiss_threshold_to_app(struct rsi_hw *adapter, u16 prev_bmiss_threshold);
int rsi_send_real_time_stats_request_frame(struct rsi_hw *adapter, struct nlmsghdr *nlh);
#ifdef CONFIG_TWT_SUPPORT
int process_twt_setup_cmd(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload_len);
int send_twt_setup_frame(struct rsi_hw *adapter, u8 twt_setup_update, twt_setup_frame_t *twt_setup_resp);
int send_twt_teardown_frame(struct rsi_hw *adapter, u8 flow_id, u8 negotiation_type);
int send_twt_session_details_to_lmac(struct rsi_hw *adapter,
                                     u8 setup,
                                     u8 twt_flow_id,
                                     twt_setup_frame_t *twt_setup_resp,
                                     rsi_twt_config_t *twt_user_cofig);
int rsi_twt_status_resp_to_app(struct rsi_hw *adapter);
int rsi_mgmt_process_twt_setup_resp(struct rsi_hw *adapter, struct sk_buff *skb);
int validate_unsupported_twt_resp_params(struct rsi_hw *adapter, twt_setup_frame_t *twt_setup_frame);
int send_twt_information_frame(struct rsi_hw *adapter, wifi_reschedule_twt_config_t reschedule_twt_config);
#endif
int process_rx_mgmt_beacon(struct rsi_hw *adapter, struct sk_buff *skb);
int rsi_process_mbssid_parameters(struct rsi_hw *adapter, struct ieee80211_mgmt *mgmt, u8 *ie_start, int ie_len);
#define DIAGNOSTIC_REPORT 85
int send_diagnostic_report_resp_to_app(struct rsi_hw *adapter, u32 reg_val, int bytes);
//__9117_CODE_END
void gpio_deinit(struct rsi_common *common);
#if defined(CONFIG_RSI_COEX_MODE) && defined(CONFIG_RSI_ZIGB)
struct rsi_mod_ops *rsi_get_zb_ops(void);
#endif
#endif
