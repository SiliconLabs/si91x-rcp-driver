// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <linux/etherdevice.h>
#include <linux/if.h>
#include <linux/version.h>
#include "rsi_debugfs.h"
#include "rsi_mgmt.h"
#include "rsi_common.h"
#include "rsi_ps.h"
#include "rsi_gpio.h"
#include "rsi_hal.h"

/**
 * str_psstate() - return the ps state in string format.
 *
 * @state - PS state.
 *
 * return: PS state in string format.
 */
char *str_psstate(enum ps_state state)
{
  switch (state) {
    case PS_NONE:
      return "PS_NONE";
    case PS_DISABLE_REQ_SENT:
      return "PS_DISABLE_REQ_SENT";
    case PS_ENABLE_REQ_SENT:
      return "PS_ENABLE_REQ_SENT";
    case PS_ENABLED:
      return "PS_ENABLED";
    default:
      return "INVALID_STATE";
  }
}

/**
 * rsi_modify_ps_state() - Modify PS state to a new state.
 *
 * @adapter: pointer to rsi_hw structure.
 * @nstate: new PS state.
 *
 * return: new state.
 */
static inline void rsi_modify_ps_state(struct rsi_hw *adapter, enum ps_state nstate)
{
  rsi_dbg(INFO_ZONE, "PS state changed %s => %s\n", str_psstate(adapter->ps_state), str_psstate(nstate));

  adapter->ps_state = nstate;
}

/**
 * rsi_default_ps_params() - Initalization of default powersave parameters.
 *
 * @adapter: pointer to rsi_hw structure.
 *
 * return: void.
 */
void rsi_default_ps_params(struct rsi_hw *adapter)
{
  struct rsi_ps_info *ps_info = &adapter->ps_info;

  ps_info->enabled                  = true;
  ps_info->sleep_type               = ps_sleep_type;
  ps_info->tx_threshold             = 0;
  ps_info->rx_threshold             = 0;
  ps_info->tx_hysterisis            = 0;
  ps_info->rx_hysterisis            = 0;
  ps_info->monitor_interval         = 0;
  ps_info->listen_interval_duration = 0;
  ps_info->num_bcns_per_lis_int     = 0;
  ps_info->dtim_interval_duration   = 0;
  ps_info->num_dtims_per_sleep      = 1;
  if (ps_sleep_type == ULP_POWER_SAVE)
    ps_info->deep_sleep_wakeup_period = 300;
  else
    ps_info->deep_sleep_wakeup_period = 100;
  ps_info->uapsd_wakeup_period = RSI_UAPSD_WAKEUP_PERIOD;
}
EXPORT_SYMBOL_GPL(rsi_default_ps_params);

/**
 * rsi_enable_ps() - enable power save
 *
 * @adapter: Pointer to rsi_hw structure.
 *
 * return: void.
 */
void rsi_enable_ps(struct rsi_hw *adapter)
{
  if (rsi_send_ps_request(adapter, true)) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send PS request to device\n", __func__);
    return;
  }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
  adapter->user_ps_en = 1;
#endif
  rsi_modify_ps_state(adapter, PS_ENABLE_REQ_SENT);
}

/**
 * rsi_disable_ps() - disable power save
 *
 * @adapter: Pointer to rsi_hw structure.
 *
 * return: void.
 */
void rsi_disable_ps(struct rsi_hw *adapter)
{
  if (rsi_send_ps_request(adapter, false)) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send PS request to device\n", __func__);
    return;
  }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
  if ((adapter->ps_info.monitor_interval == 0 && adapter->priv->disable_ps_from_lmac == false))
    adapter->user_ps_en = 0;
#endif
  rsi_modify_ps_state(adapter, PS_DISABLE_REQ_SENT);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void traffic_timer_callback(struct rsi_hw *adapter)
{
#else
static void traffic_timer_callback(struct timer_list *t)
{
  struct rsi_hw *adapter    = from_timer(adapter, t, traffic_timer);
#endif
  struct rsi_common *common   = adapter->priv;
  struct rsi_ps_info *ps_info = &adapter->ps_info;
  bool assoc;
#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = adapter->vifs[adapter->sc_nvifs - 1];
#else
  struct ieee80211_vif *vif = rsi_get_sta_vif(common->priv);
#endif
  int duration_i;

  if (!vif) {
    return;
  }

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  assoc = vif && vif->bss_conf.assoc;
#else
  assoc                     = vif && vif->cfg.assoc;
#endif

  if (!assoc) {
    rsi_dbg(INFO_ZONE, "STATION IS NOT IN CONNECTED STATE\n");
    return;
  }

  if (assoc) {
    if ((adapter->user_ps_en) && (ps_info->monitor_interval || common->rx_data_inactive_interval)) {
      if (!adapter->wifi_paused && adapter->ps_state == PS_NONE) {
        if (common->disable_ps_from_lmac)
          common->disable_ps_from_lmac = false;
        rsi_enable_ps(adapter);
      } else if (adapter->ps_state != PS_ENABLED) {
        if (ps_info->monitor_interval)
          duration_i = ps_info->monitor_interval;
        else
          duration_i = common->rx_data_inactive_interval;
        mod_timer(&adapter->traffic_timer, msecs_to_jiffies(duration_i) + jiffies);
      }
    }
  }
}

void check_data_load(struct rsi_hw *adapter)
{
  struct rsi_ps_info *ps_info = &adapter->ps_info;
  int duration_i              = 0;

  if (adapter->ps_state == PS_ENABLED) {
    rsi_dbg(INFO_ZONE, "%s: Sending disable PS_req to TA in %d", __func__, __LINE__);

    rsi_disable_ps(adapter);
  }

  if (ps_info->monitor_interval)
    duration_i = ps_info->monitor_interval;
  else
    duration_i = adapter->priv->rx_data_inactive_interval;
  adapter->var_interval = msecs_to_jiffies(duration_i);
  adapter->old_jiffies  = jiffies;

  if (adapter->traffic_timer.function) {
    mod_timer(&adapter->traffic_timer, msecs_to_jiffies(duration_i) + jiffies);
  } else {
    init_traffic_timer(adapter, msecs_to_jiffies(duration_i));
  }
}

void check_traffic_pwr_save(struct rsi_hw *adapter)
{
  if (adapter->user_ps_en && (adapter->ps_info.monitor_interval || adapter->priv->disable_ps_from_lmac)) {
    check_data_load(adapter);
  }
}
EXPORT_SYMBOL_GPL(check_traffic_pwr_save);

void init_traffic_timer(struct rsi_hw *adapter, unsigned long timeout)
{
  if (timer_pending(&adapter->traffic_timer)) {
    rsi_dbg(ERR_ZONE, "%s : Timer Pending. This Case Should not occur\n", __func__);
    del_timer(&adapter->traffic_timer);
  }
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
  init_timer(&adapter->traffic_timer);
  adapter->traffic_timer.data     = (unsigned long)adapter;
  adapter->traffic_timer.function = (void *)&traffic_timer_callback;
#else
  timer_setup(&adapter->traffic_timer, traffic_timer_callback, 0);
#endif
  adapter->traffic_timer.expires = msecs_to_jiffies(timeout) + jiffies;

  add_timer(&adapter->traffic_timer);
}

/**
 * rsi_conf_uapsd() - configures UAPSD powersave.
 *
 * @adapter - Pointer to rsi_hw structure.
 *
 * return: void.
 */
void rsi_conf_uapsd(struct rsi_hw *adapter)
{
  if (adapter->ps_state != PS_ENABLED)
    return;

  if (rsi_send_ps_request(adapter, false)) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send PS request to device\n", __func__);
    return;
  }

  if (rsi_send_ps_request(adapter, true)) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send PS request to device\n", __func__);
  }
}

/**
 * rsi_handle_ps_confirm() - Processes powersave confirmation.
 *
 * @adapter - Pointer to rsi_hw structure.
 * @msg - Recevied buffer.
 *
 * return: 0 on success.
 */
int rsi_handle_ps_confirm(struct rsi_hw *adapter, u8 *msg)
{
  u16 cfm_type = 0;

  cfm_type = *(u16 *)&msg[PS_CONFIRM_INDEX];

  switch (cfm_type) {
    case SLEEP_REQUEST:
      if (adapter->ps_state == PS_ENABLE_REQ_SENT)
        rsi_modify_ps_state(adapter, PS_ENABLED);
      break;
    case WAKEUP_REQUEST:
      if (adapter->ps_state == PS_DISABLE_REQ_SENT)
        rsi_modify_ps_state(adapter, PS_NONE);
      break;
    default:
      rsi_dbg(ERR_ZONE, "Invalid PS confirm type %x in state %s\n", cfm_type, str_psstate(adapter->ps_state));
      return -1;
  }

  return 0;
}

int protocol_tx_access(struct rsi_common *common)
{
  u8 ii = 0;

  for (ii = 0; ii < MAX_IDS; ii++) {
    if (common->techs[ii].tx_intention)
      return 1;
  }
  return 0;
}
EXPORT_SYMBOL_GPL(protocol_tx_access);

void sleep_exit_recvd(struct rsi_common *common)
{
  u8 proto_id = 0;

  down(&common->tx_access_lock);
  common->common_hal_tx_access = true;
  up(&common->tx_access_lock);
  while (proto_id < MAX_IDS) {
    if (common->techs[proto_id].wait_for_tx_access) {
      common->techs[proto_id].wait_for_tx_access = 0;
      rsi_dbg(INFO_ZONE, "Waking up Event for proto %d\n", proto_id);
      rsi_set_event(&common->cfm_sleep_exit);
      wake_up(&common->techs[proto_id].tx_access_event);
    }
    proto_id++;
  }
}
EXPORT_SYMBOL_GPL(sleep_exit_recvd);

#if defined(CONFIG_ARCH_HAVE_CUSTOM_GPIO_H)
#define MAX_RETRY_LIMIT 30
/*
 * process_tx_gpio_hand_shake() - This function performs GPIO handshake
 * between host driver and LMAC to grant tx_access for host.
 *
 * @proto_id: ID of the protocol that calls the function.
 * @set: Used to set/reset tx_intention for host.
 *
 * Return: None.
 */
int process_tx_gpio_handshake(struct rsi_common *common, u8 proto_id, u8 set)
{
  struct wireless_techs *tech;
  int retry_count = 0;

  tech = &common->techs[proto_id];
  if (!set) {
    tech->tx_intention = false;
    /*
		 * Every thread sets their tx_intention bits indivdually before
		 * dequeuing their respective packets, once if any thread
		 * finishes its job it should clear its tx_intention bit, but
		 * it needs to check all the other threads tx_intention bits
		 * before pulling the host gpio down, because other threads
		 * still need the tx access.
		 */
    if (!protocol_tx_access(common)) {
      common->common_hal_tx_access = false;
      set_host_status(set, common);
    }
    return 0;
  }
  /* Set intention */
  set_host_status(set, common);
  tech->tx_intention = true;

retry_to_wake_lmac:
  if (!get_device_status(common)) {
    while (1) {
      msleep(1); // Device need some delay to wake up
      if (get_device_status(common)) {
        /*
				 * Check the GPIO status after some delay, just
				 * to confirm its stablity.
				 */
        msleep(2);
        if (get_device_status(common)) {
          common->common_hal_tx_access = true;
          break;
        }
        continue;
      }
      if (retry_count++ > MAX_RETRY_LIMIT) {
        rsi_dbg(ERR_ZONE, "%s: Retry limit crossed\n", __func__);
        return -ETIME;
      }
    }
  } else {
    msleep(2);
    if (get_device_status(common))
      common->common_hal_tx_access = true;
    else
      goto retry_to_wake_lmac;
  }
  return 0;
}
#endif
int set_clr_tx_intention(struct rsi_common *common, u8 proto_id, u8 set)
{
  struct wireless_techs *tech;
  struct rsi_hw *adapter;
  int status = 0;

  adapter = common->priv;
  tech    = &common->techs[proto_id];
  down(&common->tx_access_lock);
#if defined(CONFIG_ARCH_HAVE_CUSTOM_GPIO_H)
  if (common->lp_ps_handshake_mode == GPIO_HAND_SHAKE || common->ulp_ps_handshake_mode == GPIO_HAND_SHAKE) {
    status = process_tx_gpio_handshake(common, proto_id, set);
    up(&common->tx_access_lock);
    return status;
  }
#endif
  if (!set) {
    tech->tx_intention = 0;
    if (common->sleep_entry_received && !common->ulp_sleep_ack_sent) {
      if (!protocol_tx_access(common)) {
        common->common_hal_tx_access = false;
        rsi_send_ack_for_ulp_entry(common);
      }
    }
    up(&common->tx_access_lock);
  } else {
    tech->tx_intention = 1;
    if (adapter->rsi_host_intf == RSI_HOST_INTF_USB && adapter->usb_intf_in_suspend) {
      rsi_dbg(INFO_ZONE, "Changing resume timeout as pkt in pending\n");
      mod_timer(&common->suspend_timer, msecs_to_jiffies(1) + jiffies);
    }
    up(&common->tx_access_lock);
    if (!common->common_hal_tx_access) {
      tech->wait_for_tx_access = true;
      if (wait_event_timeout(tech->tx_access_event, tech->wait_for_tx_access == 0, msecs_to_jiffies(6000))) {
        if (!common->common_hal_tx_access) {
          rsi_dbg(ERR_ZONE, "%s,%d: Unable to get tx_access\n", __func__, __LINE__);
          return -EPERM;
        }
      } else {
        rsi_dbg(INFO_ZONE, "%s,%d: Wait event failed\n", __func__, __LINE__);
        return -ETIME;
      }
    }
  }
  return status;
}
EXPORT_SYMBOL_GPL(set_clr_tx_intention);
