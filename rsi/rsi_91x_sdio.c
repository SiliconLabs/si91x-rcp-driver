// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <linux/module.h>
#include "rsi_sdio.h"
#include "rsi_common.h"
#include "rsi_hal.h"
#include "rsi_hci.h"
#include "rsi_mgmt.h"

/**
 * rsi_sdio_set_cmd52_arg() - This function prepares cmd 52 read/write arg.
 * @rw: Read/write
 * @func: function number
 * @raw: indicates whether to perform read after write
 * @address: address to which to read/write
 * @writedata: data to write
 *
 * Return: argument
 */
int sdio_clock = 50;
module_param(sdio_clock, int, S_IRUGO);
static u32 rsi_sdio_set_cmd52_arg(bool rw, u8 func, u8 raw, u32 address, u8 writedata)
{
  return ((rw & 1) << 31) | ((func & 0x7) << 28) | ((raw & 1) << 27) | (1 << 26) | ((address & 0x1FFFF) << 9) | (1 << 8)
         | (writedata & 0xFF);
}

static int rsi_sdio_validate_clock(int sdio_clock)
{
  if (sdio_clock > 50 || sdio_clock <= 0) {
    rsi_dbg(ERR_ZONE,
            "%s: Wrong Value given for SDIO clock\n"
            "\t\tSetting default clock to SDIO\n",
            __func__);
    sdio_clock = 50;
  }
  return sdio_clock;
}

/**
 * rsi_cmd52writebyte() - This function issues cmd52 byte write onto the card.
 * @card: Pointer to the mmc_card.
 * @address: Address to write.
 * @byte: Data to write.
 *
 * Return: Write status.
 */
static int rsi_cmd52writebyte(struct mmc_card *card, u32 address, u8 byte, bool expect_resp)
{
  struct mmc_command io_cmd;
  u32 arg;

  memset(&io_cmd, 0, sizeof(io_cmd));
  arg           = rsi_sdio_set_cmd52_arg(1, 0, 0, address, byte);
  io_cmd.opcode = SD_IO_RW_DIRECT;
  io_cmd.arg    = arg;
  io_cmd.flags  = MMC_CMD_AC;
  if (expect_resp)
    io_cmd.flags |= MMC_RSP_R5;

  return mmc_wait_for_cmd(card->host, &io_cmd, 0);
}

/**
 * rsi_cmd52readbyte() - This function issues cmd52 byte read onto the card.
 * @card: Pointer to the mmc_card.
 * @address: Address to read from.
 * @byte: Variable to store read value.
 *
 * Return: Read status.
 */
static int rsi_cmd52readbyte(struct mmc_card *card, u32 address, u8 *byte, bool expect_resp)
{
  struct mmc_command io_cmd;
  u32 arg;
  int err;

  memset(&io_cmd, 0, sizeof(io_cmd));
  arg           = rsi_sdio_set_cmd52_arg(0, 0, 0, address, 0);
  io_cmd.opcode = SD_IO_RW_DIRECT;
  io_cmd.arg    = arg;
  io_cmd.flags  = MMC_CMD_AC;
  if (expect_resp)
    io_cmd.flags |= MMC_RSP_R5;

  err = mmc_wait_for_cmd(card->host, &io_cmd, 0);
  if ((!err) && (byte))
    *byte = io_cmd.resp[0] & 0xFF;
  return err;
}

/**
 * rsi_issue_sdiocommand() - This function issues sdio commands.
 * @func: Pointer to the sdio_func structure.
 * @opcode: Opcode value.
 * @arg: Arguments to pass.
 * @flags: Flags which are set.
 * @resp: Pointer to store response.
 *
 * Return: err: command status as 0 or -1.
 */
static int rsi_issue_sdiocommand(struct sdio_func *func, u32 opcode, u32 arg, u32 flags, u32 *resp)
{
  struct mmc_command cmd;
  struct mmc_host *host;
  int err;

  host = func->card->host;

  memset(&cmd, 0, sizeof(struct mmc_command));
  cmd.opcode = opcode;
  cmd.arg    = arg;
  cmd.flags  = flags;
  err        = mmc_wait_for_cmd(host, &cmd, 3);

  if ((!err) && (resp))
    *resp = cmd.resp[0];

  return err;
}

/**
 * rsi_handle_interrupt() - This function is called upon the occurrence
 *			    of an interrupt.
 * @function: Pointer to the sdio_func structure.
 *
 * Return: None.
 */
static void rsi_handle_interrupt(struct sdio_func *function)
{
  struct rsi_hw *adapter      = sdio_get_drvdata(function);
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;

  if (adapter->priv->fsm_state == FSM_FW_NOT_LOADED)
    return;
  dev->sdio_irq_task = current;
  rsi_interrupt_handler(adapter);
  dev->sdio_irq_task = NULL;
}

static void rsi_gspi_init(struct rsi_hw *adapter)
{
  unsigned long gspi_ctrl_reg0_val;

  /* Programming gspi frequency = soc_frequency / 2 */
  /* Warning : ULP seemed to be not working
	 * well at high frequencies. Modify accordingly */
  gspi_ctrl_reg0_val = 0x4;
  /* csb_setup_time [5:4] */
  gspi_ctrl_reg0_val |= 0x10;
  /* csb_hold_time [7:6] */
  gspi_ctrl_reg0_val |= 0x40;
  /* csb_high_time [9:8] */
  gspi_ctrl_reg0_val |= 0x100;
  /* spi_mode [10] */
  gspi_ctrl_reg0_val |= 0x000;
  /* clock_phase [11] */
  gspi_ctrl_reg0_val |= 0x000;
  /* Initializing GSPI for ULP read/writes */
  rsi_sdio_master_reg_write(adapter, GSPI_CTRL_REG0, gspi_ctrl_reg0_val, 2);
}

static void ulp_read_write(struct rsi_hw *adapter, u16 addr, u16 *data, u16 len_in_bits)
{
  rsi_sdio_master_reg_write(adapter, GSPI_DATA_REG1, ((addr << 6) | (data[1] & 0x3f)), 2);
  rsi_sdio_master_reg_write(adapter, GSPI_DATA_REG0, *(u16 *)&data[0], 2);
  rsi_gspi_init(adapter);
  rsi_sdio_master_reg_write(adapter, GSPI_CTRL_REG1, ((len_in_bits - 1) | GSPI_TRIG), 2);
  msleep(10);
}

static void rsi_reset_chip(struct rsi_hw *adapter)
{
  u16 temp[4] = { 0 };
  u8 *data;
  u8 sdio_interrupt_status = 0;

  data = kzalloc(sizeof(u32), GFP_KERNEL);
  if (!data)
    return;

  put_unaligned_le32(TA_SDIO_WAKE_REQUEST, data);
  rsi_dbg(INFO_ZONE, "Writing disable to wakeup register\n");
  if (rsi_sdio_write_register(adapter, 0, SDIO_WAKEUP_REG, data) < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to Write SDIO WAKEUP REG\n", __func__);
    goto err;
  }
  msleep(3);
  if (rsi_sdio_read_register(adapter, RSI_FN1_INT_REGISTER, &sdio_interrupt_status) < 0) {
    rsi_dbg(INFO_ZONE, "%s: Failed to Read Intr Status Register\n", __func__);
    goto err;
  }
  rsi_dbg(INFO_ZONE, "%s: Intr Status Register value = %d \n", __func__, sdio_interrupt_status);

  /* Put TA on hold */
  if (rsi_sdio_master_access_msword(adapter, 0x2200)) {
    rsi_dbg(ERR_ZONE, "%s: Unable to set ms word to common reg\n", __func__);
    goto err;
  }

  put_unaligned_le32(TA_HOLD_THREAD_VALUE, data);
  if (rsi_sdio_write_register_multiple(adapter, TA_HOLD_THREAD_REG | SD_REQUEST_MASTER, (u8 *)data, 4)) {
    rsi_dbg(ERR_ZONE, "%s: Unable to hold TA threads\n", __func__);
    goto err;
  }

  /* This msleep will ensure TA processor to go to hold and any pending dma
	 * transfers to rf spi in device to finish */
  msleep(100);
  if (DEV_MODEL_9116) {
    if ((rsi_sdio_master_reg_write(adapter, NWP_WWD_INTERRUPT_TIMER, 5, 4)) < 0) {
      rsi_dbg(ERR_ZONE, "Failed to write to intr timer\n");
    }
    if ((rsi_sdio_master_reg_write(adapter, NWP_WWD_SYSTEM_RESET_TIMER, 4, 4)) < 0) {
      rsi_dbg(ERR_ZONE, "Failed to write to system reset timer\n");
    }
    if ((rsi_sdio_master_reg_write(adapter, NWP_WWD_MODE_AND_RSTART, 0xAA0001, 4)) < 0) {
      rsi_dbg(ERR_ZONE, "Failed to write to mode and restart\n");
    }
    rsi_dbg(ERR_ZONE, "***** Watch Dog Reset Successful *****\n");
  } else {
    *(u32 *)temp = 0;
    ulp_read_write(adapter, ULP_RESET_REG, temp, 32);
    *(u32 *)temp = 2;
    ulp_read_write(adapter, WATCH_DOG_TIMER_1, temp, 32);
    *(u32 *)temp = 0;
    ulp_read_write(adapter, WATCH_DOG_TIMER_2, temp, 32);
    *(u32 *)temp = 50;
    ulp_read_write(adapter, WATCH_DOG_DELAY_TIMER_1, temp, 32);
    *(u32 *)temp = 0;
    ulp_read_write(adapter, WATCH_DOG_DELAY_TIMER_2, temp, 32);
    *(u32 *)temp = ((0xaa000) | RESTART_WDT | BYPASS_ULP_ON_WDT);
    ulp_read_write(adapter, WATCH_DOG_TIMER_ENABLE, temp, 32);
  }

  msleep(1000);

err:
  kfree(data);
  return;
}

/**
 * rsi_reset_card() - This function resets and re-initializes the card.
 * @pfunction: Pointer to the sdio_func structure.
 *
 * Return: None.
 */
static void rsi_reset_card(struct sdio_func *pfunction)
{
  int err;
  struct mmc_card *card = pfunction->card;
  struct mmc_host *host = card->host;
  u8 cmd52_resp         = 0;
  u32 clock, i;
  u32 resp = 0;
  u16 rca;
  u32 cmd_delay = 0;

#ifdef CONFIG_CARACALLA_BOARD
  /* Reset chip */
  err = rsi_cmd52writebyte(pfunction->card, SDIO_CCCR_ABORT, (1 << 3), true);

  /* Card will not send any response as it is getting reset immediately
	 * Hence expect a timeout status from host controller
	 */
  if (err != -ETIMEDOUT)
    rsi_dbg(ERR_ZONE, "%s: Reset failed : %d\n", __func__, err);

  cmd_delay = 20;
#else
  cmd_delay = 2;
#endif

  /* Wait for few milli seconds to get rid of residue charges if any */
  msleep(cmd_delay);

  /* Initialize the SDIO card */
  host->ios.chip_select = MMC_CS_DONTCARE;
  host->ios.bus_mode    = MMC_BUSMODE_OPENDRAIN;
  host->ios.power_mode  = MMC_POWER_UP;
  host->ios.bus_width   = MMC_BUS_WIDTH_1;
  host->ios.timing      = MMC_TIMING_LEGACY;
  host->ops->set_ios(host, &host->ios);

  /*
	 * This delay should be sufficient to allow the power supply
	 * to reach the minimum voltage.
	 */
  msleep(cmd_delay);

  host->ios.clock      = host->f_min;
  host->ios.power_mode = MMC_POWER_ON;
  host->ops->set_ios(host, &host->ios);

  /*
	 * This delay must be at least 74 clock sizes, or 1 ms, or the
	 * time required to reach a stable voltage.
	 */
  msleep(cmd_delay);

  /* Issue CMD0. Goto idle state */
  host->ios.chip_select = MMC_CS_HIGH;
  host->ops->set_ios(host, &host->ios);
  msleep(cmd_delay);
  err                   = rsi_issue_sdiocommand(pfunction, MMC_GO_IDLE_STATE, 0, (MMC_RSP_NONE | MMC_CMD_BC), NULL);
  host->ios.chip_select = MMC_CS_DONTCARE;
  host->ops->set_ios(host, &host->ios);
  msleep(cmd_delay);
  host->use_spi_crc = 0;

  if (err)
    rsi_dbg(ERR_ZONE, "%s: CMD0 failed : %d\n", __func__, err);

#ifdef CONFIG_CARACALLA_BOARD
  if (!host->ocr_avail) {
#else
  if (1) {
#endif
    /* Issue CMD5, arg = 0 */
    err = rsi_issue_sdiocommand(pfunction, SD_IO_SEND_OP_COND, 0, (MMC_RSP_R4 | MMC_CMD_BCR), &resp);
    if (err)
      rsi_dbg(ERR_ZONE, "%s: CMD5 failed : %d\n", __func__, err);
#ifdef CONFIG_CARACALLA_BOARD
    host->ocr_avail = resp;
#else
    card->ocr = resp;
#endif
  }

  /* Issue CMD5, arg = ocr. Wait till card is ready  */
  for (i = 0; i < 100; i++) {
    err = rsi_issue_sdiocommand(pfunction,
                                SD_IO_SEND_OP_COND,
#ifdef CONFIG_CARACALLA_BOARD
                                host->ocr_avail,
#else
                                card->ocr,
#endif
                                (MMC_RSP_R4 | MMC_CMD_BCR),
                                &resp);
    if (err) {
      rsi_dbg(ERR_ZONE, "%s: CMD5 failed : %d\n", __func__, err);
      break;
    }

    if (resp & MMC_CARD_BUSY)
      break;
    msleep(cmd_delay);
  }

  if ((i == 100) || (err)) {
    rsi_dbg(ERR_ZONE, "%s: card in not ready : %d %d\n", __func__, i, err);
    return;
  }

  /* Issue CMD3, get RCA */
  err = rsi_issue_sdiocommand(pfunction, SD_SEND_RELATIVE_ADDR, 0, (MMC_RSP_R6 | MMC_CMD_BCR), &resp);
  if (err) {
    rsi_dbg(ERR_ZONE, "%s: CMD3 failed : %d\n", __func__, err);
    return;
  }
  rca                = resp >> 16;
  host->ios.bus_mode = MMC_BUSMODE_PUSHPULL;
  host->ops->set_ios(host, &host->ios);

  /* Issue CMD7, select card  */
  err = rsi_issue_sdiocommand(pfunction, MMC_SELECT_CARD, (rca << 16), (MMC_RSP_R1 | MMC_CMD_AC), NULL);
  if (err) {
    rsi_dbg(ERR_ZONE, "%s: CMD7 failed : %d\n", __func__, err);
    return;
  }

  /* Enable high speed */
  if (card->host->caps & MMC_CAP_SD_HIGHSPEED) {
    rsi_dbg(ERR_ZONE, "%s: Set high speed mode\n", __func__);
    err = rsi_cmd52readbyte(card, SDIO_CCCR_SPEED, &cmd52_resp, true);
    if (err) {
      rsi_dbg(ERR_ZONE, "%s: CCCR speed reg read failed: %d\n", __func__, err);
    } else {
      err = rsi_cmd52writebyte(card, SDIO_CCCR_SPEED, (cmd52_resp | SDIO_SPEED_EHS), true);
      if (err) {
        rsi_dbg(ERR_ZONE, "%s: CCR speed regwrite failed %d\n", __func__, err);
        return;
      }
      host->ios.timing = MMC_TIMING_SD_HS;
      host->ops->set_ios(host, &host->ios);
    }
  }

  /* Set clock */
  if (mmc_card_hs(card))
    clock = (rsi_sdio_validate_clock(sdio_clock) * 1000000);
  else
    clock = card->cis.max_dtr;

  if (clock > host->f_max)
    clock = host->f_max;

  host->ios.clock = clock;
  host->ops->set_ios(host, &host->ios);

  if (card->host->caps & MMC_CAP_4_BIT_DATA) {
    /* CMD52: Set bus width & disable card detect resistor */
    err = rsi_cmd52writebyte(card, SDIO_CCCR_IF, (SDIO_BUS_CD_DISABLE | SDIO_BUS_WIDTH_4BIT), true);
    if (err) {
      rsi_dbg(ERR_ZONE, "%s: Set bus mode failed : %d\n", __func__, err);
      return;
    }
    host->ios.bus_width = MMC_BUS_WIDTH_4;
    host->ops->set_ios(host, &host->ios);
  }
  mdelay(cmd_delay);
}

/**
 * rsi_setclock() - This function sets the clock frequency.
 * @adapter: Pointer to the adapter structure.
 * @freq: Clock frequency.
 *
 * Return: None.
 */
static void rsi_setclock(struct rsi_hw *adapter, u32 freq)
{
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
  struct mmc_host *host       = dev->pfunction->card->host;
  u32 clock;

  clock = freq * 1000;
  if (clock > host->f_max)
    clock = host->f_max;
  host->ios.clock = clock;
  host->ops->set_ios(host, &host->ios);
}

/**
 * rsi_setblocklength() - This function sets the host block length.
 * @adapter: Pointer to the adapter structure.
 * @length: Block length to be set.
 *
 * Return: status: 0 on success, -1 on failure.
 */
static int rsi_setblocklength(struct rsi_hw *adapter, u32 length)
{
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
  int status;

  rsi_dbg(INIT_ZONE, "%s: Setting the block length\n", __func__);

  status                      = sdio_set_block_size(dev->pfunction, length);
  dev->pfunction->max_blksize = 256;

  rsi_dbg(INFO_ZONE, "%s: Operational blk length is %d\n", __func__, length);
  return status;
}

/**
 * rsi_setupcard() - This function queries and sets the card's features.
 * @adapter: Pointer to the adapter structure.
 *
 * Return: status: 0 on success, -1 on failure.
 */
static int rsi_setupcard(struct rsi_hw *adapter)
{
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
  int status                  = 0;

  rsi_setclock(adapter, (rsi_sdio_validate_clock(sdio_clock) * 1000));

  dev->tx_blk_size     = 256;
  adapter->tx_blk_size = dev->tx_blk_size;
  status               = rsi_setblocklength(adapter, dev->tx_blk_size);
  if (status)
    rsi_dbg(ERR_ZONE, "%s: Unable to set block length\n", __func__);
  return status;
}

/**
 * rsi_sdio_read_register() - This function reads one byte of information
 *			      from a register.
 * @adapter: Pointer to the adapter structure.
 * @addr: Address of the register.
 * @data: Pointer to the data that stores the data read.
 *
 * Return: 0 on success, -1 on failure.
 */
int rsi_sdio_read_register(struct rsi_hw *adapter, u32 addr, u8 *data)
{
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
  u8 fun_num                  = 0;
  int status;

  if (likely(dev->sdio_irq_task != current))
    sdio_claim_host(dev->pfunction);

  if (fun_num == 0)
    *data = sdio_f0_readb(dev->pfunction, addr, &status);
  else
    *data = sdio_readb(dev->pfunction, addr, &status);

  if (likely(dev->sdio_irq_task != current))
    sdio_release_host(dev->pfunction);

  return status;
}

/**
 * rsi_sdio_write_register() - This function writes one byte of information
 *			       into a register.
 * @adapter: Pointer to the adapter structure.
 * @function: Function Number.
 * @addr: Address of the register.
 * @data: Pointer to the data tha has to be written.
 *
 * Return: 0 on success, -1 on failure.
 */
int rsi_sdio_write_register(struct rsi_hw *adapter, u8 function, u32 addr, u8 *data)
{
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
  int status                  = 0;

  if (likely(dev->sdio_irq_task != current))
    sdio_claim_host(dev->pfunction);

  if (function == 0)
    sdio_f0_writeb(dev->pfunction, *data, addr, &status);
  else
    sdio_writeb(dev->pfunction, *data, addr, &status);

  if (likely(dev->sdio_irq_task != current))
    sdio_release_host(dev->pfunction);

  return status;
}

/**
 * rsi_sdio_ack_intr() - This function acks the interrupt received.
 * @adapter: Pointer to the adapter structure.
 * @int_bit: Interrupt bit to write into register.
 *
 * Return: None.
 */
void rsi_sdio_ack_intr(struct rsi_hw *adapter, u8 int_bit)
{
  int status;

  status = rsi_sdio_write_register(adapter, 1, (SDIO_FUN1_INTR_CLR_REG | SD_REQUEST_MASTER), &int_bit);
  if (status)
    rsi_dbg(ERR_ZONE, "%s: unable to send ack\n", __func__);
}

/**
 * rsi_sdio_read_register_multiple() - This function read multiple bytes of
 *				       information from the SD card.
 * @adapter: Pointer to the adapter structure.
 * @addr: Address of the register.
 * @count: Number of multiple bytes to be read.
 * @data: Pointer to the read data.
 *
 * Return: 0 on success, -1 on failure.
 */
int rsi_sdio_read_register_multiple(struct rsi_hw *adapter, u32 addr, u8 *data, u16 count)
{
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
  u32 status                  = 0;

  if (likely(dev->sdio_irq_task != current))
    sdio_claim_host(dev->pfunction);

  status = sdio_readsb(dev->pfunction, data, addr, count);

  if (likely(dev->sdio_irq_task != current))
    sdio_release_host(dev->pfunction);

  if (status != 0)
    rsi_dbg(ERR_ZONE, "%s: Synch Cmd53 read failed\n", __func__);
  return status;
}

/**
 * rsi_sdio_write_register_multiple() - This function writes multiple bytes of
 *					information to the SD card.
 * @adapter: Pointer to the adapter structure.
 * @addr: Address of the register.
 * @data: Pointer to the data that has to be written.
 * @count: Number of multiple bytes to be written.
 *
 * Return: 0 on success, -1 on failure.
 */
int rsi_sdio_write_register_multiple(struct rsi_hw *adapter, u32 addr, u8 *data, u16 count)
{
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
  int status;

  if (dev->write_fail > 1) {
    rsi_dbg(ERR_ZONE, "%s: Stopping card writes\n", __func__);
    return 0;
  } else if (dev->write_fail == 1) {
    /**
		 * Assuming it is a CRC failure, we want to allow another
		 *  card write
		 */
    rsi_dbg(ERR_ZONE, "%s: Continue card writes\n", __func__);
    dev->write_fail++;
  }

  if (likely(dev->sdio_irq_task != current))
    sdio_claim_host(dev->pfunction);

  status = sdio_writesb(dev->pfunction, addr, data, count);

  if (likely(dev->sdio_irq_task != current))
    sdio_release_host(dev->pfunction);

#if defined(COMMAND_84_ERROR_WAR)
  if (status && status != -84) {
#else
  if (status) {
#endif
    rsi_dbg(ERR_ZONE, "%s: Synch Cmd53 write failed %d\n", __func__, status);
    dev->write_fail = 2;
  } else {
#if defined(COMMAND_84_ERROR_WAR)
    if (status == -84) {
      printk("Continue Even after -84 error \n");
      status = 0;
    }
#endif
    memcpy(dev->prev_desc, data, FRAME_DESC_SZ);
  }
  return status;
}

int rsi_sdio_load_data_master_write(struct rsi_hw *adapter,
                                    u32 base_address,
                                    u32 instructions_sz,
                                    u16 block_size,
                                    u8 *ta_firmware)
{
  u32 num_blocks;
  u16 msb_address;
  u32 offset, ii;
  u8 *temp_buf;
  u16 lsb_address;

  temp_buf = kzalloc(block_size, GFP_KERNEL);
  if (!temp_buf)
    return -ENOMEM;

  num_blocks  = instructions_sz / block_size;
  msb_address = base_address >> 16;

  rsi_dbg(INFO_ZONE, "ins_size: %d\n", instructions_sz);
  rsi_dbg(INFO_ZONE, "num_blocks: %d\n", num_blocks);

  /* Loading DM ms word in the sdio slave */
  if (rsi_sdio_master_access_msword(adapter, msb_address)) {
    rsi_dbg(ERR_ZONE, "%s: Unable to set ms word reg\n", __func__);
    goto err;
  }

  for (offset = 0, ii = 0; ii < num_blocks; ii++, offset += block_size) {
    memset(temp_buf, 0, block_size);
    memcpy(temp_buf, ta_firmware + offset, block_size);
    lsb_address = (u16)base_address;
    if (rsi_sdio_write_register_multiple(adapter, lsb_address | SD_REQUEST_MASTER, temp_buf, block_size)) {
      rsi_dbg(ERR_ZONE, "%s: failed to write\n", __func__);
      goto err;
    }
    rsi_dbg(INFO_ZONE, "%s: loading block: %d\n", __func__, ii);
    base_address += block_size;

    if ((base_address >> 16) != msb_address) {
      msb_address += 1;

      /* Loading DM ms word in the sdio slave */
      if (rsi_sdio_master_access_msword(adapter, msb_address)) {
        rsi_dbg(ERR_ZONE, "%s: Unable to set ms word reg\n", __func__);
        goto err;
      }
    }
  }

  if (instructions_sz % block_size) {
    memset(temp_buf, 0, block_size);
    memcpy(temp_buf, ta_firmware + offset, instructions_sz % block_size);
    lsb_address = (u16)base_address;
    if (rsi_sdio_write_register_multiple(adapter,
                                         lsb_address | SD_REQUEST_MASTER,
                                         temp_buf,
                                         instructions_sz % block_size)) {
      goto err;
    }
    rsi_dbg(INFO_ZONE, "Written Last Block in Address 0x%x Successfully\n", offset | SD_REQUEST_MASTER);
  }
  kfree(temp_buf);
  return 0;

err:
  kfree(temp_buf);
  return -EIO;
}

int rsi_sdio_master_reg_read(struct rsi_hw *adapter, u32 addr, u32 *read_buf, u16 size)
{
  u32 *data   = NULL;
  u16 ms_addr = 0;
  u32 addr_on_bus;

  data = kzalloc(SDIO_BLOCK_SIZE, GFP_KERNEL);
  if (!data)
    return -ENOMEM;

  ms_addr = (addr >> 16);
  if (rsi_sdio_master_access_msword(adapter, ms_addr)) {
    rsi_dbg(ERR_ZONE, "%s: Unable to set ms word to common reg\n", __func__);
    kfree(data);
    return -EIO;
  }
  addr = addr & 0xFFFF;

  addr_on_bus = (addr & 0xFF000000);
  if ((addr_on_bus == (FLASH_SIZE_ADDR & 0xFF000000)) || (addr_on_bus == 0x0)) {
    addr_on_bus = (addr & ~(0x3));
  } else
    addr_on_bus = addr;

  /* Bring TA out of reset */
  if (rsi_sdio_read_register_multiple(adapter, (addr_on_bus | SD_REQUEST_MASTER), (u8 *)data, SDIO_BLOCK_SIZE)) {
    rsi_dbg(ERR_ZONE, "%s: AHB register read failed\n", __func__);
    kfree(data);
    return -EIO;
  }
  if (size == 2) {
    if ((addr & 0x3) == 0)
      *read_buf = *data;
    else
      *read_buf = (*data >> 16);
    *read_buf = (*read_buf & 0xFFFF);
  } else if (size == 1) {
    if ((addr & 0x3) == 0)
      *read_buf = *data;
    else if ((addr & 0x3) == 1)
      *read_buf = (*data >> 8);
    else if ((addr & 0x3) == 2)
      *read_buf = (*data >> 16);
    else
      *read_buf = (*data >> 24);
    *read_buf = (*read_buf & 0xFF);
  } else { /*size is 4 */
    *read_buf = *data;
  }

  kfree(data);
  return 0;
}

int rsi_sdio_master_reg_write(struct rsi_hw *adapter, unsigned long addr, unsigned long data, u16 size)
{
  unsigned long *data_aligned;

  data_aligned = kzalloc(RSI_MASTER_REG_BUF_SIZE, GFP_KERNEL);
  if (!data_aligned)
    return -ENOMEM;

  if (size == 2) {
    *data_aligned = ((data << 16) | (data & 0xFFFF));
  } else if (size == 1) {
    u32 temp_data;

    temp_data     = (data & 0xFF);
    *data_aligned = ((temp_data << 24) | (temp_data << 16) | (temp_data << 8) | (temp_data));
  } else {
    *data_aligned = data;
  }
  size = 4;

  if (rsi_sdio_master_access_msword(adapter, (addr >> 16))) {
    rsi_dbg(ERR_ZONE, "%s: Unable to set ms word to common reg\n", __func__);
    kfree(data_aligned);
    return -EIO;
  }
  addr = addr & 0xFFFF;

  /* Bring TA out of reset */
  if (rsi_sdio_write_register_multiple(adapter, (addr | SD_REQUEST_MASTER), (u8 *)data_aligned, size)) {
    rsi_dbg(ERR_ZONE, "%s: Unable to do AHB reg write\n", __func__);
    kfree(data_aligned);
    return -EIO;
  }

  kfree(data_aligned);
  return 0;
}

/**
 * rsi_sdio_host_intf_write_pkt() - This function writes the packet to device.
 * @adapter: Pointer to the adapter structure.
 * @pkt: Pointer to the data to be written on to the device.
 * @len: length of the data to be written on to the device.
 *
 * Return: 0 on success, -1 on failure.
 */
int rsi_sdio_host_intf_write_pkt(struct rsi_hw *adapter, u8 *pkt, u32 len)
{
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
  u32 block_size              = dev->tx_blk_size;
  u32 num_blocks, address, length;
  u32 queueno;
  int status;
#ifdef CONFIG_RSI_NO_SDIO_MULTIBLOCK
  u8 *tbuf = pkt;
  u32 remain_length;
#endif

  queueno = ((pkt[1] >> 4) & 0xf);
  if ((queueno == RSI_BT_DATA_Q) || (queueno == RSI_BT_MGMT_Q))
    queueno = RSI_BT_Q;

  num_blocks = len / block_size;

  if (len % block_size)
    num_blocks++;

  address = (num_blocks * block_size | (queueno << 12));
  length  = num_blocks * block_size;

#ifndef CONFIG_RSI_NO_SDIO_MULTIBLOCK
  status = rsi_sdio_write_register_multiple(adapter, address, pkt, length);
#else
  remain_length = length;
  do {
    if (remain_length == length)
      status = rsi_sdio_write_register_multiple(adapter, address, tbuf, 256);
    else
      status = rsi_sdio_write_register_multiple(adapter, 0, tbuf, 256);
    if (status)
      break;
    if (remain_length > 256)
      remain_length -= 256;
    else
      break;
    tbuf += 256;
  } while (1);
#endif
  if (status < 0)
    rsi_dbg(ERR_ZONE, "%s: Unable to write onto the card: %d\n", __func__, status);
  rsi_dbg(DATA_TX_ZONE, "%s: Successfully written onto card\n", __func__);

  return status;
}

/**
 * rsi_sdio_host_intf_read_pkt() - This function reads the packet
				   from the device.
 * @adapter: Pointer to the adapter data structure.
 * @pkt: Pointer to the packet data to be read from the the device.
 * @length: Length of the data to be read from the device.
 *
 * Return: 0 on success, -1 on failure.
 */
int rsi_sdio_host_intf_read_pkt(struct rsi_hw *adapter, u8 *pkt, u32 length)
{
  int status = -EINVAL;

  if (!length) {
    rsi_dbg(ERR_ZONE, "%s: Pkt size is zero\n", __func__);
    return status;
  }

  status = rsi_sdio_read_register_multiple(adapter, length, (u8 *)pkt, length);

  if (status)
    rsi_dbg(ERR_ZONE, "%s: Failed to read frame: %d\n", __func__, status);
  return status;
}

static int rsi_sdio_ta_reset_ops(struct rsi_hw *adapter)
{
  u8 *data;

  data = kzalloc(sizeof(u32), GFP_KERNEL);
  if (!data)
    return -ENOMEM;

  if (rsi_sdio_master_access_msword(adapter, 0x2200) < 0) {
    rsi_dbg(ERR_ZONE, "Unable to set ms word to common reg\n");
    goto err;
  }

  rsi_dbg(INIT_ZONE, "%s: Bringing TA Out of Reset\n", __func__);
  put_unaligned_le32(TA_HOLD_THREAD_VALUE, data);

  /* Bringing TA out of reset */
  if (rsi_sdio_write_register_multiple(adapter, TA_HOLD_THREAD_REG | SD_REQUEST_MASTER, data, 4) < 0) {
    rsi_dbg(ERR_ZONE, "Unable to hold TA threads\n");
    goto err;
  }
  /* Bringing TA out of reset */
  put_unaligned_le32(TA_SOFT_RST_CLR, data);
  if (rsi_sdio_write_register_multiple(adapter, TA_SOFT_RESET_REG | SD_REQUEST_MASTER, data, 4) < 0) {
    rsi_dbg(ERR_ZONE, "Unable to get TA out of reset state\n");
    goto err;
  }

  /* Assuming TA will go to hold by this time
	 * If you find that TA is not in hold by this time
	 * in any chip or any project, then wait till TA goes to
	 * hold by polling poll_status register.
	 **/
  /* Bringing TA out of reset */
  put_unaligned_le32(TA_PC_ZERO, data);
  if (rsi_sdio_write_register_multiple(adapter, TA_TH0_PC_REG | SD_REQUEST_MASTER, data, 4) < 0) {
    rsi_dbg(ERR_ZONE, "Unable to Reset TA PC value\n");
    goto err;
  }
  put_unaligned_le32(TA_RELEASE_THREAD_VALUE, data);
  /* Bringing TA out of reset */
  if (rsi_sdio_write_register_multiple(adapter, TA_RELEASE_THREAD_REG | SD_REQUEST_MASTER, data, 4) < 0) {
    rsi_dbg(ERR_ZONE, "Unable to release TA threads\n");
    goto err;
  }
  if (rsi_sdio_master_access_msword(adapter, 0x4105) < 0) {
    rsi_dbg(ERR_ZONE, "Unable to set ms word to common reg\n");
    goto err;
  }

  rsi_dbg(INIT_ZONE, "Setting ms word to common reg 0x41050000\n");
  kfree(data);
  return 0;

err:
  kfree(data);
  return -EINVAL;
}

int rsi_sdio_reinit_device(struct rsi_hw *adapter)
{
  struct rsi_91x_sdiodev *sdev = adapter->rsi_dev;
  struct sdio_func *pfunction  = sdev->pfunction;
  int ii;

  /* Flush soft queues */
  for (ii = 0; ii < NUM_SOFT_QUEUES; ii++)
    skb_queue_purge(&adapter->priv->tx_queue[ii]);

  /* Initialize device again */
  sdio_claim_host(pfunction);

  sdio_release_irq(pfunction);
  rsi_reset_card(pfunction);

  sdio_enable_func(pfunction);
  rsi_setupcard(adapter);
  rsi_init_sdio_slave_regs(adapter);
  sdio_claim_irq(pfunction, rsi_handle_interrupt);
  rsi_hal_device_init(adapter);

  sdio_release_host(pfunction);

  return 0;
}

/**
 * rsi_init_sdio_interface() - This function does init specific to SDIO.
 *
 * @adapter: Pointer to the adapter data structure.
 * @pkt: Pointer to the packet data to be read from the the device.
 *
 * Return: 0 on success, -1 on failure.
 */

static int rsi_init_sdio_interface(struct rsi_hw *adapter, struct sdio_func *pfunction)
{
  struct rsi_91x_sdiodev *rsi_91x_dev;
  int status = -ENOMEM;

  rsi_91x_dev = kzalloc(sizeof(*rsi_91x_dev), GFP_KERNEL);
  if (!rsi_91x_dev)
    return status;

  adapter->rsi_dev           = rsi_91x_dev;
  rsi_91x_dev->sdio_irq_task = NULL;

  sdio_claim_host(pfunction);

  pfunction->enable_timeout = 100;
  status                    = sdio_enable_func(pfunction);
  if (status) {
    rsi_dbg(ERR_ZONE, "%s: Failed to enable interface\n", __func__);
    sdio_release_host(pfunction);
    return status;
  }

  rsi_dbg(INIT_ZONE, "%s: Enabled the interface\n", __func__);

  rsi_91x_dev->pfunction = pfunction;
  adapter->device        = &pfunction->dev;

  sdio_set_drvdata(pfunction, adapter);

  status = rsi_setupcard(adapter);
  if (status) {
    rsi_dbg(ERR_ZONE, "%s: Failed to setup card\n", __func__);
    goto fail;
  }

  rsi_dbg(INIT_ZONE, "%s: Setup card successfully\n", __func__);

  status = rsi_init_sdio_slave_regs(adapter);
  if (status) {
    rsi_dbg(ERR_ZONE, "%s: Failed to init slave regs\n", __func__);
    goto fail;
  }
  sdio_release_host(pfunction);

  adapter->determine_event_timeout = rsi_sdio_determine_event_timeout;
  adapter->process_isr_hci         = rsi_interrupt_handler;
  adapter->check_intr_status_reg   = rsi_read_intr_status_reg;

  return status;
fail:
  sdio_disable_func(pfunction);
  sdio_release_host(pfunction);
  return status;
}

static struct rsi_host_intf_ops sdio_host_intf_ops = {
  .write_pkt              = rsi_sdio_host_intf_write_pkt,
  .read_pkt               = rsi_sdio_host_intf_read_pkt,
  .master_access_msword   = rsi_sdio_master_access_msword,
  .master_reg_read        = rsi_sdio_master_reg_read,
  .reg_read               = rsi_sdio_read_register,
  .master_reg_write       = rsi_sdio_master_reg_write,
  .read_reg_multiple      = rsi_sdio_read_register_multiple,
  .write_reg_multiple     = rsi_sdio_write_register_multiple,
  .load_data_master_write = rsi_sdio_load_data_master_write,
  .check_hw_queue_status  = rsi_sdio_check_buffer_status,
  .ta_reset_ops           = rsi_sdio_ta_reset_ops,
  .reinit_device          = rsi_sdio_reinit_device,
};

/**
 * rsi_probe() - This function is called by kernel when the driver provided
 *		 Vendor and device IDs are matched. All the initialization
 *		 work is done here.
 * @pfunction: Pointer to the sdio_func structure.
 * @id: Pointer to sdio_device_id structure.
 *
 * Return: 0 on success, 1 on failure.
 */
static int rsi_probe(struct sdio_func *pfunction, const struct sdio_device_id *id)
{
  struct rsi_hw *adapter;
  struct rsi_91x_sdiodev *sdev;
  struct rsi_common *common;
  int status;

  rsi_dbg(INIT_ZONE, "%s: Init function called\n", __func__);

  if (ps_sleep_type == 0) {
    ps_sleep_type = ULP_POWER_SAVE;
  }
  adapter = rsi_91x_init();
  if (!adapter) {
    rsi_dbg(ERR_ZONE, "%s: Failed to init os intf ops\n", __func__);
    return 1;
  }

  common                 = adapter->priv;
  adapter->rsi_host_intf = RSI_HOST_INTF_SDIO;
  adapter->host_intf_ops = &sdio_host_intf_ops;
#ifdef CONFIG_RSI_MULTI_MODE
  if (rsi_opermode_instances(adapter)) {
    rsi_dbg(ERR_ZONE, "%s: Invalid operating modes\n", __func__);
    goto fail_free_adapter;
  }
#else
  adapter->priv->oper_mode = common->dev_oper_mode;
  if (rsi_validate_oper_mode(common->dev_oper_mode)) {
    rsi_dbg(ERR_ZONE, "%s: Invalid operating mode %d\n", __func__, common->dev_oper_mode);
    goto fail_free_adapter;
  }

#endif

  if (rsi_init_sdio_interface(adapter, pfunction)) {
    rsi_dbg(ERR_ZONE, "%s: Failed to init sdio interface\n", __func__);
    goto fail_free_adapter;
  }

  rsi_dbg(INFO_ZONE, "Vendor Id:%x, Device Id:%x\n", pfunction->vendor, pfunction->device);
  if ((pfunction->device == 0X9330)) {
    rsi_dbg(ERR_ZONE, "%s: ***** 9113 Module *****\n", __func__);
    adapter->device_model = RSI_DEV_9113;
  } else if ((pfunction->device == 0X9116)) {
    rsi_dbg(ERR_ZONE, "%s: ***** 9116 Module *****\n", __func__);
    adapter->device_model = RSI_DEV_9116;
    //__9117_CODE_START
  } else if ((pfunction->device == 0x9117)) {
    rsi_dbg(ERR_ZONE, "%s: ***** 9117_A0 Module *****\n", __func__);
    adapter->device_model = RSI_DEV_9117;
  } else if ((pfunction->device == 0x917)) {
    rsi_dbg(ERR_ZONE, "%s: ***** 9117_B0 Module *****\n", __func__);
    adapter->device_model = RSI_DEV_917;
    //__9117_CODE_END
  } else {
    rsi_dbg(ERR_ZONE, "##### Invalid RSI device id 0x%x\n", pfunction->device);
    goto fail_free_adapter;
  }

  /* Initialize receive path */
  sdev = adapter->rsi_dev;
  rsi_init_event(&sdev->rx_thread.event);
  status = rsi_create_kthread(adapter->priv, &sdev->rx_thread, rsi_sdio_rx_thread, "SDIO-RX-Thread");
  if (status) {
    rsi_dbg(ERR_ZONE, "%s: Unable to init rx thrd\n", __func__);
    goto fail_kill_thread;
  }
  skb_queue_head_init(&sdev->rx_q.head);
  sdev->rx_q.num_rx_pkts = 0;

  /*Receive buffer for handling RX interrupts in case of memory full*/
  sdev->temp_rcv_buf = kzalloc((RCV_BUFF_LEN * 4), GFP_KERNEL);

#ifdef CONFIG_SDIO_INTR_POLL
  init_sdio_intr_status_poll_thread(adapter->priv);
#endif
  sdio_claim_host(pfunction);
  if (sdio_claim_irq(pfunction, rsi_handle_interrupt)) {
    rsi_dbg(ERR_ZONE, "%s: Failed to request IRQ\n", __func__);
    sdio_release_host(pfunction);
    goto fail_claim_irq;
  }
  sdio_release_host(pfunction);
  rsi_dbg(INIT_ZONE, "%s: Registered Interrupt handler\n", __func__);

#ifdef CONFIG_RSI_DEBUGFS
  //__9117_CODE_START
  if (adapter->device_model >= RSI_DEV_9117)
    adapter->num_debugfs_entries = MAX_DEBUGFS_ENTRIES_917;
  else
    //__9117_CODE_END
    adapter->num_debugfs_entries = MAX_DEBUGFS_ENTRIES;
#endif

#ifndef NO_FIRMWARE_LOAD_SUPPORT
  if (rsi_hal_device_init(adapter)) {
    rsi_dbg(ERR_ZONE, "%s: Failed in device init\n", __func__);
    goto fail_dev_init;
  }
  rsi_dbg(INFO_ZONE, "===> RSI Device Init Done <===\n");

  if (rsi_sdio_master_access_msword(adapter, MISC_CFG_BASE_ADDR)) {
    rsi_dbg(ERR_ZONE, "%s: Unable to set ms word reg\n", __func__);
    goto fail_dev_init;
  }
  rsi_dbg(INIT_ZONE, "%s: Setting ms word to 0x41050000\n", __func__);
#endif

  adapter->priv->hibernate_resume = false;
#ifdef NO_FIRMWARE_LOAD_SUPPORT
  adapter->common_hal_fsm      = COMMON_HAL_TX_ACCESS;
  common->common_hal_tx_access = true;
#endif

#if defined(CONFIG_ARCH_HAVE_CUSTOM_GPIO_H)
  if (common->ulp_ps_handshake_mode == GPIO_HAND_SHAKE)
    gpio_init(common);
#endif
  return 0;

#ifndef NO_FIRMWARE_LOAD_SUPPORT
fail_dev_init:
  sdio_claim_host(pfunction);
  sdio_release_irq(pfunction);
  sdio_release_host(pfunction);
#endif
fail_claim_irq:
  rsi_kill_thread(&sdev->rx_thread);
  kfree(sdev->temp_rcv_buf);
fail_kill_thread:
  sdio_claim_host(pfunction);
  sdio_disable_func(pfunction);
  sdio_release_host(pfunction);
fail_free_adapter:
#ifdef CONFIG_SDIO_INTR_POLL
  rsi_kill_thread(&adapter->priv->sdio_intr_poll_thread);
#endif
  rsi_91x_deinit(adapter);
  rsi_dbg(ERR_ZONE, "%s: Failed in probe...Exiting\n", __func__);
  return 1;
}

/**
 * rsi_disconnect() - This function performs the reverse of the probe function.
 * @pfunction: Pointer to the sdio_func structure.
 *
 * Return: void.
 */
static void rsi_disconnect(struct sdio_func *pfunction)
{
  struct rsi_hw *adapter    = sdio_get_drvdata(pfunction);
  struct rsi_common *common = adapter->priv;
  struct rsi_91x_sdiodev *dev;
  //__9117_CODE_START
  struct rsi_host_intf_ops *hif_ops = adapter->host_intf_ops;
  //__9117_CODE_END
  int status;

  if (!adapter)
    return;

  dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;

  status = set_clr_tx_intention(common, COMMON_ID, 1);
  if (status) {
    rsi_dbg(ERR_ZONE, "%s,%d:  Failed to get tx_access\n", __func__, __LINE__);
    rsi_dbg(ERR_ZONE, "%s,%d:  Waiting for sleep_exit \n", __func__, __LINE__);
    rsi_reset_event(&common->cfm_sleep_exit);
    rsi_wait_event(&common->cfm_sleep_exit, EVENT_WAIT_FOREVER);
  }

  //__9117_CODE_START
  if ((hif_ops->master_reg_write(adapter, TASS_FLASH_LDO_REGISTER, FLASH_LDO_ON, 4)) < 0) {
    rsi_dbg(ERR_ZONE, "%s: Power on LDO Flash failed\n", __func__);
  }
//__9117_CODE_END
#ifdef CONFIG_SDIO_INTR_POLL
  rsi_kill_thread(&adapter->priv->sdio_intr_poll_thread);
#endif
  rsi_kill_thread(&dev->rx_thread);

  sdio_claim_host(pfunction);
  sdio_release_irq(pfunction);
  sdio_release_host(pfunction);
  kfree(dev->temp_rcv_buf);

  rsi_mac80211_detach(adapter);

#if defined(CONFIG_RSI_BT_ALONE) || defined(CONFIG_RSI_COEX_MODE)
  if ((adapter->priv->coex_mode == 2) || (adapter->priv->coex_mode == 4))
    rsi_hci_detach(adapter->priv);
#endif

  /* Reset Chip */
  rsi_reset_chip(adapter);

  /* Resetting to take care of the case, where-in driver
	 * is re-loaded */
  sdio_claim_host(pfunction);
  rsi_reset_card(pfunction);
  sdio_disable_func(pfunction);
  sdio_release_host(pfunction);
  dev->write_fail = 2;
  rsi_91x_deinit(adapter);

  rsi_dbg(ERR_ZONE, "##### RSI SDIO device disconnected #####\n");
}

#ifdef CONFIG_PM
static int rsi_set_sdio_pm_caps(struct rsi_hw *adapter)
{
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
  struct sdio_func *func      = dev->pfunction;
  int ret;
  mmc_pm_flag_t flags;
  flags = sdio_get_host_pm_caps(dev->pfunction);
  if (!(flags & MMC_PM_KEEP_POWER)) {
    rsi_dbg(ERR_ZONE, "%s: Returning failure for suspend as SDIO can not keep power\n", __func__);
    return -ENOSYS;
  }
  /* Keep Power to the MMC while suspend */
  ret = sdio_set_host_pm_flags(func, MMC_PM_KEEP_POWER);
  if (ret) {
    rsi_dbg(ERR_ZONE, "set sdio keep pwr flag failed: %d\n", ret);
    return ret;
  }

  return ret;
}

static int rsi_sdio_disable_interrupts(struct sdio_func *pfunction)
{
  struct rsi_hw *adapter = sdio_get_drvdata(pfunction);
  u8 isr_status = 0, data = 0;
  int ret;

  rsi_dbg(ERR_ZONE, "Waiting for interrupts to be cleared..");
  do {
    rsi_sdio_read_register(adapter, RSI_FN1_INT_REGISTER, &isr_status);
    rsi_dbg(ERR_ZONE, ".");
  } while (isr_status);
  rsi_dbg(ERR_ZONE, "\nInterrupts cleared");

  sdio_claim_host(pfunction);
  ret = rsi_cmd52readbyte(pfunction->card, 0x04, &data, false);
  if (ret < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to read INTR_EN register\n", __func__);
    sdio_release_host(pfunction);
    return ret;
  }
  rsi_dbg(INFO_ZONE, "INTR_EN reg content = %x\n", data);

  /* And bit0 and b1 */
  data &= 0xfc;

  ret = rsi_cmd52writebyte(pfunction->card, 0x04, data, false);
  if (ret < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to Write to INTR_EN register\n", __func__);
    sdio_release_host(pfunction);
    return ret;
  }
  ret = rsi_cmd52readbyte(pfunction->card, 0x04, &data, false);
  if (ret < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to read INTR_EN register\n", __func__);
    sdio_release_host(pfunction);
    return ret;
  }
  rsi_dbg(INFO_ZONE, "INTR_EN reg content. = %x\n", data);

  sdio_release_host(pfunction);

  return 0;
}

static int rsi_sdio_enable_interrupts(struct sdio_func *pfunction)
{
  u8 data = 0;
  int ret;

  sdio_claim_host(pfunction);
  ret = rsi_cmd52readbyte(pfunction->card, 0x04, &data, false);
  if (ret < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to read INTR_EN register\n", __func__);
    sdio_release_host(pfunction);
    return ret;
  }
  rsi_dbg(INFO_ZONE, "INTR_EN reg content1 = %x\n", data);

  /* Enable b1 and b0 */
  data |= 0x03;

  ret = rsi_cmd52writebyte(pfunction->card, 0x04, data, false);
  if (ret < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to Write to INTR_EN register\n", __func__);
    sdio_release_host(pfunction);
    return ret;
  }

  ret = rsi_cmd52readbyte(pfunction->card, 0x04, &data, false);
  if (ret < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to read INTR_EN register\n", __func__);
    sdio_release_host(pfunction);
    return ret;
  }
  rsi_dbg(INFO_ZONE, "INTR_EN reg content1.. = %x\n", data);
  sdio_release_host(pfunction);

  return ret;
}
static int rsi_suspend(struct device *dev)
{
  int ret                      = 0;
  struct sdio_func *pfunction  = dev_to_sdio_func(dev);
  struct rsi_hw *adapter       = sdio_get_drvdata(pfunction);
  struct rsi_common *common    = adapter->priv;
  struct rsi_91x_sdiodev *sdev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
  u16 sleep_time               = 200;
  rsi_dbg(ERR_ZONE, "SDIO Bus suspend ===>\n");

  if (!adapter) {
    rsi_dbg(ERR_ZONE, "Device is not ready\n");
    return -ENODEV;
  }

  rsi_mac80211_hw_scan_cancel(adapter->hw, adapter->priv->scan_vif);
CHECK_AGAIN:
  mutex_lock(&common->rx_lock);
  down(&common->tx_access_lock);
  if ((!protocol_tx_access(common)) && (!common->rx_in_prog) && (adapter->ps_state != PS_DISABLE_REQ_SENT)
      && (adapter->ps_state != PS_ENABLE_REQ_SENT)) {
    if (adapter->ps_state == PS_NONE)
      rsi_dbg(INFO_ZONE, "Device Power save not enabled\n");

    rsi_dbg(ERR_ZONE, "Triggering Suspend\n");
    common->suspend_in_prog = true;
    up(&common->tx_access_lock);
    mutex_unlock(&common->rx_lock);
  } else {
    up(&common->tx_access_lock);
    mutex_unlock(&common->rx_lock);
    if (sleep_time) {
      msleep(10);
      sleep_time -= 10;
      goto CHECK_AGAIN;
    } else {
      rsi_dbg(ERR_ZONE, ("Returning failure for suspend as TX/RX is pending even after 200ms wait\n"));
      return -ENOSYS;
    }
  }
#ifdef CONFIG_RSI_WOW
  if ((common->wow_flags & RSI_WOW_ENABLED) && (common->wow_flags & RSI_WOW_NO_CONNECTION))
    rsi_dbg(ERR_ZONE, "##### Device can not wake up through WLAN\n");

#endif

  //ret = rsi_sdio_disable_interrupts(pfunction);

  if (sdev->write_fail)
    rsi_dbg(INFO_ZONE, "###### Device is not ready #######\n");

  ret = rsi_set_sdio_pm_caps(adapter);
  if (ret)
    rsi_dbg(INFO_ZONE, "Setting power management caps failed\n");

  common->fsm_state = FSM_CARD_NOT_READY;
  rsi_dbg(INFO_ZONE, "***** RSI module suspended ******\n");

  return 0;
}

static int rsi_resume(struct device *dev)
{
  //int ret = 0;
  struct sdio_func *pfunction = dev_to_sdio_func(dev);
  struct rsi_hw *adapter      = sdio_get_drvdata(pfunction);
  struct rsi_common *common   = adapter->priv;

  rsi_dbg(INFO_ZONE, "SDIO Bus resume =====>\n");

  common->suspend_in_prog = false;
  common->fsm_state       = FSM_MAC_INIT_DONE;

  //ret = rsi_sdio_enable_interrupts(pfunction);

  rsi_dbg(INFO_ZONE, "***** RSI module resumed *****\n");

  return 0;
}

static int rsi_freeze(struct device *dev)
{
  int ret                     = 0;
  struct sdio_func *pfunction = dev_to_sdio_func(dev);
  struct rsi_hw *adapter      = sdio_get_drvdata(pfunction);
  struct rsi_common *common;
  struct rsi_91x_sdiodev *sdev;

  rsi_dbg(INFO_ZONE, "SDIO Bus freeze ===>\n");

  if (!adapter) {
    rsi_dbg(ERR_ZONE, "Device is not ready\n");
    return -ENODEV;
  }
  common = adapter->priv;
  sdev   = (struct rsi_91x_sdiodev *)adapter->rsi_dev;

  common->suspend_in_prog = true;
#ifdef CONFIG_RSI_WOW
  if ((common->wow_flags & RSI_WOW_ENABLED) && (common->wow_flags & RSI_WOW_NO_CONNECTION))
    rsi_dbg(ERR_ZONE, "##### Device can not wake up through WLAN\n");
#endif
#if defined(CONFIG_RSI_BT_ALONE) || defined(CONFIG_RSI_COEX_MODE)
  rsi_hci_detach(common);
#endif

  ret = rsi_sdio_disable_interrupts(pfunction);

  if (sdev->write_fail)
    rsi_dbg(INFO_ZONE, "###### Device is not ready #######\n");

  ret = rsi_set_sdio_pm_caps(adapter);
  if (ret)
    rsi_dbg(INFO_ZONE, "Setting power management caps failed\n");

  rsi_dbg(INFO_ZONE, "***** RSI module freezed *****\n");

  return 0;
}

static int rsi_thaw(struct device *dev)
{
  struct sdio_func *pfunction = dev_to_sdio_func(dev);
  struct rsi_hw *adapter      = sdio_get_drvdata(pfunction);

  rsi_dbg(ERR_ZONE, "SDIO Bus thaw =====>\n");

  adapter->priv->hibernate_resume = true;
  adapter->priv->fsm_state        = FSM_CARD_NOT_READY;
  adapter->priv->bt_fsm_state     = BT_DEVICE_NOT_READY;
  adapter->priv->iface_down       = true;

  rsi_sdio_enable_interrupts(pfunction);

  rsi_dbg(INFO_ZONE, "***** RSI module thaw done *****\n");

  return 0;
}

static int rsi_poweroff(struct device *dev)
{
  return rsi_freeze(dev);
}

static void rsi_shutdown(struct device *dev)
{
  struct sdio_func *pfunction  = dev_to_sdio_func(dev);
  struct rsi_hw *adapter       = sdio_get_drvdata(pfunction);
  struct rsi_91x_sdiodev *sdev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;
#ifdef CONFIG_RSI_WOW
  struct ieee80211_hw *hw        = adapter->hw;
  struct cfg80211_wowlan *wowlan = NULL;
#endif

  rsi_dbg(ERR_ZONE, "SDIO Bus shutdown =====>\n");

  adapter->priv->suspend_in_prog = true;

#ifdef CONFIG_RSI_WOW
  if (hw) {
    wowlan = hw->wiphy->wowlan_config;
    if (rsi_config_wowlan(adapter, wowlan))
      rsi_dbg(ERR_ZONE, "Failed to configure WoWLAN\n");
  }
#endif

#if defined(CONFIG_RSI_BT_ALONE) || defined(CONFIG_RSI_COEX_MODE)
  rsi_hci_detach(adapter->priv);
#endif

  rsi_sdio_disable_interrupts(sdev->pfunction);

  if (sdev->write_fail)
    rsi_dbg(INFO_ZONE, "###### Device is not ready #######\n");

  if (rsi_set_sdio_pm_caps(adapter))
    rsi_dbg(INFO_ZONE, "Setting power management caps failed\n");

  rsi_dbg(INFO_ZONE, "***** RSI module shut down *****\n");
}

static int rsi_restore(struct device *dev)
{
  struct sdio_func *pfunction = dev_to_sdio_func(dev);
  struct rsi_hw *adapter      = sdio_get_drvdata(pfunction);

  rsi_dbg(INFO_ZONE, "SDIO Bus restore ======>\n");

  adapter->priv->suspend_in_prog  = false;
  adapter->priv->hibernate_resume = true;
  adapter->priv->fsm_state        = FSM_FW_NOT_LOADED;
  adapter->priv->bt_fsm_state     = BT_DEVICE_NOT_READY;
  adapter->priv->iface_down       = true;

  adapter->sc_nvifs = 0;
  rsi_mac80211_hw_scan_cancel(adapter->hw, adapter->priv->scan_vif);
  flush_workqueue(adapter->priv->scan_workqueue);
  ieee80211_stop_queues(adapter->hw);
  ieee80211_restart_hw(adapter->hw);

  /* Initialize device again */
  adapter->priv->reinit_hw = true;
  rsi_sdio_reinit_device(adapter);

#ifdef CONFIG_RSI_WOW
  adapter->priv->wow_flags = 0;
#endif
  adapter->priv->iface_down = false;

  rsi_dbg(INFO_ZONE, "RSI module restored\n");

  return 0;
}

static const struct dev_pm_ops rsi_pm_ops = {
  .suspend  = rsi_suspend,
  .resume   = rsi_resume,
  .freeze   = rsi_freeze,
  .thaw     = rsi_thaw,
  .poweroff = rsi_poweroff,
  .restore  = rsi_restore,
};
#endif

static const struct sdio_device_id rsi_dev_table[] = {
  { SDIO_DEVICE(SDIO_VENDOR_ID_RSI, SDIO_DEVICE_ID_RSI_9113) },
  { SDIO_DEVICE(SDIO_VENDOR_ID_RSI, SDIO_DEVICE_ID_RSI_9116) },
  //__9117_CODE_START
  { SDIO_DEVICE(SDIO_VENDOR_ID_RSI, SDIO_DEVICE_ID_RSI_9117_A0) },
  { SDIO_DEVICE(SDIO_VENDOR_ID_RSI, SDIO_DEVICE_ID_RSI_9117_B0) },
  //__9117_CODE_END
  { /* Blank */ },
};

static struct sdio_driver rsi_driver = {
	.name       = "RSI-SDIO WLAN",
	.probe      = rsi_probe,
	.remove     = rsi_disconnect,
	.id_table   = rsi_dev_table,
#ifdef CONFIG_PM
	.drv = {
		.pm = &rsi_pm_ops,
	        .shutdown   = rsi_shutdown,
	}
#endif
};

/**
 * rsi_module_init() - This function registers the sdio module.
 * @void: Void.
 *
 * Return: 0 on success.
 */
static int rsi_module_init(void)
{
  int ret;

  ret = sdio_register_driver(&rsi_driver);
  rsi_dbg(INIT_ZONE, "%s: Registering driver\n", __func__);
  return ret;
}

/**
 * rsi_module_exit() - This function unregisters the sdio module.
 * @void: Void.
 *
 * Return: None.
 */
static void rsi_module_exit(void)
{
  sdio_unregister_driver(&rsi_driver);
  rsi_dbg(INFO_ZONE, "%s: Unregistering driver\n", __func__);
}

module_init(rsi_module_init);
module_exit(rsi_module_exit);

MODULE_AUTHOR("Redpine Signals Inc");
MODULE_DESCRIPTION("Common SDIO layer for RSI drivers");
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 12, 0)
MODULE_SUPPORTED_DEVICE("RSI-91x");
#endif
MODULE_DEVICE_TABLE(sdio, rsi_dev_table);
MODULE_FIRMWARE(FIRMWARE_RSI9113);
MODULE_VERSION(DRV_VER);
MODULE_LICENSE("GPL");
