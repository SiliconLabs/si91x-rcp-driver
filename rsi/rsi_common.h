/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#ifndef __RSI_COMMON_H__
#define __RSI_COMMON_H__

#include <linux/kthread.h>
#include "rsi_hci.h"

#define EVENT_WAIT_FOREVER 0
#define QUEUE_NOT_FULL     1
#define QUEUE_FULL         0
#define E2E_MODE           1
#define RF_EVAL_MODE_ON    2
#define SNIFFER_MODE       7
#define PER_MODE_EN        1

#define RF_EVAL_CLASSIC   0x01
#define RF_EVAL_LE        0x02
#define RF_EVAL_DUAL_MODE 0x03

static inline int rsi_init_event(struct rsi_event *pevent)
{
  atomic_set(&pevent->event_condition, 1);
  init_waitqueue_head(&pevent->event_queue);
  return 0;
}

static inline int rsi_wait_event(struct rsi_event *event, u32 timeout)
{
  int status = 0;

  if (!timeout)
    status = wait_event_interruptible(event->event_queue, (!atomic_read(&event->event_condition)));
  else
    status = wait_event_interruptible_timeout(event->event_queue, (!atomic_read(&event->event_condition)), timeout);
  return status;
}

static inline void rsi_set_event(struct rsi_event *event)
{
  atomic_set(&event->event_condition, 0);
  wake_up_interruptible(&event->event_queue);
}

static inline void rsi_reset_event(struct rsi_event *event)
{
  atomic_set(&event->event_condition, 1);
}

static inline int rsi_create_kthread(struct rsi_common *common, struct rsi_thread *thread, void *func_ptr, u8 *name)
{
  init_completion(&thread->completion);
  atomic_set(&thread->thread_done, 0);
  thread->task = kthread_run(func_ptr, common, "%s", name);
  if (IS_ERR(thread->task))
    return (int)PTR_ERR(thread->task);

  return 0;
}

static inline int rsi_kill_thread(struct rsi_thread *handle)
{
  if (atomic_read(&handle->thread_done) > 0)
    return 0;
  atomic_inc(&handle->thread_done);
  rsi_set_event(&handle->event);

  return kthread_stop(handle->task);
}

static inline struct sk_buff *rsi_get_aligned_skb(struct sk_buff *skb)
{
  u8 *skb_data = skb->data;
  int skb_len  = skb->len;

  skb_push(skb, RSI_DMA_ALIGN);
  skb_pull(skb, PTR_ALIGN(skb->data, RSI_DMA_ALIGN) - skb->data);
  memmove(skb->data, skb_data, skb_len);
  skb_trim(skb, skb_len);

  return skb;
}

enum countrycode {
  CTRY_AUSTRALIA     = 36,  /* Australia */
  CTRY_BELGIUM       = 56,  /* Belgium */
  CTRY_CANADA        = 124, /* Canada */
  CTRY_CHINA         = 156, /* People's Republic of China */
  CTRY_TAIWAN        = 158, /* Taiwan */
  CTRY_FRANCE        = 250, /* France */
  CTRY_GERMANY       = 276, /* Germany */
  CTRY_INDIA         = 356, /* India */
  CTRY_IRAN          = 364, /* Iran */
  CTRY_ITALY         = 380, /* Italy */
  CTRY_JAPAN         = 392, /* Japan */
  CTRY_MALAYSIA      = 458, /* Malaysia */
  CTRY_MEXICO        = 484, /* Mexico */
  CTRY_NEW_ZEALAND   = 554, /* New Zealand */
  CTRY_RUSSIA        = 643, /* Russia */
  CTRY_SINGAPORE     = 702, /* Singapore */
  CTRY_SOUTH_AFRICA  = 710, /* South Africa */
  CTRY_UNITED_STATES = 840, /* United States */
};

int rsi_load_radio_caps(struct rsi_common *common);
void rsi_mac80211_detach(struct rsi_hw *hw);
u16 rsi_get_connected_channel(struct rsi_hw *adapter);
struct rsi_hw *rsi_91x_init(void);
void rsi_91x_deinit(struct rsi_hw *adapter);
int rsi_read_pkt(struct rsi_common *common, u8 *rx_pkt, s32 rcv_pkt_len);
void rsi_indicate_bcnmiss(struct rsi_common *common);
void rsi_resume_conn_channel(struct rsi_hw *adapter, struct ieee80211_vif *vif);
char *dot11_pkt_type(__le16 frame_control);
struct rsi_sta *rsi_find_sta(struct rsi_common *common, u8 *mac_addr);
void rsi_init_bcn_timer(struct rsi_common *common);
void rsi_del_bcn_timer(struct rsi_common *common);
void rsi_bcn_scheduler_thread(struct rsi_common *common);
#ifdef CONFIG_SDIO_INTR_POLL
void init_sdio_intr_status_poll_thread(struct rsi_common *common);
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void rsi_roc_timeout(unsigned long data);
#else
void rsi_roc_timeout(struct timer_list *t);
#endif
struct ieee80211_vif *rsi_get_vif(struct rsi_hw *adapter, u8 *mac);
void rsi_mac80211_hw_scan_cancel(struct ieee80211_hw *hw, struct ieee80211_vif *vif);
#ifdef CONFIG_RSI_WOW
int rsi_config_wowlan(struct rsi_hw *adapter, struct cfg80211_wowlan *wowlan);
#endif
void sleep_exit_recvd(struct rsi_common *common);
int protocol_tx_access(struct rsi_common *common);
int set_clr_tx_intention(struct rsi_common *common, u8 proto_id, u8 set);
void set_host_status(int value, struct rsi_common *common);
int get_device_status(struct rsi_common *common);
int rsi_send_ack_for_ulp_entry(struct rsi_common *common);
#endif

/* Macros for support in higher kernel */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
#define BSS_ASSOC bss->assoc
#define AID       bss->aid
#else
#define BSS_ASSOC vif->cfg.assoc
#define AID       vif->cfg.aid
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 19, 0)
#define HT_CAP     sta->ht_cap
#define HE_CAP     sta->he_cap
#define SUPP_RATES sta->supp_rates
#else
#define HT_CAP     sta->deflink.ht_cap
#define HE_CAP     sta->deflink.he_cap
#define SUPP_RATES sta->deflink.supp_rates
#endif
