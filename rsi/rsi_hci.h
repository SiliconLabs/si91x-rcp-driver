/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#ifndef __RSI_HCI_H__
#define __RSI_HCI_H__

#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>
#include <net/genetlink.h>
#include <linux/version.h>

#include "rsi_main.h"

#define BB_READ           0x0
#define BB_WRITE          0x1
#define RF_READ           0x2
#define RF_WRITE          0x3
#define BT_PER_TRANSMIT   0x4
#define BT_RECEIVE        0x5
#define BUFFER_READ       0x6
#define BUFFER_WRITE      0x7
#define BT_PER_STATS      0x8
#define ANT_SEL           0x9
#define BT_BER_PKT_CNT    0xA
#define BT_BER_RECEIVE    0xB
#define BT_BER_MODE       0xC
#define BT_CW_MODE        0xD
#define TX_STATUS         0xE
#define GET_DRV_COEX_MODE 0xF

#define BT_E2E_MODE_ON  1
#define RF_EVAL_MODE_ON 2

/* RX frame types */
#define RESULT_CONFIRM 0x80
#define BT_PER         0x10
#define BT_BER         0x11
#define BT_CW          0x12

#define PER_BR_EDR_TRANSMIT 0x15
#define PER_BR_EDR_RECEIVE  0x16

#define TX_IS_IN_PROG     -1
#define TX_IS_NOT_IN_PROG 1

#define RSI_DMA_ALIGN                8
#define REQUIRED_HEADROOM_FOR_BT_HAL (16 + RSI_DMA_ALIGN)

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 6, 11)
#define get_portid(_info) (_info)->snd_pid
#else
#define get_portid(_info) (_info)->snd_portid
#endif

enum {
  RSI_USER_A_UNSPEC,
  RSI_USER_A_MSG,
  __RSI_USER_A_MAX,
};

enum {
  RSI_USER_C_UNSPEC,
  RSI_USER_C_CMD,
  __RSI_USER_C_MAX,
};

enum { BT_DEVICE_NOT_READY = 0, BT_DEVICE_READY };

struct rsi_hci_adapter {
  struct rsi_common *priv;
  struct hci_dev *hdev;
  struct sk_buff_head hci_tx_queue;
};
#ifdef CONFIG_RSI_BT_ANDROID
struct rsi_hci_data {
  struct hci_dev *hdev;

  wait_queue_head_t read_wait;
  struct sk_buff_head readq;

  struct delayed_work open_timeout;
  struct rsi_common *priv;

  dev_t bt_devid;              /* bt char device number */
  struct cdev bt_char_dev;     /* bt character device structure */
  struct class *bt_char_class; /* device class for usb char driver */
};
#endif

/* TX BT command packet types */
#define RSI_BT_PKT_TYPE_DEREGISTR 0x11
#define RSI_BT_PKT_TYPE_RFMODE    0x55

struct rsi_bt_cmd_frame {
#ifdef __LITTLE_ENDIAN
  u16 len : 12;
  u16 q_no : 4;
#else
  u16 reserved1 : 4;
  u16 q_no : 12;
#endif
  __le16 reserved2[6];
  u8 pkt_type;
  u8 reserved3;
};

struct rsi_bt_rfmode_frame {
  struct rsi_bt_cmd_frame desc;
#ifdef __LITTLE_ENDIAN
  u8 bt_rf_tx_power_mode : 4;
  u8 bt_rf_rx_power_mode : 4;
#else
  u8 bt_rf_rx_power_mode : 4;
  u8 bt_rf_tx_power_mode : 4;
#endif
  u8 reserved;
};

typedef struct bt_stats_s {
  unsigned short crc_pass;
  unsigned short crc_fail;
  unsigned short tx_aborts;
  unsigned short cca_stk;
  unsigned short cca_idle;
  unsigned short fls_rx_start;
  unsigned short rx_phy_abort;
  unsigned short tx_dones;
  unsigned short fifo_occupied;
  signed short int rssi;
  unsigned short id_pkts_rcvd;
} bt_stats_t;

int rsi_hci_attach(struct rsi_common *common);
void rsi_hci_detach(struct rsi_common *common);
int rsi_hci_recv_pkt(struct rsi_common *common, u8 *pkt);

#ifdef CONFIG_RSI_BT_ANDROID
int rsi_bdroid_init(struct rsi_common *common);
void rsi_bdroid_deinit(struct rsi_common *common);
void rsi_send_to_stack(struct rsi_common *common, struct sk_buff *skb);
#endif

#endif
