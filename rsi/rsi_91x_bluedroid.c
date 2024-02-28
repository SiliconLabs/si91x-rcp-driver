// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <linux/cdev.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/signal.h>
#include <linux/ioctl.h>
#include <linux/skbuff.h>

#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>
#include "rsi_hci.h"
#include "rsi_mgmt.h"
#include "rsi_coex.h"
#include "rsi_hal.h"

#define VERSION "2.2"
struct rsi_hci_adapter *gh_adapter;
#define BT_CHAR_DEVICE_NAME "rsi"

struct rsi_data {
  struct hci_dev *hdev;

  wait_queue_head_t read_wait;
  struct sk_buff_head readq;

  struct mutex open_mutex;
  struct delayed_work open_timeout;
};
static int hci_device_close(struct rsi_hci_adapter *h_adapter)
{
  struct hci_dev *hdev = h_adapter->hdev;

  if (!hdev) {
    rsi_dbg(ERR_ZONE, "%s: failed to get hci dev[Null]", __func__);
    return -ENODEV;
  }

  hdev->flush(hdev);
  hdev->close(hdev);
  /* Clear flags */
  hdev->flags = 0;

  return 0;
}

static int hci_device_open(struct rsi_hci_adapter *h_adapter)
{
  struct hci_dev *hdev = NULL;
  int ret              = 0;

  hdev = h_adapter->hdev;
  if (!hdev) {
    rsi_dbg(ERR_ZONE, "%s: Failed to get hci dev", __func__);
    return -ENODEV;
  }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
  if (test_bit(HCI_UNREGISTER, hdev->dev_flags)) {
#else
  if (test_bit(HCI_UNREGISTER, &hdev->dev_flags)) {
#endif
    ret = -ENODEV;
    goto done;
  }

  if (test_bit(HCI_UP, &hdev->flags)) {
    ret = -EALREADY;
    goto done;
  }

  if (hdev->open(hdev)) {
    ret = -EIO;
    goto done;
  }

  set_bit(HCI_UP, &hdev->flags);
done:
  return ret;
}

static int rsi_btchr_open(struct inode *inode_p, struct file *file_p)
{
  struct rsi_hci_adapter *h_adapter = gh_adapter;
  struct hci_dev *hdev              = NULL;

  if (!h_adapter) {
    rsi_dbg(ERR_ZONE, "%s: out of memory -1", __func__);
    return -ENOMEM;
  }
  hdev = h_adapter->hdev;
  if (!hdev) {
    rsi_dbg(ERR_ZONE, "%s: Failed to get hci dev[NULL]", __func__);
    return -1;
  }

  atomic_inc(&hdev->promisc);
  file_p->private_data = (void *)h_adapter;

  hci_device_open(h_adapter);

  return nonseekable_open(inode_p, file_p);
}

static int rsi_btchr_close(struct inode *inode_p, struct file *file_p)
{
  struct hci_dev *hdev = NULL;
  struct rsi_hci_adapter *h_adapter;

  rsi_dbg(ERR_ZONE, "%s: BT usb char device is closing\n", __func__);

  h_adapter = (struct rsi_hci_adapter *)file_p->private_data;
  if (!h_adapter) {
    rsi_dbg(ERR_ZONE, "%s: Out of memory -2 ", __func__);
    return -ENOMEM;
  }
  hdev = h_adapter->hdev;
  if (hdev) {
    atomic_set(&hdev->promisc, 0);
    hci_device_close(h_adapter);
  }
  file_p->private_data = NULL;

  return 0;
}

static void rsi_enqueue(struct rsi_common *common, struct sk_buff *skb)
{
  if (common->rsi_skb_queue_front == (common->rsi_skb_queue_rear + 1) % QUEUE_SIZE) {
    /*
		 * If queue is full, current solution is to drop
		 * the following entries.
		 */
    dev_kfree_skb(skb);
  } else {
    if (common->rsi_skb_queue_front == -1) {
      common->rsi_skb_queue_front = 0;
      common->rsi_skb_queue_rear  = 0;
    } else {
      common->rsi_skb_queue_rear++;
      common->rsi_skb_queue_rear %= QUEUE_SIZE;
    }

    common->rsi_skb_queue[common->rsi_skb_queue_rear] = skb;
  }
}

void rsi_send_to_stack(struct rsi_common *common, struct sk_buff *skb)
{
  struct rsi_hci_adapter *h_adapter = NULL;
  struct hci_dev *hdev              = NULL;
  struct sk_buff *rtk_skb_copy      = NULL;
  h_adapter                         = (struct rsi_hci_adapter *)common->hci_adapter;

  if (!h_adapter) {
    rsi_dbg(ERR_ZONE, "h_adapter MEM check failed\n");
    return;
  }
  hdev = h_adapter->hdev;
  if (!hdev) {
    rsi_dbg(ERR_ZONE, "Frame for unknown HCI device\n");
    return;
  }

  if (!test_bit(HCI_RUNNING, &hdev->flags)) {
    rsi_dbg(ERR_ZONE, "HCI not running\n");
    return;
  }
  rtk_skb_copy = pskb_copy(skb, GFP_ATOMIC);
  if (!rtk_skb_copy) {
    rsi_dbg(ERR_ZONE, " Copy skb error\n");
    return;
  }

  memcpy(skb_push(rtk_skb_copy, 1), &bt_cb(skb)->pkt_type, 1);
  rsi_enqueue(common, rtk_skb_copy);

  /* Make sure bt char device existing before wakeup read queue */
  if (hdev)
    rsi_set_event(&common->rsi_btchr_read_wait);

  return;
}

static struct sk_buff *rsi_dequeue(struct rsi_common *common, unsigned int deq_len)
{

  struct sk_buff *skb;
  struct sk_buff *skb_copy;

  if (common->rsi_skb_queue_front == -1) {
    rsi_dbg(ERR_ZONE, "%s: Queue is empty\n", __func__);
    return NULL;
  }

  skb = common->rsi_skb_queue[common->rsi_skb_queue_front];
  if (deq_len >= skb->len) {
    if (common->rsi_skb_queue_front == common->rsi_skb_queue_rear) {
      common->rsi_skb_queue_front = -1;
      common->rsi_skb_queue_rear  = -1;
    } else {
      common->rsi_skb_queue_front++;
      common->rsi_skb_queue_front %= QUEUE_SIZE;
    }
    /*  Return skb addr to be dequeued, and the caller
		    should free the skb eventually.
		*/
    return skb;
  } else {
    skb_copy = pskb_copy(skb, GFP_ATOMIC);
    skb_pull(skb, deq_len);
    dev_kfree_skb(skb);
    return skb_copy;
  }
  return NULL;
}

int is_queue_empty(struct rsi_common *common)
{
  rsi_dbg(INFO_ZONE, "Queue front value is: %d\n", common->rsi_skb_queue_front);
  return (common->rsi_skb_queue_front == -1) ? 1 : 0;
}

static ssize_t rsi_btchr_read(struct file *file_p, char __user *buf_p, size_t count, loff_t *pos_p)
{
  struct rsi_hci_adapter *h_adapter = NULL;
  struct rsi_common *common         = NULL;
  struct hci_dev *hdev;
  struct sk_buff *skb;
  int ret = 0;
  int len;
  h_adapter = (struct rsi_hci_adapter *)file_p->private_data;

  if (!h_adapter)
    return -ENOMEM;

  common = h_adapter->priv;
  if (!common)
    return -ENOMEM;

  rsi_dbg(INFO_ZONE, "*************RSI BT READ******************\n");

  while (count) {
    hdev = h_adapter->hdev;
    if (!hdev) {
      rsi_dbg(ERR_ZONE, "%s: Failed to get hci dev\n", __func__);
      ret = -EINVAL;
      break;
    }

    if (is_queue_empty(common)) {
      rsi_dbg(ERR_ZONE, " Queue is empty\n");
      rsi_reset_event(&common->chan_set_event);
      ret = rsi_wait_event(&common->rsi_btchr_read_wait, !is_queue_empty(common));
      if (ret < 0) {
        rsi_dbg(ERR_ZONE, "%s: wait event is signaled ret value is %d\n", __func__, ret);
        ret = -EINVAL;
        break;
      }
    }

    skb = rsi_dequeue(common, count);
    if (skb) {
      rsi_hex_dump(DATA_RX_ZONE, "RX BT Pkt", skb->data, skb->len);
      len = min_t(unsigned int, skb->len, count);
      if (copy_to_user(buf_p, skb->data, len)) {
        rsi_dbg(ERR_ZONE, "%s: Failed to put data to user space", __func__);
        dev_kfree_skb(skb);
        ret = -EINVAL;
        break;
      }
      return len;
    } else {
      rsi_dbg(ERR_ZONE, "%s: Out of memory", __func__);
      return -ENOMEM;
    }
  }

  return ret;
}

static ssize_t rsi_btchr_write(struct kiocb *iocb, struct iov_iter *from)
{
  struct file *file                 = iocb->ki_filp;
  struct rsi_hci_adapter *h_adapter = NULL;
  struct hci_dev *hdev              = NULL;
  size_t len                        = iov_iter_count(from);
  struct sk_buff *skb;
  h_adapter = (struct rsi_hci_adapter *)file->private_data;
  hdev      = h_adapter->hdev;

  rsi_dbg(INFO_ZONE, "*********RSI BT WRITE**************\n");

  if (!hdev) {
    rsi_dbg(ERR_ZONE, "%s: Failed to get hci dev\n", __func__);
    return len;
  }
  if (len < 2 || len > HCI_MAX_FRAME_SIZE)
    return -EINVAL;

  skb = bt_skb_alloc(len, GFP_KERNEL);
  if (!skb)
    return -ENOMEM;

  skb_reserve(skb, -1);
  if (!copy_from_iter(skb_put(skb, len), len, from)) {
    dev_kfree_skb(skb);
    return -EFAULT;
  }

  skb->dev             = (void *)hdev;
  bt_cb(skb)->pkt_type = *((__u8 *)skb->data);
  skb_pull(skb, 1);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0)
  hdev->send(hdev, skb);
#else
  hdev->send(skb);
#endif

  return len;
}

static unsigned int rsi_btchr_poll(struct file *file_p, poll_table *wait)
{
  struct rsi_hci_adapter *h_adapter = NULL;
  struct rsi_common *common         = NULL;
  struct hci_dev *hdev              = NULL;
  h_adapter                         = (struct rsi_hci_adapter *)file_p->private_data;

  if (!h_adapter)
    return -ENOMEM;

  if (!h_adapter->priv)
    return -ENOMEM;

  common = h_adapter->priv;
  hdev   = h_adapter->hdev;
  if (!hdev) {
    rsi_dbg(ERR_ZONE, "%s: Failed to get hci dev\n", __func__);
    return POLLOUT | POLLWRNORM;
  }
  poll_wait(file_p, &common->rsi_btchr_read_wait.event_queue, wait);

  if (!is_queue_empty(common))
    return POLLIN | POLLRDNORM;
  else
    return POLLOUT | POLLWRNORM;

  return 0;
}

static struct file_operations rsi_btdev_ops = { .open       = rsi_btchr_open,
                                                .release    = rsi_btchr_close,
                                                .read       = rsi_btchr_read,
                                                .write_iter = rsi_btchr_write,
                                                .poll       = rsi_btchr_poll };

/**
 * registers with the bluedroid interface
 *
 * @return  - 0 on success
 */
int rsi_bdroid_init(struct rsi_common *common)
{
  int res = 0;
  struct device *dev;

  gh_adapter = (struct rsi_hci_adapter *)common->hci_adapter;

  /*
	 * btchr mutex is used to sync between
	 * 1) downloading patch and opening bt char driver
	 * 2) the file operations of bt char driver
	 */
  common->bt_char_class = class_create(THIS_MODULE, BT_CHAR_DEVICE_NAME);
  if (IS_ERR(common->bt_char_class))
    return PTR_ERR(common->bt_char_class);

  res = alloc_chrdev_region(&common->bt_devid, 0, 1, BT_CHAR_DEVICE_NAME);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "Failed to allocate bt char device\n");
    goto err_alloc;
  }

  dev = device_create(common->bt_char_class, NULL, common->bt_devid, NULL, BT_CHAR_DEVICE_NAME);
  if (IS_ERR(dev)) {
    rsi_dbg(ERR_ZONE, "Failed to create bt char device\n");
    res = PTR_ERR(dev);
    goto err_create;
  }
  common->rsi_skb_queue_front = -1;
  common->rsi_skb_queue_rear  = -1;
  cdev_init(&common->bt_char_dev, &rsi_btdev_ops);
  res = cdev_add(&common->bt_char_dev, common->bt_devid, 1);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "Failed to add bt char device\n");
    goto err_add;
  }
  rsi_dbg(ERR_ZONE, "Finished bt char device initialization\n");

  return 0;

err_add:
  device_destroy(common->bt_char_class, common->bt_devid);
err_create:
  unregister_chrdev_region(common->bt_devid, 1);
err_alloc:
  class_destroy(common->bt_char_class);
  return res;
}
/**
 * unregisters with the bluedroid interface
 *
 * @return  - 0 on success
 */
void rsi_bdroid_deinit(struct rsi_common *common)
{
  device_destroy(common->bt_char_class, common->bt_devid);
  cdev_del(&common->bt_char_dev);
  unregister_chrdev_region(common->bt_devid, 1);
  class_destroy(common->bt_char_class);
}
