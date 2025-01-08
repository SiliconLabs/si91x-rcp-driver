/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#ifndef __RSI_RRM_H__
#define __RSI_RRM_H__

#include "rsi_main.h"

#define RRM_MIN_CHLOAD_FRM_LEN 6
#define RRM_MIN_FRM_REQ_LEN    13
#define RRM_MIN_BEACON_FRM_LEN 13

#define MAX_RRM_REQ 5

/* RRM FSM States */
#define RRM_IDLE      0
#define RRM_REQ_SENT  1
#define RRM_RESP_RCVD 2

#define MAX_OPT_SUB_ELM_SIZE 128
#define DRV_HDR_SIZE         (FRAME_DESC_SZ + 64)

#define WLAN_ACTION_RADIO_MEASUREMENT   5
#define WLAN_ACTION_SPECTRUM_MANAGEMENT 0
#define MEAS_REQ                        38
#define MEAS_RPT                        39

/* 11k Actions */
#define IEEE80211_ACTION_RADIO_MEAS_REQ 0
#define IEEE80211_ACTION_RADIO_MEAS_RPT 1
#define IEEE80211_ACTION_LINK_MEAS_REQ  2
#define IEEE80211_ACTION_LINK_MEAS_REP  3
#define IEEE80211_ACTION_NG_RPT_REQ     4
#define IEEE80211_ACTION_NG_RPT_RSP     5

struct ieee80211_basic_hdr {
  __le16 frame_control;
  __le16 duration_id;
  u8 addr1[ETH_ALEN];
  u8 addr2[ETH_ALEN];
  u8 addr3[ETH_ALEN];
  __le16 seq_ctrl;
} __packed;

/* Radio Measurement types */
#define RRM_TYPE_CHANNEL_LOAD 3
#define RRM_TYPE_FRAME        6
#define RRM_TYPE_BEACON       5

/* Element ID Fields  */
#define SSID_ELEM_ID       0
#define SSID_LEN           32
#define BEACON_REPORT_INFO 1
#define REPORTING_DETAIL   2
#define AP_CHANNEL_RPT     51
#define VENDOR_SPECIFIC    221

#define MAX_REGIONS   4
#define MAX_REG_CLASS 255
#define RSI_BW_5      5
#define RSI_BW_10     10
#define RSI_BW_20     20
#define RSI_BW_25     25
#define RSI_BW_40     40

struct dot11_elem {
  u8 elem_id;
  u8 elem_len;
  u8 elem_data[0];
} __packed;

struct wl_action {
  u8 category;
  u8 action;
  u8 dialog_token;
};

struct wl_action_req {
  u8 category;
  u8 action;
  u8 dialog_token;
  u16 num_repetitions;
} __packed;

struct rm_element {
  u8 element_id;
  u8 length;
  u8 token;
  u8 mode;
  u8 type;
} __packed;

struct dot11_radio_meas_req {
  struct ieee80211_basic_hdr mac_hdr;
  struct wl_action_req action_body;
  struct rm_element rmelem;
  u8 regulatory_class;
  u8 channel_num;
  u16 rand_int;
  u16 meas_duration;
} __packed;

struct dot11_frame_meas_req {
  struct ieee80211_basic_hdr mac_hdr;
  struct wl_action_req action_body;
  struct rm_element rmelem;
  u8 regulatory_class;
  u8 channel_num;
  u16 rand_int;
  u16 meas_duration;
  u8 frame_req_type;
  u8 macid[6];
} __packed;

struct dot11_beacon_meas_req {
  struct ieee80211_basic_hdr mac_hdr;
  struct wl_action_req action_body;
  struct rm_element rmelem;
  u8 regulatory_class;
  u8 channel_num;
  u16 rand_int;
  u16 meas_duration;
  u8 meas_mode;
  u8 bssid[6];
  u8 opt_elems[0];
} __packed;

struct channel_load_rpt {
  struct ieee80211_basic_hdr mac_hdr;
  struct wl_action action_body;
  struct rm_element rmelem;
  u8 regulatory_class;
  u8 channel_num;
  u64 actual_meas_start_time;
  u16 meas_duration;
  u8 channel_load;
} __packed;

struct frame_report {
  struct ieee80211_basic_hdr mac_hdr;
  struct wl_action action_body;
  struct rm_element rmelem;
  u8 regulatory_class;
  u8 channel_num;
  u64 actual_meas_start_time;
  u16 meas_duration;

  /* optional subelements */
  u8 elem_id;
  u8 length;
  u8 tx_addr[6];
  u8 bssid[6];
  u8 phy_type;
  u8 avg_rcpi;
  u8 last_rsni;
  u8 last_rcpi;
  u8 ant_id;
  u16 frame_count;
} __packed;

struct beacon_report {
  struct ieee80211_basic_hdr mac_hdr;
  struct wl_action action_body;
  struct rm_element rmelem;
  u8 regulatory_class;
  u8 channel_num;
  u64 actual_meas_start_time;
  u16 meas_duration;
  u8 reported_frame_info;
  u8 rcpi;
  u8 rsni;
  u8 bssid[6];
  u8 antenna_id;
  u32 parent_tsf;
} __packed;

int rsi_rrm_send_channel_load_req(struct rsi_common *common);
int rsi_rrm_send_frame_req(struct rsi_common *common);
int rsi_rrm_send_beacon_req(struct rsi_common *common);
int rsi_rrm_parse_radio_action_frame(struct rsi_common *common, u8 *rx_rrm, s32 msg_len);
int rsi_rrm_parse_spectrum_action_frame(struct rsi_common *common, struct ieee80211_hdr *tmp_hdr, u8 *data);
int rsi_rrm_parse_channel_load_req(struct rsi_common *common, struct sk_buff *skb, struct rsi_meas_params *params);
int rsi_rrm_sched_req(struct rsi_common *common);
void rsi_rrm_recv_cmd_frame(struct rsi_common *common, u8 *msg, int len);
int rsi_prepare_channel_load_rpt(struct rsi_common *common, u8 *msg, int len);
int rsi_prepare_frame_rpt(struct rsi_common *common, u8 *msg, int len);
int rsi_prepare_beacon_rpt(struct rsi_common *common, u8 *msg, int len);
int rsi_rrm_parse_frame_req(struct rsi_common *common, struct sk_buff *skb, struct rsi_frame_meas_params *params);
int rsi_rrm_parse_beacon_req(struct rsi_common *common, struct sk_buff *skb, struct rsi_beacon_meas_params *params);
#endif
