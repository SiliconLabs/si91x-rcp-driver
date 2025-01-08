// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include "rsi_mgmt.h"
#include "rsi_common.h"
#include "rsi_main.h"
#include "rsi_sdio.h"
#include "rsi_hal.h"
#include "rsi_usb.h"

#define RSI_USB_REQ_IN (USB_TYPE_VENDOR | USB_DIR_IN | USB_RECIP_DEVICE)

int rsi_stats_frame(struct rsi_hw *adapter)
{

  int status = 0;
  struct rsi_mac_frame *mgmt_frame;
  struct sk_buff *skb;

  rsi_dbg(MGMT_DEBUG_ZONE, "===> Sending PER STATS REQUEST FRAME <===\n");

  /* Allocating Memory For Mgmt Pkt From Mgmt Free Pool */
  skb        = dev_alloc_skb(FRAME_DESC_SZ);
  mgmt_frame = (struct rsi_mac_frame *)skb->data;
  memset(mgmt_frame, 0, FRAME_DESC_SZ);

  /* Do not fill with the Zero length, eventhough there is no framebody  */
  /* If you fill Zero length, DMA will stuck*/
  /* FrameType*/
  mgmt_frame->desc_word[1] = cpu_to_le16(STATS_REQUEST);
  mgmt_frame->desc_word[3] = cpu_to_le16(adapter->ch_util_start_flag);
  mgmt_frame->desc_word[4] = cpu_to_le16(adapter->stats_interval);

  //__9117_CODE_START
  if (adapter->device_model >= RSI_DEV_9117)
    mgmt_frame->desc_word[5] = cpu_to_le16(adapter->false_cca_rssi_threshold | PER_RATE_STATS_ENABLE_917);
  else
    //__9117_CODE_END
    mgmt_frame->desc_word[5] = cpu_to_le16(adapter->false_cca_rssi_threshold | PER_RATE_STATS_ENABLE);

  /* Indication to PPE to request statistics */
  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  mgmt_frame->desc_word[0] |= cpu_to_le16(BIT(15));
  if (adapter->recv_stop) {
    // this is reserved field
    //set this value to Indicate firmware to stop sending stats
    mgmt_frame->desc_word[2] = cpu_to_le16(1 << 8);
  }
  skb_put(skb, FRAME_DESC_SZ);
  status = rsi_send_internal_mgmt_frame(adapter->priv, skb);
  return status;
}

int rsi_mgmt_send_rf_reset_req(struct rsi_hw *adapter, u16 *bb_prog_vals)
{
  struct rsi_mac_frame *mgmt_frame;
  int status;
  struct sk_buff *skb;

  rsi_dbg(MGMT_DEBUG_ZONE, "===> Frame request to reset RF <===\n");
  skb        = dev_alloc_skb(FRAME_DESC_SZ);
  mgmt_frame = (struct rsi_mac_frame *)skb->data;

  memset(mgmt_frame, 0, FRAME_DESC_SZ);
  if (bb_prog_vals == NULL) {
    rsi_dbg(ERR_ZONE, "RF_RESET REQUEST NULL PTR\n");
    return -1;
  }
  /* FrameType */
  mgmt_frame->desc_word[1] = cpu_to_le16(RF_RESET_FRAME);
  mgmt_frame->desc_word[3] = cpu_to_le16(bb_prog_vals[1] & 0x00ff);
  mgmt_frame->desc_word[4] = cpu_to_le16((bb_prog_vals[2]) >> 8);
  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  rsi_dbg(INFO_ZONE, " rf_reset: value is 0x%x, rf_delay: %d\n", mgmt_frame->desc_word[3], mgmt_frame->desc_word[4]);
  skb_put(skb, FRAME_DESC_SZ);
  status = rsi_send_internal_mgmt_frame(adapter->priv, skb);
  return status;
}

int rsi_send_bb_reset_req(struct rsi_hw *adapter)
{
  u16 bb_prog_vals[3];

  bb_prog_vals[1] = 0x3;
  bb_prog_vals[2] = 0xABAB;

  if (rsi_mgmt_send_rf_reset_req(adapter, bb_prog_vals) != 0) {
    return -1;
  }
  return 0;
}
int set_per_configurations(struct rsi_hw *adapter)
{
  u8 status                 = 0;
  struct rsi_common *common = adapter->priv;

  if (rsi_load_bootup_params(common) == 0) {
    rsi_dbg(INFO_ZONE, "%s: BOOTUP Parameters loaded successfully\n", __FUNCTION__);
  } else {
    rsi_dbg(ERR_ZONE, "%s: Failed to load bootup parameters\n", __FUNCTION__);
  }
  status = rsi_load_radio_caps(common);
  if (status) {
    rsi_dbg(ERR_ZONE, "%s: Failed to Send Radio parameters\n", __FUNCTION__);
    return status;
  }
  rsi_dbg(INFO_ZONE, "%s: Send Radio parameters\n", __func__);
  if (DEV_MODEL_9116) {
    status = rsi_send_w9116_features(common);
    if (status) {
      rsi_dbg(ERR_ZONE, "%s: Failed to Send wlan_9116 features parameters\n", __FUNCTION__);
      return status;
    }
  }
  status = rsi_program_bb_rf(common);
  rsi_dbg(INFO_ZONE, "%s: bb rf programmed\n", __func__);
  if (status) {
    rsi_dbg(ERR_ZONE, "%s: Failed to SEND bb_rf_init \n", __FUNCTION__);
    return status;
  }
  return status;
}
EXPORT_SYMBOL_GPL(set_per_configurations);

int rsi_send_rx_stats_cmd(struct rsi_hw *adapter, struct nlmsghdr *nlh)
{
  struct rsi_common *common = adapter->priv;
#if 1
  u8 band                           = 0;
  struct ieee80211_channel *channel = &adapter->channel;
  int status                        = 0;
#endif

#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = adapter->vifs[0];

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  bool assoc = vif && vif->bss_conf.assoc;
#else
  bool assoc = vif && vif->cfg.assoc;
#endif

#else
  struct ieee80211_vif *sta_vif = rsi_get_sta_vif(adapter);

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  bool assoc                    = sta_vif && sta_vif->bss_conf.assoc;
#else
  bool assoc = sta_vif && sta_vif->cfg.assoc;
#endif

#endif

  if (common->driver_mode == E2E_MODE) {
    if (!assoc && adapter->ps_state == PS_ENABLED)
      rsi_disable_ps(adapter);
    goto SEND_STATS_FRAME;
  }
#if 1
  /* BW Configuration is BIT(2)-BIT(4) 3 bits in rate flags; */
  adapter->per_params.per_ch_bw = (adapter->per_params.rate_flags) & (0x07);
  channel->hw_value = adapter->priv->channel = adapter->per_params.channel;
  rsi_dbg(INFO_ZONE,
          "%s: \nch_width = %d recv_channel = %d\n",
          __func__,
          adapter->per_params.per_ch_bw,
          adapter->per_params.channel);
  if (channel->hw_value > 14)
    band = NL80211_BAND_5GHZ;
  else
    band = NL80211_BAND_2GHZ;
  if (adapter->per_params.channel == 0xFF) {
    if (rsi_send_bb_reset_req(adapter) != 0) {
      rsi_dbg(ERR_ZONE, "%s: Failed to send bb reset req\n", __func__);
      return -1;
    }
  } else if (adapter->per_params.channel) {
    if (DEV_MODEL_9116) {
      if (adapter->disable_programming) {
        goto SEND_STATS_FRAME;
      } else {
        rsi_band_check(adapter->priv, band);
        set_per_configurations(adapter);
      }
    } else {
      rsi_band_check(adapter->priv, band);
    }
    common->fsm_state = FSM_SCAN_CFM;
#ifndef CONFIG_STA_PLUS_AP
    if (rsi_set_channel(common, channel)) {
#else
    if (rsi_set_channel(common, channel, sta_vif)) {
#endif
      rsi_dbg(ERR_ZONE, "%s: Failed to set the channel\n", __func__);
      return -1;
    }
    rsi_reset_event(&common->chan_set_event);
    status = rsi_wait_event(&common->chan_set_event, EVENT_WAIT_FOREVER);
    if (status < 0)
      return status;

    rsi_reset_event(&common->chan_set_event);
    common->fsm_state = FSM_MAC_INIT_DONE;
  }
#endif
SEND_STATS_FRAME:
  if ((!adapter->rx_stats_inprog) && !(rsi_stats_frame(adapter))) {
    adapter->rx_stats_inprog = 1;
    if (adapter->recv_stop) {
      adapter->recv_stop       = 0;
      adapter->rx_stats_inprog = 0;
      return 0;
    }
  }
  return 0;
}

int rsi_process_rx_stats(struct rsi_hw *adapter)
{
  struct sk_buff *skb_out = { 0 };
  struct nlmsghdr *nlh;
  int msg_size, res;
  msg_size = sizeof(adapter->sta_info);
  skb_out  = nlmsg_new(msg_size, 0);
  if (!skb_out) {
    rsi_dbg(ERR_ZONE, "%s: Failed to allocate new skb\n", __func__);
    return 0;
  }
  nlh = nlmsg_put(skb_out, adapter->wlan_nl_pid, 0, NLMSG_DONE, msg_size, 0);
  /* NETLINK_CB(skb_out).dst_group = 0; */
  memcpy(nlmsg_data(nlh), &adapter->sta_info, msg_size);
  res = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->wlan_nl_pid);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send stats to App\n", __func__);
  }
  return 0;
}

//__9117_CODE_START
int rsi_send_he_tb_mu_params(struct rsi_hw *adapter)
{
  struct he_tb_mu_params_t *he_tb_mu_params;
  int len    = (FRAME_DESC_SZ + sizeof(struct he_tb_mu_params_t));
  int status = 0;
  struct sk_buff *skb;

  skb = dev_alloc_skb(len);
  if (!skb)
    return -1;
  memset(skb->data, 0, len);
  he_tb_mu_params = (struct he_tb_mu_params_t *)skb->data;
  if (adapter->per_params.heconf.he_ppdu_type == HE_TB_PPDU) {
    he_tb_mu_params->HE_SIGA2_RESERVED = adapter->per_params.heconf.HE_SIGA2_RESERVED;
    he_tb_mu_params->RU_ALLOCATION_TB  = adapter->per_params.heconf.RU_ALLOCATION;
    he_tb_mu_params->N_HELTF_TOT       = adapter->per_params.heconf.N_HELTF_TOT;
    he_tb_mu_params->SPATIAL_REUSE_4   = adapter->per_params.heconf.spatial_reuse;
    he_tb_mu_params->SPATIAL_REUSE_3   = adapter->per_params.heconf.spatial_reuse;
    he_tb_mu_params->SPATIAL_REUSE_2   = adapter->per_params.heconf.spatial_reuse;
  } else if (adapter->per_params.heconf.he_ppdu_type == HE_MU_PPDU) {
    he_tb_mu_params->HE_LTF_SYM             = 0;
    he_tb_mu_params->SIGB_COMPRESSION_FIELD = adapter->per_params.heconf.SIGB_COMPRESSION_FIELD;
    he_tb_mu_params->SIGB_DCM               = adapter->per_params.heconf.SIGB_DCM;
    he_tb_mu_params->RU_ALLOCATION_MU       = adapter->per_params.heconf.RU_ALLOCATION;
    he_tb_mu_params->USER_CODING_TYPE       = adapter->per_params.heconf.coding_type;
    he_tb_mu_params->USER_DCM               = adapter->per_params.heconf.DCM;
    he_tb_mu_params->USER_MCS               = adapter->per_params.rate & MCS_AX_RATE_MASK;
    he_tb_mu_params->USER_TxBF              = adapter->per_params.heconf.TX_BF;
    he_tb_mu_params->USER_NSTS              = 0;
    he_tb_mu_params->USER_STA_ID            = adapter->per_params.heconf.USER_STA_ID;
    he_tb_mu_params->USER_IDX               = adapter->per_params.heconf.USER_IDX;
  }
  he_tb_mu_params->desc_word[0] =
    cpu_to_le16(RSI_WIFI_MGMT_Q << 12 | (sizeof(struct he_tb_mu_params_t) - FRAME_DESC_SZ));
  he_tb_mu_params->desc_word[1] = cpu_to_le16(HE_TB_MU_PARAM);
  skb_put(skb, len);
  rsi_hex_dump(INT_MGMT_ZONE, "HE FRAME", skb->data, skb->len);
  status = rsi_send_internal_mgmt_frame(adapter->priv, skb);
  return status;
}
//__9117_CODE_END

int prepare_per_pkt(struct rsi_hw *adapter, struct sk_buff *skb)
{
  unsigned int *frame_desc, temp_word;
  struct ieee80211_hdr *hdr = NULL;
  struct rsi_common *common = adapter->priv;
  unsigned char size_of_hdr;
  unsigned char intrnl_hdr;
  unsigned char actual_len;
  unsigned char extended_desc = 4; //For future use
                                   //__9117_CODE_START
  unsigned char _11ax_transmit     = 0;
  unsigned short bbp_info          = 0;
  unsigned short rate_field_params = 0;
  //__9117_CODE_END
  unsigned char mac_addr[6] = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5 };
  unsigned char offset      = 0;
  unsigned int rate_flags, greenfield, ch_bw, i = 0;

  size_of_hdr = FRAME_DESC_SZ + extended_desc + MIN_802_11_HDR_LEN;
  frame_desc  = (unsigned int *)&skb->data[0];
  memset((u8 *)frame_desc, 0, (FRAME_DESC_SZ + extended_desc));
  //__9117_CODE_START
  _11ax_transmit = adapter->per_params.enable_11ax;
  if (_11ax_transmit) {
    bbp_info = ((adapter->per_params.heconf.coding_type << 0) | (adapter->per_params.heconf.nominal_pe << 1)
                | (adapter->per_params.heconf.UL_DL << 3) | (adapter->per_params.heconf.he_ppdu_type << 4)
                | (adapter->per_params.heconf.beam_change << 6) | (adapter->per_params.heconf.BW << 7)
                | (adapter->per_params.heconf.STBC << 8) | (adapter->per_params.heconf.TX_BF << 9)
                | (adapter->per_params.heconf.DCM << 10) | (adapter->per_params.heconf.GI_LTF << 11)
                | (adapter->per_params.heconf.NSTS_MIDAMBLE << 13));
  }
  rate_field_params =
    ((adapter->per_params.heconf.nominal_pe << RATE_OFFSET_NOMINAL_PE)
     | (adapter->per_params.heconf.coding_type << RATE_OFFSET_CODING_TYPE)
     | (adapter->per_params.heconf.GI_LTF << RATE_OFFSET_GI_LTF) | (adapter->per_params.heconf.DCM << RATE_OFFSET_DCM));
  //__9117_CODE_END

  temp_word     = (skb->len - FRAME_DESC_SZ) & 0xffff;
  frame_desc[0] = cpu_to_le32(temp_word);
  if (skb->len >= (MIN_802_11_HDR_LEN + FRAME_DESC_SZ + extended_desc)) {
    frame_desc[1] = cpu_to_le32(MIN_802_11_HDR_LEN << 8);
  } else {
    frame_desc[1] = cpu_to_le32(adapter->per_packet.length << 8);
  }
  if ((adapter->per_packet.enable && !adapter->per_packet.insert_seq) || !adapter->per_packet.enable)
    frame_desc[1] |= (BIT(2) << 16); //insert seq_no
  if (adapter->per_params.aggr_enable) {
    temp_word = ((ENABLE_MAC_INFO) << 16); //In mac_info set bit0 and bit9 for bcast pkt
    frame_desc[3] |= cpu_to_le32(QOS_EN);
  } else
    temp_word = ((BROADCAST_IND | ENABLE_MAC_INFO) << 16);
  rate_flags = (adapter->per_params.rate_flags << 2);
  ch_bw      = (rate_flags & 0x00F0);
  if (ch_bw & BIT(4))
    ch_bw = 0;
  greenfield = (rate_flags & 0x0008);
  greenfield = (greenfield << 17);
  if (rate_flags & 0x0004) //checking short_GI
  {
    adapter->per_params.rate |= BIT(9);
  }
  if (adapter->per_params.mode) {
    rate_flags |= CONTINUOUS_MODE;
  }
  frame_desc[1] |= cpu_to_le32(temp_word | extended_desc);
  //__9117_CODE_START
  if (_11ax_transmit && (adapter->device_model >= RSI_DEV_9117)) {
    frame_desc[2] = cpu_to_le32((adapter->per_params.rate & 0xf) | BIT(10) | rate_field_params);
    frame_desc[2] |= cpu_to_le32(bbp_info << 16);
  } else {
    //__9117_CODE_END
    frame_desc[2] = cpu_to_le32((adapter->per_params.rate & 0x3ff) | (ch_bw << 12) | greenfield);
    //__9117_CODE_START
  }
  //__9117_CODE_END
  frame_desc[4] = cpu_to_le32(adapter->per_params.power) & 0xff;
  if (adapter->per_params.mode == PER_CONT_MODE) {
    frame_desc[4] |= cpu_to_le32(3 << 8);
    rsi_dbg(ERR_ZONE, "CONTINUOUS PER MODE STARTED\n");
  }
  offset += FRAME_DESC_SZ + extended_desc;

  //Form the Mac header for the per packet to be transmitted
  hdr = (struct ieee80211_hdr *)&skb->data[offset];

  if (skb->len >= size_of_hdr) {
    hdr->frame_control = 0x8;
    hdr->frame_control |= 0x01 << 8;
    hdr->duration_id = 0x00;
    hdr->duration_id |= 0x00;

    memcpy(hdr->addr1, mac_addr, ETH_ALEN);
    memcpy(hdr->addr2, common->mac_addr, ETH_ALEN);
    memcpy(hdr->addr3, common->mac_addr, ETH_ALEN);
    offset += MIN_802_11_HDR_LEN;

    //Fill the packet with dummy data
    for (i = offset; i <= (skb->len); i += 4) {
      skb->data[i]     = 0xff;
      skb->data[i + 1] = 0x00;
      skb->data[i + 2] = 0xbb;
      skb->data[i + 3] = 0x55;
    }
    if (((skb->len - offset) % 4))
      memset(&skb->data[i - 4], 0xdd, ((skb->len - offset) % 4));
  } else {
    intrnl_hdr = skb->len + extended_desc;
    actual_len = intrnl_hdr + adapter->per_packet.length;
    for (i = offset; i <= actual_len; i++) {
      skb->data[i] = 0xff;
    }
  }
  skb->data[1] |= (RSI_WIFI_DATA_Q << 4);
  if (adapter->per_params.aggr_enable)
    skb->data[14] |= 1;
  else
    skb->data[14] |= 1;

  return 0;
}

int do_continuous_send(struct rsi_hw *adapter)
{
  int status = 0;
  unsigned short length, len, seq_num;
  unsigned char extended_desc          = 4;
  unsigned short dword_align_req_bytes = 0;
  u32 usb_buf_status                   = 0;
  u8 sdio_buf_status                   = 0;
  struct sk_buff *skb                  = NULL;
  struct rsi_91x_sdiodev *dev          = (struct rsi_91x_sdiodev *)adapter->rsi_dev;

  if (adapter->per_params.aggr_enable == 1) {
    if (adapter->no_of_per_fragments != 1) {
      length = PER_AGGR_LIMIT_PER_PKT;
    } else {
      length = (adapter->per_params.pkt_length - ((adapter->per_params.aggr_count - 1) * PER_AGGR_LIMIT_PER_PKT));
    }
    adapter->no_of_per_fragments--;
  } else
    length = (adapter->per_params.pkt_length > 1536) ? 1536 : adapter->per_params.pkt_length;
  len = length + FRAME_DESC_SZ + extended_desc - 4 /* CRC */;
  if (adapter->rsi_host_intf == RSI_HOST_INTF_USB) {
    skb = dev_alloc_skb(
      len
      + (RSI_USB_TX_HEAD_ROOM + FRAME_DESC_SZ + 64 /*64 bit dword align req */ + 26 /*extended desc sz is variable*/));
    if (!skb) {
      return -1;
    }
    skb_reserve(skb, (RSI_USB_TX_HEAD_ROOM + FRAME_DESC_SZ + 26));
  } else {
    skb = dev_alloc_skb(len + NET_IP_ALIGN + FRAME_DESC_SZ
                        + 64 /*64 bit dword align req */ + 26 /*extended desc sz is variable*/);
    if (skb && NET_IP_ALIGN)
      skb_reserve(skb, NET_IP_ALIGN);
    skb_reserve(skb, (FRAME_DESC_SZ + 26 + 64));
  }
  skb->data             = skb_put(skb, len);
  dword_align_req_bytes = ((unsigned long)skb->data & 0x3f);
  if (dword_align_req_bytes > skb_headroom(skb)) {
    rsi_dbg(ERR_ZONE,
            "%s:  ERROR: Not Enough Head room: headroom = %d, len = %d\n",
            __func__,
            skb_headroom(skb),
            dword_align_req_bytes);
    dump_stack();
  }
  skb_push(skb, dword_align_req_bytes);
  skb_trim(skb, skb->len - dword_align_req_bytes);
  prepare_per_pkt(adapter, skb);

  if (adapter->rsi_host_intf == RSI_HOST_INTF_SDIO) {
    status = adapter->host_intf_ops->reg_read(adapter, RSI_DEVICE_BUFFER_STATUS_REGISTER, &sdio_buf_status);
    if (status) {
      return -1;
    }
    if (sdio_buf_status & (1 << PKT_BUFF_FULL)) {
      if (!dev->rx_info.buffer_full) {
        dev->rx_info.buf_full_counter++;
      }
      dev->rx_info.buffer_full = 1;
      return 0;
    } else {
      dev->rx_info.buffer_full = 0;
    }
  } else {
    status = adapter->host_intf_ops->master_reg_read(adapter, adapter->usb_buffer_status_reg, &usb_buf_status, 1);
    if (status < 0)
      return status;

    if (usb_buf_status & (1 << PKT_BUFF_FULL)) {
      if (!dev->rx_info.buffer_full) {
        dev->rx_info.buf_full_counter++;
      }
      dev->rx_info.buf_full_counter++;
      return 0;
    } else {
      dev->rx_info.buffer_full = 0;
    }
  }

  if (skb->len > (FRAME_DESC_SZ + MIN_802_11_HDR_LEN + extended_desc)) {
    seq_num = cpu_to_le16(*(unsigned char *)(&skb->data[FRAME_DESC_SZ + MIN_802_11_HDR_LEN + extended_desc - 2])) >> 4;
    seq_num = ((seq_num + 1) % 4096);
    skb->data[FRAME_DESC_SZ + MIN_802_11_HDR_LEN + extended_desc - 2] = (seq_num << 4) & 0xff;
    skb->data[FRAME_DESC_SZ + MIN_802_11_HDR_LEN + extended_desc - 1] = ((seq_num << 4) >> 8);
  }
  if (adapter->per_packet.enable)
    memcpy(&skb->data[FRAME_DESC_SZ + extended_desc],
           &adapter->per_packet.packet[0],
           ((adapter->per_packet.length) > adapter->per_params.pkt_length) ? (adapter->per_params.pkt_length)
                                                                           : (adapter->per_packet.length));

  skb->data[1] |= BIT(7); // Immediate wakeup bit
  rsi_hex_dump(INT_MGMT_ZONE, "DO_CONTI", skb->data, skb->len);
  status = rsi_send_pkt(adapter->priv, skb);
  if (status < 0) {
    rsi_dbg(ERR_ZONE, "Failed to write the packet\n");
    return -1;
  }
  adapter->total_per_pkt_sent++;
  return 0;
}

int send_per_frame(struct rsi_hw *adapter, unsigned char mode)
{
  struct rsi_mac_frame *mgmt_frame;
  struct sk_buff *skb;

  skb        = dev_alloc_skb(FRAME_DESC_SZ + sizeof(adapter->per_params));
  mgmt_frame = (struct rsi_mac_frame *)skb->data;
  memset(mgmt_frame, 0, FRAME_DESC_SZ + sizeof(adapter->per_params));
  memset(skb->data, 0, FRAME_DESC_SZ + sizeof(adapter->per_params));

  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12 | sizeof(adapter->per_params));
  mgmt_frame->desc_word[1] = cpu_to_le16(PER_CMD_PKT);
  mgmt_frame->desc_word[3] = cpu_to_le16(mode);
  memcpy(&skb->data[FRAME_DESC_SZ], &adapter->per_params, sizeof(adapter->per_params));
  skb_put(skb, sizeof(adapter->per_params) + FRAME_DESC_SZ);
  rsi_hex_dump(INT_MGMT_ZONE, "PER FRAME", skb->data, skb->len);
  return rsi_send_internal_mgmt_frame(adapter->priv, skb);
}

int send_per_ampdu_indication_frame(struct rsi_common *common)
{
  struct sk_buff *skb = NULL;
  struct rsi_mac_frame *mgmt_frame;
  struct rsi_hw *adapter = common->priv;

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, FRAME_DESC_SZ);
  mgmt_frame = (struct rsi_mac_frame *)skb->data;

  rsi_dbg(INT_MGMT_ZONE, "<===== SENDING AMPDU_IND FRAME =====>\n");

  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  mgmt_frame->desc_word[1] = cpu_to_le16(AMPDU_IND);

  mgmt_frame->desc_word[4] = cpu_to_le16(0);                              //tid
  mgmt_frame->desc_word[5] = cpu_to_le16(adapter->per_params.aggr_count); //baw_sise
  mgmt_frame->desc_word[7] = cpu_to_le16(0 | (START_AMPDU_AGGR << 4) | (0 << 8));

  skb_put(skb, FRAME_DESC_SZ);
  rsi_hex_dump(INT_MGMT_ZONE, "PER_AMPDU_IND FRAME", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

int rsi_start_per_burst(struct rsi_hw *adapter)
{
  if (adapter->per_params.enable) {
    if (do_continuous_send(adapter)) {
      return -1;
    }
  }
  return 0;
}

int start_per_tx(struct rsi_hw *adapter)
{
  struct rsi_common *common         = adapter->priv;
  struct ieee80211_channel *channel = &adapter->channel;
  unsigned int set_band             = 0;
  int status                        = 0;
  int no_of_fragments;
  channel->hw_value = adapter->priv->channel = adapter->per_params.channel;

  if (adapter->per_params.enable) {
    if (adapter->tx_running) {
      printk("tx already running\n");
      return -1;
    }
    if (adapter->per_params.channel <= 14 && (!adapter->per_params.enable_11j))
      set_band = NL80211_BAND_2GHZ;
    if ((adapter->per_params.channel >= 36) && (adapter->per_params.channel <= 165))
      set_band = NL80211_BAND_5GHZ;
    if (DEV_MODEL_9116) {
      if (adapter->disable_programming) {
        goto SEND_PER_CMD;
      } else {
        rsi_band_check(common, set_band);
        set_per_configurations(adapter);
      }
    }
    if (adapter->per_params.channel == 0xFF) {
      if (rsi_send_bb_reset_req(adapter)) {
        return -1;
      }
    } else if (adapter->per_params.channel) {
      common->fsm_state = FSM_SCAN_CFM;
#ifndef CONFIG_STA_PLUS_AP
      rsi_set_channel(common, channel);
#else
      rsi_set_channel(common, channel, NULL);
#endif
      rsi_reset_event(&common->chan_set_event);
      status = rsi_wait_event(&common->chan_set_event, EVENT_WAIT_FOREVER);
      if (status < 0) {
        return status;
      }
      common->fsm_state = FSM_MAC_INIT_DONE;
      rsi_reset_event(&common->chan_set_event);
    }

SEND_PER_CMD:
    if ((!adapter->per_params.pkt_length)) {
      rsi_dbg(ERR_ZONE, "Continuous TX len not supported\n");
      return -1;
    }
    //__9117_CODE_START
    if (adapter->device_model >= RSI_DEV_9117) {
      status = rsi_send_he_tb_mu_params(adapter);
      if (status)
        return -1;
    }
    //__9117_CODE_END
    if (adapter->per_params.mode == PER_CONT_MODE) {
      rsi_dbg(INFO_ZONE, "Enabling the PER continuous mode\n");
      if (do_continuous_send(adapter)) {
        rsi_dbg(ERR_ZONE, "Can't send do_continuous failed : PER\n");
        return -1;
      }
      adapter->tx_running = CONTINUOUS_RUNNING; //indicating PER_CONT_MODE
    }

    send_per_frame(adapter, adapter->per_params.mode);
    if (adapter->per_params.mode == PER_BURST_MODE) {
      if (adapter->per_params.aggr_enable == 1) {
        send_per_ampdu_indication_frame(common);
        common->fsm_state = FSM_AMPDU_IND_SENT;
      } else {
        if (common->fsm_state == FSM_MAC_INIT_DONE) {
          no_of_fragments = adapter->no_of_per_fragments = adapter->per_params.aggr_count;
          do {
            if (adapter->per_params.enable) {
              rsi_start_per_burst(adapter);
              no_of_fragments--;
            }
          } while (no_of_fragments && adapter->per_params.aggr_enable);
          adapter->tx_running = BURST_RUNNING; //indicating PER_BURST_MODE
        } else {
          rsi_dbg(ERR_ZONE, "Driver is not in MAC_INIT_DONE state\n");
          return -1;
        }
      }
    }
  } else {
    if (adapter->tx_running == BURST_RUNNING) {
      send_per_frame(adapter, PER_BURST_MODE);
    } else if (adapter->tx_running == CONTINUOUS_RUNNING) {
      send_per_frame(adapter, PER_CONT_MODE);
    } else {
      return -1;
    }
    adapter->tx_running = 0;
  }
  return 0;
}

int rsi_transmit_stats_cmd(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload)
{
  struct rsi_common *common = adapter->priv;
  unsigned short country_code;
  if (common->driver_mode == RF_EVAL_MODE_ON) {
    memcpy((&adapter->per_params), nlmsg_data(nlh) + FRAME_DESC_SZ, payload);
    if (adapter->per_params.enable) {
      if (adapter->per_params.ctry_region == 127) {
        //Code for world domain is 3
        country_code = 3;
      } else {
        country_code = adapter->per_params.ctry_region;
      }
      adapter->dfs_region = country_code;

      if (!(common->band == NL80211_BAND_5GHZ)
          && ((adapter->per_params.channel >= 36) || (adapter->per_params.enable_11j))) {
        rsi_dbg(ERR_ZONE, "Invalid Channel Number for Module type\n");
        return -1;
      }
      //__9117_CODE_START
      if (adapter->per_params.enable_11ax && !(adapter->device_model >= RSI_DEV_9117)) {
        rsi_dbg(ERR_ZONE, "Trying to do 11ax in 9116\n");
        return -1;
      }
      //__9117_CODE_END
    }
    if (start_per_tx(adapter)) {
      printk("Invalid Arguments by user\n");
      return -1;
    }
  } else {
    printk("Invalid Driver mode selected\n");
    return -1;
  }
  return 0;
}
