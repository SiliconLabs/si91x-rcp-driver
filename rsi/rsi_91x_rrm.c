// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <linux/etherdevice.h>
#include "rsi_mgmt.h"
#include "rsi_common.h"
#include "rsi_rrm.h"

#if defined(CONFIG_RSI_11K) && defined(RSI_DEBUG_RRM)
int rsi_rrm_send_channel_load_req(struct rsi_common *common)
{
  u8 frame_len, chload_req_len;
  struct sk_buff *skb = NULL;
  struct dot11_radio_meas_req *chload_req;
  struct ieee80211_tx_info *info;
#ifdef RSI_DEBUG_RRM
#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif      = common->priv->vifs[common->priv->sc_nvifs - 1];
  struct ieee80211_bss_conf *bss = &vif->bss_conf;
#else
  struct ieee80211_vif *vif      = NULL;
  struct ieee80211_bss_conf *bss = NULL;

  vif = rsi_get_sta_vif(common->priv);
  if (!vif)
    return -ENODEV;
  bss = &vif->bss_conf;
#endif
#endif

  /* preparing on air channel load request */
  frame_len = sizeof(struct dot11_radio_meas_req);
  skb       = dev_alloc_skb(frame_len + DRV_HDR_SIZE);
  if (!skb)
    return -ENOMEM;
  skb_reserve(skb, DRV_HDR_SIZE);

  chload_req                              = (struct dot11_radio_meas_req *)skb->data;
  chload_req->action_body.category        = WLAN_ACTION_RADIO_MEASUREMENT;
  chload_req->action_body.action          = IEEE80211_ACTION_RADIO_MEAS_REQ;
  chload_req->action_body.num_repetitions = 0;
#ifdef RSI_DEBUG_RRM
  chload_req->mac_hdr.frame_control = IEEE80211_STYPE_ACTION;
  chload_req->mac_hdr.duration_id   = 0;
  chload_req->mac_hdr.seq_ctrl      = 0;
  ether_addr_copy(chload_req->mac_hdr.addr1, common->rrm_chload_params.macid);
  ether_addr_copy(chload_req->mac_hdr.addr2, vif->addr);
  ether_addr_copy(chload_req->mac_hdr.addr3, bss->bssid);
  chload_req->action_body.dialog_token = 2; /* FIXME: Fill proper value */
  chload_req->rmelem.mode              = common->rrm_chload_params.meas_req_mode;
  chload_req->rmelem.element_id        = MEAS_REQ;
  chload_req_len                       = sizeof(*chload_req) - sizeof(struct ieee80211_basic_hdr);
  chload_req_len -= sizeof(struct wl_action_req);
  chload_req_len -= sizeof(struct rm_element);
  chload_req->rmelem.length    = sizeof(struct rm_element) - 2 + chload_req_len;
  chload_req->rmelem.token     = 0;
  chload_req->rmelem.type      = 3;
  chload_req->regulatory_class = common->rrm_chload_params.regulatory_class;
  chload_req->channel_num      = common->rrm_chload_params.channel_num;
  chload_req->rand_int         = common->rrm_chload_params.rand_interval;
  chload_req->meas_duration    = common->rrm_chload_params.meas_duration;
#endif

  if ((chload_req->rmelem.mode >> 1 & 1) || (chload_req->rmelem.mode >> 2 & 1)) {
    rsi_dbg(INFO_ZONE, "station incapable/refusing frame report\n");
    goto err;
  }

  skb_put(skb, frame_len);
  info              = IEEE80211_SKB_CB(skb);
  info->control.vif = vif;
  rsi_dbg(MGMT_TX_ZONE, "Sending channel load measurement request\n");
  rsi_core_xmit(common, skb);

  return 0;

err:
  dev_kfree_skb(skb);
  return -EINVAL;
}

int rsi_rrm_send_frame_req(struct rsi_common *common)
{
  u8 frame_len;
  struct sk_buff *skb = NULL;
  struct dot11_frame_meas_req *frame_req;
  struct ieee80211_tx_info *info;
  u8 frame_req_len;
#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif      = common->priv->vifs[common->priv->sc_nvifs - 1];
  struct ieee80211_bss_conf *bss = &vif->bss_conf;
#else
  struct ieee80211_vif *vif      = NULL;
  struct ieee80211_bss_conf *bss = NULL;

  vif = rsi_get_sta_vif(common->priv);
  if (!vif)
    return -ENODEV;
  bss                            = &vif->bss_conf;
#endif

  frame_len = sizeof(struct dot11_frame_meas_req);
  skb       = dev_alloc_skb(frame_len + DRV_HDR_SIZE);
  if (!skb)
    return -ENOMEM;
  skb_reserve(skb, DRV_HDR_SIZE);

  frame_req                        = (struct dot11_frame_meas_req *)skb->data;
  frame_req->mac_hdr.frame_control = IEEE80211_STYPE_ACTION;
  frame_req->mac_hdr.duration_id   = 0;
  frame_req->mac_hdr.seq_ctrl      = 0;
  ether_addr_copy(frame_req->mac_hdr.addr1, common->rrm_frame_params.destid);
  ether_addr_copy(frame_req->mac_hdr.addr2, vif->addr);
  ether_addr_copy(frame_req->mac_hdr.addr3, bss->bssid);

  frame_req->action_body.category        = WLAN_ACTION_RADIO_MEASUREMENT;
  frame_req->action_body.action          = IEEE80211_ACTION_RADIO_MEAS_REQ;
  frame_req->action_body.dialog_token    = 2; /* FIXME: Fill proper value */
  frame_req->action_body.num_repetitions = 0;

  frame_req->rmelem.mode       = common->rrm_frame_params.meas_req_mode;
  frame_req->rmelem.element_id = MEAS_REQ;
  frame_req_len                = sizeof(*frame_req) - sizeof(struct ieee80211_basic_hdr);
  frame_req_len -= sizeof(struct wl_action_req);
  frame_req_len -= sizeof(struct rm_element);
  frame_req->rmelem.length = sizeof(struct rm_element) - 2 + frame_req_len;
  frame_req->rmelem.token  = 0;
  frame_req->rmelem.type   = 6;

  frame_req->regulatory_class = common->rrm_frame_params.regulatory_class;
  frame_req->channel_num      = common->rrm_frame_params.channel_num;
  frame_req->rand_int         = common->rrm_frame_params.rand_interval;
  frame_req->meas_duration    = common->rrm_frame_params.meas_duration;
  frame_req->frame_req_type   = common->rrm_frame_params.frame_req_type;
  ether_addr_copy(frame_req->macid, common->rrm_frame_params.macid);

  skb_put(skb, frame_len);
  info              = IEEE80211_SKB_CB(skb);
  info->control.vif = vif;
  rsi_dbg(MGMT_TX_ZONE, "Sending frame measurement request\n");
  rsi_core_xmit(common, skb);

  return 0;
}

int rsi_rrm_send_beacon_req(struct rsi_common *common)
{
  u8 frame_len;
  struct sk_buff *skb = NULL;
  struct dot11_beacon_meas_req *beacon_req;
  struct dot11_elem *opt_sub;
  struct ieee80211_tx_info *info;
  u16 opt_elems_len;
  u8 beacon_req_len;
#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif      = common->priv->vifs[common->priv->sc_nvifs - 1];
  struct ieee80211_bss_conf *bss = &vif->bss_conf;
#else
  struct ieee80211_vif *vif      = NULL;
  struct ieee80211_bss_conf *bss = NULL;

  vif = rsi_get_sta_vif(common->priv);
  if (!vif)
    return -ENODEV;
  bss = &vif->bss_conf;
#endif

  frame_len = sizeof(struct dot11_beacon_meas_req) + MAX_OPT_SUB_ELM_SIZE;
  skb       = dev_alloc_skb(frame_len + DRV_HDR_SIZE);
  if (!skb)
    return -ENOMEM;
  skb_reserve(skb, DRV_HDR_SIZE);
  memset(skb->data, 0, frame_len);

  beacon_req = (struct dot11_beacon_meas_req *)skb->data;

  beacon_req->mac_hdr.frame_control = IEEE80211_STYPE_ACTION;
  beacon_req->mac_hdr.duration_id   = 0;
  beacon_req->mac_hdr.seq_ctrl      = 0;
  ether_addr_copy(beacon_req->mac_hdr.addr1, common->rrm_beacon_params.destid);
  ether_addr_copy(beacon_req->mac_hdr.addr2, vif->addr);
  ether_addr_copy(beacon_req->mac_hdr.addr3, bss->bssid);

  beacon_req->action_body.category        = WLAN_ACTION_RADIO_MEASUREMENT;
  beacon_req->action_body.action          = IEEE80211_ACTION_RADIO_MEAS_REQ;
  beacon_req->action_body.dialog_token    = 2; /* FIXME fill proper value */
  beacon_req->action_body.num_repetitions = 0;
  beacon_req->rmelem.mode                 = common->rrm_beacon_params.meas_req_mode;
  beacon_req->rmelem.element_id           = MEAS_REQ;
  beacon_req_len                          = sizeof(*beacon_req) - sizeof(struct ieee80211_basic_hdr);
  beacon_req_len -= sizeof(struct wl_action_req);
  beacon_req_len -= sizeof(struct rm_element);
  beacon_req->rmelem.length = sizeof(struct rm_element) - 2 + beacon_req_len;
  beacon_req->rmelem.token  = 0;
  beacon_req->rmelem.type   = 5;

  beacon_req->regulatory_class = common->rrm_beacon_params.regulatory_class;
  beacon_req->channel_num      = common->rrm_beacon_params.channel_num;
  beacon_req->rand_int         = common->rrm_beacon_params.rand_interval;
  beacon_req->meas_duration    = common->rrm_beacon_params.meas_duration;
  beacon_req->meas_mode        = common->rrm_beacon_params.meas_mode;
  ether_addr_copy(beacon_req->bssid, common->rrm_beacon_params.bssid);

  opt_sub               = (struct dot11_elem *)beacon_req->opt_elems;
  opt_sub->elem_id      = 2;
  opt_sub->elem_len     = 1;
  opt_sub->elem_data[0] = 0;
  opt_elems_len         = 3;

  opt_sub           = (struct dot11_elem *)(beacon_req->opt_elems + opt_elems_len);
  opt_sub->elem_id  = 0;
  opt_sub->elem_len = strlen(common->rrm_beacon_params.str);
  memcpy(opt_sub->elem_data, common->rrm_beacon_params.str, opt_sub->elem_len);

  opt_elems_len += (2 + opt_sub->elem_len);
  beacon_req->rmelem.length += opt_elems_len;

  skb_put(skb, sizeof(*beacon_req) + opt_elems_len);
  info              = IEEE80211_SKB_CB(skb);
  info->control.vif = vif;
  rsi_dbg(MGMT_TX_ZONE, "Sending beacon measurement request\n");
  rsi_core_xmit(common, skb);

  return 0;
}
#endif

int rsi_rrm_parse_channel_load_req(struct rsi_common *common, struct sk_buff *skb, struct rsi_meas_params *params)
{
  u8 *frm = &skb->data[MIN_802_11_HDR_LEN];

  if (skb->len < RRM_MIN_CHLOAD_FRM_LEN)
    return -EINVAL;

  params->dialog_token     = frm[2];
  params->meas_req_mode    = frm[8];
  params->meas_type        = frm[9];
  params->regulatory_class = frm[10];
  params->channel_num      = frm[11];
  params->rand_interval    = *(u16 *)&frm[12];
  params->meas_duration    = *(u16 *)&frm[14];

  return 0;
}

int rsi_rrm_parse_frame_req(struct rsi_common *common, struct sk_buff *skb, struct rsi_frame_meas_params *params)
{
  u8 *frm = &skb->data[MIN_802_11_HDR_LEN];

  if (skb->len < RRM_MIN_FRM_REQ_LEN)
    return -EINVAL;

  params->mp.dialog_token     = frm[2];
  params->mp.meas_req_mode    = frm[8];
  params->mp.meas_type        = frm[9];
  params->mp.regulatory_class = frm[10];
  params->mp.channel_num      = frm[11];
  params->mp.rand_interval    = *(u16 *)&frm[12];
  params->mp.meas_duration    = *(u16 *)&frm[14];
  params->frame_req_type      = frm[16];
  ether_addr_copy(params->mac_addr, &skb->data[17]);

  return 0;
}

int rsi_rrm_parse_beacon_req(struct rsi_common *common, struct sk_buff *skb, struct rsi_beacon_meas_params *params)
{
  u8 *frm  = &skb->data[MIN_802_11_HDR_LEN];
  u8 index = 0, flags = 0;
  u8 opt_elem_id, opt_elem_len;

  if (skb->len < RRM_MIN_BEACON_FRM_LEN)
    return -EINVAL;

  params->mp.dialog_token     = frm[2];
  params->mp.meas_req_mode    = frm[8];
  params->mp.meas_type        = frm[9];
  params->mp.regulatory_class = frm[10];
  params->mp.channel_num      = frm[11];
  params->mp.rand_interval    = *(u16 *)&frm[12];
  params->mp.meas_duration    = *(u16 *)&frm[14];
  params->meas_mode           = frm[16];
  ether_addr_copy(params->mac_addr, &frm[17]);

  /* Parse through optional ids */
  index = 23; // + MIN_802_11_HDR_LEN;
  while (index < skb->len) {
    opt_elem_id  = frm[index];
    opt_elem_len = frm[index + 1];
    rsi_dbg(ERR_ZONE, "parse optional elemid %d elem_len %d\n", opt_elem_id, opt_elem_len);
    switch (opt_elem_id) {
      case SSID_ELEM_ID:
        flags = flags | (1 << 0);
        rsi_dbg(ERR_ZONE, "SSID INFO\n");
        memset(&params->ssid_ie[2], 0, SSID_LEN);
        memcpy(&params->ssid_ie[2], &frm[index + 2], opt_elem_len);
        rsi_dbg(ERR_ZONE, "SSID INFO %s %d\n", &params->ssid_ie[2], params->ssid_ie[1]);
        break;

      case BEACON_REPORT_INFO:
        flags = flags | (1 << 1);
        rsi_dbg(ERR_ZONE, "BEACON REPORT INFO\n");
        memcpy(params->bcn_rpt_info, &frm[index], frm[index + 1] + 2);
        break;

      case REPORTING_DETAIL:
        flags              = flags | (1 << 2);
        params->rpt_detail = frm[index + 2];
        rsi_dbg(ERR_ZONE, "REPORTING DETAILS VALUE %d\n", params->rpt_detail);
        break;

      case AP_CHANNEL_RPT:
        flags = flags | (1 << 3);
        rsi_dbg(ERR_ZONE, "Not supported in Firmware\n");
        return -EOPNOTSUPP;

      case VENDOR_SPECIFIC:
        flags = flags | (1 << 4);
        rsi_dbg(ERR_ZONE, "Not supported in Firmware\n");
        return -EOPNOTSUPP;

      default:
        rsi_dbg(ERR_ZONE, "Reserved element id\n");
        return -EOPNOTSUPP;
    }
    index += (2 + opt_elem_len);
  }

  return 0;
}

int rsi_rrm_sched_req(struct rsi_common *common)
{
  struct sk_buff *skb = NULL;
  u8 meas_req_type;
  struct rsi_meas_params chl_params;
  struct rsi_frame_meas_params frm_params;
  struct rsi_beacon_meas_params bcn_params;

  rsi_dbg(MGMT_RX_ZONE, "%s: Dequeing the radio action packets\n", __func__);

  if (skb_queue_len(&common->rrm_queue) <= 0)
    return 0;

  if (!common->priv->rrm_enq_state)
    skb = skb_dequeue(&common->rrm_queue);

  if (!skb)
    return 0;

  common->rrm_pending_frame = skb;
  rsi_hex_dump(ERR_ZONE, "RRM schedule frame\n", skb->data, skb->len);

  meas_req_type = skb->data[9 + MIN_802_11_HDR_LEN];

  if (skb->data[1 + MIN_802_11_HDR_LEN] == IEEE80211_ACTION_RADIO_MEAS_REQ) {
    common->priv->rrm_state     = RRM_REQ_SENT;
    common->priv->rrm_enq_state = 1;
    switch (meas_req_type) {
      case RRM_TYPE_CHANNEL_LOAD:
        rsi_dbg(INFO_ZONE, "Received channel load request\n");
        rsi_rrm_parse_channel_load_req(common, skb, &chl_params);
        rsi_get_channel_load_meas(common, chl_params);
        memcpy((u8 *)&common->chload_meas, (u8 *)&chl_params, sizeof(struct rsi_meas_params));
        break;
      case RRM_TYPE_FRAME:
        rsi_dbg(INFO_ZONE, "Received Frame request\n");
        rsi_rrm_parse_frame_req(common, skb, &frm_params);
        rsi_get_frame_meas(common, frm_params);
        memcpy((u8 *)&common->frame_meas, (u8 *)&frm_params, sizeof(struct rsi_frame_meas_params));
        break;
      case RRM_TYPE_BEACON:
        rsi_dbg(INFO_ZONE, "Received Beacon request\n");
        rsi_rrm_parse_beacon_req(common, skb, &bcn_params);
        rsi_get_beacon_meas(common, bcn_params);
        memcpy((u8 *)&common->beacon_meas, (u8 *)&bcn_params, sizeof(struct rsi_beacon_meas_params));
        break;
    }
  } else {
    rsi_dbg(INFO_ZONE, "Measurement not supported\n");
    return -EPERM;
  }

  return 0;
}

int rsi_rrm_parse_radio_action_frame(struct rsi_common *common, u8 *rx_rrm, s32 msg_len)
{
  struct rsi_hw *adapter = common->priv;
  struct sk_buff *skb    = NULL;

  rsi_dbg(MGMT_RX_ZONE, "%s: Parsing Radio action frame\n", __func__);
  if (common->num_pend_rrm_reqs > MAX_RRM_REQ) {
    rsi_dbg(ERR_ZONE, "Maxm requests received; dropping the req\n");
    kfree(rx_rrm);
    return 0;
  }
  skb = dev_alloc_skb(MAX_MGMT_PKT_SIZE);
  if (!skb)
    return -ENOMEM;
  memcpy(skb->data, rx_rrm, msg_len);
  skb_put(skb, msg_len);
  skb_queue_tail(&common->rrm_queue, skb);
  rsi_dbg(MGMT_RX_ZONE, "%s:Queued frame to skb queue\n", __func__);

  if (adapter->rrm_state == RRM_REQ_SENT)
    return 0;

  rsi_rrm_sched_req(common);

  return 0;
}

int rsi_rrm_parse_spectrum_action_frame(struct rsi_common *common, struct ieee80211_hdr *tmp_hdr, u8 *frm)
{
  return 0;
}

int rsi_prepare_channel_load_rpt(struct rsi_common *common, u8 *msg, int len)
{
  struct rsi_hw *adapter = common->priv;
#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = adapter->vifs[0];
#else
  struct ieee80211_vif *vif      = NULL;
#endif

#ifndef RSI_DEBUG_RRM
  u8 *rrm = NULL, tmp[6];
#else
#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_bss_conf *bss = &vif->bss_conf;
#else
  struct ieee80211_bss_conf *bss = NULL;
#endif
#endif
  u8 frame_len, chload_rpt_len;
  struct sk_buff *skb = NULL;
  struct channel_load_rpt *chload_rpt;
  struct ieee80211_tx_info *info;

#ifdef CONFIG_STA_PLUS_AP
  vif = rsi_get_sta_vif(adapter);
  if (!vif)
    return -ENODEV;
#ifdef RSI_DEBUG_RRM
  bss = &vif->bss_conf;
#endif
#endif
  /* preparing on air channel load report */
  frame_len = sizeof(struct channel_load_rpt);
  skb       = dev_alloc_skb(frame_len + DRV_HDR_SIZE);

  if (!skb)
    return -ENOMEM;
  skb_reserve(skb, DRV_HDR_SIZE);

  chload_rpt                       = (struct channel_load_rpt *)skb->data;
  chload_rpt->action_body.category = WLAN_ACTION_RADIO_MEASUREMENT;
  chload_rpt->action_body.action   = IEEE80211_ACTION_RADIO_MEAS_RPT;
#ifdef RSI_DEBUG_RRM
  chload_rpt->mac_hdr.frame_control = IEEE80211_STYPE_ACTION;
  chload_rpt->mac_hdr.duration_id   = 0;
  chload_rpt->mac_hdr.seq_ctrl      = 0;
  ether_addr_copy(chload_rpt->mac_hdr.addr1, &common->rrm_pending_frame->data[10]);
  ether_addr_copy(chload_rpt->mac_hdr.addr2, common->mac_addr);
  ether_addr_copy(chload_rpt->mac_hdr.addr3, bss->bssid);
  chload_rpt->action_body.dialog_token = 2; /* fill dialog_token*/
  chload_rpt->regulatory_class         = common->chload_meas.regulatory_class;
  chload_rpt->channel_num              = common->chload_meas.channel_num;
#else
  rrm                            = (u8 *)common->rrm_pending_frame->data;
  memcpy(&chload_rpt->mac_hdr, (struct ieee80211_min_hdr *)common->rrm_pending_frame->data, MIN_802_11_HDR_LEN);
  ether_addr_copy(tmp, chload_rpt->mac_hdr.addr1);
  ether_addr_copy(chload_rpt->mac_hdr.addr1, chload_rpt->mac_hdr.addr2);
  ether_addr_copy(chload_rpt->mac_hdr.addr2, tmp);
  ether_addr_copy(chload_rpt->mac_hdr.addr3, chload_rpt->mac_hdr.addr1);

  chload_rpt->action_body.dialog_token = common->rrm_pending_frame->data[MIN_802_11_HDR_LEN + 2];
  chload_rpt->regulatory_class         = common->rrm_pending_frame->data[MIN_802_11_HDR_LEN + 10];
  chload_rpt->channel_num              = common->rrm_pending_frame->data[MIN_802_11_HDR_LEN + 11];
#endif
  chload_rpt->rmelem.element_id = MEAS_RPT;
  chload_rpt_len                = sizeof(*chload_rpt) - sizeof(struct ieee80211_basic_hdr);
  chload_rpt_len -= sizeof(struct wl_action);
  chload_rpt_len -= sizeof(struct rm_element);
  chload_rpt->rmelem.length          = sizeof(struct rm_element) - 2 + chload_rpt_len;
  chload_rpt->rmelem.token           = 0;
  chload_rpt->rmelem.type            = 3;
  chload_rpt->actual_meas_start_time = *(u64 *)&msg[16];
  chload_rpt->meas_duration          = *(u16 *)&msg[10];
  chload_rpt->channel_load           = msg[8];
  chload_rpt->rmelem.mode            = msg[6];

  if ((chload_rpt->rmelem.mode >> 1 & 1) || (chload_rpt->rmelem.mode >> 2 & 1)) {
    rsi_dbg(INFO_ZONE, "station incapable/refusing channel load report\n");
    chload_rpt->actual_meas_start_time = 0;
  }
  rsi_dbg(ERR_ZONE, "sending onair channel load report\n");
  skb_put(skb, sizeof(*chload_rpt));
  info              = IEEE80211_SKB_CB(skb);
  info->control.vif = vif;
  rsi_core_xmit(common, skb);
  dev_kfree_skb(common->rrm_pending_frame);

  return 0;
}

int rsi_prepare_frame_rpt(struct rsi_common *common, u8 *msg, int len)
{
#ifdef RSI_DEBUG_RRM
  struct ieee80211_bss_conf *bss = NULL;
  struct ieee80211_vif *vif      = NULL;
  struct rsi_hw *adapter         = common->priv;
#else
  u8 *rrm                              = NULL, tmp[6];
#endif
  int status          = 0;
  struct sk_buff *skb = NULL;
  struct frame_report *frame_rpt;
  u8 frame_len, frame_rpt_len;
  u16 frame_entries;
  struct ieee80211_tx_info *info;

  /* preparing on air frame report */
  frame_len = sizeof(struct frame_report);
  skb       = dev_alloc_skb(frame_len + DRV_HDR_SIZE);
  if (!skb)
    return -ENOMEM;
  skb_reserve(skb, DRV_HDR_SIZE);

  frame_rpt                       = (struct frame_report *)skb->data;
  frame_rpt->action_body.category = WLAN_ACTION_RADIO_MEASUREMENT;
  frame_rpt->action_body.action   = IEEE80211_ACTION_RADIO_MEAS_RPT;
#ifdef RSI_DEBUG_RRM
  vif = rsi_get_vif(adapter, common->mac_addr);
  if (!vif) {
    rsi_dbg(ERR_ZONE, "%s: Failed to get vif\n", __func__);
    status = -ENOSPC;
    goto err;
  }
  bss                              = &vif->bss_conf;
  frame_rpt->mac_hdr.frame_control = IEEE80211_STYPE_ACTION;
  frame_rpt->mac_hdr.duration_id   = 0;
  frame_rpt->mac_hdr.seq_ctrl      = 0;
  ether_addr_copy(frame_rpt->mac_hdr.addr1, &common->rrm_pending_frame->data[10]);
  ether_addr_copy(frame_rpt->mac_hdr.addr2, common->mac_addr);
  ether_addr_copy(frame_rpt->mac_hdr.addr3, bss->bssid);
  frame_rpt->action_body.dialog_token = 2; /*fill dialog_token*/
  frame_rpt->rmelem.mode              = msg[6];
  frame_rpt->regulatory_class         = common->frame_meas.mp.regulatory_class;
  frame_rpt->channel_num              = common->frame_meas.mp.channel_num;
#else
  rrm                                  = (u8 *)common->rrm_pending_frame;
  memcpy(&frame_rpt->mac_hdr, (struct ieee80211_hdr *)common->rrm_pending_frame->data, MIN_802_11_HDR_LEN);
  ether_addr_copy(tmp, frame_rpt->mac_hdr.addr1);
  ether_addr_copy(frame_rpt->mac_hdr.addr1, frame_rpt->mac_hdr.addr2);
  ether_addr_copy(frame_rpt->mac_hdr.addr2, tmp);
  ether_addr_copy(frame_rpt->mac_hdr.addr3, frame_rpt->mac_hdr.addr1);
  frame_rpt->action_body.dialog_token = common->rrm_pending_frame->data[MIN_802_11_HDR_LEN + 2];
  frame_rpt->regulatory_class         = common->rrm_pending_frame->data[MIN_802_11_HDR_LEN + 10];
  frame_rpt->channel_num              = common->rrm_pending_frame->data[MIN_802_11_HDR_LEN + 11];

#endif
  frame_rpt->rmelem.element_id      = MEAS_RPT;
  frame_rpt->rmelem.token           = 0;
  frame_rpt->rmelem.type            = 6;
  frame_entries                     = *(u16 *)&msg[8];
  frame_rpt->actual_meas_start_time = *(u64 *)&msg[36];
  frame_rpt->meas_duration          = *(u16 *)&msg[10];
  frame_rpt->elem_id                = 1;
  frame_rpt_len                     = sizeof(*frame_rpt) - sizeof(struct ieee80211_basic_hdr);
  frame_rpt_len -= sizeof(struct wl_action);
  frame_rpt_len -= sizeof(struct rm_element);

  frame_rpt->rmelem.length = sizeof(struct rm_element) - 2 + frame_rpt_len;
  frame_rpt->length        = (19 * frame_entries);

  memcpy(&frame_rpt->tx_addr[0], &msg[24], 2);
  memcpy(&frame_rpt->tx_addr[2], &msg[26], 2);
  memcpy(&frame_rpt->tx_addr[4], &msg[28], 2);
  memcpy(&frame_rpt->bssid[0], &msg[30], 2);
  memcpy(&frame_rpt->bssid[2], &msg[32], 2);
  memcpy(&frame_rpt->bssid[4], &msg[34], 2);
  frame_rpt->phy_type    = msg[16];
  frame_rpt->avg_rcpi    = msg[18];
  frame_rpt->last_rcpi   = msg[19];
  frame_rpt->ant_id      = msg[17];
  frame_rpt->last_rsni   = msg[20];
  frame_rpt->frame_count = *(u16 *)&msg[22];
  frame_rpt->rmelem.mode = msg[6];

  rsi_dbg(INFO_ZONE, "frame_rpt->actual_meas_start_time %llu\n", frame_rpt->actual_meas_start_time);
  rsi_dbg(INFO_ZONE, "frame_rpt->meas_duration %d\n", frame_rpt->meas_duration);
  rsi_dbg(INFO_ZONE, "frame_rpt->phy_type %d\n", frame_rpt->phy_type);
  rsi_dbg(INFO_ZONE, "frame_rpt->avg_rcpi %d\n", frame_rpt->avg_rcpi);
  rsi_dbg(INFO_ZONE, "frame_rpt->last_rcpi %d\n", frame_rpt->last_rcpi);
  rsi_dbg(INFO_ZONE, "frame_rpt->ant_id %d\n", frame_rpt->ant_id);
  rsi_dbg(INFO_ZONE, "frame_rpt->last_rsni %d\n", frame_rpt->last_rsni);
  rsi_dbg(INFO_ZONE, "frame_rpt->frame_count %d\n", frame_rpt->frame_count);
  rsi_dbg(INFO_ZONE, "frame_rpt->length %d\n", frame_rpt->length);
  rsi_dbg(INFO_ZONE, "size of frame_report %d FRAME LEN %d\n", (int)sizeof(struct frame_report), frame_len);
  rsi_dbg(INFO_ZONE, "meas_rpt->length %d\n", frame_rpt->rmelem.length);
  rsi_hex_dump(ERR_ZONE, "frame report dump", skb->data, frame_len);
  skb_put(skb, sizeof(*frame_rpt));
  info = IEEE80211_SKB_CB(skb);
#ifndef CONFIG_STA_PLUS_AP
  info->control.vif = common->priv->vifs[0];
#else
  info->control.vif                   = rsi_get_first_valid_vif(common->priv);
#endif
  rsi_core_xmit(common, skb);
  dev_kfree_skb(common->rrm_pending_frame);

  return status;
#ifdef RSI_DEBUG_RRM
err:
  dev_kfree_skb(skb);
  return status;
#endif
}

int rsi_prepare_beacon_rpt(struct rsi_common *common, u8 *msg, int len)
{
#ifdef RSI_DEBUG_RRM
  struct ieee80211_bss_conf *bss = NULL;
  struct ieee80211_vif *vif      = NULL;
  struct rsi_hw *adapter         = common->priv;
#else
  u8 tmp[6], *rrm = NULL;
#endif
  int status = 0;
  u8 frame_len, bcon_rpt_len;
  u8 extended_desc    = msg[4], frame_body_len;
  struct sk_buff *skb = NULL;
  struct beacon_report *bcon_rpt;
  struct ieee80211_tx_info *info;

  frame_body_len = len - extended_desc;
  frame_len      = sizeof(struct beacon_report);
  skb            = dev_alloc_skb(frame_len + DRV_HDR_SIZE);
  if (!skb)
    return -ENOMEM;
  skb_reserve(skb, DRV_HDR_SIZE);

  bcon_rpt = (struct beacon_report *)skb->data;

  bcon_rpt->action_body.category = WLAN_ACTION_RADIO_MEASUREMENT;
  bcon_rpt->action_body.action   = IEEE80211_ACTION_RADIO_MEAS_RPT;
#ifdef RSI_DEBUG_RRM
  vif = rsi_get_vif(adapter, common->mac_addr);
  if (!vif) {
    rsi_dbg(ERR_ZONE, "%s: Failed to get vif\n", __func__);
    status = -ENOSPC;
    goto err;
  }
  bss                             = &vif->bss_conf;
  bcon_rpt->mac_hdr.frame_control = IEEE80211_STYPE_ACTION;
  bcon_rpt->mac_hdr.duration_id   = 0;
  bcon_rpt->mac_hdr.seq_ctrl      = 0;
  ether_addr_copy(bcon_rpt->mac_hdr.addr1, &common->rrm_pending_frame->data[10]);
  ether_addr_copy(bcon_rpt->mac_hdr.addr2, common->mac_addr);
  ether_addr_copy(bcon_rpt->mac_hdr.addr3, bss->bssid);
  bcon_rpt->action_body.dialog_token = 2; /*fill dialog_token*/
  bcon_rpt->regulatory_class         = common->beacon_meas.mp.regulatory_class;
  bcon_rpt->channel_num              = common->beacon_meas.mp.channel_num;
  ether_addr_copy(bcon_rpt->bssid, bss->bssid);
#else

  rrm = (u8 *)common->rrm_pending_frame->data;
  memcpy(&bcon_rpt->mac_hdr, (struct ieee80211_hdr *)common->rrm_pending_frame->data, MIN_802_11_HDR_LEN);
  ether_addr_copy(tmp, bcon_rpt->mac_hdr.addr1);
  ether_addr_copy(bcon_rpt->mac_hdr.addr1, bcon_rpt->mac_hdr.addr2);
  ether_addr_copy(bcon_rpt->mac_hdr.addr2, tmp);
  ether_addr_copy(bcon_rpt->mac_hdr.addr3, bcon_rpt->mac_hdr.addr1);

  bcon_rpt->action_body.dialog_token = common->rrm_pending_frame->data[MIN_802_11_HDR_LEN + 2];
  bcon_rpt->regulatory_class         = common->rrm_pending_frame->data[MIN_802_11_HDR_LEN + 10];
  bcon_rpt->channel_num              = rrm[MIN_802_11_HDR_LEN + 11];
  memcpy(&bcon_rpt->bssid, &common->rrm_pending_frame->data[MIN_802_11_HDR_LEN + 17], 6);
#endif
  bcon_rpt->rmelem.element_id = MEAS_RPT;
  bcon_rpt->rmelem.token      = 0;
  bcon_rpt->rmelem.type       = 5;
  bcon_rpt->rmelem.mode       = msg[6];
  bcon_rpt_len                = sizeof(*bcon_rpt) - sizeof(struct ieee80211_basic_hdr);
  bcon_rpt_len -= sizeof(struct wl_action);
  bcon_rpt_len -= sizeof(struct rm_element);

  bcon_rpt->rmelem.length          = sizeof(struct rm_element) - 2 + bcon_rpt_len;
  bcon_rpt->actual_meas_start_time = *(u64 *)&msg[24];
  bcon_rpt->meas_duration          = *(u16 *)&msg[10];
  bcon_rpt->reported_frame_info    = msg[16];
  bcon_rpt->rcpi                   = msg[18];
  bcon_rpt->rsni                   = msg[19];
  bcon_rpt->antenna_id             = msg[17];
  bcon_rpt->parent_tsf             = *(u32 *)&msg[20];

  rsi_dbg(INFO_ZONE, "common->rrm_state %d\n", common->priv->rrm_state);
  rsi_hex_dump(ERR_ZONE, "BEACON REPORT ", skb->data, skb->len);
  skb_put(skb, sizeof(*bcon_rpt));
  info = IEEE80211_SKB_CB(skb);
#ifndef CONFIG_STA_PLUS_AP
  info->control.vif = common->priv->vifs[0];
#else
  info->control.vif = rsi_get_first_valid_vif(common->priv);
#endif
  rsi_core_xmit(common, skb);
  dev_kfree_skb(common->rrm_pending_frame);

  return status;
#ifdef RSI_DEBUG_RRM
err:
  dev_kfree_skb(skb);
  return status;
#endif
}

void rsi_rrm_recv_cmd_frame(struct rsi_common *common, u8 *msg, int len)
{
  u8 meas_type = msg[5];

  switch (meas_type) {
    case RRM_TYPE_CHANNEL_LOAD:
      rsi_dbg(INFO_ZONE, "Preparing channel load report\n");
      rsi_prepare_channel_load_rpt(common, msg, len);
      break;
    case RRM_TYPE_BEACON:
      rsi_dbg(INFO_ZONE, "Preparing Beacon report\n");
      rsi_prepare_beacon_rpt(common, msg, len);
      break;
    case RRM_TYPE_FRAME:
      rsi_dbg(INFO_ZONE, "Preparing Frame report\n");
      rsi_prepare_frame_rpt(common, msg, len);
      break;
    default:
      rsi_dbg(INFO_ZONE, "Invalid cmd frame received\n");
  }
}
