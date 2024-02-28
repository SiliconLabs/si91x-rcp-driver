// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <linux/module.h>
#include <linux/usb.h>
#include "rsi_usb.h"
#include "rsi_hal.h"

static struct rsi_host_intf_ops usb_host_intf_ops = {
  .write_pkt              = rsi_usb_host_intf_write_pkt,
  .master_reg_read        = rsi_usb_master_reg_read,
  .master_reg_write       = rsi_usb_master_reg_write,
  .read_reg_multiple      = rsi_usb_read_register_multiple,
  .write_reg_multiple     = rsi_usb_write_register_multiple,
  .load_data_master_write = rsi_usb_load_data_master_write,
  .check_hw_queue_status  = rsi_usb_check_queue_status,
};

#ifdef CONFIG_PM
#if KERNEL_VERSION(4, 15, 0) > LINUX_VERSION_CODE
void rsi_usb_suspend_timeout(struct rsi_common *common)
{
#else
static void rsi_usb_suspend_timeout(struct timer_list *timer)
{
  struct rsi_common *common = from_timer(common, timer, suspend_timer);
#endif
  struct rsi_hw *adapter = common->priv;

  if (adapter->usb_intf_in_suspend) {
    if (adapter->usb_in_deep_ps && !protocol_tx_access(common)) {
      mod_timer(&common->suspend_timer, msecs_to_jiffies(1000) + jiffies);
      return;
    }
    usb_autopm_get_interface_async(adapter->usb_iface);
    if (atomic_read(&adapter->usb_iface->dev.power.usage_count) > 0)
      usb_autopm_put_interface_async(adapter->usb_iface);
  }
}
#endif

static int usb_start_wait_urb(struct urb *urb, int timeout, int *actual_length)
{
  struct api_context ctx;
  unsigned long expire;
  int retval;

  init_completion(&ctx.done);
  urb->context       = &ctx;
  urb->actual_length = 0;
  retval             = usb_submit_urb(urb, GFP_NOIO);
  if (unlikely(retval))
    goto out;

  expire = timeout ? msecs_to_jiffies(timeout) : MAX_SCHEDULE_TIMEOUT;
  if (!wait_for_completion_timeout(&ctx.done, expire)) {
    usb_kill_urb(urb);
    retval = (ctx.status == -ENOENT ? -ETIMEDOUT : ctx.status);

    dev_dbg(&urb->dev->dev,
            "%s timed out on ep%d%s len=%u/%u\n",
            current->comm,
            usb_endpoint_num(&urb->ep->desc),
            usb_urb_dir_in(urb) ? "in" : "out",
            urb->actual_length,
            urb->transfer_buffer_length);
  } else
    retval = ctx.status;
out:
  if (actual_length)
    *actual_length = urb->actual_length;

  usb_free_urb(urb);
  return retval;
}

static void usb_api_blocking_completion(struct urb *urb)
{
  struct api_context *ctx = urb->context;

  ctx->status = urb->status;
  complete(&ctx->done);
}

int usb_bulk_msg_rsi(struct usb_device *usb_dev,
                     unsigned int pipe,
                     void *data,
                     int len,
                     int *actual_length,
                     int timeout)
{
  struct urb *urb;
  struct usb_host_endpoint *ep;

  ep = usb_pipe_endpoint(usb_dev, pipe);
  if (!ep || len < 0)
    return -EINVAL;

  urb = usb_alloc_urb(0, GFP_KERNEL);
  if (!urb)
    return -ENOMEM;

  if ((ep->desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
    pipe = (pipe & ~(3 << 30)) | (PIPE_INTERRUPT << 30);
    usb_fill_int_urb(urb, usb_dev, pipe, data, len, usb_api_blocking_completion, NULL, ep->desc.bInterval);
  } else
    usb_fill_bulk_urb(urb, usb_dev, pipe, data, len, usb_api_blocking_completion, NULL);
  urb->transfer_flags |= URB_ZERO_PACKET; //Added this to support USB v1.2
  return usb_start_wait_urb(urb, timeout, actual_length);
}
/**
 * rsi_usb_card_write() - This function writes data to the USB Card.
 * @adapter: Pointer to the adapter structure.
 * @buf: Pointer to the buffer from where the data has to be taken.
 * @len: Length to be written.
 * @endpoint: Type of endpoint.
 *
 * Return: status: 0 on success, a negative error code on failure.
 */
static int rsi_usb_card_write(struct rsi_hw *adapter, u8 *buf, u16 len, u32 endpoint)
{
  struct rsi_91x_usbdev *dev = (struct rsi_91x_usbdev *)adapter->rsi_dev;
  int status                 = 0;
  u8 *seg                    = dev->tx_buffer;
  int transfer               = 0;
  int ep                     = dev->bulkout_endpoint_addr[endpoint - 1];

  if (adapter->priv->zb_fsm_state == ZB_DEVICE_READY && (DEV_MODEL_9116 ? endpoint == ZIGB_EP : endpoint == DATA_EP)) {
    memcpy(seg, buf, len);
  } else {
    memset(seg, 0, len + 128);
    memcpy(seg + 128, buf, len);
    len += 128;
  }
  transfer = len;
  atomic_inc(&adapter->tx_pending_urbs);
  status = usb_bulk_msg_rsi(dev->usbdev, usb_sndbulkpipe(dev->usbdev, ep), (void *)seg, (int)len, &transfer, TIMEOUT);
  if (status < 0) {
    rsi_dbg(ERR_ZONE, "Card write failed with error code :%d\n", status);
    dev->write_fail = 1;
    goto fail;
  }
  rsi_dbg(MGMT_TX_ZONE, "%s: Sent Message successfully\n", __func__);
  atomic_dec(&adapter->tx_pending_urbs);

fail:
  return status;
}

/**
 * rsi_write_multiple() - This function writes multiple bytes of information
 *			  to the USB card.
 * @adapter: Pointer to the adapter structure.
 * @addr: Address of the register.
 * @data: Pointer to the data that has to be written.
 * @count: Number of multiple bytes to be written.
 *
 * Return: 0 on success, a negative error code on failure.
 */
static int rsi_write_multiple(struct rsi_hw *adapter, u32 addr, u8 *data, u32 count)
{
  struct rsi_91x_usbdev *dev = (struct rsi_91x_usbdev *)adapter->rsi_dev;

  if (!adapter || addr == 0) {
    rsi_dbg(INFO_ZONE, "%s: Unable to write to card\n", __func__);
    return -1;
  }

  if (dev->write_fail)
    return -1;

  return rsi_usb_card_write(adapter, data, count, addr);
}

#define RSI_USB_REQ_OUT (USB_TYPE_VENDOR | USB_DIR_OUT | USB_RECIP_DEVICE)
#define RSI_USB_REQ_IN  (USB_TYPE_VENDOR | USB_DIR_IN | USB_RECIP_DEVICE)

/* rsi_usb_reg_read() - This function reads data from given register address.
 * @usbdev: Pointer to the usb_device structure.
 * @reg: Address of the register to be read.
 * @value: Value to be read.
 * @len: length of data to be read.
 *
 * Return: status: 0 on success, a negative error code on failure.
 */
static int rsi_usb_reg_read(struct usb_device *usbdev, u32 reg, u32 *value, u16 len)
{
  u8 *buf;
  int status = 0;
  u16 reg_value;
  u16 index;

  buf = kmalloc(4, GFP_KERNEL);
  if (!buf)
    return -ENOMEM;
  len       = 2;
  reg_value = cpu_to_le16(((u16 *)&reg)[1] & 0xffff);
  index     = cpu_to_le16(((u16 *)&reg)[0] & 0xffff);
  status    = usb_control_msg(usbdev,
                           usb_rcvctrlpipe(usbdev, 0),
                           USB_VENDOR_REGISTER_READ,
                           RSI_USB_REQ_IN,
                           reg_value,
                           index,
                           (void *)buf,
                           len,
                           USB_CTRL_GET_TIMEOUT);

  *value = (buf[0] | (buf[1] << 8));
  if (status < 0) {
    rsi_dbg(ERR_ZONE, "%s: Reg read failed with error code :%d\n", __func__, status);
  }
  kfree(buf);

  return status;
}

/**
 * rsi_usb_reg_write() - This function writes the given data into the given
 *			 register address.
 * @usbdev: Pointer to the usb_device structure.
 * @reg: Address of the register.
 * @value: Value to write.
 * @len: Length of data to be written.
 *
 * Return: status: 0 on success, a negative error code on failure.
 */
static int rsi_usb_reg_write(struct usb_device *usbdev, unsigned long reg, unsigned long value, u16 len)
{
  u8 *usb_reg_buf;
  int status = 0;
  u16 reg_value, index;

  usb_reg_buf = kmalloc(4, GFP_KERNEL);
  if (!usb_reg_buf)
    return -ENOMEM;
  usb_reg_buf[0] = (value & 0x00ff);
  usb_reg_buf[1] = (value & 0xff00) >> 8;
  usb_reg_buf[2] = (value & 0x00ff0000) >> 16;
  usb_reg_buf[3] = (value & 0xff000000) >> 24;

  reg_value = ((u16 *)&reg)[1] & 0xffff;
  index     = ((u16 *)&reg)[0] & 0xffff;
  status    = usb_control_msg(usbdev,
                           usb_sndctrlpipe(usbdev, 0),
                           USB_VENDOR_REGISTER_WRITE,
                           RSI_USB_REQ_OUT,
                           reg_value,
                           index,
                           (void *)usb_reg_buf,
                           len,
                           USB_CTRL_SET_TIMEOUT);
  if (status < 0) {
    rsi_dbg(ERR_ZONE, "%s: Reg write failed with error code :%d\n", __func__, status);
  }
  kfree(usb_reg_buf);

  return status;
}

/**
 * rsi_usb_read_register_multiple() - This function reads multiple
 *					bytes of data from the address.
 * @adapter:	Pointer to the adapter structure.
 * @addr:	Address of the register.
 * @data:	Read data.
 * @len:	Number of bytes to read.
 *
 * Return: status: 0 on success, a negative error code on failure.
 */
int rsi_usb_read_register_multiple(struct rsi_hw *adapter, u32 addr, u8 *data, u16 count)
{
  struct rsi_91x_usbdev *dev = (struct rsi_91x_usbdev *)adapter->rsi_dev;
  u8 *buf;
  u16 transfer;
  int status = 0;
  u16 reg_val, index;

  if (addr == 0)
    return -EINVAL;

  buf = kzalloc(4096, GFP_KERNEL);
  if (!buf)
    return -ENOMEM;

  reg_val = ((u16 *)&addr)[1] & 0xffff;
  index   = ((u16 *)&addr)[0] & 0xffff;
  while (count) {
    transfer = min_t(u16, count, 4096);
    status   = usb_control_msg(dev->usbdev,
                             usb_rcvctrlpipe(dev->usbdev, 0),
                             USB_VENDOR_REGISTER_READ,
                             RSI_USB_REQ_IN,
                             reg_val,
                             index,
                             (void *)buf,
                             transfer,
                             USB_CTRL_GET_TIMEOUT);
    if (status < 0) {
      rsi_dbg(ERR_ZONE, "Reg read failed with error code :%d\n", status);
      kfree(buf);
      return status;

    } else {
      memcpy(data, buf, transfer);
      count -= transfer;
      data += transfer;
      addr += transfer;
    }
  }
  kfree(buf);
  return status;
}

/**
 * rsi_usb_write_register_multiple() - This function writes multiple bytes of
 *				       information to the given address.
 * @adapter:	Pointer to the adapter structure.
 * @addr:	Address of the register.
 * @data:	Pointer to the data that has to be written.
 * @count:	Number of multiple bytes to be written on to the registers.
 *
 * Return: status: 0 on success, a negative error code on failure.
 */
int rsi_usb_write_register_multiple(struct rsi_hw *adapter, u32 addr, u8 *data, u16 count)
{
  struct rsi_91x_usbdev *dev = (struct rsi_91x_usbdev *)adapter->rsi_dev;
  u8 *buf;
  u16 transfer;
  int status = 0;
  u16 reg_val, index;

  buf = kzalloc(4096, GFP_KERNEL);
  if (!buf)
    return -ENOMEM;

  reg_val = ((u16 *)&addr)[1] & 0xffff;
  index   = ((u16 *)&addr)[0] & 0xffff;
  while (count) {
    transfer = min_t(u16, count, 4096);
    memcpy(buf, data, transfer);
    status = usb_control_msg(dev->usbdev,
                             usb_sndctrlpipe(dev->usbdev, 0),
                             USB_VENDOR_REGISTER_WRITE,
                             RSI_USB_REQ_OUT,
                             reg_val,
                             index,
                             (void *)buf,
                             transfer,
                             USB_CTRL_SET_TIMEOUT);
    if (status < 0) {
      rsi_dbg(ERR_ZONE, "Reg write failed with error code :%d\n", status);
      kfree(buf);
      return status;
    }
    count -= transfer;
    data += transfer;
    addr += transfer;
  }

  kfree(buf);
  return status;
}

/**
 * rsi_rx_done_handler() - This function is called when a packet is received
 *			   from USB stack. This is callback to receive done.
 * @urb: Received URB.
 *
 * Return: None.
 */
static void rsi_rx_done_handler(struct urb *urb)
{
  struct rx_usb_ctrl_block *rx_cb = urb->context;
  struct rsi_91x_usbdev *dev      = (struct rsi_91x_usbdev *)rx_cb->data;

  if (urb->status) {
    dev_kfree_skb(rx_cb->rx_skb);
    return;
  }

  if (urb->actual_length <= 0 || urb->actual_length > rx_cb->rx_skb->len) {
    rsi_dbg(INFO_ZONE, "%s: Invalid packet length = %d\n", __func__, urb->actual_length);
    return;
  }

  skb_trim(rx_cb->rx_skb, urb->actual_length);
  skb_queue_tail(&dev->rx_q[rx_cb->ep_num - 1], rx_cb->rx_skb);

  rsi_set_event(&dev->rx_thread.event);

  if (rsi_rx_urb_submit(dev->priv, rx_cb->ep_num))
    rsi_dbg(ERR_ZONE, "%s: Failed in urb submission", __func__);
}

/**
 * rsi_rx_urb_submit() - This function submits the given URB to the USB stack.
 * @adapter: Pointer to the adapter structure.
 *
 * Return: 0 on success, a negative error code on failure.
 */
int rsi_rx_urb_submit(struct rsi_hw *adapter, u8 ep_num)
{
  struct rsi_91x_usbdev *dev      = (struct rsi_91x_usbdev *)adapter->rsi_dev;
  struct rx_usb_ctrl_block *rx_cb = &dev->rx_cb[ep_num - 1];
  struct urb *urb                 = rx_cb->rx_urb;
  int status;
  struct sk_buff *skb;
  u8 dword_align_bytes;
  uint32_t total_len = 0;

  if (adapter->priv->driver_mode == SNIFFER_MODE)
    total_len = RSI_RECV_BUFFER_LEN * 4;
  else
    total_len = RSI_RECV_BUFFER_LEN;

  skb = dev_alloc_skb(total_len);
  if (!skb)
    return -ENOMEM;
  skb_reserve(skb, 64); /* For dword alignment */
  skb_put(skb, total_len - 64);
  dword_align_bytes = (unsigned long)skb->data & 0x3f;
  if (dword_align_bytes)
    skb_push(skb, dword_align_bytes);

  urb->transfer_buffer = skb->data;
  rx_cb->rx_skb        = skb;

  usb_fill_bulk_urb(urb,
                    dev->usbdev,
                    usb_rcvbulkpipe(dev->usbdev, dev->bulkin_endpoint_addr[ep_num - 1]),
                    urb->transfer_buffer,
                    skb->len,
                    rsi_rx_done_handler,
                    rx_cb);

  status = usb_submit_urb(urb, GFP_KERNEL);
  if (status)
    rsi_dbg(ERR_ZONE, "%s: Failed in urb submission\n", __func__);

  return status;
}

/**
 *rsi_usb_host_intf_write_pkt() - This function writes the packet to the
 *				   USB card.
 * @adapter: Pointer to the adapter structure.
 * @pkt: Pointer to the data to be written on to the card.
 * @len: Length of the data to be written on to the card.
 *
 * Return: 0 on success, a negative error code on failure.
 */
int rsi_usb_host_intf_write_pkt(struct rsi_hw *adapter, u8 *pkt, u32 len)
{
  u32 queueno = ((pkt[1] >> 4) & 0x7);
  u8 endpoint;

  rsi_dbg(DATA_TX_ZONE, "%s: queueno=%d\n", __func__, queueno);
  if (DEV_MODEL_9116 && queueno == RSI_ZIGB_Q) {
    endpoint = ZIGB_EP;
  } else {
    endpoint =
      ((queueno == RSI_WIFI_MGMT_Q || queueno == RSI_COEX_Q || queueno == RSI_WIFI_DATA_Q) ? MGMT_EP : DATA_EP);
  }

  return rsi_write_multiple(adapter, endpoint, pkt, len);
}

int rsi_usb_master_reg_read(struct rsi_hw *adapter, u32 reg, u32 *value, u16 len)
{
  struct usb_device *usbdev = ((struct rsi_91x_usbdev *)adapter->rsi_dev)->usbdev;

  return rsi_usb_reg_read(usbdev, reg, value, len);
}

int rsi_usb_master_reg_write(struct rsi_hw *adapter, unsigned long reg, unsigned long value, u16 len)
{
  struct usb_device *usbdev = ((struct rsi_91x_usbdev *)adapter->rsi_dev)->usbdev;

  return rsi_usb_reg_write(usbdev, reg, value, len);
}

int rsi_usb_load_data_master_write(struct rsi_hw *adapter,
                                   u32 base_address,
                                   u32 instructions_sz,
                                   u16 block_size,
                                   u8 *ta_firmware)
{
  u16 num_blocks;
  u32 cur_indx, ii;
  u8 temp_buf[256];

  num_blocks = instructions_sz / block_size;
  rsi_dbg(INFO_ZONE, "num_blocks: %d\n", num_blocks);

  for (cur_indx = 0, ii = 0; ii < num_blocks; ii++, cur_indx += block_size) {
    memset(temp_buf, 0, block_size);
    memcpy(temp_buf, ta_firmware + cur_indx, block_size);
    if ((rsi_usb_write_register_multiple(adapter, base_address, (u8 *)(temp_buf), block_size)) < 0)
      return -EIO;

    rsi_dbg(INFO_ZONE, "%s: loading block: %d\n", __func__, ii);
    base_address += block_size;
  }

  if (instructions_sz % block_size) {
    memset(temp_buf, 0, block_size);
    memcpy(temp_buf, ta_firmware + cur_indx, instructions_sz % block_size);
    if ((rsi_usb_write_register_multiple(adapter, base_address, (u8 *)temp_buf, instructions_sz % block_size)) < 0)
      return -EIO;
    rsi_dbg(INFO_ZONE, "Written Last Block in Address 0x%x Successfully\n", cur_indx);
  }
  return 0;
}

int rsi_usb_check_queue_status(struct rsi_hw *adapter, u8 q_num)
{
  return QUEUE_NOT_FULL;

#if 0
	struct rsi_91x_usbdev *dev = (struct rsi_91x_usbdev *)adapter->rsi_dev;
	int status;
	u32 buf_status = 0;

	if (adapter->priv->fsm_state != FSM_MAC_INIT_DONE)
		return QUEUE_NOT_FULL;

	status = rsi_usb_reg_read(dev->usbdev, adapter->usb_buffer_status_reg,
				  &buf_status, 2);
	if (status < 0)
		return status;

	printk("buffer_status = %x\n", buf_status);
	if (buf_status & (BIT(PKT_MGMT_BUFF_FULL))) {
		if (!dev->rx_info.mgmt_buffer_full)
			dev->rx_info.mgmt_buf_full_counter++;
		dev->rx_info.mgmt_buffer_full = true;
	} else {
		dev->rx_info.mgmt_buffer_full = false;
	}

	if (buf_status & (BIT(PKT_BUFF_FULL))) {
		if (!dev->rx_info.buffer_full)
			dev->rx_info.buf_full_counter++;
		dev->rx_info.buffer_full = true;
	} else {
		dev->rx_info.buffer_full = false;
	}

	if (buf_status & (BIT(PKT_BUFF_SEMI_FULL))) {
		if (!dev->rx_info.semi_buffer_full)
			dev->rx_info.buf_semi_full_counter++;
		dev->rx_info.semi_buffer_full = true;
	} else {
		dev->rx_info.semi_buffer_full = false;
	}

	if ((q_num == MGMT_SOFT_Q) && (dev->rx_info.mgmt_buffer_full))
		return QUEUE_FULL;

	if ((q_num < MGMT_SOFT_Q) && (dev->rx_info.buffer_full))
		return QUEUE_FULL;

	return QUEUE_NOT_FULL;
#endif
}

/**
 * rsi_deinit_usb_interface() - This function deinitializes the usb interface.
 * @adapter: Pointer to the adapter structure.
 *
 * Return: None.
 */
static void rsi_deinit_usb_interface(struct rsi_hw *adapter)
{
  struct rsi_91x_usbdev *dev = (struct rsi_91x_usbdev *)adapter->rsi_dev;

  rsi_dbg(INFO_ZONE, "Deinitializing USB interface...\n");

  rsi_kill_thread(&dev->rx_thread);
  //kfree(dev->rx_cb[0].rx_buffer);
  skb_queue_purge(&dev->rx_q[0]);
  usb_free_urb(dev->rx_cb[0].rx_urb);
#if defined(CONFIG_RSI_BT_ALONE) || defined(CONFIG_RSI_COEX_MODE)
  //kfree(dev->rx_cb[1].rx_buffer);
  skb_queue_purge(&dev->rx_q[1]);
  usb_free_urb(dev->rx_cb[1].rx_urb);
#endif
  kfree(dev->saved_tx_buffer);
}

/**
 * rsi_find_bulk_in_and_out_endpoints() - This function initializes the bulk
 *					  endpoints to the device.
 * @interface: Pointer to the USB interface structure.
 * @adapter: Pointer to the adapter structure.
 *
 * Return: ret_val: 0 on success, -ENOMEM on failure.
 */
static int rsi_find_bulk_in_and_out_endpoints(struct usb_interface *interface, struct rsi_hw *adapter)
{
  struct rsi_91x_usbdev *dev = (struct rsi_91x_usbdev *)adapter->rsi_dev;
  struct usb_host_interface *iface_desc;
  struct usb_endpoint_descriptor *endpoint;
  __le16 buffer_size;
  int ii, bin_found = 0, bout_found = 0;

  iface_desc = &interface->altsetting[0];

  for (ii = 0; ii < iface_desc->desc.bNumEndpoints; ++ii) {
    endpoint = &(iface_desc->endpoint[ii].desc);

    if ((!dev->bulkin_endpoint_addr[bin_found]) && (endpoint->bEndpointAddress & USB_DIR_IN)
        && ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK)) {
      buffer_size                          = endpoint->wMaxPacketSize;
      dev->bulkin_size[bin_found]          = buffer_size;
      dev->bulkin_endpoint_addr[bin_found] = endpoint->bEndpointAddress;
      rsi_dbg(INIT_ZONE, "bulkin addr[%d] = %d\n", bin_found, dev->bulkin_endpoint_addr[bin_found]);
      bin_found++;
    }

    if (!dev->bulkout_endpoint_addr[bout_found] && !(endpoint->bEndpointAddress & USB_DIR_IN)
        && ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK)) {
      rsi_dbg(INIT_ZONE, "%s:%d\n", __func__, __LINE__);
      buffer_size = endpoint->wMaxPacketSize;
      if (buffer_size == USB_FULL_SPEED_SIZE)
        adapter->usb_full_speed = true;
      dev->bulkout_endpoint_addr[bout_found] = endpoint->bEndpointAddress;
      buffer_size                            = endpoint->wMaxPacketSize;
      dev->bulkout_size[bout_found]          = buffer_size;
      rsi_dbg(INIT_ZONE, "bulkout addr[%d] = %d\n", bout_found, dev->bulkout_endpoint_addr[bout_found]);
      bout_found++;
    }

    if ((bin_found >= MAX_BULK_EP) || (bout_found >= MAX_BULK_EP))
      break;
  }

  if (!(dev->bulkin_endpoint_addr[0]) && (dev->bulkout_endpoint_addr[0]))
    return -EINVAL;

  return 0;
}

static int rsi_usb_init_rx(struct rsi_hw *adapter)
{
  struct rsi_91x_usbdev *dev = (struct rsi_91x_usbdev *)adapter->rsi_dev;
  struct rx_usb_ctrl_block *rx_cb;
  u8 idx;

  for (idx = 0; idx < MAX_RX_URBS; idx++) {
    rx_cb = &dev->rx_cb[idx];

    rx_cb->rx_urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!rx_cb->rx_urb) {
      rsi_dbg(ERR_ZONE, "Failed alloc rx urb[%d]\n", idx);
      goto err;
    }
    rx_cb->ep_num = idx + 1;
    rx_cb->data   = (void *)dev;

    skb_queue_head_init(&dev->rx_q[idx]);
  }
  return 0;

err:
  kfree(rx_cb[0].rx_urb);
  kfree(rx_cb[1].rx_urb);
  return -1;
}

/**
 * rsi_init_usb_interface() - This function initializes the usb interface.
 * @adapter: Pointer to the adapter structure.
 * @pfunction: Pointer to USB interface structure.
 *
 * Return: 0 on success, a negative error code on failure.
 */
static int rsi_init_usb_interface(struct rsi_hw *adapter, struct usb_interface *pfunction)
{
  struct rsi_91x_usbdev *rsi_dev;
  struct rsi_common *common = adapter->priv;
  int status                = 0;
  u8 dword_align_bytes      = 0;

  rsi_dev = kzalloc(sizeof(*rsi_dev), GFP_KERNEL);
  if (!rsi_dev)
    return -ENOMEM;

  adapter->rsi_dev   = rsi_dev;
  adapter->usb_iface = pfunction;
  rsi_dev->usbdev    = interface_to_usbdev(pfunction);
  rsi_dev->priv      = (void *)adapter;

  if (rsi_find_bulk_in_and_out_endpoints(pfunction, adapter))
    return -EINVAL;

  adapter->device = &pfunction->dev;
  usb_set_intfdata(pfunction, adapter);

  rsi_dev->tx_buffer = kmalloc(2048, GFP_KERNEL);
  if (!rsi_dev->tx_buffer) {
    status = -ENOMEM;
    goto fail_1;
  }
  rsi_dev->saved_tx_buffer = rsi_dev->tx_buffer;
  dword_align_bytes        = (unsigned long)rsi_dev->tx_buffer & 0x3f;
  if (dword_align_bytes)
    rsi_dev->tx_buffer = rsi_dev->tx_buffer + (64 - dword_align_bytes);

  /* Initialize RX handle */
  if (rsi_usb_init_rx(adapter)) {
    rsi_dbg(ERR_ZONE, "Failed to init RX handle\n");
    goto fail_1;
  }

  rsi_dev->tx_blk_size = 252;
  adapter->tx_blk_size = rsi_dev->tx_blk_size;

  /* Initializing function callbacks */
  adapter->rx_urb_submit           = rsi_rx_urb_submit;
  adapter->check_hw_queue_status   = rsi_usb_check_queue_status;
  adapter->determine_event_timeout = rsi_usb_event_timeout;
  adapter->host_intf_ops           = &usb_host_intf_ops;

  rsi_init_event(&rsi_dev->rx_thread.event);
  status = rsi_create_kthread(common, &rsi_dev->rx_thread, rsi_usb_rx_thread, "USB-RX-Thread");
  if (status) {
    rsi_dbg(ERR_ZONE, "%s: Unable to init rx thrd\n", __func__);
    goto fail_2;
  }

#ifdef CONFIG_RSI_DEBUGFS
  /* In USB, one less than the MAX_DEBUGFS_ENTRIES entries
	 * is required */
  adapter->num_debugfs_entries = MAX_DEBUGFS_ENTRIES - 1;
#endif

  rsi_dbg(INIT_ZONE, "%s: Enabled the interface\n", __func__);
  return 0;

fail_2:
  kfree(rsi_dev->saved_tx_buffer);
  rsi_kill_thread(&rsi_dev->rx_thread);
fail_1:
  return status;
}

static int rsi_usb_gspi_init(struct rsi_hw *adapter)
{
  u32 gspi_ctrl_reg0_val;

  /**
	 * Programming gspi frequency = soc_frequency / 2
	 * Warning : ULP seemed to be not working
	 * well at high frequencies. Modify accordingly
	 */
  gspi_ctrl_reg0_val = 0x4;
  gspi_ctrl_reg0_val |= 0x10;
  gspi_ctrl_reg0_val |= 0x40;
  gspi_ctrl_reg0_val |= 0x100;
  gspi_ctrl_reg0_val |= 0x000;
  gspi_ctrl_reg0_val |= 0x000;

  /* Initializing GSPI for ULP read/writes */
  return rsi_usb_master_reg_write(adapter, GSPI_CTRL_REG0, gspi_ctrl_reg0_val, 2);
}

static int usb_ulp_read_write(struct rsi_hw *adapter, u16 addr, u16 *data, u16 len_in_bits)
{
  if ((rsi_usb_master_reg_write(adapter, GSPI_DATA_REG1, ((addr << 6) | (data[1] & 0x3f)), 2) < 0))
    goto fail;

  if ((rsi_usb_master_reg_write(adapter, GSPI_DATA_REG0, (*(u16 *)&data[0]), 2)) < 0)
    goto fail;

  if ((rsi_usb_gspi_init(adapter)) < 0)
    goto fail;

  if ((rsi_usb_master_reg_write(adapter, GSPI_CTRL_REG1, ((len_in_bits - 1) | GSPI_TRIG), 2)) < 0)
    goto fail;

  msleep(10);

  return 0;

fail:
  return -1;
}

static int rsi_reset_card(struct rsi_hw *adapter)
{
  u16 temp[4] = { 0 };

  rsi_dbg(INFO_ZONE, "Resetting Card...\n");

  if (rsi_usb_master_reg_write(adapter, SWBL_REGOUT, FW_WDT_DISABLE_REQ, 2) < 0) {
    rsi_dbg(ERR_ZONE, "%s: FW WDT Disable failed...\n", __func__);
    goto fail;
  }

#define TA_HOLD_REG 0x22000844
  rsi_usb_master_reg_write(adapter, TA_HOLD_REG, 0xE, 4);
  msleep(100);

  if (adapter->device_model < RSI_DEV_9116) {
    *(u32 *)temp = 2;
    if ((usb_ulp_read_write(adapter, WATCH_DOG_TIMER_1, &temp[0], 32)) < 0) {
      goto fail;
    }

    *(u32 *)temp = 0;
    if ((usb_ulp_read_write(adapter, WATCH_DOG_TIMER_2, temp, 32)) < 0) {
      goto fail;
    }

    *(u32 *)temp = 50;
    if ((usb_ulp_read_write(adapter, WATCH_DOG_DELAY_TIMER_1, temp, 32)) < 0) {
      goto fail;
    }

    *(u32 *)temp = 0;
    if ((usb_ulp_read_write(adapter, WATCH_DOG_DELAY_TIMER_2, temp, 32)) < 0) {
      goto fail;
    }

    *(u32 *)temp = ((0xaa000) | RESTART_WDT | BYPASS_ULP_ON_WDT);
    if ((usb_ulp_read_write(adapter, WATCH_DOG_TIMER_ENABLE, temp, 32)) < 0) {
      goto fail;
    }
  } else {
    if ((rsi_usb_master_reg_write(adapter, NWP_WWD_INTERRUPT_TIMER, 5, 4)) < 0) {
      goto fail;
    }
    if ((rsi_usb_master_reg_write(adapter, NWP_WWD_SYSTEM_RESET_TIMER, 4, 4)) < 0) {
      goto fail;
    }
    if ((rsi_usb_master_reg_write(adapter, NWP_WWD_MODE_AND_RSTART, 0xAA0001, 4)) < 0) {
      goto fail;
    }
  }

  rsi_dbg(INFO_ZONE, "***** Card Reset Done *****\n");
  return 0;

fail:
  rsi_dbg(ERR_ZONE, "Reset card Failed\n");
  return -1;
}

/**
 * rsi_probe() - This function is called by kernel when the driver provided
 *		 Vendor and device IDs are matched. All the initialization
 *		 work is done here.
 * @pfunction: Pointer to the USB interface structure.
 * @id: Pointer to the usb_device_id structure.
 *
 * Return: 0 on success, a negative error code on failure.
 */
static int rsi_probe(struct usb_interface *pfunction, const struct usb_device_id *id)
{
  struct rsi_hw *adapter;
  struct rsi_91x_usbdev *dev;
  struct rsi_common *common;
  u32 fw_status      = 0;
  u32 endpoint_ready = 0;
  int status         = 0;

  rsi_dbg(INIT_ZONE, "%s: Init function called\n", __func__);

  ps_sleep_type = LP_POWER_SAVE;
  adapter       = rsi_91x_init();
  if (!adapter) {
    rsi_dbg(ERR_ZONE, "%s: Failed to init os intf ops\n", __func__);
    return -ENOMEM;
  }

  common                 = adapter->priv;
  adapter->rsi_host_intf = RSI_HOST_INTF_USB;
#ifdef CONFIG_RSI_MULTI_MODE
  if (rsi_opermode_instances(adapter)) {
    rsi_dbg(ERR_ZONE, "%s: Invalid operating modes\n", __func__);
    goto err;
  }
#else
  adapter->priv->oper_mode = common->dev_oper_mode;
  if (rsi_validate_oper_mode(common->dev_oper_mode)) {
    rsi_dbg(ERR_ZONE, "%s: Invalid operating mode %d\n", __func__, common->dev_oper_mode);
    goto err;
  }

#endif
  status = rsi_init_usb_interface(adapter, pfunction);
  if (status) {
    rsi_dbg(ERR_ZONE, "%s: Failed to init usb interface\n", __func__);
    goto err;
  }

  rsi_dbg(INIT_ZONE, "%s: Initialized os intf ops\n", __func__);

  rsi_dbg(INIT_ZONE, "%s: product id = 0x%x vendor id = 0x%x\n", __func__, id->idProduct, id->idVendor);

  if (id && (id->idProduct == 0x9113)) {
    rsi_dbg(INIT_ZONE, "%s: 9113 MODULE IS CONNECTED\n", __func__);
    adapter->device_model = RSI_DEV_9113;
  } else if (id && (id->idProduct == 0x9116)) {
    adapter->device_model = RSI_DEV_9116;
    rsi_dbg(INIT_ZONE, "%s: 9116 MODULE IS CONNECTED\n", __func__);
  } else {
    rsi_dbg(ERR_ZONE, "##### Invalid RSI device id 0x%x\n", id->idProduct);
    goto err1;
  }

  dev = (struct rsi_91x_usbdev *)adapter->rsi_dev;

  rsi_usb_reg_read(dev->usbdev, 0x202101c4, &endpoint_ready, 2);
  rsi_dbg(INIT_ZONE, "Endpoint status = %4x \n", endpoint_ready);
  if (endpoint_ready == 0x88) {
    rsi_dbg(INIT_ZONE, "Endpoint Ready before Loading Firmware \n");
    status = rsi_rx_urb_submit(adapter, 1 /* RX_WLAN_EP */);
    if (status)
      goto err1;
  }

  status = rsi_usb_reg_read(dev->usbdev, FW_STATUS_REG, &fw_status, 2);
  if (status < 0)
    goto err1;
  else
    fw_status &= 1;

  if (!fw_status) {
    rsi_dbg(INIT_ZONE, "Loading firmware...\n");
    status = rsi_hal_device_init(adapter);
    if (status) {
      rsi_dbg(ERR_ZONE, "%s: Failed in device init\n", __func__);
      goto err1;
    }
    rsi_dbg(INIT_ZONE, "%s: Device Init Done\n", __func__);
  }
  msleep(1);
  if (endpoint_ready != 0x88) {
    rsi_dbg(INIT_ZONE, "Endpoint Ready after Loading Firmware \n");
    status = rsi_rx_urb_submit(adapter, 1 /* RX_WLAN_EP */);
    if (status)
      goto err1;
  }

#if defined(CONFIG_RSI_BT_ALONE) || defined(CONFIG_RSI_COEX_MODE)
  status = rsi_rx_urb_submit(adapter, 2 /* RX_BT_EP */);
  if (status)
    goto err1;
#endif

#ifdef CONFIG_PM
  usb_enable_autosuspend(dev->usbdev);
#endif
  return 0;
err1:
  rsi_deinit_usb_interface(adapter);
err:
  rsi_91x_deinit(adapter);
  rsi_dbg(ERR_ZONE, "%s: Failed in probe...Exiting\n", __func__);
  return status;
}

/**
 * rsi_disconnect() - This function performs the reverse of the probe function,
 *		      it deintialize the driver structure.
 * @pfunction: Pointer to the USB interface structure.
 *
 * Return: None.
 */
static void rsi_disconnect(struct usb_interface *pfunction)
{
  struct rsi_hw *adapter = usb_get_intfdata(pfunction);
  struct usb_device *usbdev;
  struct rsi_common *common;

  if (!adapter)
    return;
  common = adapter->priv;

  /* Making sure that all the frames triggered from
   * upper layers or internally generated are not given to
   * the usb layer As in the disconnect path, usb controller doesn't allow 
   * urb submissions. Hence killing tx thread here to avoid queing frames
   * to usb layer generated by rsi_mac80211_detach function.
   */
  rsi_kill_thread(&common->tx_thread);
  rsi_mac80211_detach(adapter);
  rsi_dbg(INFO_ZONE, "mac80211 detach done\n");

#if defined(CONFIG_RSI_BT_ALONE) || defined(CONFIG_RSI_COEX_MODE)
  if ((adapter->priv->coex_mode == 2) || (adapter->priv->coex_mode == 4))
    rsi_hci_detach(adapter->priv);
  rsi_dbg(INFO_ZONE, "HCI Detach Done\n");
#endif

  rsi_reset_card(adapter);

  rsi_deinit_usb_interface(adapter);
  rsi_dbg(INFO_ZONE, "USB interface down\n");

  rsi_91x_deinit(adapter);

  usbdev = ((struct rsi_91x_usbdev *)adapter->rsi_dev)->usbdev;
#ifdef CONFIG_PM
  usb_disable_autosuspend(usbdev);
#endif
  rsi_dbg(INFO_ZONE, "%s: Deinitialization completed\n", __func__);
}

#ifdef CONFIG_PM
static int rsi_suspend(struct usb_interface *pfunction, pm_message_t message)
{
  struct rsi_hw *adapter = usb_get_intfdata(pfunction);
  struct rsi_common *common;
  struct rsi_91x_usbdev *dev;
  struct usb_device *usbdev;
  struct timer_list *timer = NULL;
  u8 *temp_buf;
  int status = 0;
  u32 suspend_duration;
  u64 duration_j = 0;

  if (!adapter)
    return -ENODEV;
  common   = adapter->priv;
  timer    = &common->suspend_timer;
  dev      = (struct rsi_91x_usbdev *)adapter->rsi_dev;
  usbdev   = dev->usbdev;
  temp_buf = kzalloc(6, GFP_KERNEL);
  if (!temp_buf) {
    rsi_dbg(ERR_ZONE, "Can't allocate memory\n");
    return -ENOMEM;
  }
#define USB_VENDOR_PS_STATUS_READ 0x18
  /* Read LMAC PS status and sleep duration */
  down(&common->tx_access_lock);
  if ((!protocol_tx_access(common)) && (!atomic_read(&adapter->tx_pending_urbs))) {
    status = usb_control_msg(usbdev,
                             usb_rcvctrlpipe(usbdev, 0),
                             USB_VENDOR_PS_STATUS_READ,
                             USB_TYPE_VENDOR | USB_DIR_IN | USB_RECIP_DEVICE,
                             0,
                             0,
                             (void *)temp_buf,
                             6 /*len*/,
                             3000);
    /* Reading takes some time use 1 ms delay */
    usleep_range(1000, 2000);
    if (status < 0) {
      rsi_dbg(ERR_ZONE, "%s: Reg read failed with error code :%d\n", __func__, status);
      up(&common->tx_access_lock);
      goto err;
    }
    /* LMAC power save status */
    if (!temp_buf[0]) {
      /*
			 * Don't process suspend
			 */
      rsi_dbg(INFO_ZONE, "Don't process suspend\n");
      status = -EBUSY;
      up(&common->tx_access_lock);
      goto err;
    }
    /* Extract sleep duration, if zero then card is in deep sleep */
    if ((*(u32 *)&temp_buf[2])) {
      suspend_duration        = ((*(u32 *)&temp_buf[2]) / 1000);
      adapter->usb_in_deep_ps = 0;
    } else {
      suspend_duration        = 1000;
      adapter->usb_in_deep_ps = 1;
    }
    rsi_dbg(INFO_ZONE, "Suspend duration: %d\n", suspend_duration);
    if (suspend_duration < 5) {
      rsi_dbg(INFO_ZONE, "Don't process suspend if duration < 5ms\n");
      status = -EBUSY;
      up(&common->tx_access_lock);
      goto err;
    }
    rsi_dbg(MGMT_DEBUG_ZONE, "Killing URB's\n");
    if (&dev->rx_cb[0]) /* Kill URB */
      usb_kill_urb(dev->rx_cb[0].rx_urb);
#if defined(CONFIG_RSI_BT_ALONE) || defined(CONFIG_RSI_COEX_MODE)
    if (&dev->rx_cb[1]) /* Kill URB */
      usb_kill_urb(dev->rx_cb[1].rx_urb);
#endif
    common->common_hal_tx_access = false;
    adapter->usb_intf_in_suspend = 1;
    up(&common->tx_access_lock);
    duration_j = msecs_to_jiffies(suspend_duration) + jiffies;
    if (!timer->function) {
      rsi_dbg(INIT_ZONE, "Init USB suspend timer\n");
      if (timer_pending(timer)) {
        rsi_dbg(ERR_ZONE, "%s : Timer Pending. This Case Should not occur\n", __func__);
        del_timer(timer);
      }
#if KERNEL_VERSION(4, 15, 0) > LINUX_VERSION_CODE
      init_timer(timer);
      timer->data     = (unsigned long)common;
      timer->function = (void *)rsi_usb_suspend_timeout;
#else
      timer_setup(timer, rsi_usb_suspend_timeout, 0);
#endif
      timer->expires = duration_j;
      add_timer(timer);
      status = 0;
    } else {
      mod_timer(timer, duration_j);
      status = 0;
    }
  } else {
    status = -EBUSY;
    up(&common->tx_access_lock);
    goto err;
  }

err:
  kfree(temp_buf);
  return status;
}

static int rsi_resume(struct usb_interface *pfunction)
{
  struct rsi_hw *adapter = usb_get_intfdata(pfunction);
  struct rsi_common *common;
  struct rsi_91x_usbdev *dev;
  struct usb_device *usbdev;
  u8 *temp_buf;
  int status = 0;

  if (!adapter)
    return -ENODEV;
  if (!adapter->usb_intf_in_suspend) {
    rsi_dbg(INFO_ZONE, "USB already in resume\n");
    return 0;
  }
  common   = adapter->priv;
  dev      = (struct rsi_91x_usbdev *)adapter->rsi_dev;
  usbdev   = dev->usbdev;
  temp_buf = kzalloc(6, GFP_KERNEL);
  if (!temp_buf) {
    rsi_dbg(ERR_ZONE, "Can't allocate memory\n");
    return -ENOMEM;
  }
  down(&common->tx_access_lock);
  status = usb_control_msg(usbdev,
                           usb_rcvctrlpipe(usbdev, 0),
                           USB_VENDOR_PS_STATUS_READ,
                           USB_TYPE_VENDOR | USB_DIR_IN | USB_RECIP_DEVICE,
                           1,
                           0,
                           (void *)temp_buf,
                           6 /*len*/,
                           3000);
  usleep_range(1000, 2000); /* Provide 1 ms delay */
  if (status < 0) {
    rsi_dbg(ERR_ZONE, "%s: Reg read failed with error code :%d\n", __func__, status);
    up(&common->tx_access_lock);
    kfree(temp_buf);
    return status;
  }
  /* RX_WLAN_EP */
  status = rsi_rx_urb_submit(adapter, 1);
  if (status)
    goto err1;

#if defined(CONFIG_RSI_BT_ALONE) || defined(CONFIG_RSI_COEX_MODE)
  /* RX_BT_EP */
  status = rsi_rx_urb_submit(adapter, 2);
  if (status)
    goto err1;
#endif

  adapter->usb_intf_in_suspend = 0;
  up(&common->tx_access_lock);
  sleep_exit_recvd(common);
  kfree(temp_buf);
  return 0;
err1:
  return status;
}
#endif

static const struct usb_device_id rsi_dev_table[] = {
  { USB_DEVICE(USB_VENDOR_ID_RSI, USB_DEVICE_ID_RSI_9113) },
  { USB_DEVICE(USB_VENDOR_ID_RSI, USB_DEVICE_ID_RSI_9116) },
  { /* Blank */ },
};

static struct usb_driver rsi_driver = {
  .name       = "RSI-USB WLAN",
  .probe      = rsi_probe,
  .disconnect = rsi_disconnect,
  .id_table   = rsi_dev_table,
#ifdef CONFIG_PM
  .suspend              = rsi_suspend,
  .resume               = rsi_resume,
  .supports_autosuspend = 1,
#endif
};

static int __init rsi_usb_module_init(void)
{
  rsi_dbg(INIT_ZONE, "=====> RSI USB Module Initialize <=====\n");
  return usb_register(&rsi_driver);
}

static void __exit rsi_usb_module_exit(void)
{
  usb_deregister(&rsi_driver);
}

module_init(rsi_usb_module_init);
module_exit(rsi_usb_module_exit);

MODULE_AUTHOR("Redpine Signals Inc");
MODULE_DESCRIPTION("Common USB layer for RSI drivers");
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 12, 0)
MODULE_SUPPORTED_DEVICE("RSI-91x");
#endif
MODULE_DEVICE_TABLE(usb, rsi_dev_table);
MODULE_FIRMWARE(FIRMWARE_RSI9113);
MODULE_VERSION(DRV_VER);
MODULE_LICENSE("GPL");
