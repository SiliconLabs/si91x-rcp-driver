// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <linux/firmware.h>
#include "rsi_sdio.h"
#include "rsi_common.h"
#include "rsi_hal.h"

/**
 * rsi_sdio_master_access_msword() - This function sets the AHB master access
 *				     MS word in the SDIO slave registers.
 * @adapter: Pointer to the adapter structure.
 * @ms_word: ms word need to be initialized.
 *
 * Return: status: 0 on success, -1 on failure.
 */
int rsi_sdio_master_access_msword(struct rsi_hw *adapter, u16 ms_word)
{
  u8 byte;
  u8 function = 0;
  int status  = 0;

  byte = (u8)(ms_word & 0x00FF);

  rsi_dbg(INFO_ZONE, "%s: MASTER_ACCESS_MSBYTE:0x%x\n", __func__, byte);

  status = rsi_sdio_write_register(adapter, function, SDIO_MASTER_ACCESS_MSBYTE, &byte);
  if (status) {
    rsi_dbg(ERR_ZONE, "%s: fail to access MASTER_ACCESS_MSBYTE\n", __func__);
    return -1;
  }

  byte = (u8)(ms_word >> 8);

  rsi_dbg(INFO_ZONE, "%s:MASTER_ACCESS_LSBYTE:0x%x\n", __func__, byte);
  status = rsi_sdio_write_register(adapter, function, SDIO_MASTER_ACCESS_LSBYTE, &byte);
  return status;
}

void rsi_sdio_rx_thread(struct rsi_common *common)
{
  struct rsi_hw *adapter       = common->priv;
  struct rsi_91x_sdiodev *sdev = adapter->rsi_dev;
  struct sk_buff *skb;
  int status;
  bool done = false;

  do {
    status = rsi_wait_event(&sdev->rx_thread.event, EVENT_WAIT_FOREVER);
    if (status < 0)
      break;

    if (atomic_read(&sdev->rx_thread.thread_done))
      break;

    while (true) {
      skb = skb_dequeue(&sdev->rx_q.head);
      if (!skb)
        break;
      status = rsi_read_pkt(common, skb->data, skb->len);
      if (status) {
        rsi_dbg(ERR_ZONE, "Failed to read the packet\n");
        dev_kfree_skb(skb);
        return;
      }
      dev_kfree_skb(skb);
      if (sdev->rx_q.num_rx_pkts > 0)
        sdev->rx_q.num_rx_pkts--;

      if (atomic_read(&sdev->rx_thread.thread_done)) {
        done = true;
        break;
      }
    }
    rsi_reset_event(&sdev->rx_thread.event);
    if (done)
      break;
  } while (1);

  rsi_dbg(INFO_ZONE, "%s: Terminated SDIO RX thread\n", __func__);
  skb_queue_purge(&sdev->rx_q.head);
  atomic_inc(&sdev->rx_thread.thread_done);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
  complete_and_exit(&sdev->rx_thread.completion, 0);
#else
  kthread_complete_and_exit(&sdev->rx_thread.completion, 0);
#endif
}

/**
 * rsi_process_pkt() - This Function reads rx_blocks register and figures out
 *		       the size of the rx pkt.
 * @common: Pointer to the driver private structure.
 *
 * Return: 0 on success, -1 on failure.
 */
static int rsi_process_pkt(struct rsi_common *common)
{
  struct rsi_hw *adapter      = common->priv;
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
  u8 num_blks                 = 0;
  u32 rcv_pkt_len             = 0;
  int status                  = 0;
  u8 value                    = 0;
  u8 protocol = 0, unaggr_pkt = 0;
  struct sk_buff *skb;

#define COEX_PKT 0
#define WLAN_PKT 3
#define ZIGB_PKT 1
#define BT_PKT   2

  num_blks = ((adapter->interrupt_status & 1) | ((adapter->interrupt_status >> 4) << 1));

  if (!num_blks) {
    status = rsi_sdio_read_register(adapter, SDIO_RX_NUM_BLOCKS_REG, &value);
    if (status) {
      rsi_dbg(ERR_ZONE, "%s: Failed to read pkt length from the card:\n", __func__);
      return status;
    }

    protocol = value >> 5;
    num_blks = value & 0x1f;
  } else {
    protocol = WLAN_PKT;
  }

  if (dev->write_fail == 2) {
    rsi_sdio_ack_intr(common->priv, (1 << MSDU_PKT_PENDING));
  }
  if (unlikely(!num_blks)) {
    dev->write_fail = 2;
    return -1;
  }

  if (protocol == BT_PKT || protocol == ZIGB_PKT) //unaggr_pkt FIXME
    unaggr_pkt = 1;

  rcv_pkt_len = (num_blks * 256);
  if (dev->rx_q.num_rx_pkts >= RSI_SDIO_MAX_RX_PKTS) {
    rsi_dbg(ISR_ZONE,
            "%s,%d: Reached MAX RX_Q size,"
            "dropping the packet\n",
            __func__,
            __LINE__);
    goto DROP_PKT;
  }

  skb = dev_alloc_skb(rcv_pkt_len);

  if (!skb) {
    rsi_dbg(ERR_ZONE,
            "%s,%d: Failed to allocate rx packet buffer,"
            "dropping packet\n",
            __func__,
            __LINE__);
    goto DROP_PKT;
  }

  skb_put(skb, rcv_pkt_len);
  status = rsi_sdio_host_intf_read_pkt(adapter, skb->data, skb->len);
  if (status) {
    rsi_dbg(ERR_ZONE, "%s,%d: Failed to read packet from card\n", __func__, __LINE__);
    dev_kfree_skb(skb);
    return status;
  }
  skb_queue_tail(&dev->rx_q.head, skb);
  dev->rx_q.num_rx_pkts++;
  rsi_set_event(&dev->rx_thread.event);
  return 0;
DROP_PKT:
  status = rsi_sdio_host_intf_read_pkt(adapter, dev->temp_rcv_buf, rcv_pkt_len);
  if (status)
    rsi_dbg(ERR_ZONE, "%s,%d: Failed to read packet from card\n", __func__, __LINE__);

  rsi_set_event(&dev->rx_thread.event);
  return 0;
}

/**
 * rsi_init_sdio_slave_regs() - This function does the actual initialization
 *				of SDBUS slave registers.
 * @adapter: Pointer to the adapter structure.
 *
 * Return: status: 0 on success, -1 on failure.
 */
int rsi_init_sdio_slave_regs(struct rsi_hw *adapter)
{
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
  u8 function                 = 0;
  u8 byte;
  int status = 0;

  if (dev->next_read_delay) {
    byte   = dev->next_read_delay;
    status = rsi_sdio_write_register(adapter, function, SDIO_NXT_RD_DELAY2, &byte);
    if (status) {
      rsi_dbg(ERR_ZONE, "%s: Failed to write SDIO_NXT_RD_DELAY2\n", __func__);
      return -1;
    }
  }

  if (dev->sdio_high_speed_enable) {
    rsi_dbg(INIT_ZONE, "%s: Enabling SDIO High speed\n", __func__);
    byte = 0x3;

    status = rsi_sdio_write_register(adapter, function, SDIO_REG_HIGH_SPEED, &byte);
    if (status) {
      rsi_dbg(ERR_ZONE, "%s: Failed to enable SDIO high speed\n", __func__);
      return -1;
    }
  }

  /* This tells SDIO FIFO when to start read to host */
  rsi_dbg(INIT_ZONE, "%s: Initialzing SDIO read start level\n", __func__);
  byte = 0x24;

  status = rsi_sdio_write_register(adapter, function, SDIO_READ_START_LVL, &byte);
  if (status) {
    rsi_dbg(ERR_ZONE, "%s: Failed to write SDIO_READ_START_LVL\n", __func__);
    return -1;
  }

  rsi_dbg(INIT_ZONE, "%s: Initialzing FIFO ctrl registers\n", __func__);
  byte = (128 - 32);

  status = rsi_sdio_write_register(adapter, function, SDIO_READ_FIFO_CTL, &byte);
  if (status) {
    rsi_dbg(ERR_ZONE, "%s: Failed to write SDIO_READ_FIFO_CTL\n", __func__);
    return -1;
  }

  byte   = 32;
  status = rsi_sdio_write_register(adapter, function, SDIO_WRITE_FIFO_CTL, &byte);
  if (status) {
    rsi_dbg(ERR_ZONE, "%s: Failed to write SDIO_WRITE_FIFO_CTL\n", __func__);
    return -1;
  }

  return 0;
}

int rsi_read_intr_status_reg(struct rsi_hw *adapter)
{
  u8 isr_status             = 0;
  struct rsi_common *common = adapter->priv;
  int status;

  status = rsi_sdio_read_register(common->priv, RSI_FN1_INT_REGISTER, &isr_status);
  isr_status &= 0xE;

  if (isr_status & BIT(MSDU_PKT_PENDING))
    adapter->isr_pending = 1;
  return 0;
}

/**
 * rsi_interrupt_handler() - This function read and process SDIO interrupts.
 * @adapter: Pointer to the adapter structure.
 *
 * Return: None.
 */
void rsi_interrupt_handler(struct rsi_hw *adapter)
{
  struct rsi_common *common   = adapter->priv;
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
  int status;
  enum sdio_interrupt_type isr_type;
  u8 isr_status = 0;
  u8 fw_status  = 0;

  dev->rx_info.sdio_int_counter++;
  mutex_lock(&common->rx_lock);
  if (!common->suspend_in_prog) {
    common->rx_in_prog = true;
    mutex_unlock(&common->rx_lock);
  } else {
    rsi_dbg(ERR_ZONE, "%s: Failed to read pkt as suspend in progress:\n", __func__);
    common->rx_in_prog = false;
    mutex_unlock(&common->rx_lock);
  }

  do {
    status = rsi_sdio_read_register(common->priv, RSI_FN1_INT_REGISTER, &isr_status);
    if (status) {
      rsi_dbg(INFO_ZONE, "%s: Failed to Read Intr Status Register\n", __func__);
      mutex_unlock(&common->rx_lock);
      common->rx_in_prog = false;
      return;
    }

    adapter->interrupt_status = isr_status;
    isr_status &= 0xE;

    if (isr_status == 0) {
      rsi_set_event(&common->tx_thread.event);
      dev->rx_info.sdio_intr_status_zero++;
      mutex_unlock(&common->rx_lock);
      common->rx_in_prog = false;
      return;
    }

    //		adapter->interrupt_status = isr_status;
    //		isr_status &= 0xE;

    rsi_dbg(ISR_ZONE,
            "%s: Intr_status = %x %d %d\n",
            __func__,
            isr_status,
            (1 << MSDU_PKT_PENDING),
            (1 << FW_ASSERT_IND));

    do {
      RSI_GET_SDIO_INTERRUPT_TYPE(isr_status, isr_type);

      switch (isr_type) {
        case BUFFER_AVAILABLE:
          rsi_sdio_ack_intr(common->priv, (1 << PKT_BUFF_AVAILABLE));
          status = rsi_sdio_check_buffer_status(adapter, 0);
          if (status < 0) {
            rsi_dbg(ERR_ZONE, "%s: Failed to check buffer status\n", __func__);
            break;
          }
          rsi_set_event(&common->tx_thread.event);

          rsi_dbg(ISR_ZONE, "%s: Buffer full/available\n", __func__);
          dev->buff_status_updated = 1;
          break;

        case FIRMWARE_ASSERT_IND:
          rsi_dbg(ERR_ZONE, "%s: ==> FIRMWARE Assert <==\n", __func__);
          status = rsi_sdio_read_register(common->priv, SDIO_FW_STATUS_REG, &fw_status);
          if (status) {
            rsi_dbg(ERR_ZONE, "%s: Failed to read f/w reg\n", __func__);
          } else {
            rsi_dbg(ERR_ZONE, "%s: Firmware Status is 0x%x\n", __func__, fw_status);
            rsi_sdio_ack_intr(common->priv, (1 << FW_ASSERT_IND));
          }

          common->fsm_state = FSM_CARD_NOT_READY;
          break;

        case MSDU_PACKET_PENDING:
          rsi_dbg(ISR_ZONE, "Pkt pending interrupt\n");
          dev->rx_info.total_sdio_msdu_pending_intr++;

          status = rsi_process_pkt(common);
          if (status) {
            rsi_dbg(ERR_ZONE, "%s: Failed to read pkt\n", __func__);
            mutex_unlock(&common->rx_lock);
            common->rx_in_prog = false;
            return;
          }
          break;
        default:
          rsi_sdio_ack_intr(common->priv, isr_status);
          dev->rx_info.total_sdio_unknown_intr++;
          isr_status = 0;
          rsi_dbg(ISR_ZONE, "Unknown Interrupt %x\n", isr_status);
          break;
      }
      isr_status ^= BIT(isr_type - 1);
    } while (isr_status);
    mutex_unlock(&common->rx_lock);
  } while (1);
  common->rx_in_prog = false;
}

/**
 * rsi_sdio_check_buffer_status() - This function is used to the read
 *					    buffer status register and set
 *					    relevant fields in
 *					    rsi_91x_sdiodev struct.
 * @adapter: Pointer to the driver hw structure.
 * @q_num: The Q number whose status is to be found.
 *
 * Return: status: -1 on failure or else queue full/stop is indicated.
 */
int rsi_sdio_check_buffer_status(struct rsi_hw *adapter, u8 q_num)
{
  struct rsi_common *common   = adapter->priv;
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
  u8 buf_status               = 0;
  int status                  = 0;
  static int counter          = 4;

  if (!dev->buff_status_updated && counter) {
    counter--;
    goto out;
  }

  dev->buff_status_updated = 0;
  status                   = rsi_sdio_read_register(common->priv, RSI_DEVICE_BUFFER_STATUS_REGISTER, &buf_status);
  if (status) {
    rsi_dbg(ERR_ZONE, "%s: Failed to read status register\n", __func__);
    return -1;
  }

  if (buf_status & (BIT(PKT_MGMT_BUFF_FULL))) {
    if (!dev->rx_info.mgmt_buffer_full)
      dev->rx_info.mgmt_buf_full_counter++;
    dev->rx_info.mgmt_buffer_full = true;
  } else
    dev->rx_info.mgmt_buffer_full = false;

  if (buf_status & (BIT(PKT_BUFF_FULL))) {
    if (!dev->rx_info.buffer_full)
      dev->rx_info.buf_full_counter++;
    dev->rx_info.buffer_full = true;
  } else
    dev->rx_info.buffer_full = false;

  if (buf_status & (BIT(PKT_BUFF_SEMI_FULL))) {
    if (!dev->rx_info.semi_buffer_full)
      dev->rx_info.buf_semi_full_counter++;
    dev->rx_info.semi_buffer_full = true;
  } else
    dev->rx_info.semi_buffer_full = false;

  if (dev->rx_info.semi_buffer_full == false)
    counter = 4;
  else
    counter = 1;

out:
  if ((q_num == MGMT_SOFT_Q) && (dev->rx_info.mgmt_buffer_full))
    return QUEUE_FULL;

  if ((q_num < MGMT_SOFT_Q) && (dev->rx_info.buffer_full))
    return QUEUE_FULL;

  return QUEUE_NOT_FULL;
}

/**
 * rsi_sdio_determine_event_timeout() - This Function determines the event
 *					timeout duration.
 * @adapter: Pointer to the adapter structure.
 *
 * Return: timeout duration is returned.
 */
int rsi_sdio_determine_event_timeout(struct rsi_hw *adapter)
{
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;

  /* Once buffer full is seen, event timeout to occur every 2 msecs */
  if (dev->rx_info.buffer_full)
    return 2;

  return EVENT_WAIT_FOREVER;
}
