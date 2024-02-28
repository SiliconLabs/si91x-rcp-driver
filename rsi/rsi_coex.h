/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#ifndef __RSI_COEX_H__
#define __RSI_COEX_H__

#include "rsi_common.h"

#define RSI_COEX_TXQ_MAX_PKTS   64
#define RSI_COEX_TXQ_WATER_MARK 50
#define COMMON_CARD_READY_IND   0

#define COEX_Q             0
#define BT_Q               1
#define WLAN_Q             2
#define VIP_Q              3
#define ZIGB_Q             4
#define NUM_COEX_TX_QUEUES 5

#include "rsi_main.h"

enum rsi_proto { RSI_PROTO_WLAN = 0, RSI_PROTO_BT };

struct rsi_coex_ctrl_block {
  struct rsi_common *priv;
  struct sk_buff_head coex_tx_qs[NUM_COEX_TX_QUEUES];
  struct semaphore tx_bus_lock;
  struct rsi_thread coex_tx_thread;
};

int rsi_coex_init(struct rsi_common *common);
int rsi_coex_send_pkt(void *priv, struct sk_buff *skb, u8 proto_type);
int rsi_coex_recv_pkt(struct rsi_common *common, u8 *msg);
void rsi_coex_deinit(struct rsi_common *common);
#endif
