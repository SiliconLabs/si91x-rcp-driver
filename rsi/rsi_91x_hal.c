// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <linux/firmware.h>
#include <linux/version.h>
#include <linux/jiffies.h>
#include <net/bluetooth/bluetooth.h>
#include "rsi_mgmt.h"
#include "rsi_hal.h"
#include "rsi_sdio.h"
#include "rsi_common.h"
#if defined(CONFIG_RSI_COEX_MODE) || defined(CONFIG_RSI_BT_ALONE)
#include "rsi_hci.h"
#endif
#ifdef CONFIG_RSI_COEX_MODE
#include "rsi_coex.h"
#endif

atomic_t drv_instances[5];

/* FLASH Firmware */
static struct ta_metadata metadata_flash_content[] = { { "flash_content", 0x00010000 },
                                                       { "RS9113_WLAN_QSPI.rps", 0x00010000 },
                                                       { "RS9113_WLAN_BT_DUAL_MODE.rps", 0x00010000 },
                                                       { "RS9113_WLAN_ZIGBEE.rps", 0x00010000 },
                                                       { "RS9113_AP_BT_DUAL_MODE.rps", 0x00010000 },
                                                       { "RS9113_WLAN_QSPI.rps", 0x00010000 },
                                                       { "RS9113_ZIGBEE_COORDINATOR.rps", 0x00010000 },
                                                       { "RS9113_ZIGBEE_ROUTER.rps", 0x00010000 }

};

#ifndef CONFIG_RS9116_FLASH_MODE
//__9117_CODE_START
#ifdef CONFIG_SECURE_FW_LOAD
static struct ta_metadata metadata_9117[] = { { "RS9117B0_WLAN_BT_RAM_IMAGE.rps",
                                                0x00000000 } }; //! Nlink 9117 Secure f/w load rps.
#else
static struct ta_metadata metadata_9117[] = { { "pmemdata_wlan_bt_9117",
                                                0x00000000 } }; //! WLAN/BLE/COEX Nlink 9117 image.
#endif
//__9117_CODE_END
static struct ta_metadata metadata[] = { { "pmemdata_dummy", 0x00000000 },
                                         { "pmemdata", 0x00000000 },
                                         { "pmemdata_wlan_bt_classic", 0x00000000 },
                                         { "pmemdata_wlan_zigb", 0x00000000 },
                                         { "pmemdata_wlan_bt_classic", 0x00000000 },
                                         { "pmemdata_dummy", 0x00000000 },
                                         { "pmemdata_zigb_coordinator", 0x00000000 },
                                         { "pmemdata_zigb_router", 0x00000000 } };

static struct ta_metadata metadata_1p5[] = { { "pmemdata_dummy", 0x00000000 },
                                             { "pmemdata_1p5", 0x00000000 },
                                             { "pmemdata_wlan_bt_classic_1p5", 0x00000000 },
                                             { "pmemdata_dummy", 0x00000000 },
                                             { "pmemdata_wlan_bt_classic_1p5", 0x00000000 },
                                             { "pmemdata_dummy", 0x00000000 },
                                             { "pmemdata_dummy", 0x00000000 },
                                             { "pmemdata_dummy", 0x00000000 } };

#elif !defined(CONFIG_RSI_LOAD_FW_FROM_FLASH_ONLY)
static struct ta_metadata metadata_9116_flash[] = { { "flash_content", 0x00010000 },
                                                    { "RS9116_NLINK_WLAN_IMAGE.rps", 0x00010000 },
                                                    { "RS9116_NLINK_WLAN_BT_IMAGE.rps", 0x00010000 },
                                                    { "RS9116_NLINK_WLAN_ZB_IMAGE.rps", 0x00010000 },
                                                    { "RS9116_NLINK_WLAN_BT_IMAGE.rps", 0x00010000 },
                                                    { "flash_content", 0x00010000 },
                                                    { "RS9116_NLINK_ZB_COORDINATOR.rps", 0x00010000 },
                                                    { "RS9116_NLINK_ZB_ROUTER.rps", 0x00010000 } };
#endif

/**
 * rsi_send_pkt() - This function sends the received packet from
 *			 driver to device.
 * @common: Pointer to the driver private structure.
 * @skb: Pointer to the socket buffer structure.
 *
 * Return: status: 0 on success, -1 on failure.
 */
int rsi_send_pkt(struct rsi_common *common, struct sk_buff *skb)
{
  struct rsi_hw *adapter = common->priv;
  int status             = -EINVAL;
  int status_tx_access   = 1;

  if (!(common->techs[WLAN_ID].tx_intention || common->techs[BT_ZB_ID].tx_intention) || !common->common_hal_tx_access) {
    status_tx_access = set_clr_tx_intention(common, COMMON_ID, 1);
    if (status_tx_access) {
      rsi_dbg(ERR_ZONE, "%s,%d:  Failed to get tx_access\n", __func__, __LINE__);
      return status;
    }
  }
  //#ifdef CONFIG_RSI_COEX_MODE
  //down(&coex_cb->tx_bus_lock);
  down(&common->tx_bus_lock);
  //#endif
  status = adapter->host_intf_ops->write_pkt(common->priv, skb->data, skb->len);
  //#ifdef CONFIG_RSI_COEX_MODE
  up(&common->tx_bus_lock);
  //#endif
  if (!status_tx_access)
    set_clr_tx_intention(common, COMMON_ID, 0);
  return status;
}

/**
 * rsi_prepare_data_desc() - This function prepares the device specific descriptor
 *			     for the given data packet
 *
 * @common: Pointer to the driver private structure.
 * @skb: Pointer to the socket buffer structure.
 *
 * Return: status: 0 on success, negative error code on failure.
 */
int rsi_prepare_data_desc(struct rsi_common *common, struct sk_buff *skb)
{
  struct rsi_hw *adapter    = common->priv;
  struct ieee80211_vif *vif = NULL;
  struct ieee80211_hdr *wh  = NULL;
  struct ieee80211_tx_info *info;
  struct skb_info *tx_params;
  struct ethhdr *ehdr   = NULL;
  int status            = -EINVAL;
  u8 ieee80211_hdr_size = MIN_802_11_HDR_LEN;
  u8 dword_align_bytes  = 0;
  u8 header_size        = 0;
  __le16 *frame_desc;
  struct xtended_desc *xtend_desc;
  u16 seq_num = 0;
  u8 vap_id   = 0;
#ifdef CONFIG_STA_PLUS_AP
  struct vif_priv *vif_info = NULL;
#endif
  struct rsi_sta *sta_info;
  struct ieee80211_sta *sta = NULL;
  //__9117_CODE_START
  u16 bbp_info = 0;
  //__9117_CODE_END

  info      = IEEE80211_SKB_CB(skb);
  tx_params = (struct skb_info *)info->driver_data;
  ehdr      = (struct ethhdr *)skb->data;

  header_size = FRAME_DESC_SZ + sizeof(struct xtended_desc);
  if (header_size > skb_headroom(skb)) {
    rsi_dbg(ERR_ZONE, "%s: Not enough headroom\n", __func__);
    status = -ENOSPC;
    goto err;
  }
  skb_push(skb, header_size);
  dword_align_bytes = ((unsigned long)skb->data & 0x3f);
  if (header_size > skb_headroom(skb)) {
    rsi_dbg(ERR_ZONE, "%s: Not enough headroom\n", __func__);
    status = -ENOSPC;
    goto err;
  }
  if (((skb->len + dword_align_bytes) % 64 == 0) && adapter->usb_full_speed == 1)
    dword_align_bytes += 1;
  skb_push(skb, dword_align_bytes);
  header_size += dword_align_bytes;

  tx_params->internal_hdr_size = header_size;
  frame_desc                   = (__le16 *)&skb->data[0];
  xtend_desc                   = (struct xtended_desc *)&skb->data[FRAME_DESC_SZ];
  memset((u8 *)frame_desc, 0, header_size);

  if (!(tx_params->flags & ENCAP_OFFLOAD_EN)) {
    wh      = (struct ieee80211_hdr *)&skb->data[header_size];
    seq_num = le16_to_cpu(IEEE80211_SEQ_TO_SN(wh->seq_ctrl));
    vif     = rsi_get_vif(adapter, wh->addr2);
  } else {
    vif = rsi_get_vif(adapter, ehdr->h_source);
  }

  if (!vif) {
    rsi_dbg(ERR_ZONE, "%s: Failed to get vif\n", __func__);
    status = -ENOSPC;
    goto err;
  }
#ifndef CONFIG_STA_PLUS_AP
  vap_id = ((struct vif_priv *)vif->drv_priv)->vap_id;
#else
  vif_info = (struct vif_priv *)vif->drv_priv;
  vap_id   = vif_info->vap_id;
#endif

  if (vif->type == NL80211_IFTYPE_STATION) {
#ifndef CONFIG_STA_PLUS_AP
    sta_info = (struct rsi_sta *)common->stations[RSI_MAX_ASSOC_STAS].sta->drv_priv;
    sta      = common->stations[RSI_MAX_ASSOC_STAS].sta;
#else
    sta_info = (struct rsi_sta *)common->stations[tx_params->sta_id].sta->drv_priv;
    sta      = common->stations[tx_params->sta_id].sta;
#endif
  } else {
    sta_info = (struct rsi_sta *)common->stations[tx_params->sta_id].sta->drv_priv;
    sta      = common->stations[tx_params->sta_id].sta;
  }

  frame_desc[2] = cpu_to_le16(header_size - FRAME_DESC_SZ);
  if (!(tx_params->flags & ENCAP_OFFLOAD_EN) && (wh != NULL)) {
    if (ieee80211_is_data_qos(wh->frame_control)) {
      ieee80211_hdr_size += 2;
      frame_desc[6] |= cpu_to_le16(BIT(12));
    }
  } else {
    frame_desc[1] = cpu_to_le16(ENCAP_OFFLOAD_DATA_PKT);
    if (sta && sta->wme)
      frame_desc[6] |= cpu_to_le16(QOS_SUPPORT);
  }

  if (!(tx_params->flags & ENCAP_OFFLOAD_EN) && (vif->type == NL80211_IFTYPE_STATION)
      && (adapter->ps_state == PS_ENABLED) && (wh != NULL))
    wh->frame_control |= BIT(12);

  if (!(tx_params->flags & ENCAP_OFFLOAD_EN)) {

    if ((!(info->flags & IEEE80211_TX_INTFL_DONT_ENCRYPT)) && (info->control.hw_key)) {
      if (rsi_is_cipher_wep(common))
        ieee80211_hdr_size += 4;
      else
        ieee80211_hdr_size += 8;
      frame_desc[6] |= cpu_to_le16(ENABLE_ENCRYPTION);
    }
  } else {
    if (!(info->flags & IEEE80211_TX_INTFL_DONT_ENCRYPT) && (skb->protocol != cpu_to_be16(ETH_P_PAE))
        && (skb->protocol != cpu_to_be16(ETH_P_802_3))) {
      frame_desc[6] |= cpu_to_le16(ENABLE_ENCRYPTION);
    }
  }

  frame_desc[0] = cpu_to_le16((skb->len - FRAME_DESC_SZ) | (RSI_WIFI_DATA_Q << 12));
  if (!(tx_params->flags & ENCAP_OFFLOAD_EN)) {
    frame_desc[2] |= cpu_to_le16(ieee80211_hdr_size << 8);
    frame_desc[6] |= cpu_to_le16(seq_num);
  }
  if (common->min_rate != 0xffff) {
    /* Send fixed rate */
    frame_desc[3] = cpu_to_le16(RATE_INFO_ENABLE);
    frame_desc[4] = cpu_to_le16(common->min_rate);

    if (conf_is_ht40(&common->priv->hw->conf))
      frame_desc[5] = cpu_to_le16(FULL40M_ENABLE);

#ifndef CONFIG_STA_PLUS_AP
    if ((common->vif_info[0].sgi) && (common->min_rate & 0x100)) {
#else
    if (vif_info->sgi && (common->min_rate & 0x100)) {
#endif
      /* Only MCS rates */
      frame_desc[4] |= cpu_to_le16(ENABLE_SHORTGI_RATE);
    }
  }

  //__9117_CODE_START
  if (adapter->ax_params._11ax_enabled && (vif->type == NL80211_IFTYPE_STATION)) {
    if (common->fixed_rate_en && (common->enable_er_su && common->config_er_su)) {
      if ((common->config_er_su == ER_SU_ONLY) || ((common->config_er_su == ERSU_HE_MIXED) && FIX_RATE_LESS_THAN_MCS3))
        adapter->ax_params.he_ppdu_type = ER_SU_PPDU;
      else
        adapter->ax_params.he_ppdu_type = SU_PPDU;
    }
    bbp_info =
      ((adapter->ax_params.coding_type << OFFSET_CODING_TYPE) | (adapter->ax_params.nominal_pe << OFFSET_NOMINAL_PE)
       | (adapter->ax_params.UL_DL << OFFSET_SPATIAL_REUSE_ENABLE)
       | (adapter->ax_params.he_ppdu_type << OFFSET_HE_PPDU_TYPE)
       | (adapter->ax_params.beam_change << OFFSET_BEAM_CHANGE) | (adapter->ax_params.BW << OFFSET_UL_DL)
       | (adapter->ax_params.STBC << OFFSET_STBC) | (adapter->ax_params.Tx_BF << OFFSET_TX_BF)
       | (adapter->ax_params.GI_LTF << OFFSET_GI_LTF) | (adapter->ax_params.DCM << OFFSET_DCM)
       | (adapter->ax_params.NSTS_MIDAMBLE << OFFSET_NSTS_MIDAMBLE));
    //#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0) * Disabling  fix rate in 11AX mode *
    //    frame_desc[3] = cpu_to_le16(RATE_INFO_ENABLE);
    //    frame_desc[4] = cpu_to_le16(adapter->ax_params.ax_rate | adapter->rate_field_params);
    //#endif
    frame_desc[5] = cpu_to_le16(bbp_info);
  }
  //__9117_CODE_END
  if (common->fixed_rate_en) {
    frame_desc[3] = cpu_to_le16(RATE_INFO_ENABLE);
    frame_desc[4] = cpu_to_le16(common->fixed_rate);
  }
  if (((vif->type == NL80211_IFTYPE_STATION)
       && ((skb->protocol == cpu_to_be16(ETH_P_PAE)) || (skb->protocol == cpu_to_be16(ETH_P_802_3))))
      || ((vif->type == NL80211_IFTYPE_AP) && (skb->protocol == cpu_to_be16(ETH_P_PAE)))) {
    rsi_dbg(INFO_ZONE, "*** Tx EAPOL ***\n");

    frame_desc[3] = cpu_to_le16(RATE_INFO_ENABLE);
    if (vif->type == NL80211_IFTYPE_STATION)
      frame_desc[4] = cpu_to_le16(sta_info->min_supported_rate);
    else if (vif->type == NL80211_IFTYPE_AP)
      frame_desc[4] = cpu_to_le16(RSI_RATE_1);
    //__9117_CODE_START
    if (adapter->ax_params._11ax_enabled)
      frame_desc[5] = 0; /* Send EAPOL pkt with non-11ax rate till S-AMPDU support */
    //__9117_CODE_END

    frame_desc[6] |= cpu_to_le16(QUEUE_TO_HEAD);
    if ((tx_params->flags & ENCAP_OFFLOAD_EN)) {
      if (vif->type == NL80211_IFTYPE_STATION) {
        frame_desc[1] = cpu_to_le16(FETCH_RETRY_CNT_FRM_HST);
        frame_desc[1] |= cpu_to_le16(ENCAP_OFFLOAD_MGMT_PKT);
      } else {
        frame_desc[1] |= cpu_to_le16(FETCH_RETRY_CNT_FRM_HST);
      }
    } else {
      frame_desc[1] |= cpu_to_le16(FETCH_RETRY_CNT_FRM_HST);
    }
    if (vif->type == NL80211_IFTYPE_STATION) {
      if (common->eapol4_confirm) {
        /* Eapol Rekeying , Change the priority to Voice _Q
			 * XXX: Check for AP*/
        skb->priority = VO_Q;
      } else {
        frame_desc[0] = cpu_to_le16((skb->len - FRAME_DESC_SZ) | (RSI_WIFI_MGMT_Q << 12));
      }

      if (!(tx_params->flags & ENCAP_OFFLOAD_EN)) {
        if (((skb->len - header_size) == 133) || ((skb->len - header_size) == 131)) {
          rsi_dbg(INFO_ZONE, "*** Tx EAPOL 4*****\n");
          frame_desc[1] |= cpu_to_le16(RSI_DESC_REQUIRE_CFM_TO_HOST);
          xtend_desc->confirm_frame_type = EAPOL4_CONFIRM;
        }
      } else {
        if (skb->len == EAPOL_4_LEN + header_size) {
          rsi_dbg(INFO_ZONE, "*** Tx EAPOL 4*****\n");
          frame_desc[1] |= cpu_to_le16(RSI_DESC_REQUIRE_CFM_TO_HOST);
          xtend_desc->confirm_frame_type = EAPOL4_CONFIRM;
        }
      }
    }
#define EAPOL_RETRY_CNT 15
    xtend_desc->retry_cnt = EAPOL_RETRY_CNT;
  }

  frame_desc[7] = cpu_to_le16(((tx_params->tid & 0xf) << 4) | (skb->priority & 0xf) | (tx_params->sta_id << 8));

  if (!(tx_params->flags & ENCAP_OFFLOAD_EN)) {
    if ((is_broadcast_ether_addr(wh->addr1)) || (is_multicast_ether_addr(wh->addr1))) {
      frame_desc[3] = cpu_to_le16(RATE_INFO_ENABLE);
      frame_desc[3] |= cpu_to_le16(RSI_BROADCAST_PKT);
      if ((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO)) {
        if (common->band == NL80211_BAND_5GHZ)
          frame_desc[4] = cpu_to_le16(RSI_RATE_6);
        else
          frame_desc[4] = cpu_to_le16(RSI_RATE_1);
      }
    }
  } else {
    if ((is_broadcast_ether_addr(ehdr->h_dest)) || (is_multicast_ether_addr(ehdr->h_dest))) {
      if ((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO)) {
        if (common->band == NL80211_BAND_5GHZ)
          frame_desc[4] = cpu_to_le16(RSI_RATE_6);
        else
          frame_desc[4] = cpu_to_le16(RSI_RATE_1);
      }
    }
  }

  frame_desc[4] |= cpu_to_le16(vap_id << 14);
  if (!(tx_params->flags & ENCAP_OFFLOAD_EN) && (wh != NULL)
      && ((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO))
      && (ieee80211_has_moredata(wh->frame_control)))
    frame_desc[3] |= cpu_to_le16(MORE_DATA_PRESENT);

  return 0;

err:
  ++common->tx_stats.total_tx_pkt_freed[skb->priority];
  rsi_indicate_tx_status(adapter, skb, status);
  return status;
}

/**
 * rsi_prepare_mgmt_desc() - This functions prepares the descriptor for
 *			     the given management packet.
 *
 * @common: Pointer to the driver private structure.
 * @skb: Pointer to the socket buffer structure.
 *
 * Return: status: 0 on success, -1 on failure.
 */
int rsi_prepare_mgmt_desc(struct rsi_common *common, struct sk_buff *skb)
{
  struct rsi_hw *adapter   = common->priv;
  struct ieee80211_hdr *wh = NULL;
  struct ieee80211_tx_info *info;
  struct ieee80211_conf *conf; // = &adapter->hw->conf;
  struct ieee80211_vif *vif = NULL;
  struct skb_info *tx_params;
  int status                      = -EINVAL;
  __le16 *desc                    = NULL;
  struct xtended_desc *xtend_desc = NULL;
  u8 header_size                  = 0;
  u8 vap_id                       = 0;
  u32 dword_align_bytes           = 0;

  if (!adapter->hw)
    goto err;
  conf      = &adapter->hw->conf;
  info      = IEEE80211_SKB_CB(skb);
  tx_params = (struct skb_info *)info->driver_data;

  /* Update header size */
  header_size = FRAME_DESC_SZ + sizeof(struct xtended_desc);
  if (header_size > skb_headroom(skb)) {
    rsi_dbg(ERR_ZONE, "%s: Failed to add extended descriptor\n", __func__);
    status = -ENOSPC;
    goto err;
  }
  skb_push(skb, header_size);
  dword_align_bytes = ((unsigned long)skb->data & 0x3f);
  if (dword_align_bytes > skb_headroom(skb)) {
    rsi_dbg(ERR_ZONE, "%s: Failed to add dword align\n", __func__);
    status = -ENOSPC;
    goto err;
  }
  skb_push(skb, dword_align_bytes);
  header_size += dword_align_bytes;

  tx_params->internal_hdr_size = header_size;
  memset(&skb->data[0], 0, header_size);

  wh  = (struct ieee80211_hdr *)&skb->data[header_size];
  vif = rsi_get_vif(adapter, wh->addr2);
  if (!vif) {
    rsi_dbg(ERR_ZONE, "%s: Failed to get vif\n", __func__);
    status = -ENOSPC;
    goto err;
  }

  vap_id = ((struct vif_priv *)vif->drv_priv)->vap_id;

  desc       = (__le16 *)skb->data;
  xtend_desc = (struct xtended_desc *)&skb->data[FRAME_DESC_SZ];

  if (skb->len > MAX_MGMT_PKT_SIZE) {
    rsi_dbg(INFO_ZONE, "%s: Dropping mgmt pkt > 512\n", __func__);
    goto err;
  }

  desc[0] = cpu_to_le16((skb->len - FRAME_DESC_SZ) | (RSI_WIFI_MGMT_Q << 12));
  desc[1] = cpu_to_le16(TX_DOT11_MGMT);
  desc[2] = cpu_to_le16(header_size - FRAME_DESC_SZ);

  if (ieee80211_has_protected(wh->frame_control)) {
    desc[2] |= cpu_to_le16(MIN_802_11_HDR_LEN_MFP << 8);
    desc[6] = cpu_to_le16(MGMT_FRAME_PROTECTION);
  } else
    desc[2] |= cpu_to_le16(MIN_802_11_HDR_LEN << 8);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 20, 17) || defined(OFFLOAD_SCAN_TO_DEVICE)
  if (ieee80211_is_probe_req(wh->frame_control) && (common->scan_in_prog))
    desc[3] = cpu_to_le16(INSERT_SEQ_IN_FW);
#endif
  desc[3] |= cpu_to_le16(RATE_INFO_ENABLE);
  if (wh->addr1[0] & BIT(0))
    desc[3] |= cpu_to_le16(RSI_BROADCAST_PKT);
  desc[6] |= cpu_to_le16(IEEE80211_SEQ_TO_SN(wh->seq_ctrl));

  if (ieee80211_is_probe_req(wh->frame_control) || ieee80211_is_probe_resp(wh->frame_control)
      || ieee80211_is_beacon(wh->frame_control)) {
    if (common->band == NL80211_BAND_2GHZ)
      if (!common->p2p_enabled)
        desc[4] = cpu_to_le16(RSI_RATE_1);
      else
        desc[4] = cpu_to_le16(RSI_RATE_6);
    else
      desc[4] = cpu_to_le16(RSI_RATE_6);
  } else {
    desc[4] = cpu_to_le16(common->minimum_basic_rate);
  }

  if (conf_is_ht40(conf)) {
    desc[5] = cpu_to_le16(FULL40M_ENABLE);
  }

  if (ieee80211_is_probe_resp(wh->frame_control)) {
    desc[1] |= cpu_to_le16(ADD_DELTA_TSF_VAP_ID | FETCH_RETRY_CNT_FRM_HST);
#define PROBE_RESP_RETRY_CNT 3
    xtend_desc->retry_cnt = PROBE_RESP_RETRY_CNT;
  }

  if (((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO))
      && (ieee80211_is_action(wh->frame_control))) {
    struct rsi_sta *sta = rsi_find_sta(common, wh->addr1);

    if (sta)
      desc[7] |= cpu_to_le16(sta->sta_id << 8);
    else
      goto err;
  } else
    desc[7] |= cpu_to_le16(vap_id << 8); /* Station ID */
  desc[4] |= cpu_to_le16(vap_id << 14);

  return 0;

err:
  return status;
}

/**
 * rsi_send_data_pkt() - This function sends the received data packet from
 *			 driver to device.
 * @common: Pointer to the driver private structure.
 * @skb: Pointer to the socket buffer structure.
 *
 * Return: status: 0 on success, -1 on failure.
 */
int rsi_send_data_pkt(struct rsi_common *common, struct sk_buff *skb)
{
  struct rsi_hw *adapter    = common->priv;
  struct ieee80211_vif *vif = NULL;
  struct ieee80211_hdr *wh  = NULL;
  struct ethhdr *ehdr       = NULL;
  struct ieee80211_tx_info *info;
  struct skb_info *tx_params;
  struct ieee80211_bss_conf *bss = NULL;
  int status                     = -EINVAL;
  u8 header_size                 = 0;

  if (!skb)
    return 0;
  if (common->iface_down)
    goto err;
  info = IEEE80211_SKB_CB(skb);
  if (!info->control.vif)
    goto err;
  bss       = &info->control.vif->bss_conf;
  tx_params = (struct skb_info *)info->driver_data;

  header_size = tx_params->internal_hdr_size;
  if (!(tx_params->flags & ENCAP_OFFLOAD_EN)) {
    wh  = (struct ieee80211_hdr *)&skb->data[header_size];
    vif = rsi_get_vif(adapter, wh->addr2);
  } else {
    ehdr = (struct ethhdr *)&skb->data[header_size];
    vif  = rsi_get_vif(adapter, ehdr->h_source);
  }

  if (!vif) {
    rsi_dbg(ERR_ZONE, "%s: Failed to get vif\n", __func__);
    status = -ENOSPC;
    goto err;
  }
  if (vif->type == NL80211_IFTYPE_STATION) {
    if (!BSS_ASSOC)
      goto err;
    if (!(tx_params->flags & ENCAP_OFFLOAD_EN) && !ether_addr_equal(wh->addr1, bss->bssid))
      goto err;
  }

  rsi_dbg(INFO_ZONE, "hal: Sending data pkt");
  rsi_hex_dump(DATA_TX_ZONE, "TX data pkt", skb->data, skb->len);

  status = rsi_send_pkt(common, skb);
  if (status)
    rsi_dbg(ERR_ZONE, "%s: Failed to write data pkt\n", __func__);

err:
  ++common->tx_stats.total_tx_pkt_freed[skb->priority];
  rsi_indicate_tx_status(common->priv, skb, status);
  return status;
}

/**
 * rsi_send_mgmt_pkt() - This function prepares sends the given mgmt packet
 *			 to device.
 *
 * @common: Pointer to the driver private structure.
 * @skb: Pointer to the socket buffer structure.
 *
 * Return: status: 0 on success, -1 on failure.
 */
int rsi_send_mgmt_pkt(struct rsi_common *common, struct sk_buff *skb)
{
  struct rsi_hw *adapter   = common->priv;
  struct ieee80211_hdr *wh = NULL;
  struct ieee80211_tx_info *info;
  struct skb_info *tx_params;
  u8 header_size                  = 0;
  int status                      = -EINVAL;
  struct ieee80211_bss_conf *bss  = NULL;
  __le16 *desc                    = NULL;
  struct xtended_desc *xtend_desc = NULL;
  struct ieee80211_vif *vif       = NULL;
  int wait_for_tx_cnfm            = 0;

  if (!skb)
    return 0;

  info        = IEEE80211_SKB_CB(skb);
  tx_params   = (struct skb_info *)info->driver_data;
  header_size = tx_params->internal_hdr_size;

  if (tx_params->flags & INTERNAL_MGMT_PKT) {
    skb->data[1] |= BIT(7); /* Immediate Wakeup bit*/
    rsi_hex_dump(MGMT_TX_ZONE, "Tx Command Packet", skb->data, skb->len);

    status = rsi_send_pkt(common, skb);
    if (status) {
      rsi_dbg(ERR_ZONE, "%s: Failed to write the packet\n", __func__);
    }
    dev_kfree_skb(skb);
    return status;
  }

  if (common->iface_down)
    goto out;

  if (tx_params->flags & ENCAP_OFFLOAD_EN)
    goto snd_encap_data;

  wh  = (struct ieee80211_hdr *)&skb->data[header_size];
  vif = rsi_get_vif(adapter, wh->addr2);
  if (!vif) {
    rsi_dbg(ERR_ZONE, "%s: Failed to get vif\n", __func__);
    status = -ENOSPC;
    goto out;
  }

  bss = &vif->bss_conf;
  if (!bss) {
    rsi_dbg(ERR_ZONE, "%s: BSS is NULL \n", __func__);
    status = -ENOSPC;
    goto out;
  }
  desc       = (__le16 *)skb->data;
  xtend_desc = (struct xtended_desc *)&skb->data[FRAME_DESC_SZ];

  if ((ieee80211_is_qos_nullfunc(wh->frame_control)) && (info->flags & IEEE80211_TX_CTL_REQ_TX_STATUS)) {
    struct rsi_sta *sta = NULL;
    int offset;
    if (vif->type == NL80211_IFTYPE_AP) {
      sta    = rsi_find_sta(common, wh->addr1);
      offset = sta->sta_id;
    } else if (vif->type == NL80211_IFTYPE_STATION) {
#ifndef CONFIG_STA_PLUS_AP
      sta    = &common->stations[RSI_MAX_ASSOC_STAS];
      offset = RSI_MAX_ASSOC_STAS;
#else
      sta    = &common->stations[STA_PEER];
      offset = STA_PEER;
#endif
    }
    if (sta) {
      rsi_dbg(MGMT_DEBUG_ZONE, "Adding NULL DATA CONFIRM  in frame desc\n");
      desc[1] |= cpu_to_le16(RSI_DESC_REQUIRE_CFM_TO_HOST);
      desc[7] |= cpu_to_le16(sta->sta_id << 8);
      xtend_desc->confirm_frame_type   = NULLDATA_CONFIRM;
      common->stations[offset].sta_skb = skb;
      wait_for_tx_cnfm                 = 1;
    } else {
      rsi_dbg(ERR_ZONE, "NULLDATA CNFM is not added ,STA is NULL  \n");
      rsi_indicate_tx_status(common->priv, skb, NULLDATA_FAIL);
      return status;
    }
  }
  /* Indicate to firmware to give cfm */
  if (ieee80211_is_probe_req(wh->frame_control)) {
    if (!BSS_ASSOC) {
      rsi_dbg(INFO_ZONE, "%s: blocking mgmt queue\n", __func__);
      desc[1] |= cpu_to_le16(RSI_DESC_REQUIRE_CFM_TO_HOST);
      xtend_desc->confirm_frame_type = PROBEREQ_CONFIRM;
    } else if (common->bgscan_en) {
      if (common->mac80211_cur_channel != rsi_get_connected_channel(adapter)) {
        /* Drop off channel probe request */
        status = 0;
        goto out;
      } else if (wh->addr1[0] == 0xff) {
        /* Drop broadcast probe in connected channel*/
        status = 0;
        goto out;
      }
    }
  }

  if (ieee80211_is_auth(wh->frame_control)) {
    rsi_dbg(MGMT_DEBUG_ZONE, "<==== Sending AUTHENTICATION Packet ====>\n");
    rsi_hex_dump(MGMT_DEBUG_ZONE, "AUTH-FRAME", skb->data, skb->len);
  } else if ((ieee80211_is_assoc_req(wh->frame_control)) || (ieee80211_is_reassoc_req(wh->frame_control))) {
    rsi_hex_dump(MGMT_DEBUG_ZONE, "ASSOC-REQUEST", skb->data, skb->len);
  } else if (ieee80211_is_deauth(wh->frame_control)) {
    rsi_dbg(MGMT_DEBUG_ZONE, "<==== Sending DE-AUTH Packet ====>\n");
    rsi_hex_dump(MGMT_DEBUG_ZONE, "DE-AUTH FRAME", skb->data, skb->len);
  }
  rsi_dbg(MGMT_TX_ZONE, "Sending Packet : %s =====>\n", dot11_pkt_type(wh->frame_control));

  rsi_hex_dump(MGMT_TX_ZONE, "Tx Mgmt Packet", skb->data, skb->len);

snd_encap_data:
  status = rsi_send_pkt(common, skb);

  if (status) {
    rsi_dbg(ERR_ZONE, "%s: Failed to write the packet\n", __func__);
  }

out:
  if (!wait_for_tx_cnfm) {
    rsi_indicate_tx_status(common->priv, skb, status);
  }
  return status;
}

int rsi_send_bt_pkt(struct rsi_common *common, struct sk_buff *skb)
{
  int status     = -EINVAL;
  u8 header_size = 0;
  __le16 *frame_desc;
  u8 queueno = ((skb->data[1] >> 4) & 0xf);

  if (queueno == RSI_BT_MGMT_Q) {
    rsi_hex_dump(MGMT_TX_ZONE, "TX BT Mgmt Pkt", skb->data, skb->len);

    status = rsi_send_pkt(common, skb);
    if (status)
      rsi_dbg(ERR_ZONE, "%s: Failed to write bt mgmt pkt\n", __func__);
    goto out;
  }

  header_size = FRAME_DESC_SZ;
  if (header_size > skb_headroom(skb)) {
    rsi_dbg(ERR_ZONE, "%s: Not enough headroom\n", __func__);
    status = -ENOSPC;
    goto out;
  }
  skb_push(skb, header_size);
  frame_desc = (__le16 *)&skb->data[0];
  memset((u8 *)frame_desc, 0, header_size);

  frame_desc[0] = cpu_to_le16(skb->len - FRAME_DESC_SZ);
  frame_desc[0] |= (cpu_to_le16(RSI_BT_DATA_Q) & 0x7) << 12;

  frame_desc[7] = cpu_to_le16(bt_cb(skb)->pkt_type);

  rsi_hex_dump(DATA_TX_ZONE, "TX BT pkt", skb->data, skb->len);
  status = rsi_send_pkt(common, skb);
  if (status)
    rsi_dbg(ERR_ZONE, "%s: Failed to write bt pkt\n", __func__);

out:
#ifdef CONFIG_RSI_BT_ANDROID
  dev_kfree_skb(skb);
#else
  kfree_skb(skb);
#endif
  return status;
}

int rsi_send_zb_pkt(struct rsi_common *common, struct sk_buff *skb)
{
  int status;

  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Null skb\n", __func__);
    return -EINVAL;
  }
  skb->data[1] |= BIT(7); /* Set immediate wake up */

  status = rsi_send_pkt(common, skb);
  if (status)
    rsi_dbg(ERR_ZONE, "%s: Failed to send ZigB packet\n", __func__);
  dev_kfree_skb(skb);

  return status;
}

#ifndef CONFIG_STA_PLUS_AP
int rsi_prepare_beacon(struct rsi_common *common, struct sk_buff *skb)
#else
int rsi_prepare_beacon(struct rsi_common *common, struct sk_buff *skb, struct ieee80211_vif *vif)
#endif
{
  struct rsi_hw *adapter        = (struct rsi_hw *)common->priv;
  struct rsi_mac_frame *bcn_frm = NULL;
  struct ieee80211_hw *hw       = common->priv->hw;
  struct ieee80211_conf *conf   = &hw->conf;
  struct sk_buff *mac_bcn       = NULL;
  u8 vap_id                     = 0;
  int status                    = 0;
  u16 tim_offset                = 0;

#ifndef CONFIG_STA_PLUS_AP
  mac_bcn = ieee80211_beacon_get_tim(adapter->hw,
                                     adapter->vifs[adapter->sc_nvifs - 1],
                                     &tim_offset,
                                     NULL
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
                                     ,
                                     0
#endif
  );
#else
  mac_bcn = ieee80211_beacon_get_tim(adapter->hw,
                                     vif,
                                     &tim_offset,
                                     NULL
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
                                     ,
                                     0
#endif
  );
#endif
  if (!mac_bcn) {
    rsi_dbg(ERR_ZONE, "Failed to get beacon from mac80211\n");
    return -EINVAL;
  }

  common->beacon_cnt++;
  bcn_frm               = (struct rsi_mac_frame *)skb->data;
  bcn_frm->desc_word[0] = cpu_to_le16(mac_bcn->len | (RSI_WIFI_DATA_Q << 12));
  bcn_frm->desc_word[1] = 0; // FIXME: Fill type later
  bcn_frm->desc_word[2] = cpu_to_le16(MIN_802_11_HDR_LEN << 8);
  bcn_frm->desc_word[3] = cpu_to_le16(MAC_BBP_INFO | NO_ACK_IND | BEACON_FRAME | INSERT_TSF | INSERT_SEQ_NO);
  bcn_frm->desc_word[3] |= cpu_to_le16(RATE_INFO_ENABLE);
  bcn_frm->desc_word[4] = cpu_to_le16(vap_id << 14);
  bcn_frm->desc_word[7] = cpu_to_le16(BEACON_HW_Q);

  if (conf_is_ht40_plus(conf)) {
    bcn_frm->desc_word[5] = cpu_to_le16(LOWER_20_ENABLE);
    bcn_frm->desc_word[5] |= cpu_to_le16(LOWER_20_ENABLE >> 12);
  } else if (conf_is_ht40_minus(conf)) {
    bcn_frm->desc_word[5] = cpu_to_le16(UPPER_20_ENABLE);
    bcn_frm->desc_word[5] |= cpu_to_le16(UPPER_20_ENABLE >> 12);
  }

  if (common->band == NL80211_BAND_2GHZ) {
    if (common->p2p_enabled)
      bcn_frm->desc_word[4] |= cpu_to_le16(RSI_RATE_6);
    else
      bcn_frm->desc_word[4] |= cpu_to_le16(RSI_RATE_1);
  } else
    bcn_frm->desc_word[4] |= cpu_to_le16(RSI_RATE_6);

  if (mac_bcn->data[tim_offset + 2] == 0)
    bcn_frm->desc_word[3] |= cpu_to_le16(DTIM_BEACON);

  memcpy(&skb->data[FRAME_DESC_SZ], mac_bcn->data, mac_bcn->len);
  skb_put(skb, mac_bcn->len + FRAME_DESC_SZ);

  rsi_hex_dump(MGMT_TX_ZONE, "Beacon Frame", skb->data, skb->len);

  if (mac_bcn)
    dev_kfree_skb(mac_bcn);

  return status;
}

/**
 * bl_cmd_timeout() - This function is called when BL command timed out
 * @priv: Pointer to the hardware structure.
 *
 * Return: NONE.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void bl_cmd_timeout(unsigned long priv)
{
  struct rsi_hw *adapter = (struct rsi_hw *)priv;
#else
static void bl_cmd_timeout(struct timer_list *t)
{
  struct rsi_hw *adapter = from_timer(adapter, t, bl_cmd_timer);
#endif
  adapter->blcmd_timer_expired = 1;
  del_timer(&adapter->bl_cmd_timer);
}

/**
 * bl_start_cmd_timer() - This function starts the BL command timer
 * @adapter: Pointer to the hardware structure.
 * @timeout: Timeout of the command in milliseconds
 *
 * Return: 0 on success.
 */
static int bl_start_cmd_timer(struct rsi_hw *adapter, u32 timeout)
{
  if (timer_pending(&adapter->bl_cmd_timer)) {
    rsi_dbg(ERR_ZONE, "%s : Timer Pending. This Case Should not occur\n", __func__);
    del_timer(&adapter->bl_cmd_timer);
  }
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
  init_timer(&adapter->bl_cmd_timer);
  adapter->bl_cmd_timer.data     = (unsigned long)adapter;
  adapter->bl_cmd_timer.function = (void *)&bl_cmd_timeout;
#else
  timer_setup(&adapter->bl_cmd_timer, bl_cmd_timeout, 0);
#endif
  adapter->bl_cmd_timer.expires = (msecs_to_jiffies(timeout) + jiffies);

  adapter->blcmd_timer_expired = 0;
  add_timer(&adapter->bl_cmd_timer);

  return 0;
}

/**
 * bl_stop_cmd_timer() - This function stops the BL command timer
 * @adapter: Pointer to the hardware structure.
 *
 * Return: 0 on success.
 */
static int bl_stop_cmd_timer(struct rsi_hw *adapter)
{
  adapter->blcmd_timer_expired = 0;
  if (timer_pending(&adapter->bl_cmd_timer))
    del_timer(&adapter->bl_cmd_timer);

  return 0;
}

/**
 * bl_write_cmd() - This function writes the BL command to device
 * @adapter: Pointer to the hardware structure.
 * @cmd: Command to write
 * @exp_resp: Expected Response
 * @cmd_resp: Received Response
 *
 * Return: 0 on success.
 */
static int bl_write_cmd(struct rsi_hw *adapter, u8 cmd, u8 exp_resp, u16 *cmd_resp)
{
  struct rsi_host_intf_ops *hif_ops = adapter->host_intf_ops;
  u32 regin_val = 0, regout_val = 0;
  u8 output       = 0;
  u32 regin_input = 0;

  regin_input = (REGIN_INPUT | adapter->priv->coex_mode);

  while (!adapter->blcmd_timer_expired) {
    regin_val = 0;
    if (hif_ops->master_reg_read(adapter, SWBL_REGIN, &regin_val, 2) < 0) {
      rsi_dbg(ERR_ZONE, "%s: Command %0x REGIN reading failed..\n", __func__, cmd);
      goto fail;
    }
    mdelay(1);
    if ((regin_val >> 12) != REGIN_VALID)
      break;
  }
  if (adapter->blcmd_timer_expired) {
    rsi_dbg(ERR_ZONE, "%s: Command %0x REGIN reading timed out..\n", __func__, cmd);
    goto fail;
  }

  rsi_dbg(INFO_ZONE, "Issuing write to Regin val:%0x sending cmd:%0x\n", regin_val, (cmd | regin_input << 8));
  if (cmd == CHECK_CRC && DEV_MODEL_9116) {
    regin_input |= 1 << 8;
    if ((hif_ops->master_reg_write(adapter, SWBL_REGIN, (cmd | regin_input << 8), 4)) < 0)
      goto fail;
  } else {
    if ((hif_ops->master_reg_write(adapter, SWBL_REGIN, (cmd | regin_input << 8), 2)) < 0)
      goto fail;
  }
  mdelay(1);

  if (cmd == LOAD_HOSTED_FW || cmd == JUMP_TO_ZERO_PC) {
    /* JUMP_TO_ZERO_PC doesn't expect
		 * any response. So return from here
		 */
    return 0;
  }

  while (!adapter->blcmd_timer_expired) {
    regout_val = 0;
    if (hif_ops->master_reg_read(adapter, SWBL_REGOUT, &regout_val, 2) < 0) {
      rsi_dbg(ERR_ZONE, "%s: Command %0x REGOUT reading failed..\n", __func__, cmd);
      goto fail;
    }
    mdelay(1);
    if ((regout_val >> 8) == REGOUT_VALID)
      break;
  }
  if (adapter->blcmd_timer_expired) {
    rsi_dbg(ERR_ZONE, "%s: Command %0x REGOUT reading timed out..\n", __func__, cmd);
    goto fail;
  }

  *cmd_resp = ((u16 *)&regout_val)[0] & 0xffff;

  output = ((u8 *)&regout_val)[0] & 0xff;

  if ((hif_ops->master_reg_write(adapter, SWBL_REGOUT, (cmd | REGOUT_INVALID << 8), 2)) < 0) {
    rsi_dbg(ERR_ZONE, "%s: Command %0x REGOUT writing failed..\n", __func__, cmd);
    goto fail;
  }
  mdelay(1);

  if (output == exp_resp || (cmd == EOF_REACHED && output == IMAGE_STORED_IN_DUMP)) {
    rsi_dbg(INFO_ZONE, "%s: received expected response 0x%0X for cmd 0x%0X\n", __func__, output, cmd);
  } else {
    rsi_dbg(INFO_ZONE, "%s: received response 0x%0X for cmd 0x%0X\n", __func__, output, cmd);
    goto fail;
  }
  return 0;

fail:
  return -EINVAL;
}

/**
 * bl_cmd() - This function initiates the BL command
 * @adapter: Pointer to the hardware structure.
 * @cmd: Command to write
 * @exp_resp: Expected Response
 * @str: Command string
 *
 * Return: 0 on success, -1 on failure.
 */
static int bl_cmd(struct rsi_hw *adapter, u8 cmd, u8 exp_resp, char *str)
{
  u16 regout_val = 0;
  u32 timeout    = 0;

  if ((cmd == EOF_REACHED) || (cmd == PING_VALID) || (cmd == PONG_VALID))
    timeout = BL_BURN_TIMEOUT;
  else
    timeout = BL_CMD_TIMEOUT;

  bl_start_cmd_timer(adapter, timeout);
  if (bl_write_cmd(adapter, cmd, exp_resp, &regout_val) < 0) {
    rsi_dbg(ERR_ZONE, "%s: Command %s (%0x) writing failed..\n", __func__, str, cmd);
    bl_stop_cmd_timer(adapter);
    goto fail;
  }
  bl_stop_cmd_timer(adapter);
  return 0;

fail:
  return -EINVAL;
}

/**
 * bl_write_header() - This function writes the BL header
 * @adapter: Pointer to the hardware structure.
 * @flash_content: Flash content
 * @content_size: Flash content size
 *
 * Return: 0 on success, -1 on failure.
 */
static int bl_write_header(struct rsi_hw *adapter, u8 *flash_content, u32 content_size)
{
  struct rsi_host_intf_ops *hif_ops = adapter->host_intf_ops;
  struct bl_header *bl_hdr          = NULL;
  u8 *rs9116_rps_header             = NULL;
  u32 write_addr, write_len;

  if (adapter->device_model == RSI_DEV_9113) {
#define CHECK_SUM_OFFSET 20
#define LEN_OFFSET       8
#define ADDR_OFFSET      16

    bl_hdr = kzalloc(sizeof(*bl_hdr), GFP_KERNEL);
    if (!bl_hdr)
      return -ENOMEM;

    bl_hdr->flags               = 0;
    bl_hdr->image_no            = cpu_to_le32(adapter->priv->coex_mode);
    bl_hdr->check_sum           = cpu_to_le32(*(u32 *)&flash_content[CHECK_SUM_OFFSET]);
    bl_hdr->flash_start_address = cpu_to_le32(*(u32 *)&flash_content[ADDR_OFFSET]);
    bl_hdr->flash_len           = cpu_to_le32(*(u32 *)&flash_content[LEN_OFFSET]);
    write_len                   = sizeof(struct bl_header);

    if (adapter->rsi_host_intf == RSI_HOST_INTF_USB) {
      write_addr = PING_BUFFER_ADDRESS;
      if ((hif_ops->write_reg_multiple(adapter, write_addr, (u8 *)bl_hdr, write_len)) < 0) {
        rsi_dbg(ERR_ZONE, "%s: Failed to load Version/CRC structure\n", __func__);
        goto fail;
      }
    } else {
      write_addr = PING_BUFFER_ADDRESS >> 16;
      if ((hif_ops->master_access_msword(adapter, write_addr)) < 0) {
        rsi_dbg(ERR_ZONE, "%s: Unable to set ms word to common reg\n", __func__);
        goto fail;
      }
      write_addr = SD_REQUEST_MASTER | (PING_BUFFER_ADDRESS & 0xFFFF);
      if ((hif_ops->write_reg_multiple(adapter, write_addr, (u8 *)bl_hdr, write_len)) < 0) {
        rsi_dbg(ERR_ZONE, "%s: Failed to load Version/CRC structure\n", __func__);
        goto fail;
      }
    }
    kfree(bl_hdr);
    return 0;

fail:
    kfree(bl_hdr);
    return -EINVAL;
  } else {
    content_size = RS9116_RPS_HEADER_SIZE;
    if (adapter->rsi_host_intf == RSI_HOST_INTF_USB) {
      write_addr = PING_BUFFER_ADDRESS_9116;
      if ((hif_ops->write_reg_multiple(adapter, write_addr, (u8 *)flash_content, content_size)) < 0) {
        rsi_dbg(ERR_ZONE, "%s: Failed to load Version/CRC structure\n", __func__);
        return -EINVAL;
      }
    } else {
      rs9116_rps_header = kzalloc(RS9116_RPS_HEADER_SIZE, GFP_KERNEL);
      if (!rs9116_rps_header)
        return -ENOMEM;

      memcpy(rs9116_rps_header, flash_content, RS9116_RPS_HEADER_SIZE);
      write_addr = PING_BUFFER_ADDRESS_9116 >> 16;
      if ((hif_ops->master_access_msword(adapter, write_addr)) < 0) {
        rsi_dbg(ERR_ZONE, "%s: Unable to set ms word to common reg\n", __func__);
        goto fail_hdr;
      }
      write_addr = SD_REQUEST_MASTER | (PING_BUFFER_ADDRESS_9116 & 0xFFFF);
      if ((hif_ops->write_reg_multiple(adapter, write_addr, (u8 *)rs9116_rps_header, content_size)) < 0) {
        rsi_dbg(ERR_ZONE, "%s: Failed to load Version/CRC structure\n", __func__);
        goto fail_hdr;
      }
    }
    kfree(rs9116_rps_header);
  }
  return 0;

fail_hdr:
  kfree(rs9116_rps_header);
  return -EINVAL;
}

/**
 * read_flash_capacity() - This function reads the flash size from device
 * @adapter: Pointer to the hardware structure.
 *
 * Return: flash capacity on success, 0 on failure.
 */
static u32 read_flash_capacity(struct rsi_hw *adapter)
{
  u32 flash_sz = 0;

  if ((adapter->host_intf_ops->master_reg_read(adapter, FLASH_SIZE_ADDR, &flash_sz, 2)) < 0) {
    rsi_dbg(ERR_ZONE, "%s: Flash size reading failed..\n", __func__);
    return 0;
  }
  rsi_dbg(INIT_ZONE, "Flash capacity: %d KiloBytes\n", flash_sz);

  return (flash_sz * 1024); /* Return size in kbytes */
}

/**
 * ping_pong_write() - This function writes the flash contents throgh ping
 *			pong buffers
 * @adapter: Pointer to the hardware structure.
 * @cmd: command ping/pong write
 * @addr: address to write
 * @size: size
 *
 * Return: 0 on success, -1 on failure.
 */
static int ping_pong_write(struct rsi_hw *adapter, u8 cmd, u8 *addr, u32 size)
{
  struct rsi_host_intf_ops *hif_ops = adapter->host_intf_ops;
  u32 block_size                    = 0;
  u32 cmd_addr;
  u16 cmd_resp = 0, cmd_req = 0;
  u8 *str;

  if (adapter->rsi_host_intf == RSI_HOST_INTF_SDIO)
    block_size = 256;
  else
    block_size = 252;

  if (cmd == PING_WRITE) {
    cmd_addr = PING_BUFFER_ADDRESS;
    cmd_resp = PONG_AVAIL;
    cmd_req  = PING_VALID;
    str      = "PING_VALID";
  } else {
    cmd_addr = PONG_BUFFER_ADDRESS;
    cmd_resp = PING_AVAIL;
    cmd_req  = PONG_VALID;
    str      = "PONG_VALID";
  }

  if (hif_ops->load_data_master_write(adapter, cmd_addr, size, block_size, addr)) {
    rsi_dbg(ERR_ZONE, "%s: Unable to write blk at addr %0x\n", __func__, *addr);
    goto fail;
  }
  if (bl_cmd(adapter, cmd_req, cmd_resp, str) < 0) {
    bl_stop_cmd_timer(adapter);
    goto fail;
  }
  return 0;

fail:
  return -EINVAL;
}

/**
 * auto_fw_upgrade() - This function loads the firmware to device
 * @adapter: Pointer to the hardware structure.
 * @flash_content: Firmware to load
 * @content_size: Size of the firmware
 *
 * Return: 0 on success, -1 on failure.
 */
static int auto_fw_upgrade(struct rsi_hw *adapter, u8 *flash_content, u32 content_size)
{
  u8 cmd;
  u8 *temp_flash_content;
  u32 temp_content_size;
  u32 num_flash;
  u32 index;
  u32 flash_start_address;

  temp_flash_content = flash_content;

  if (content_size > MAX_FLASH_FILE_SIZE) {
    rsi_dbg(ERR_ZONE, "%s: Flash Content size is more than 400K %u\n", __func__, MAX_FLASH_FILE_SIZE);
    goto fail;
  }

  flash_start_address = cpu_to_le32(*(u32 *)&flash_content[FLASHING_START_ADDRESS]);
  rsi_dbg(INFO_ZONE, "flash start address: %08x\n", flash_start_address);

  if (flash_start_address < FW_IMAGE_MIN_ADDRESS) {
    rsi_dbg(ERR_ZONE, "%s: Fw image Flash Start Address is less than 64K\n", __func__);
    goto fail;
  }

  if (flash_start_address % FLASH_SECTOR_SIZE) {
    rsi_dbg(ERR_ZONE, "%s: Flash Start Address is not multiple of 4K\n", __func__);
    goto fail;
  }
  if ((flash_start_address + content_size) > adapter->flash_capacity) {
    rsi_dbg(ERR_ZONE, "%s: Flash Content will cross max flash size\n", __func__);
    goto fail;
  }

  temp_content_size = content_size;
  num_flash         = content_size / FLASH_WRITE_CHUNK_SIZE;

  rsi_dbg(INFO_ZONE, "content_size: %d\n", content_size);
  rsi_dbg(INFO_ZONE, "num_flash: %d\n", num_flash);

  for (index = 0; index <= num_flash; index++) {
    rsi_dbg(INFO_ZONE, "flash index: %d\n", index);
    if (index != num_flash) {
      content_size = FLASH_WRITE_CHUNK_SIZE;
      rsi_dbg(INFO_ZONE, "QSPI content_size:%d\n", content_size);
    } else {
      content_size = temp_content_size % FLASH_WRITE_CHUNK_SIZE;
      rsi_dbg(INFO_ZONE, "Writing last sector content_size:%d\n", content_size);
      if (!content_size) {
        rsi_dbg(INFO_ZONE, "INSTRUCTION SIZE ZERO\n");
        break;
      }
    }

    if (index % 2)
      cmd = PING_WRITE;
    else
      cmd = PONG_WRITE;

    if (ping_pong_write(adapter, cmd, flash_content, content_size)) {
      rsi_dbg(ERR_ZONE, "%s: Unable to load %d block\n", __func__, index);
      goto fail;
    }

    rsi_dbg(INFO_ZONE, "%s: Successfully loaded %d instructions\n", __func__, index);
    flash_content += content_size;
  }

  if (bl_cmd(adapter, EOF_REACHED, FW_LOADING_SUCCESSFUL, "EOF_REACHED") < 0) {
    bl_stop_cmd_timer(adapter);
    goto fail;
  }
  rsi_dbg(INFO_ZONE, "FW loading is done and FW is running..\n");
  return 0;

fail:
  return -EINVAL;
}

#if (defined(CONFIG_RS9116_FLASH_MODE) && !defined(CONFIG_RSI_LOAD_FW_FROM_FLASH_ONLY))
static int rsi_check_crc(struct rsi_hw *adapter)
{

  struct rsi_host_intf_ops *hif_ops = adapter->host_intf_ops;
  u16 tmp_regout_val                = 0;

  if ((hif_ops->master_reg_write(adapter, MEM_ACCESS_CTRL_FROM_HOST, RAM_384K_ACCESS_FROM_TA, 4)) < 0) {
    rsi_dbg(ERR_ZONE, "%s: Unable to access full RAM memory\n", __func__);
    return -EIO;
  }
  bl_start_cmd_timer(adapter, BL_CMD_TIMEOUT);
  if (bl_write_cmd(adapter, CHECK_CRC, CMD_PASS, &tmp_regout_val) < 0) {
    bl_stop_cmd_timer(adapter);
    rsi_dbg(INFO_ZONE, "%s: CHECK_CRC Command writing failed..\n", __func__);
    if ((tmp_regout_val & 0xff) == CMD_FAIL) {
      rsi_dbg(ERR_ZONE, "device firmware doesn't match  proceed to upgrade ...\n");
      return -EINVAL;
    }
  }
  bl_stop_cmd_timer(adapter);
  return 0;
}

static int rsi_check_firmware(struct rsi_hw *adapter, const struct firmware *fw_entry)
{
  struct rsi_common *common = adapter->priv;
  u32 content_size          = 0;
  struct lmac_version_info *version_info;

  content_size = fw_entry->size;
  version_info = (struct lmac_version_info *)&fw_entry->data[RSI_LMAC_VER_OFFSET_RS9116];
  rsi_dbg(ERR_ZONE, " FW Length = %d bytes\n", content_size);
  common->lmac_ver.major       = version_info->major_id;
  common->lmac_ver.minor       = version_info->minor_id;
  common->lmac_ver.build_id    = (u16)(version_info->build_num);
  common->lmac_ver.patch_id    = (u16)(version_info->patch_num);
  common->lmac_ver.chip_id     = (u16)((version_info->chip_id << 8) | version_info->rom_ver);
  common->lmac_ver.customer_id = version_info->cust_id;
  rsi_print_version(common);
  if (bl_write_header(adapter, (u8 *)fw_entry->data, content_size)) {
    rsi_dbg(ERR_ZONE, "%s: RPS Image header loading failed\n", __func__);
    return -1;
  }
  return 0;
}
#endif
#ifdef CONFIG_RS9116_FLASH_MODE
static int rsi_load_9116_flash_fw(struct rsi_hw *adapter)
{
  struct rsi_host_intf_ops *hif_ops = adapter->host_intf_ops;

  if ((hif_ops->master_reg_write(adapter, MEM_ACCESS_CTRL_FROM_HOST, RAM_384K_ACCESS_FROM_TA, 4)) < 0) {
    rsi_dbg(ERR_ZONE, "%s: Unable to access full RAM memory\n", __func__);
    return -EIO;
  }
  if ((bl_cmd(adapter, LOAD_HOSTED_FW, LOADING_INITIATED, "LOAD_HOSTED_FW")) < 0)
    return -EIO;
  rsi_dbg(ERR_ZONE, "***** Loaded Firmware *****\n");
  return 0;
}
#else
//__9117_CODE_START
#ifdef CONFIG_SECURE_FW_LOAD
/**
 * rsi_secure_firmware_load() - firmware is loaded securely with slave writes
 * @adapter: Pointer to the hardware structure.
 * @firmware_ptr: Firmware to load
 * @fw_sz: Size of the firmware
 *
 * Return: 0 on success, non-zero error code on failure.
 */
static int rsi_secure_firmware_load(struct rsi_hw *adapter, u8 *firmware_ptr, u32 fw_sz)
{
  struct rsi_host_intf_ops *hif_ops = adapter->host_intf_ops;
  u8 *fw_block                      = NULL;
  u32 no_of_blocks, block_size = RSI_FW_BLOCK_SIZE;
  char cmd;
  u16 cmd_resp = 0, cmd_req = 0;
  u8 *str;
  int status = 0;

  /* This bl_cmd is sent to make the bootloader ready to read the 4k firmware chunk which is sent from this API*/
  status = bl_cmd(adapter, RSI_FW_UPGRADE, SEND_RPS_FILE, "FW_UPGRADE");
  if (status != 0) {
    rsi_dbg(ERR_ZONE, "bl_cmd failed, return value = %x\n", status);
    return -EINVAL;
  }
  rsi_dbg(INIT_ZONE, "BURN_TA_VIA_NLINK command success, upgrading the firmware\n");
  /* firmware_ptr is a data block allocated through vmalloc, somehow sdio_writesb is unable to access that vmalloc memory, hence using heap memory*/
  fw_block = kzalloc(block_size, GFP_KERNEL);
  if (!fw_block)
    return -ENOMEM;
  no_of_blocks = fw_sz / block_size + (fw_sz % block_size != 0);
  cmd          = PING_WRITE;
  while (1) {
    memset(fw_block, 0, block_size);
    memcpy(fw_block, firmware_ptr, block_size);
    /* second argument should be length of the data which we intended to write*/
    if ((hif_ops->write_reg_multiple(adapter, block_size, fw_block, block_size)) < 0) {
      rsi_dbg(ERR_ZONE, "%s: Failed to write firmware block\n", __func__);
      status = -EINVAL;
      goto fail;
    }
    if (cmd == PING_WRITE) {
      cmd_resp = PONG_AVAIL;
      cmd_req  = PING_VALID;
      str      = "PING_VALID";
      cmd      = PONG_WRITE;
    } else {
      cmd_resp = PING_AVAIL;
      cmd_req  = PONG_VALID;
      str      = "PONG_VALID";
      cmd      = PING_WRITE;
    }
    if (bl_cmd(adapter, cmd_req, cmd_resp, str) < 0) {
      rsi_dbg(ERR_ZONE, "%s:bl_cmd PING PONG failed\n", __func__);
      bl_stop_cmd_timer(adapter);
      status = -EINVAL;
      goto fail;
    }
    firmware_ptr = firmware_ptr + block_size;
    no_of_blocks--;
    if (!no_of_blocks)
      break;
  }
  /* bootloader expects an EOF command after sending the last firmware chunk*/
  if (bl_cmd(adapter, EOF_REACHED, CMD_PASS, "EOF_REACHED") < 0) {
    rsi_dbg(ERR_ZONE, "%s:bl_cmd EOF_REACHED failed\n", __func__);
    bl_stop_cmd_timer(adapter);
    status = -EINVAL;
    goto fail;
  }
  /*PC is set to zero, hence firmware execution starts*/
  rsi_dbg(INIT_ZONE, "%s:bl_cmd EOF_REACHED success\n", __func__);
  if (bl_cmd(adapter, JUMP_TO_ZERO_PC, CMD_PASS, "JUMP_TO_ZERO") < 0) {
    rsi_dbg(ERR_ZONE, "%s:bl_cmd JUMP TO ZERO failed\n", __func__);
    status = -EINVAL;
    goto fail;
  }
  rsi_dbg(INIT_ZONE, "%s:bl_cmd JUMP TO ZERO success\n", __func__);
  rsi_dbg(INIT_ZONE, "%s: Firmware loaded with slave writes successful\n", __func__);
fail:
  kfree(fw_block);
  return status;
}
#endif
//__9117_CODE_END

static int rsi_load_9116_firmware(struct rsi_hw *adapter)
{
  struct rsi_common *common         = adapter->priv;
  struct rsi_host_intf_ops *hif_ops = adapter->host_intf_ops;
  int status                        = 0;
  struct ta_metadata *metadata_p    = NULL;
  u8 *firmware_ptr;
  u32 instructions_sz             = 0;
  const struct firmware *fw_entry = NULL;
  int ii;
  struct bootload_ds bootload_ds;
  u32 base_address;
  u32 block_size;
  struct lmac_version_info *version_info;
#ifndef _9117_MACRO_DEF
  u32 chip_rev = 0;
#endif

  rsi_dbg(INIT_ZONE, "***** Load 9116 TA Instructions *****\n");

  //__9117_CODE_START
  /*when firmware is loaded via slave mode, driver need not request RAM access*/
#ifndef CONFIG_SECURE_FW_LOAD
  //__9117_CODE_END
  if ((hif_ops->master_reg_write(adapter, MEM_ACCESS_CTRL_FROM_HOST, RAM_384K_ACCESS_FROM_TA, 4)) < 0) {
    rsi_dbg(ERR_ZONE, "%s: Unable to access full RAM memory\n", __func__);
    return -EIO;
  }
//__9117_CODE_START
#endif
  //__9117_CODE_END

  //__9117_CODE_START
  if (adapter->device_model >= RSI_DEV_9117) {
    metadata_p = metadata_9117;
  } else
  //__9117_CODE_END
  {
    if (hif_ops->master_reg_read(adapter, REG_CHIP_REV, &chip_rev, 2) < 0) {
      rsi_dbg(ERR_ZONE, "%s: REG_CHIP_REV reading failed\n", __func__);
      return -EIO;
    }
    adapter->chip_rev = chip_rev & 0xff;
    if (adapter->chip_rev == CHIP_REV_1P5)
      metadata_p = &metadata_1p5[adapter->priv->coex_mode];
    else
      metadata_p = &metadata[adapter->priv->coex_mode];
  }

  rsi_dbg(INIT_ZONE, "%s: loading file %s\n", __func__, metadata_p->name);
  adapter->fw_file_name = metadata_p->name;

  if ((request_firmware(&fw_entry, metadata_p->name, adapter->device)) < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to open file %s\n", __func__, metadata_p->name);
    return -EINVAL;
  }
  firmware_ptr    = (u8 *)fw_entry->data;
  instructions_sz = fw_entry->size;
  //__9117_CODE_START
  if (adapter->device_model == RSI_DEV_9117)
    version_info = (struct lmac_version_info *)&fw_entry->data[LMAC_VER_OFFSET_RS9117];
  else if (adapter->device_model == RSI_DEV_917)
    version_info = (struct lmac_version_info *)&fw_entry->data[LMAC_VER_OFFSET_RS917];
  else
    //__9117_CODE_END
    version_info = (struct lmac_version_info *)&fw_entry->data[LMAC_VER_OFFSET_RS9116];
  rsi_dbg(INFO_ZONE, "FW Length = %d bytes\n", instructions_sz);
  common->lmac_ver.major            = version_info->major_id;
  common->lmac_ver.minor            = version_info->minor_id;
  common->lmac_ver.security_version = version_info->Reserved;
  common->lmac_ver.build_id         = (u16)(version_info->build_num);
  common->lmac_ver.patch_id         = (u16)(version_info->patch_num);

  common->lmac_ver.chip_id     = (u16)((version_info->chip_id << 8) | version_info->rom_ver);
  common->lmac_ver.customer_id = version_info->cust_id;
  rsi_print_version(common);
//__9117_CODE_START
#ifdef CONFIG_SECURE_FW_LOAD
  status = rsi_secure_firmware_load(adapter, firmware_ptr, instructions_sz);
  if (status)
    rsi_dbg(ERR_ZONE, "%s: Secure f/w load failed with %d\n", __func__, status);
  goto fail_free_fw;
#endif
  //__9117_CODE_END
  if (instructions_sz % 4)
    instructions_sz += (4 - (instructions_sz % 4));

  rsi_dbg(INFO_ZONE, "instructions_sz : %d\n", instructions_sz);

  base_address = metadata_p->address;

  if (adapter->rsi_host_intf == RSI_HOST_INTF_SDIO)
    block_size = 256;
  else
    block_size = 252;
  if (*(u16 *)firmware_ptr == 0x5aa5) {
    /* memcpy to boot descriptor from TA firmware len should be assign */
    memcpy(&bootload_ds, (u8 *)firmware_ptr, sizeof(struct bootload_ds));
    /* move ta firmware poiner to ta_fw+ len*/
    firmware_ptr = firmware_ptr + bootload_ds.offset;

    ii = 0;

    do {
      rsi_dbg(INIT_ZONE, "%s: Loading chunk %d\n", __func__, ii);

      rsi_dbg(INIT_ZONE,
              "length %d: destination %x\n",
              (bootload_ds.bl_entry[ii].control & RSI_BL_CTRL_LEN_MASK),
              bootload_ds.bl_entry[ii].dst_addr);

      rsi_dbg(INIT_ZONE, "FW start %x\n", *(u32 *)firmware_ptr);
      status = hif_ops->load_data_master_write(adapter,
                                               bootload_ds.bl_entry[ii].dst_addr,
                                               (bootload_ds.bl_entry[ii].control & RSI_BL_CTRL_LEN_MASK),
                                               block_size,
                                               firmware_ptr);

      firmware_ptr += (bootload_ds.bl_entry[ii].control & RSI_BL_CTRL_LEN_MASK);
      if (bootload_ds.bl_entry[ii].control & RSI_BL_CTRL_LAST_ENTRY)
        break;
      ii++;
    } while (1);
  } else {
    status = hif_ops->load_data_master_write(adapter, base_address, instructions_sz, block_size, (u8 *)fw_entry->data);
  }
  if (status) {
    rsi_dbg(ERR_ZONE, "%s: Unable to load %s blk\n", __func__, metadata_p->name);
    goto fail_free_fw;
  }

  rsi_dbg(INIT_ZONE, "%s: Successfully loaded %s instructions\n", __func__, metadata_p->name);
  rsi_dbg(ERR_ZONE, "***** Firmware Loading successful *****\n");

  if (adapter->rsi_host_intf == RSI_HOST_INTF_SDIO) {
    if (adapter->host_intf_ops->ta_reset_ops(adapter))
      rsi_dbg(ERR_ZONE, "Unable to put ta in reset\n");
  }

fail_free_fw:
  release_firmware(fw_entry);
  return status;
}
#endif

/**
 * rsi_load_firmware () - This function loads the TA firmware for 9113
 *				device.
 * @adapter: Pointer to the rsi hw.
 *
 * Return: status: 0 on success, -1 on failure.
 */
static int rsi_load_firmware(struct rsi_hw *adapter)
{
  struct rsi_common *common         = adapter->priv;
  struct rsi_host_intf_ops *hif_ops = adapter->host_intf_ops;
  const struct firmware *fw_entry   = NULL;
  u32 regout_val                    = 0;
  u16 tmp_regout_val                = 0;
  u32 content_size                  = 0;

#ifdef CONFIG_RS9116_FLASH_MODE
  u32 flash_data_start = 0;
#endif
  struct ta_metadata *metadata_p;
  int status;

  bl_start_cmd_timer(adapter, BL_CMD_TIMEOUT);
#ifdef CONFIG_RS9116_FLASH_MODE
  if (adapter->priv->coex_mode == 2) {
    if ((hif_ops->master_reg_read(adapter, RSI_FLASH_READ_COEX_IMAGE, &flash_data_start, 2)) < 0) {
      rsi_dbg(ERR_ZONE, "%s: RSI_FLASH_READ failed\n", __func__);
      goto bl_cmd_fail;
    }
  } else {
    if ((hif_ops->master_reg_read(adapter, RSI_FLASH_READ_WLAN_IMAGE, &flash_data_start, 2)) < 0) {

      rsi_dbg(ERR_ZONE, "%s: RSI_FLASH_READ failed\n", __func__);
      goto bl_cmd_fail;
    }
  }
#endif
  while (!adapter->blcmd_timer_expired) {
    if ((hif_ops->master_reg_read(adapter, SWBL_REGOUT, &regout_val, 2)) < 0) {
      rsi_dbg(ERR_ZONE, "%s: REGOUT read failed\n", __func__);
      goto bl_cmd_fail;
    }
    mdelay(1);
    if ((regout_val >> 8) == REGOUT_VALID)
      break;
  }
  if (adapter->blcmd_timer_expired) {
    rsi_dbg(ERR_ZONE, "%s: REGOUT read timedout\n", __func__);
    rsi_dbg(ERR_ZONE, "%s: Soft boot loader not present\n", __func__);
    goto bl_cmd_fail;
  }
  bl_stop_cmd_timer(adapter);

  rsi_dbg(INFO_ZONE, "Received Board Version Number: %x\n", (regout_val & 0xff));

  if ((hif_ops->master_reg_write(adapter, SWBL_REGOUT, (REGOUT_INVALID | REGOUT_INVALID << 8), 2)) < 0) {
    rsi_dbg(ERR_ZONE, "%s: REGOUT writing failed..\n", __func__);
    goto fail;
  }
  mdelay(1);
  if (DEV_MODEL_9116) {
#ifdef CONFIG_RS9116_FLASH_MODE
#ifdef CONFIG_RSI_LOAD_FW_FROM_FLASH_ONLY
    if (flash_data_start == 0x5aa5) {
      status = rsi_load_9116_flash_fw(adapter);
      mdelay(3000);
      if (adapter->rsi_host_intf == RSI_HOST_INTF_USB) {
        if (bl_cmd(adapter, POLLING_MODE, CMD_PASS, "POLLING_MODE") < 0) {
          goto fail;
        }
      }
      return status;
    } else {
      rsi_dbg(ERR_ZONE, "%s: *** Flash is Empty ***\n", __func__);
      return -EINVAL;
    }
#else
    metadata_p = &metadata_9116_flash[adapter->priv->coex_mode];
    rsi_dbg(ERR_ZONE, "%s: Loading file %s\n", __func__, metadata_p->name);
    adapter->fw_file_name = metadata_p->name;
    if ((request_firmware(&fw_entry, metadata_p->name, adapter->device)) < 0) {
      rsi_dbg(ERR_ZONE, "%s: Failed to open file %s\n", __func__, metadata_p->name);
      goto fail;
    }
    if (flash_data_start == 0x5aa5) {
      if (rsi_check_firmware(adapter, fw_entry)) {
        return -EINVAL;
      } else {
        if (rsi_check_crc(adapter))
          goto upgrade_9116_flash_fw;
      }
    } else {
      rsi_dbg(ERR_ZONE, " *** Flash is Empty ***\n");
      if (rsi_check_firmware(adapter, fw_entry)) {
        release_firmware(fw_entry);
        return -EINVAL;
      }
      goto upgrade_9116_flash_fw;
    }
load_9116_flash_fw:
    status = rsi_load_9116_flash_fw(adapter);
    release_firmware(fw_entry);
    return status;
upgrade_9116_flash_fw:
    adapter->flash_capacity = RSI_9116_FLASH_SIZE;
    content_size            = fw_entry->size;
    if (bl_cmd(adapter, BURN_HOSTED_FW, SEND_RPS_FILE, "FW_UPGRADE") < 0)
      goto fail;
    rsi_dbg(ERR_ZONE, " Burn Command Pass.. Upgrading the firmware\n");
    if (auto_fw_upgrade(adapter, (u8 *)fw_entry->data, content_size) == 0) {
      rsi_dbg(ERR_ZONE, "Firmware upgradation Done\n");
      msleep(5000);
      if (bl_cmd(adapter, POLLING_MODE, CMD_PASS, "POLLING_MODE") < 0) {
        goto fail;
      }
      goto load_9116_flash_fw;
    }
    rsi_dbg(ERR_ZONE, "Firmware upgrade failed\n");
    goto fail;

#endif
#else
    if (adapter->rsi_host_intf == RSI_HOST_INTF_USB) {
      if (bl_cmd(adapter, POLLING_MODE, CMD_PASS, "POLLING_MODE") < 0) {
        goto fail;
      }
    }
    status = rsi_load_9116_firmware(adapter);
    if (adapter->rsi_host_intf == RSI_HOST_INTF_USB) {
      if (bl_cmd(adapter, JUMP_TO_ZERO_PC, CMD_PASS, "JUMP_TO_ZERO") < 0)
        rsi_dbg(INFO_ZONE, "Jump to zero command failed\n");
      else
        rsi_dbg(INFO_ZONE, "Jump to zero command successful\n");
    }
    return status;
#endif
  } else {
    if ((bl_cmd(adapter, CONFIG_AUTO_READ_MODE, CMD_PASS, "AUTO_READ_CMD")) < 0)
      goto fail;

    adapter->flash_capacity = read_flash_capacity(adapter);
    if (adapter->flash_capacity <= 0) {
      rsi_dbg(ERR_ZONE, "%s: Unable to read flash size from EEPROM\n", __func__);
      goto fail;
    }

    metadata_p = &metadata_flash_content[adapter->priv->coex_mode];

    rsi_dbg(INIT_ZONE, "%s: Loading file %s\n", __func__, metadata_p->name);
    adapter->fw_file_name = metadata_p->name;

    if ((request_firmware(&fw_entry, metadata_p->name, adapter->device)) < 0) {
      rsi_dbg(ERR_ZONE, "%s: Failed to open file %s\n", __func__, metadata_p->name);
      goto fail;
    }
    content_size = fw_entry->size;
    rsi_dbg(INFO_ZONE, "FW Length = %d bytes\n", content_size);

    /* Get the firmware version */
    common->lmac_ver.ver.info.fw_ver[0] = fw_entry->data[LMAC_VER_OFFSET] & 0xFF;
    common->lmac_ver.ver.info.fw_ver[1] = fw_entry->data[LMAC_VER_OFFSET + 1] & 0xFF;
    common->lmac_ver.major              = fw_entry->data[LMAC_VER_OFFSET + 2] & 0xFF;
    common->lmac_ver.release_num        = fw_entry->data[LMAC_VER_OFFSET + 3] & 0xFF;
    common->lmac_ver.minor              = fw_entry->data[LMAC_VER_OFFSET + 4] & 0xFF;
    common->lmac_ver.patch_num          = 0;
    rsi_print_version(common);

    if (bl_write_header(adapter, (u8 *)fw_entry->data, content_size)) {
      rsi_dbg(ERR_ZONE, "%s: RPS Image header loading failed\n", __func__);
      goto fail;
    }

    bl_start_cmd_timer(adapter, BL_CMD_TIMEOUT);
    if (bl_write_cmd(adapter, CHECK_CRC, CMD_PASS, &tmp_regout_val) < 0) {
      bl_stop_cmd_timer(adapter);
      rsi_dbg(INFO_ZONE, "%s: CHECK_CRC Command writing failed..\n", __func__);
      if ((tmp_regout_val & 0xff) == CMD_FAIL) {
        rsi_dbg(ERR_ZONE, "device firmware doesnt match %s, proceed to upgrade ...\n", metadata_p->name);
        goto fw_upgrade;
      }
    }
    bl_stop_cmd_timer(adapter);

    if (bl_cmd(adapter, POLLING_MODE, CMD_PASS, "POLLING_MODE") < 0)
      goto fail;

load_image_cmd:
    if ((bl_cmd(adapter, LOAD_HOSTED_FW, LOADING_INITIATED, "LOAD_HOSTED_FW")) < 0)
      goto fail;
    rsi_dbg(INFO_ZONE, "Load Image command passed..\n");
    goto success;

fw_upgrade:
    if (bl_cmd(adapter, BURN_HOSTED_FW, SEND_RPS_FILE, "FW_UPGRADE") < 0)
      goto fail;

    rsi_dbg(INFO_ZONE, "Burn Command Pass.. Upgrading the firmware\n");

    if (auto_fw_upgrade(adapter, (u8 *)fw_entry->data, content_size) == 0) {
      rsi_dbg(ERR_ZONE, "Firmware upgradation Done\n");
      goto load_image_cmd;
    }
    rsi_dbg(ERR_ZONE, "Firmware upgrade failed\n");

    if (bl_cmd(adapter, CONFIG_AUTO_READ_MODE, CMD_PASS, "AUTO_READ_MODE") < 0)
      goto fail;
  }

success:
  rsi_dbg(ERR_ZONE, "***** Firmware Loading successful *****\n");
  release_firmware(fw_entry);
  return 0;

bl_cmd_fail:
  bl_stop_cmd_timer(adapter);
  return -EINVAL;

fail:
  rsi_dbg(ERR_ZONE, "##### Firmware loading failed #####\n");
  release_firmware(fw_entry);
  return -EINVAL;
}

int rsi_validate_oper_mode(u16 oper_mode)
{
#if defined(CONFIG_RS9116_PURISM)
  if ((oper_mode == 13) || (oper_mode == 14))
    return 0;
  else {
    rsi_dbg(ERR_ZONE,
            "Operating mode %d is not supported, "
            "it should be either 13 or 14\n",
            oper_mode);
    return -EINVAL;
  }
#endif
  switch (oper_mode) {
    case 1:
#if defined(CONFIG_RSI_BT_ALONE)
      rsi_dbg(ERR_ZONE,
              "Operating mode %d not supported with"
              "build flag 'CONFIG_RSI_BT_ALONE enabled'\n",
              oper_mode);
      return -EINVAL;
#else
      return 0;
#endif
    case 4:
    case 8:
    case 12:
#ifndef CONFIG_RSI_BT_ALONE
      rsi_dbg(ERR_ZONE,
              "Operating mode %d not supported without"
              "build flag 'CONFIG_RSI_BT_ALONE enabled'\n",
              oper_mode);
      return -EINVAL;
#else
      return 0;
#endif
    case 5:
    case 6:
    case 9:
    case 10:
    case 13:
    case 14:
#ifndef CONFIG_RSI_COEX_MODE
      rsi_dbg(ERR_ZONE,
              "Operating mode %d not supported without"
              " build flag 'CONFIG_RSI_COEX_MODE enabled'\n",
              oper_mode);
      return -EINVAL;
#else
      return 0;
#endif
    case 16:
    case 17:
    case 32:
    case 48:
#if !defined(CONFIG_RSI_COEX_MODE) || !defined(CONFIG_RSI_ZIGB)
      rsi_dbg(ERR_ZONE,
              "Operating mode %d not supported without"
              " build flags 'CONFIG_RSI_COEX_MODE and"
              " CONFIG_RSI_ZIGB are enabled'\n",
              oper_mode);
      return -EINVAL;
#else
#if defined(CONFIG_RSI_MULTI_MODE)
      rsi_dbg(ERR_ZONE,
              "Operating mode %d not supported with build"
              " flags 'CONFIG_RSI_MULTI_MODE'\n",
              oper_mode);
      return -EINVAL;
#endif
      return 0;
#endif
#ifdef CONFIG_RSI_MULTI_MODE
    case 0xFF:
      return 0xFF;
#endif
    default:
      return -EINVAL;
  }
}
EXPORT_SYMBOL_GPL(rsi_validate_oper_mode);

#ifdef CONFIG_RSI_MULTI_MODE
int rsi_opermode_instances(struct rsi_hw *adapter)
{
  struct rsi_common *common = adapter->priv;
  u16 *opermodes            = common->dev_oper_mode;
  int err, i, count = opermodes[0];

  if (count > MAX_INSTANCES) {
    rsi_dbg(ERR_ZONE, "%s: unable to handle %d instances\n", __func__, count);
    return -EINVAL;
  }

  for (i = 1; i <= MAX_INSTANCES; i++) {
    if (DRV_INSTANCE(i))
      continue;
    err = rsi_validate_oper_mode(opermodes[i]);
    if (err < 0)
      return -EINVAL;
    /*
		 * User ain't provide opermode for
		 * later instances, using the first.
		 */
    if (err == 0xFF)
      opermodes[i] = opermodes[1];

    DRV_INSTANCE_SET(i, opermodes[i]);
    adapter->priv->oper_mode    = opermodes[i];
    adapter->drv_instance_index = i;
    rsi_dbg(INFO_ZONE, "%s: drv instance index %d opermode %d\n", __func__, i, opermodes[i]);
    return 0;
  }
  return -EINVAL;
}
EXPORT_SYMBOL_GPL(rsi_opermode_instances);
#endif

/**
 * rsi_hal_device_init() - This function initializes the Device
 * @adapter: Pointer to the hardware structure
 *
 * Return: status: 0 on success, -1 on failure.
 */
int rsi_hal_device_init(struct rsi_hw *adapter)
{
  struct rsi_common *common = adapter->priv;
#if defined(CONFIG_RSI_COEX_MODE) || defined(CONFIG_RSI_BT_ALONE) || defined(CONFIG_RSI_ZIGB)
  switch (common->oper_mode) {
    case DEV_OPMODE_STA_BT_DUAL:
    case DEV_OPMODE_STA_BT:
    case DEV_OPMODE_STA_BT_LE:
    case DEV_OPMODE_BT_ALONE:
    case DEV_OPMODE_BT_LE_ALONE:
    case DEV_OPMODE_BT_DUAL:
      common->coex_mode = 2;
      break;
    case DEV_OPMODE_AP_BT_DUAL:
    case DEV_OPMODE_AP_BT_LE:
    case DEV_OPMODE_AP_BT:
      if (DEV_MODEL_9116)
        common->coex_mode = 2;
      else
        common->coex_mode = 4;
      break;
    case DEV_OPMODE_WIFI_ALONE:
      common->coex_mode = 1;
      break;
    case DEV_OPMODE_ZB_ALONE:
    case DEV_OPMODE_STA_ZB:
      common->coex_mode    = 3;
      common->zb_fsm_state = ZB_DEVICE_NOT_READY;
      break;
    case DEV_OPMODE_ZB_COORDINATOR:
      common->coex_mode    = 6;
      common->zb_fsm_state = ZB_DEVICE_NOT_READY;
      break;
    case DEV_OPMODE_ZB_ROUTER:
      common->coex_mode    = 7;
      common->zb_fsm_state = ZB_DEVICE_NOT_READY;
      break;
    default:
#if !defined(CONFIG_CARACALLA_BOARD) || !defined(CONFIG_RS9116_PURISM)
      common->oper_mode = 1;
      common->coex_mode = 1;
#else
      common->oper_mode = DEV_OPMODE_STA_BT_DUAL;
      common->coex_mode = 2;
#endif
  }
#else
  common->oper_mode = 1;
  common->coex_mode = 1;
#endif

  rsi_dbg(ERR_ZONE, "%s: oper_mode = %d, coex_mode = %d\n", __func__, common->oper_mode, common->coex_mode);

  switch (adapter->device_model) {
    case RSI_DEV_9113:
    case RSI_DEV_9116:
      //__9117_CODE_START
    case RSI_DEV_9117:
    case RSI_DEV_917:
      //__9117_CODE_END
      if (rsi_load_firmware(adapter)) {
        rsi_dbg(ERR_ZONE, "%s: Failed to load TA instructions\n", __func__);
        return -EINVAL;
      }
      break;
    default:
      return -EINVAL;
  }
  adapter->common_hal_fsm = COMMAN_HAL_WAIT_FOR_CARD_READY;
  common->fsm_state       = FSM_CARD_NOT_READY;

#if defined(CONFIG_RSI_BT_ALONE) || defined(CONFIG_RSI_COEX_MODE)
  adapter->priv->bt_fsm_state = BT_DEVICE_NOT_READY;
#endif

  return 0;
}
EXPORT_SYMBOL_GPL(rsi_hal_device_init);
