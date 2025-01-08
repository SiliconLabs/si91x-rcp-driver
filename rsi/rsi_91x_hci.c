// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include "rsi_hci.h"
#include "rsi_mgmt.h"
#include "rsi_coex.h"
#include "rsi_hal.h"

/**
 * rsi_hci_open() - This function is called when HCI device is
 * 						opened 
 * 
 * @hdev - pointer to HCI device
 * @return - 0 on success
 */
static int rsi_hci_open(struct hci_dev *hdev)
{
  rsi_dbg(ERR_ZONE, "RSI HCI DEVICE \"%s\" open\n", hdev->name);

  if (test_and_set_bit(HCI_RUNNING, &hdev->flags))
    rsi_dbg(ERR_ZONE, "%s: device `%s' already running\n", __func__, hdev->name);

  return 0;
}

/**
 * rsi_hci_close() - This function is called when HCI device is
 * 						closed 
 * 
 * @hdev - pointer to HCI device
 * @return - 0 on success
 */
static int rsi_hci_close(struct hci_dev *hdev)
{
  rsi_dbg(ERR_ZONE, "RSI HCI DEVICE \"%s\" closed\n", hdev->name);

  if (!test_and_clear_bit(HCI_RUNNING, &hdev->flags))
    rsi_dbg(ERR_ZONE, "%s: device `%s' not running\n", __func__, hdev->name);

  return 0;
}

/**
 * rsi_hci_flush() - This function is called when HCI device is
 * 						flushed 
 * 
 * @hdev - pointer to HCI device
 * @return - 0 on success; negative error code on failure
 */
static int rsi_hci_flush(struct hci_dev *hdev)
{
  struct rsi_hci_adapter *h_adapter;

  if (!(h_adapter = hci_get_drvdata(hdev)))
    return -EFAULT;

  rsi_dbg(ERR_ZONE, "RSI `%s' flush\n", hdev->name);

  return 0;
}

/**
 * rsi_hci_send_pkt() - This function is used send the packet received 
 * 						from HCI layer to co-ex module
 *
 * @hdev - pointer to HCI device
 * @skb - Received packet from HCI
 * @return - 0 on success; negative error code on failure
 *  
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0)
static int rsi_hci_send_pkt(struct sk_buff *skb)
#else
static int rsi_hci_send_pkt(struct hci_dev *hdev, struct sk_buff *skb)
#endif
{
  struct rsi_hci_adapter *h_adapter = NULL;
  struct sk_buff *new_skb           = NULL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0)
  struct hci_dev *hdev = (struct hci_dev *)skb->dev;
#endif
  int status = 0;
  if (!hdev) {
    status = -ENOMEM;
    goto fail;
  }
  if (skb->len <= 0) {
    rsi_dbg(ERR_ZONE, "Zero length packet\n");
    //hdev->sta.err_tx++;
    status = -EINVAL;
    goto fail;
  }

  if (!(h_adapter = hci_get_drvdata(hdev))) {
    //hdev->sta.err_tx++;
    status = -EFAULT;
    goto fail;
  }

#ifdef CONFIG_RSI_WOW
  /* Stop here when in suspend */
  if (h_adapter->priv->wow_flags & RSI_WOW_ENABLED) {
    rsi_dbg(INFO_ZONE, "In suspend: Dropping the pkt\n");
    status = -ENETDOWN;
    goto fail;
  }
#endif

#ifndef CONFIG_RSI_BT_ANDROID
  if (h_adapter->priv->bt_fsm_state != BT_DEVICE_READY) {
    rsi_dbg(ERR_ZONE, "BT Device not ready\n");
    status = -ENODEV;
    goto fail;
  }
#endif
  if (!test_bit(HCI_RUNNING, &hdev->flags)) {
    status = -EBUSY;
    goto fail;
  }

  switch (bt_cb(skb)->pkt_type) {
    case HCI_COMMAND_PKT:
      hdev->stat.cmd_tx++;
      break;

    case HCI_ACLDATA_PKT:
      hdev->stat.acl_tx++;
      break;

    case HCI_SCODATA_PKT:
      hdev->stat.sco_tx++;
      break;

    default:
      status = -EILSEQ;
      goto fail;
  }

  if (skb_headroom(skb) < REQUIRED_HEADROOM_FOR_BT_HAL) {
    /* Re-allocate one more skb with sufficent headroom 
		 * make copy of input-skb to new one */
    new_skb = skb_realloc_headroom(skb, REQUIRED_HEADROOM_FOR_BT_HAL);
    if (unlikely(!new_skb))
      return -ENOMEM;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)
    bt_cb(new_skb)->pkt_type = hci_skb_pkt_type(skb);
#else
    bt_cb(new_skb)->pkt_type = bt_cb((skb))->pkt_type;
#endif
    dev_kfree_skb(skb);
    skb = new_skb;
    if (!IS_ALIGNED((unsigned long)skb->data, RSI_DMA_ALIGN))
      skb = rsi_get_aligned_skb(skb);
  }

  rsi_hex_dump(DATA_RX_ZONE, "TX BT Pkt", skb->data, skb->len);

#ifdef CONFIG_RSI_COEX_MODE
  rsi_coex_send_pkt(h_adapter->priv, skb, BT_Q);
#else
  rsi_send_bt_pkt(h_adapter->priv, skb);
#endif
  return 0;

fail:
  return status;
}

int rsi_send_rfmode_frame(struct rsi_common *common)
{
  struct sk_buff *skb;
  struct rsi_bt_rfmode_frame *cmd_frame;
  int status;

  rsi_dbg(MGMT_TX_ZONE, "%s: Sending BT RF mode frame\n", __func__);

  skb = dev_alloc_skb(sizeof(struct rsi_bt_rfmode_frame));
  if (!skb)
    return -ENOMEM;

  memset(skb->data, 0, sizeof(struct rsi_bt_rfmode_frame));
  cmd_frame = (struct rsi_bt_rfmode_frame *)skb->data;

  /* Length is 0 */
  cmd_frame->desc.q_no           = RSI_BT_MGMT_Q;
  cmd_frame->desc.pkt_type       = RSI_BT_PKT_TYPE_RFMODE;
  cmd_frame->bt_rf_tx_power_mode = 0;
  cmd_frame->bt_rf_tx_power_mode = 0;

  skb_put(skb, sizeof(struct rsi_bt_rfmode_frame));

  status = common->priv->host_intf_ops->write_pkt(common->priv, skb->data, skb->len);
  dev_kfree_skb(skb);
  return status;
}
EXPORT_SYMBOL_GPL(rsi_send_rfmode_frame);

int rsi_deregister_bt(struct rsi_common *common)
{
  struct sk_buff *skb;
  struct rsi_bt_cmd_frame *cmd_frame;
  int status;

  rsi_dbg(MGMT_TX_ZONE, "%s: Sending BT register frame\n", __func__);

  skb = dev_alloc_skb(sizeof(struct rsi_bt_cmd_frame));
  if (!skb)
    return -ENOMEM;

  memset(skb->data, 0, sizeof(struct rsi_bt_cmd_frame));
  cmd_frame = (struct rsi_bt_cmd_frame *)skb->data;

  /* Length is 0 */
  cmd_frame->q_no     = RSI_BT_MGMT_Q;
  cmd_frame->pkt_type = RSI_BT_PKT_TYPE_DEREGISTR;

  skb_put(skb, sizeof(struct rsi_bt_cmd_frame));

  //return rsi_coex_send_pkt(common, skb, RSI_BT_Q);
  status = common->priv->host_intf_ops->write_pkt(common->priv, skb->data, skb->len);
  dev_kfree_skb(skb);
  return status;
}
EXPORT_SYMBOL_GPL(rsi_deregister_bt);

int rsi_bt_e2e_stats(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload_len, u16 cmd)
{
  struct rsi_common *common = adapter->priv;
  struct sk_buff *skb;
  struct rsi_hci_adapter *h_adapter = (struct rsi_hci_adapter *)common->hci_adapter;

  if (h_adapter->priv->bt_fsm_state != BT_DEVICE_READY) {
    rsi_dbg(ERR_ZONE, "BT Device not ready\n");
    return -ENODEV;
  }
  rsi_dbg(MGMT_TX_ZONE, "%s: <==== Sending BT_E2E_STATS frame ====>\n", __func__);

  skb = dev_alloc_skb(payload_len);
  if (!skb)
    return -ENOMEM;

  memset(skb->data, 0, payload_len);
  memcpy(skb->data, nlmsg_data(nlh) + FRAME_DESC_SZ, payload_len);
  skb_put(skb, payload_len);
  bt_cb(skb)->pkt_type = cpu_to_le16(cmd);
  rsi_hex_dump(MGMT_TX_ZONE, "BT E2E STATS ", skb->data, skb->len);

#ifdef CONFIG_RSI_COEX_MODE
  rsi_coex_send_pkt(common, skb, BT_Q);
#else
  rsi_send_bt_pkt(common, skb);
#endif
  return 0;
}

int rsi_process_rx_bt_e2e_stats(struct rsi_common *common, bt_stats_t bt_stats)
{
  struct sk_buff *skb_out = NULL;
  struct nlmsghdr *nlh    = NULL;
  int msg_size, res;
  struct rsi_hw *adapter = common->priv;
  msg_size               = sizeof(bt_stats_t);
  skb_out                = nlmsg_new(msg_size, 0);
  if (!skb_out) {
    rsi_dbg(ERR_ZONE, "%s: Failed to allocate skb\n", __func__);
    return -1;
  }
  nlh = nlmsg_put(skb_out, adapter->bt_nl_pid, 0, NLMSG_DONE, msg_size, 0);
  memcpy(nlmsg_data(nlh), &bt_stats, msg_size);
  rsi_dbg(MGMT_RX_ZONE, "<==== Sending BT E2E STATS to Application ====>\n");
  res = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->bt_nl_pid);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send BT E2E stats to Application\n", __func__);
    return -1;
  }
  return 0;
}

int rsi_bt_per_stats(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload_len, u16 cmd)
{
  struct rsi_common *common = adapter->priv;
  struct sk_buff *skb;
  int status                        = 0;
  struct rsi_hci_adapter *h_adapter = (struct rsi_hci_adapter *)common->hci_adapter;

  if (h_adapter->priv->bt_fsm_state != BT_DEVICE_READY) {
    rsi_dbg(ERR_ZONE, "BT Device not ready\n");
    return -ENODEV;
  }
  rsi_dbg(MGMT_TX_ZONE, "%s: <==== Sending BT_PER_STATS frame ====>\n", __func__);

  skb = dev_alloc_skb(payload_len);
  if (!skb)
    return -ENOMEM;

  memset(skb->data, 0, payload_len);
  memcpy(skb->data, nlmsg_data(nlh) + FRAME_DESC_SZ, payload_len);
  skb_put(skb, payload_len);
  bt_cb(skb)->pkt_type = cpu_to_le16(BT_PER);
  rsi_hex_dump(MGMT_TX_ZONE, "BT PER STATS ", skb->data, skb->len);

#ifdef CONFIG_RSI_COEX_MODE
  rsi_coex_send_pkt(common, skb, BT_Q);
#else
  rsi_send_bt_pkt(common, skb);
#endif
  if (adapter->read_cmd) {
    rsi_reset_event(&common->rsi_bt_per_event);
    status = rsi_wait_event(&common->rsi_bt_per_event, EVENT_WAIT_FOREVER);
    if (status < 0)
      return status;
  }

  return 0;
}

int rsi_send_bb_read_data_to_app(struct rsi_hw *adapter, bb_rf_params_bt_t bb_rf_params_bt)
{
  struct sk_buff *skb_out = { 0 };
  struct nlmsghdr *nlh;
  int msg_size, res;
  msg_size = sizeof(bb_rf_params_bt);
  skb_out  = nlmsg_new(msg_size, 0);
  if (!skb_out) {
    rsi_dbg(ERR_ZONE, "%s: Failed to allocate new skb\n", __func__);
    return 0;
  }

  nlh = nlmsg_put(skb_out, adapter->bt_nl_pid, 0, NLMSG_DONE, msg_size, 0);
  memcpy(nlmsg_data(nlh), &bb_rf_params_bt, msg_size);
  res = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->bt_nl_pid);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send stats to App\n", __func__);
    return -1;
  }
  return 0;
}

int rsi_process_rx_bt_per_stats(struct rsi_common *common, bt_stats_t bt_stats)
{
  struct sk_buff *skb_out = NULL;
  struct nlmsghdr *nlh    = NULL;
  int msg_size, res;
  struct rsi_hw *adapter = common->priv;
  msg_size               = sizeof(bt_stats_t);
  skb_out                = nlmsg_new(msg_size, 0);
  if (!skb_out) {
    rsi_dbg(ERR_ZONE, "%s: Failed to allocate skb\n", __func__);
    return -1;
  }
  nlh = nlmsg_put(skb_out, adapter->bt_nl_pid, 0, NLMSG_DONE, msg_size, 0);
  memcpy(nlmsg_data(nlh), &bt_stats, msg_size);
  rsi_dbg(MGMT_RX_ZONE, "<==== Sending BT PER STATS to Application ====>\n");
  res = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->bt_nl_pid);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send BT P2P stats to Application\n", __func__);
    return -1;
  }
  return 0;
}

int rsi_bt_ble_update_gain_table(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload_len, u16 cmd)
{
  struct rsi_common *common = adapter->priv;
  struct sk_buff *skb;
  struct rsi_hci_adapter *h_adapter = (struct rsi_hci_adapter *)common->hci_adapter;

  if (h_adapter->priv->bt_fsm_state != BT_DEVICE_READY) {
    rsi_dbg(ERR_ZONE, "BT Device not ready\n");
    return -ENODEV;
  }
  rsi_dbg(MGMT_TX_ZONE, "%s: <==== Sending BT_BLE_GAIN_TABLE frame ====>\n", __func__);
  if (common->acx_module == true) {
    rsi_dbg(ERR_ZONE, " ERROR : Update BLE Gain table not supported for ACx module \n");
    return -EINVAL;
  }

  skb = dev_alloc_skb(payload_len);
  if (!skb)
    return -ENOMEM;

  memset(skb->data, 0, payload_len);
  memcpy(skb->data, nlmsg_data(nlh) + FRAME_DESC_SZ, payload_len);
  skb_put(skb, payload_len);
  bt_cb(skb)->pkt_type = cpu_to_le16(cmd);
  rsi_hex_dump(MGMT_TX_ZONE, "BT/BLE GAIN TABLE UPDATE ", skb->data, skb->len);

#ifdef CONFIG_RSI_COEX_MODE
  rsi_coex_send_pkt(common, skb, BT_Q);
#else
  rsi_send_bt_pkt(common, skb);
#endif
  return 0;
}

int rsi_process_rx_bt_ble_gain_table_update(struct rsi_common *common, unsigned short status)
{
  struct sk_buff *skb_out = NULL;
  struct nlmsghdr *nlh    = NULL;
  int msg_size, res;
  struct rsi_hw *adapter = common->priv;
  msg_size               = sizeof(status);
  skb_out                = nlmsg_new(msg_size, 0);
  if (!skb_out) {
    rsi_dbg(ERR_ZONE, "%s: Failed to allocate skb\n", __func__);
    return -1;
  }
  nlh = nlmsg_put(skb_out, adapter->bt_nl_pid, 0, NLMSG_DONE, msg_size, 0);
  memcpy(nlmsg_data(nlh), (unsigned char *)&status, msg_size);
  rsi_dbg(MGMT_RX_ZONE, "<==== Sending BT/BLE GAIN TABLE UPDATE STATUS to Application ====>\n");
  res = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->bt_nl_pid);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send BT/LE GAIN TABLE UPDATE STATUS to Application\n", __func__);
    return -1;
  }
  return 0;
}

int rsi_hci_recv_pkt(struct rsi_common *common, u8 *pkt)
{
  struct rsi_hci_adapter *h_adapter = (struct rsi_hci_adapter *)common->hci_adapter;
  struct sk_buff *skb               = NULL;
  struct hci_dev *hdev              = NULL;
  int pkt_len                       = rsi_get_length(pkt, 0);
  u8 queue_no                       = rsi_get_queueno(pkt, 0);
  bt_stats_t bt_stats;
  bb_rf_params_bt_t bb_rf_params_bt;
  char status;

  if ((common->bt_fsm_state == BT_DEVICE_NOT_READY) && (pkt[14] == BT_CARD_READY_IND)) {
    rsi_dbg(INIT_ZONE, "%s: ===> BT Card Ready Received <===\n", __func__);

    if (common->suspend_in_prog) {
      rsi_dbg(INFO_ZONE, "Suspend is in prog; Do not process\n");
      return 0;
    }
    if (rsi_send_bt_reg_params(common)) {
      rsi_dbg(ERR_ZONE, "%s: Failed to write BT reg params\n", __func__);
    }
    rsi_dbg(INFO_ZONE, "Attaching HCI module\n");

    if (rsi_hci_attach(common)) {
      rsi_dbg(ERR_ZONE, "Failed to attach HCI module\n");
      return 0;
    }

    return 0;
  }

  if (common->bt_fsm_state != BT_DEVICE_READY) {
    rsi_dbg(INFO_ZONE, "BT Device not ready\n");
    return 0;
  }
  if (queue_no == RSI_BT_MGMT_Q) {
    u8 msg_type = pkt[14] & 0xFF;

    switch (msg_type) {
      case RESULT_CONFIRM:
        rsi_dbg(MGMT_RX_ZONE, "BT Result Confirm\n");
        return 0;
      case BT_BER:
        rsi_dbg(MGMT_RX_ZONE, "BT Ber\n");
        return 0;
      case BT_CW:
        rsi_dbg(MGMT_RX_ZONE, "BT CW\n");
        return 0;
      case BT_E2E_STAT:
        rsi_dbg(MGMT_RX_ZONE, " Received BT E2E STATS confirm from LMAC\n");
        memcpy(&bt_stats, pkt + FRAME_DESC_SZ, sizeof(bt_stats_t));
        rsi_hex_dump(MGMT_RX_ZONE, "BT E2E STATS From LMAC", (char *)&bt_stats, sizeof(bt_stats_t));
        rsi_process_rx_bt_e2e_stats(common, bt_stats);
        return 0;
      case BT_PER:
        if (common->priv->bb_read_cmd) {
          rsi_dbg(MGMT_RX_ZONE, " Received BB_READ confirm from LMAC\n");
          memcpy(&bb_rf_params_bt.Data[0], pkt + FRAME_DESC_SZ, pkt_len);
          bb_rf_params_bt.no_of_values = pkt_len / 2;
          rsi_send_bb_read_data_to_app(common->priv, bb_rf_params_bt);
        } else {
          rsi_dbg(MGMT_RX_ZONE, " Received BT PER STATS confirm from LMAC\n");
          memcpy(&bt_stats, pkt + FRAME_DESC_SZ, sizeof(bt_stats_t));
          rsi_hex_dump(MGMT_RX_ZONE, "BT PER STATS From LMAC", (char *)&bt_stats, sizeof(bt_stats_t));
          rsi_set_event(&common->rsi_bt_per_event);
          rsi_process_rx_bt_per_stats(common, bt_stats);
        }
        return 0;
      case BT_BLE_GAIN_TABLE:
        rsi_dbg(MGMT_RX_ZONE, " Received BT/BLE GAIN TABLE UPDATE confirm from LMAC\n");
        memcpy((char *)&status, pkt + FRAME_DESC_SZ, sizeof(status));
        rsi_hex_dump(MGMT_RX_ZONE, "BT/BLE GAIN TABLE UPDATE confirm From LMAC", (char *)&status, sizeof(status));
        rsi_process_rx_bt_ble_gain_table_update(common, status);
        return 0;
      default:
        break;
    }
  }
  skb = dev_alloc_skb(pkt_len);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed to alloc skb\n", __func__);
    return -ENOMEM;
  }
  hdev = h_adapter->hdev;
  memcpy(skb->data, pkt + FRAME_DESC_SZ, pkt_len);
  skb_put(skb, pkt_len);
  h_adapter->hdev->stat.byte_rx += skb->len;

  skb->dev             = (void *)hdev;
  bt_cb(skb)->pkt_type = pkt[14];

#ifdef CONFIG_RSI_BT_ANDROID
  rsi_send_to_stack(common, skb);
  dev_kfree_skb(skb);
  return 0;
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0)
  return hci_recv_frame(skb);
#else
  return hci_recv_frame(hdev, skb);
#endif
}
EXPORT_SYMBOL_GPL(rsi_hci_recv_pkt);

int rsi_bt_transmit_cmd(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload_len, u16 cmd)
{
  struct rsi_common *common = adapter->priv;
  struct sk_buff *skb;
  struct rsi_hci_adapter *h_adapter = (struct rsi_hci_adapter *)common->hci_adapter;
  if (h_adapter->priv->bt_fsm_state != BT_DEVICE_READY) {
    rsi_dbg(ERR_ZONE, "BT Device not ready\n");
    return -ENODEV;
  }
  rsi_dbg(MGMT_TX_ZONE, "%s: <==== Sending BT PER TX cmd ====>\n", __func__);

  skb = dev_alloc_skb(payload_len);
  if (!skb)
    return -ENOMEM;

  memset(skb->data, 0, payload_len);
  memcpy(skb->data, nlmsg_data(nlh) + FRAME_DESC_SZ, payload_len);
  skb_put(skb, payload_len);
  bt_cb(skb)->pkt_type = cpu_to_le16(cmd);
  rsi_hex_dump(MGMT_TX_ZONE, "BT PER", skb->data, skb->len);

#ifdef CONFIG_RSI_COEX_MODE
  rsi_coex_send_pkt(common, skb, BT_Q);
#else
  rsi_send_bt_pkt(common, skb);
#endif
  return 0;
}

/**
 * rsi_hci_attach () - This function initializes HCI interface
 *				      
 * @common: Pointer to the driver private structure.
 *
 * Return: 0 on success, negative error code on failure
 */
int rsi_hci_attach(struct rsi_common *common)
{
  struct rsi_hci_adapter *h_adapter = NULL;
  struct hci_dev *hdev;
  int status = 0;

  /* Allocate HCI adapter */
  /* TODO: Check GFP_ATOMIC */
  h_adapter = kzalloc(sizeof(*h_adapter), GFP_KERNEL);
  if (!h_adapter) {
    rsi_dbg(ERR_ZONE, "Failed to alloc HCI adapter\n");
    return -ENOMEM;
  }
  h_adapter->priv = common;

  /* Create HCI Interface */
  hdev = hci_alloc_dev();
  if (!hdev) {
    rsi_dbg(ERR_ZONE, "Failed to alloc HCI device\n");
    goto err;
  }
  h_adapter->hdev = hdev;

  if (common->priv->rsi_host_intf == RSI_HOST_INTF_SDIO)
    hdev->bus = HCI_SDIO;
  else
    hdev->bus = HCI_USB;

  hci_set_drvdata(hdev, h_adapter);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0) && LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
  hdev->dev_type = HCI_PRIMARY;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
  hdev->dev_type = HCI_BREDR;
#endif
  SET_HCIDEV_DEV(hdev, common->priv->device);

  hdev->open  = rsi_hci_open;
  hdev->close = rsi_hci_close;
  hdev->flush = rsi_hci_flush;
  hdev->send  = rsi_hci_send_pkt;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 3, 8)
  hdev->destruct = rsi_hci_destruct;
  hdev->owner    = THIS_MODULE;
#endif

  /* Initialize TX queue */
  skb_queue_head_init(&h_adapter->hci_tx_queue);
  common->hci_adapter = (void *)h_adapter;

  if (common->driver_mode == E2E_MODE) {
    status = hci_register_dev(hdev);
    if (status < 0) {
      rsi_dbg(ERR_ZONE, "%s: HCI registration failed with errcode %d\n", __func__, status);
      goto err;
    }
  }
#ifdef CONFIG_RSI_BT_ANDROID
  rsi_init_event(&common->rsi_btchr_read_wait);
  status = rsi_bdroid_init(common);
  if (status < 0) {
    rsi_dbg(ERR_ZONE, "%s: HCI registration failed with errcode %d\n", __func__, status);
    goto err;
  }
#endif
#ifndef CONFIG_RSI_BT_ANDROID
  if (common->driver_mode == E2E_MODE)
    rsi_dbg(INIT_ZONE, "HCI Interface Created with name \'%s\'\n", hdev->name);
#endif
  common->bt_fsm_state = BT_DEVICE_READY;
  rsi_dbg(ERR_ZONE, " HCI module init done...\n");

  return 0;

err:
  if (hdev) {
    hci_unregister_dev(hdev);
    hci_free_dev(hdev);
    h_adapter->hdev = NULL;
  }
  kfree(h_adapter);

  return -EINVAL;
}
EXPORT_SYMBOL_GPL(rsi_hci_attach);

/**
 * rsi_hci_attach () - This function initializes HCI interface
 *				      
 * @common: Pointer to the driver private structure.
 *
 * Return: 0 on success, negative error code on failure
 */
void rsi_hci_detach(struct rsi_common *common)
{
  struct rsi_hci_adapter *h_adapter = (struct rsi_hci_adapter *)common->hci_adapter;
  struct hci_dev *hdev;

  rsi_dbg(INFO_ZONE, "Detaching HCI...\n");

  if (!h_adapter)
    return;

#ifdef CONFIG_RSI_BT_ANDROID
  rsi_bdroid_deinit(common);
#endif
  hdev = h_adapter->hdev;
  if (hdev) {
    //hci_dev_hold(hdev);
    if (common->driver_mode == E2E_MODE)
      hci_unregister_dev(hdev);
    //hci_dev_put(hdev);
    hci_free_dev(hdev);
    h_adapter->hdev = NULL;
  }
  kfree(h_adapter);
  common->bt_fsm_state = BT_DEVICE_NOT_READY;

  return;
}
EXPORT_SYMBOL_GPL(rsi_hci_detach);
