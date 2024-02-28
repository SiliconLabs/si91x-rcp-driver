// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <linux/firmware.h>
#include "rsi_usb.h"

/**
 * rsi_usb_rx_thread() - This is a kernel thread to receive the packets from
 *			 the USB device.
 * @common: Pointer to the driver private structure.
 *
 * Return: None.
 */
void rsi_usb_rx_thread(struct rsi_common *common)
{
  struct rsi_hw *adapter     = common->priv;
  struct rsi_91x_usbdev *dev = (struct rsi_91x_usbdev *)adapter->rsi_dev;
  struct sk_buff *skb;
  int status, idx;

  do {
    status = rsi_wait_event(&dev->rx_thread.event, EVENT_WAIT_FOREVER);
    if (status < 0)
      break;
    rsi_reset_event(&dev->rx_thread.event);

    if (atomic_read(&dev->rx_thread.thread_done))
      break;

    for (idx = 0; idx < MAX_RX_URBS; idx++) {
      while (true) {
        skb = skb_dequeue(&dev->rx_q[idx]);
        if (!skb)
          break;

        status = rsi_read_pkt(common, skb->data, 0);
        if (status) {
          rsi_dbg(ERR_ZONE, "%s: Failed To read data", __func__);
        }
        dev_kfree_skb(skb);
      }
    }
  } while (1);

  rsi_dbg(INFO_ZONE, "%s: Terminated USB RX thread\n", __func__);
  atomic_inc(&dev->rx_thread.thread_done);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
  complete_and_exit(&dev->rx_thread.completion, 0);
#else
  kthread_complete_and_exit(&dev->rx_thread.completion, 0);
#endif
}
