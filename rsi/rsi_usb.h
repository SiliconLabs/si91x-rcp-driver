/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#ifndef __RSI_USB_INTF__
#define __RSI_USB_INTF__

#include <linux/usb.h>
#include "rsi_main.h"
#include "rsi_common.h"

/* USB VENDOR ID for RSI*/
#define USB_VENDOR_ID_RSI 0x1618

/* Device ID for RS9113 */
#define USB_DEVICE_ID_RSI_9113 0x9113
/* Device ID for RS9116 */
#define USB_DEVICE_ID_RSI_9116 0x9116

#define FW_STATUS_REG 0x41050012

#define USB_VENDOR_REGISTER_READ  0x15
#define USB_VENDOR_REGISTER_WRITE 0x16
#define RSI_USB_TX_HEAD_ROOM      128
#define TIMEOUT                   0

#define MAX_TX_URBS 1
#if defined(CONFIG_RSI_BT_ALONE) || defined(CONFIG_RSI_COEX_MODE)
#define MAX_RX_URBS 2
#else
#define MAX_RX_URBS 1
#endif
#define MAX_BULK_EP         8
#define MGMT_EP             1
#define DATA_EP             2
#define ZIGB_EP             3
#define RSI_RECV_BUFFER_LEN 2000

struct rx_usb_ctrl_block {
  u8 *data;
  struct urb *rx_urb;
  struct sk_buff *rx_skb;
  u8 ep_num;
};

struct usb_receive_info {
  bool buffer_full;
  bool semi_buffer_full;
  bool mgmt_buffer_full;
  u32 mgmt_buf_full_counter;
  u32 buf_semi_full_counter;
  u8 watch_bufferfull_count;
  u32 buf_full_counter;
  u32 buf_available_counter;
};

struct rsi_91x_usbdev {
  void *priv;
  struct usb_receive_info rx_info;
  struct rsi_thread rx_thread;
  u8 endpoint;
  struct usb_device *usbdev;
  struct usb_interface *pfunction;
  struct rx_usb_ctrl_block rx_cb[MAX_RX_URBS];
  u8 *tx_buffer;
  u8 *saved_tx_buffer;
  __le16 bulkin_size[MAX_BULK_EP];
  u8 bulkin_endpoint_addr[MAX_BULK_EP];
  __le16 bulkout_size[MAX_BULK_EP];
  u8 bulkout_endpoint_addr[MAX_BULK_EP];
  u32 tx_blk_size;
  u8 write_fail;
  struct sk_buff_head rx_q[MAX_RX_URBS];
};

struct api_context {
  struct completion done;
  int status;
};

static inline int rsi_usb_event_timeout(struct rsi_hw *adapter)
{
  return EVENT_WAIT_FOREVER;
}

int rsi_usb_device_init(struct rsi_common *common);
int rsi_usb_read_register_multiple(struct rsi_hw *adapter, u32 addr, u8 *data, u16 count);
int rsi_usb_write_register_multiple(struct rsi_hw *adapter, u32 addr, u8 *data, u16 count);
void rsi_usb_rx_thread(struct rsi_common *common);

int rsi_usb_host_intf_write_pkt(struct rsi_hw *adapter, u8 *pkt, u32 len);
int rsi_usb_master_reg_read(struct rsi_hw *adapter, u32 reg, u32 *value, u16 len);
int rsi_usb_master_reg_write(struct rsi_hw *adapter, unsigned long reg, unsigned long value, u16 len);
int rsi_usb_load_data_master_write(struct rsi_hw *adapter,
                                   u32 base_address,
                                   u32 instructions_sz,
                                   u16 block_size,
                                   u8 *ta_firmware);
int rsi_usb_check_queue_status(struct rsi_hw *adapter, u8 q_num);
int rsi_rx_urb_submit(struct rsi_hw *adapter, u8 ep_num);
#endif
