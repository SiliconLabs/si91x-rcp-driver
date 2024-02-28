// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include "rsi_hci.h"
#include "rsi_main.h"
#include "rsi_mgmt.h"
#include "rsi_common.h"

struct rsi_hw *adapter_g;

int rsi_response(struct rsi_hw *adapter, struct nlmsghdr *nlh, int status)
{
  struct sk_buff *skb_out  = NULL;
  struct rsi_hw *adapter_n = adapter;
  int msg_size, res;
  msg_size = sizeof(status);
  skb_out  = nlmsg_new(msg_size, 0);
  if (!skb_out) {
    rsi_dbg(ERR_ZONE, "%s: Failed to allocate skb\n", __func__);
    return -1;
  }
  nlh = nlmsg_put(skb_out, adapter_n->bt_nl_pid, 0, NLMSG_DONE, msg_size, 0);
  memcpy(nlmsg_data(nlh), (int *)&status, msg_size);
  rsi_dbg(MGMT_RX_ZONE, "<==== Sending BT/BLE status to application ====>\n");
  res = nlmsg_unicast(adapter_n->nl_sk, skb_out, adapter_n->bt_nl_pid);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send BT/BLE response STATUS to Application\n", __func__);
    return -1;
  }
  return 0;
}

static void rsi_nl_recv_msg(struct sk_buff *skb)
{
  //__9117_CODE_START
  u32 reg_addr = 0;
  u32 reg_val  = 0;
  u32 bytes    = 0;
  //__9117_CODE_END
  struct nlmsghdr *nlh;
  struct rsi_hw *adapter = adapter_g;
  int pid, payload_len, pkt_type, cmd;
  unsigned long current_time;
#if defined(CONFIG_RSI_COEX_MODE) || defined(CONFIG_RSI_BT_ALONE)
  int status;
  struct rsi_common *common = adapter->priv;
  bb_rf_params_bt_t bb_rf_params_bt;
#endif
  //__9117_CODE_START
  u16 prev_bmiss_threshold_value;
#ifdef CONFIG_TWT_SUPPORT
  wifi_reschedule_twt_config_t reschedule_twt_config;
#endif
  //__9117_CODE_END
  struct rsi_nl_desc *nl_desc = NULL;
  nlh                         = (struct nlmsghdr *)skb->data;
  nl_desc                     = (struct rsi_nl_desc *)nlmsg_data(nlh);

  pid      = nlh->nlmsg_pid; /*pid of sending process */
  pkt_type = nlh->nlmsg_type;
  cmd      = nl_desc->desc_word[0];

  if (pkt_type == WLAN_PACKET) {
    switch (cmd) {
      case PER_RECV_STOP:
        rsi_dbg(INFO_ZONE, "%s: stop PER receive case\n", __func__);
        adapter->recv_stop       = 1;
        adapter->rx_stats_inprog = 0;
        /* GCC ver 9.3.0 expects fall through comment if we want to combine two switch cases */
        /* fall-through */
      case PER_RECEIVE:
        adapter->wlan_nl_pid = pid;
        if (adapter->priv->fsm_state == FSM_MAC_INIT_DONE) {
          adapter->per_params.rate_flags = nl_desc->desc_word[1];
          adapter->per_params.channel    = nl_desc->desc_word[2];
          adapter->per_stop_bit          = nl_desc->desc_word[3];
          rsi_dbg(MGMT_DEBUG_ZONE, "%s: PER receive case\n", __func__);
          rsi_send_rx_stats_cmd(adapter, nlh);
        } else {
          rsi_dbg(INFO_ZONE, "%s: uninitialized fsm state\n", __func__);
          return;
        }
        break;
      case UPDATE_WLAN_GAIN_TABLE:
        if (adapter->priv->fsm_state == FSM_MAC_INIT_DONE) {
          payload_len = nl_desc->desc_word[1];
          rsi_update_wlan_gain_table(adapter, nlh, payload_len);
        } else {
          rsi_dbg(INFO_ZONE, "%s: uninitialized fsm state\n", __func__);
          return;
        }
        break;
      case PER_TRANSMIT:
        if (adapter->priv->fsm_state == FSM_MAC_INIT_DONE) {
          rsi_dbg(MGMT_DEBUG_ZONE, "%s: PER TRANSMIT case\n", __func__);
          payload_len = sizeof(adapter->per_params);
          rsi_transmit_stats_cmd(adapter, nlh, payload_len);
        } else {
          rsi_dbg(ERR_ZONE, "Driver not installed before issuing transmit command\n");
        }
        break;
      case PER_PACKET:
        rsi_dbg(MGMT_DEBUG_ZONE, "%s: PER PACKET case\n", __func__);
        payload_len = sizeof(adapter->per_packet);
        memcpy((&adapter->per_packet), nlmsg_data(nlh) + FRAME_DESC_SZ, payload_len);
        break;
      case MASTER_OPS:
        rsi_dbg(MGMT_DEBUG_ZONE, "performing master ops\n");
        adapter->wlan_nl_pid = pid;
        rsi_dbg(MGMT_DEBUG_ZONE, "%s: MASTER OPS case\n", __func__);
        payload_len = sizeof(adapter->master_ops);
        memcpy((&adapter->master_ops), nlmsg_data(nlh) + FRAME_DESC_SZ, payload_len);
        if ((rsi_do_master_ops(adapter, nlh->nlmsg_flags)) < 0) {
          rsi_dbg(ERR_ZONE, "Invalid Arguments by user\n");
        }
        break;

      case RSI_SET_BB_RF:
        adapter->wlan_nl_pid = pid;
        rsi_dbg(MGMT_DEBUG_ZONE, "%s: RSI_SET_BB_RF case\n", __func__);
        payload_len = sizeof(adapter->bb_rf_params);
        memcpy((&adapter->bb_rf_params), nlmsg_data(nlh) + FRAME_DESC_SZ, payload_len);
        if ((rsi_set_bb_rf_values(adapter)) < 0) {
          rsi_dbg(ERR_ZONE, "Invalid Arguments by user\n");
        }
        break;
      case RSI_EFUSE_MAP:
        adapter->wlan_nl_pid = pid;
        rsi_dbg(MGMT_DEBUG_ZONE, "%s: EFUSE MAP case\n", __func__);
        rsi_copy_efuse_content(adapter);
        break;
        //__9117_CODE_START
      case REAL_TIME_STATS:
        if (adapter->device_model >= RSI_DEV_9117) {
          adapter->wlan_nl_pid = pid;
          rsi_send_real_time_stats_request_frame(adapter, nlh);
        } else {
          rsi_dbg(ERR_ZONE, "Driver not installed before issuing command\n");
        }
        break;
#ifdef CONFIG_TWT_SUPPORT
      case TWT_CONFIG_CMD:
        adapter->wlan_nl_pid = pid;
        payload_len          = sizeof(rsi_twt_config_t);
        if (process_twt_setup_cmd(adapter, nlh, payload_len)) {
          rsi_dbg(ERR_ZONE, " Failed to send TWT SETUP/TEAR_DOWN \n");
          return;
        }
        rsi_dbg(MGMT_DEBUG_ZONE, " wake_duration = %d\n", adapter->rsi_twt_config.wake_duration);
        rsi_dbg(MGMT_DEBUG_ZONE, " wake_duration_tolerance = %d\n", adapter->rsi_twt_config.wake_duration_tol);
        rsi_dbg(MGMT_DEBUG_ZONE, " wake_int_exp = %d\n", adapter->rsi_twt_config.wake_int_exp);
        rsi_dbg(MGMT_DEBUG_ZONE, " wake_int_exp_tol = %d\n", adapter->rsi_twt_config.wake_int_exp_tol);
        rsi_dbg(MGMT_DEBUG_ZONE, " wake_int_mantissa = %d\n", adapter->rsi_twt_config.wake_int_mantissa);
        rsi_dbg(MGMT_DEBUG_ZONE, " wake_int_mantissa_tol = %d\n", adapter->rsi_twt_config.wake_int_mantissa_tol);
        rsi_dbg(MGMT_DEBUG_ZONE, " implicit_twt = %d\n", adapter->rsi_twt_config.implicit_twt);
        rsi_dbg(MGMT_DEBUG_ZONE, " un_announced_twt = %d\n", adapter->rsi_twt_config.un_announced_twt);
        rsi_dbg(MGMT_DEBUG_ZONE, " triggered_twt = %d\n", adapter->rsi_twt_config.triggered_twt);
        rsi_dbg(MGMT_DEBUG_ZONE, " negotiation_type = %d\n", adapter->rsi_twt_config.negotiation_type);
        rsi_dbg(MGMT_DEBUG_ZONE, " twt_channel = %d\n", adapter->rsi_twt_config.twt_channel);
        rsi_dbg(MGMT_DEBUG_ZONE, " twt_protection = %d\n", adapter->rsi_twt_config.twt_protection);
        rsi_dbg(MGMT_DEBUG_ZONE, " twt_flow_id = %d\n", adapter->rsi_twt_config.twt_flow_id);
        rsi_dbg(MGMT_DEBUG_ZONE, " restrict_tx_outside_tsp = %d\n", adapter->rsi_twt_config.restrict_tx_outside_tsp);
        rsi_dbg(MGMT_DEBUG_ZONE, " twt_retry_limit = %d\n", adapter->rsi_twt_config.twt_retry_limit);
        rsi_dbg(MGMT_DEBUG_ZONE, " twt_retry_interval = %d\n ", adapter->rsi_twt_config.twt_retry_interval);
        rsi_dbg(MGMT_DEBUG_ZONE, " req_type = %d\n", adapter->rsi_twt_config.req_type);
        rsi_dbg(MGMT_DEBUG_ZONE, " twt_enable = %d\n", adapter->rsi_twt_config.twt_enable);
        rsi_dbg(MGMT_DEBUG_ZONE, " twt_wake_duration_unit = %d\n", adapter->rsi_twt_config.twt_wake_duration_unit);

        if (adapter->ax_params._11ax_enabled) {
          if (adapter->rsi_twt_config.twt_enable)
            send_twt_setup_frame(adapter, 0, NULL);
          else {
            send_twt_teardown_frame(adapter,
                                    adapter->rsi_twt_config.twt_flow_id,
                                    adapter->rsi_twt_config.negotiation_type);
            send_twt_session_details_to_lmac(adapter,
                                             0,
                                             adapter->rsi_twt_config.twt_flow_id,
                                             NULL,
                                             &adapter->rsi_twt_config);
            adapter->twt_current_status = TWT_TEARDOWN_SUCC;
          }
        } else
          rsi_dbg(ERR_ZONE, "STA is not yet connected to AP, TWT Setup/Teardown will trigger after connection\n");
        break;
      case TWT_STATUS:
        adapter->wlan_nl_pid = pid;
        if (adapter->device_model >= RSI_DEV_9117)
          rsi_twt_status_resp_to_app(adapter);
        else
          rsi_dbg(ERR_ZONE, "%s: 9116 Dosen't support TWT\n", __func__);
        break;
      case RESCHEDULE_TWT:
        adapter->wlan_nl_pid = pid;
        payload_len          = sizeof(wifi_reschedule_twt_config_t);
        memset(&reschedule_twt_config, 0, sizeof(wifi_reschedule_twt_config_t));
        memcpy((&reschedule_twt_config), nlmsg_data(nlh) + FRAME_DESC_SZ, payload_len);
        if (adapter->twt_session_active && adapter->ap_twt_info_frame_support)
          send_twt_information_frame(adapter, reschedule_twt_config);
        else {
          rsi_dbg(ERR_ZONE, "%s: RESCHEDULLE_TWT Failed\n", __func__);
          adapter->twt_current_status = TWT_RESCHEDULE_FAIL;
        }

        break;
#endif
      case SET_BMISS_THRESHOLD:
        if (adapter->device_model >= RSI_DEV_9117) {
          adapter->wlan_nl_pid       = pid;
          prev_bmiss_threshold_value = adapter->priv->hw_bmiss_threshold;
          send_bmiss_threshold_to_app(adapter, prev_bmiss_threshold_value);
          adapter->priv->hw_bmiss_threshold = nl_desc->desc_word[1];
          rsi_send_vap_dynamic_update(adapter->priv);
        } else
          rsi_dbg(ERR_ZONE, "%s: 9116 Dosen't support set_bmiss_threshold", __func__);
        break;
      case KEEP_ALIVE_PERIOD:
        if (adapter->device_model >= RSI_DEV_9117) {
          adapter->priv->keep_alive_period = nl_desc->desc_word[1];
          rsi_send_vap_dynamic_update(adapter->priv);
        } else
          rsi_dbg(ERR_ZONE, "%s: 9116 Dosen't support set keep_alive_period", __func__);
        break;
        //__9117_CODE_END
      case GET_RSSI:
        adapter->wlan_nl_pid = pid;
        if (adapter->device_model < RSI_DEV_9116) {
          rsi_dbg(ERR_ZONE, "GET_RSSI is not supproted in 9113\n");
          return;
        }
        current_time = jiffies_to_msecs(jiffies);
        if ((current_time - adapter->prev_rssi_fetch_time) <= 5000) {
          rsi_dbg(ERR_ZONE, "Time b/w get_rssi cmd is less than 5sec, updating previous RSSI\n");
          send_rssi_to_app(adapter);
        } else {
          send_get_rssi_frame_to_fw(adapter);
          adapter->prev_rssi_fetch_time = current_time;
        }
        break;
      case FILTER_BCAST:
        if (adapter->device_model < RSI_DEV_9116) {
          rsi_dbg(ERR_ZONE, "FILTER_BCAST feature is not supproted in 9113\n");
          return;
        }
        payload_len = nl_desc->desc_word[1];
        if (send_filter_broadcast_frame_to_fw(adapter, nlh, payload_len) < 0) {
          rsi_dbg(ERR_ZONE, "Invalid Arguments by user\n");
        }
        break;
        //__9117_CODE_START
      case DIAGNOSTIC_REPORT:
        adapter->wlan_nl_pid = pid;
        bytes                = nl_desc->desc_word[3];
        reg_addr             = (nl_desc->desc_word[1] << 16) | nl_desc->desc_word[2];
        adapter->host_intf_ops->master_reg_read(adapter, reg_addr, &reg_val, bytes);
        send_diagnostic_report_resp_to_app(adapter, reg_val, bytes);
        break;
      case DIAG_TOOL_UPDATE:
        adapter->wlan_nl_pid = pid;
        diag_tool_update_dpd_fw(adapter, nlh, nl_desc->desc_word[1], nl_desc->desc_word[2], nl_desc->desc_word[3]);
        break;
        //__9117_CODE_END
      case WLAN_9116_FEATURE:
        memcpy(&adapter->priv->w9116_features, nlmsg_data(nlh) + FRAME_DESC_SZ, sizeof(struct rsi_wlan_9116_features));
        rsi_send_w9116_features(adapter->priv);
        break;
      default:
        rsi_dbg(ERR_ZONE, "Invalid Message Type\n");
    }
  } else if (pkt_type == BT_PACKET || pkt_type == BLE_PACKET) {
    switch (cmd) {
#if defined(CONFIG_RSI_COEX_MODE) || defined(CONFIG_RSI_BT_ALONE)
      case BT_PER:
        payload_len = sizeof(bb_rf_params_bt_t);
        memcpy((&bb_rf_params_bt), nlmsg_data(nlh) + FRAME_DESC_SZ, payload_len);
        switch (bb_rf_params_bt.value) {
          case TX_STATUS:
            adapter->bt_nl_pid = pid;
            if (adapter->tx_running) {
              rsi_dbg(ERR_ZONE, "%s:ALREADY IN TRANSMIT SO STOP THE TRASMIT FIRST \n", __func__);
              rsi_response(adapter, nlh, TX_IS_IN_PROG);
            }
            rsi_response(adapter, nlh, TX_IS_NOT_IN_PROG);
            return;
          case GET_DRV_COEX_MODE:
            adapter->bt_nl_pid = pid;
            if (common->driver_mode == RF_EVAL_MODE_ON) {
              if ((common->oper_mode & (OP_BT_CLASSIC_MODE | OP_BT_LE_MODE)) == OP_BT_CLASSIC_MODE) {
                rsi_response(adapter, nlh, RF_EVAL_CLASSIC);
                return;
              } else if ((common->oper_mode & (OP_BT_CLASSIC_MODE | OP_BT_LE_MODE)) == OP_BT_LE_MODE) {
                rsi_response(adapter, nlh, RF_EVAL_LE);
                return;
              } else if ((common->oper_mode & (OP_BT_CLASSIC_MODE | OP_BT_LE_MODE)) == OP_BT_DUAL_MODE) {
                rsi_response(adapter, nlh, RF_EVAL_DUAL_MODE);
                return;
              }
            }
            return;

          case PER_BR_EDR_TRANSMIT:
          case PER_BLE_TRANSMIT:
            adapter->tx_running = bb_rf_params_bt.Data[0];
        }
        rsi_dbg(INFO_ZONE, "BT PER STATS Request from Application\n");
        adapter->bt_nl_pid = pid;
        payload_len        = nl_desc->desc_word[1];
        rsi_hex_dump(INFO_ZONE, "BT PER pkt", skb->data, skb->len);
        status = rsi_bt_transmit_cmd(adapter, nlh, payload_len, cmd);
        if (status < 0)
          rsi_dbg(ERR_ZONE, " Failed in BT PER STATS\n");
        return;
        break;

      case BT_E2E_STAT:
        rsi_dbg(INFO_ZONE, "BT E2E STAT from App\n");
        adapter->bt_nl_pid = pid;
        payload_len        = nl_desc->desc_word[1];
        rsi_hex_dump(DATA_TX_ZONE, "TX BT pkt", skb->data, skb->len);
        status = rsi_bt_e2e_stats(adapter, nlh, payload_len, cmd);
        if (status < 0)
          rsi_dbg(ERR_ZONE, " Failed in BT E2E STAT\n");
        return;
        break;
      case BT_PER_STATS:
        rsi_dbg(INFO_ZONE, "BT PER STAT from App\n");
        adapter->read_cmd  = 1;
        adapter->bt_nl_pid = pid;
        payload_len        = nl_desc->desc_word[1];
        rsi_hex_dump(INFO_ZONE, "BT PER STATS", skb->data, skb->len);
        status = rsi_bt_per_stats(adapter, nlh, payload_len, cmd);
        if (status < 0)
          rsi_dbg(ERR_ZONE, " Failed in BT PER STAT\n");
        return;
        break;

      case BT_BLE_GAIN_TABLE:
        rsi_dbg(INFO_ZONE, "BT GAIN TABLE UPDATE from App\n");
        adapter->bt_nl_pid = pid;
        payload_len        = nl_desc->desc_word[1];
        rsi_hex_dump(DATA_TX_ZONE, "TX BT pkt", skb->data, skb->len);
        status = rsi_bt_ble_update_gain_table(adapter, nlh, payload_len, cmd);
        if (status < 0)
          rsi_dbg(ERR_ZONE, " Failed in BT/BLE GAIN TABLE UPDATE\n");
        break;
#endif
      default:
        rsi_dbg(ERR_ZONE, "Invalid Message Type\n");
        return;
    }
  }
}

int nl_sk_init(struct rsi_hw *adapter)
{
  struct sock *nl_sk;
  struct netlink_kernel_cfg cfg = {
    .input = rsi_nl_recv_msg,
  };

  rsi_dbg(INFO_ZONE, "%s\n", __func__);
  adapter_g = adapter;
  nl_sk     = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
  if (!nl_sk) {
    printk(KERN_ALERT "Error creating socket.\n");
    return -10;
  }
  adapter->nl_sk = nl_sk;
  return 0;
}

void nl_sk_exit(struct sock *nl_sk)
{
  rsi_dbg(INFO_ZONE, "%s\n", __func__);
  netlink_kernel_release(nl_sk);
}
