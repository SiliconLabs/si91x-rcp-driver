// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include "rsi_debugfs.h"
#include "rsi_sdio.h"
#include "rsi_mgmt.h"
#ifdef CONFIG_RSI_11K
#include "rsi_rrm.h"
#endif
//__9117_CODE_START
#include "rsi_he.h"
//__9117_CODE_END

/**
 * rsi_sdio_stats_read() - This function returns the sdio status of the driver.
 * @seq: Pointer to the sequence file structure.
 * @data: Pointer to the data.
 *
 * Return: 0 on success, -1 on failure.
 */
static int rsi_sdio_stats_read(struct seq_file *seq, void *data)
{
  struct rsi_common *common   = seq->private;
  struct rsi_hw *adapter      = common->priv;
  struct rsi_91x_sdiodev *dev = (struct rsi_91x_sdiodev *)adapter->rsi_dev;

  seq_printf(seq, "total_sdio_interrupts: %d\n", dev->rx_info.sdio_int_counter);
  seq_printf(seq, "sdio_msdu_pending_intr_count: %d\n", dev->rx_info.total_sdio_msdu_pending_intr);
  seq_printf(seq, "sdio_buff_full_count : %d\n", dev->rx_info.buf_full_counter);
  seq_printf(seq, "sdio_buf_semi_full_count %d\n", dev->rx_info.buf_semi_full_counter);
  seq_printf(seq, "sdio_unknown_intr_count: %d\n", dev->rx_info.total_sdio_unknown_intr);
  /* RX Path Stats */
  seq_printf(seq, "BUFFER FULL STATUS  : %d\n", dev->rx_info.buffer_full);
  seq_printf(seq, "SEMI BUFFER FULL STATUS  : %d\n", dev->rx_info.semi_buffer_full);
  seq_printf(seq, "MGMT BUFFER FULL STATUS  : %d\n", dev->rx_info.mgmt_buffer_full);
  seq_printf(seq, "BUFFER FULL COUNTER  : %d\n", dev->rx_info.buf_full_counter);
  seq_printf(seq, "BUFFER SEMI FULL COUNTER  : %d\n", dev->rx_info.buf_semi_full_counter);
  seq_printf(seq, "MGMT BUFFER FULL COUNTER  : %d\n", dev->rx_info.mgmt_buf_full_counter);

  return 0;
}

/**
 * rsi_sdio_stats_open() - This function calls single open function of seq_file
 *			   to open file and read contents from it.
 * @inode: Pointer to the inode structure.
 * @file: Pointer to the file structure.
 *
 * Return: Pointer to the opened file status: 0 on success, ENOMEM on failure.
 */
static int rsi_sdio_stats_open(struct inode *inode, struct file *file)
{
  return single_open(file, rsi_sdio_stats_read, inode->i_private);
}

/**
 * rsi_version_read() - This function gives driver and firmware version number.
 * @seq: Pointer to the sequence file structure.
 * @data: Pointer to the data.
 *
 * Return: 0 on success, -1 on failure.
 */
static int rsi_version_read(struct seq_file *seq, void *data)
{
  struct rsi_common *common = seq->private;
  struct rsi_hw *adapter    = common->priv;

  //__9117_CODE_START
  if (adapter->device_model >= RSI_DEV_9117) {
    seq_printf(seq,
               "Driver : %s\nLMAC   : %04x.%d.%d.%d.%d.%d.%d\n",
               common->driver_ver,
               common->lmac_ver.chip_id,
               common->lmac_ver.major,
               common->lmac_ver.minor,
               common->lmac_ver.security_version,
               common->lmac_ver.patch_id,
               common->lmac_ver.customer_id,
               common->lmac_ver.build_id);
  } else
    //__9117_CODE_END
    if (adapter->device_model == RSI_DEV_9116) {
    seq_printf(seq,
               "Driver : %s\nLMAC   : %04x.%d.%d.%d.%d.%d\n",
               common->driver_ver,
               common->lmac_ver.chip_id,
               common->lmac_ver.major,
               common->lmac_ver.minor,
               common->lmac_ver.patch_id,
               common->lmac_ver.customer_id,
               common->lmac_ver.build_id);
  } else {
    seq_printf(seq,
               "Driver : %s\nLMAC   : %d.%d.%d\n",
               common->driver_ver,
               common->lmac_ver.major,
               common->lmac_ver.minor,
               common->lmac_ver.release_num);
  }
  return 0;
}

/**
 * rsi_version_open() - This function calls single open function of seq_file to
 *			open file and read contents from it.
 * @inode: Pointer to the inode structure.
 * @file: Pointer to the file structure.
 *
 * Return: Pointer to the opened file status: 0 on success, ENOMEM on failure.
 */
static int rsi_version_open(struct inode *inode, struct file *file)
{
  return single_open(file, rsi_version_read, inode->i_private);
}

/**
 * rsi_stats_read() - This function return the status of the driver.
 * @seq: Pointer to the sequence file structure.
 * @data: Pointer to the data.
 *
 * Return: 0 on success, -1 on failure.
 */
static int rsi_stats_read(struct seq_file *seq, void *data)
{
  struct rsi_common *common = seq->private;

  unsigned char fsm_state[][32] = { "FSM_CARD_NOT_READY", "FSM_BOOT_PARAMS_SENT", "FSM_EEPROM_READ_MAC_ADDR",
                                    "FSM_RESET_MAC_SENT", "FSM_RADIO_CAPS_SENT",  "FSM_BB_RF_PROG_SENT",
                                    "FSM_MAC_INIT_DONE" };
  seq_puts(seq, "==> RSI STA DRIVER STATUS <==\n");
  seq_puts(seq, "DRIVER_FSM_STATE: ");

  if (common->fsm_state <= FSM_MAC_INIT_DONE)
    seq_printf(seq, "%s", fsm_state[common->fsm_state]);

  seq_printf(seq, "(%d)\n\n", common->fsm_state);

  /* Mgmt TX Path Stats */
  seq_printf(seq, "total_mgmt_pkt_send : %d\n", common->tx_stats.total_tx_pkt_send[MGMT_SOFT_Q]);
  seq_printf(seq, "total_mgmt_pkt_queued : %d\n", skb_queue_len(&common->tx_queue[MGMT_SOFT_Q]));
  seq_printf(seq, "total_mgmt_pkt_freed  : %d\n", common->tx_stats.total_tx_pkt_freed[MGMT_SOFT_Q]);

  /* Data TX Path Stats */
  seq_printf(seq, "total_data_vo_pkt_send: %8d\t", common->tx_stats.total_tx_pkt_send[VO_Q]);
  seq_printf(seq, "total_data_vo_pkt_queued:  %8d\t", skb_queue_len(&common->tx_queue[VO_Q]));
  seq_printf(seq, "total_vo_pkt_freed: %8d\n", common->tx_stats.total_tx_pkt_freed[VO_Q]);
  seq_printf(seq, "total_data_vi_pkt_send: %8d\t", common->tx_stats.total_tx_pkt_send[VI_Q]);
  seq_printf(seq, "total_data_vi_pkt_queued:  %8d\t", skb_queue_len(&common->tx_queue[VI_Q]));
  seq_printf(seq, "total_vi_pkt_freed: %8d\n", common->tx_stats.total_tx_pkt_freed[VI_Q]);
  seq_printf(seq, "total_data_be_pkt_send: %8d\t", common->tx_stats.total_tx_pkt_send[BE_Q]);
  seq_printf(seq, "total_data_be_pkt_queued:  %8d\t", skb_queue_len(&common->tx_queue[BE_Q]));
  seq_printf(seq, "total_be_pkt_freed: %8d\n", common->tx_stats.total_tx_pkt_freed[BE_Q]);
  seq_printf(seq, "total_data_bk_pkt_send: %8d\t", common->tx_stats.total_tx_pkt_send[BK_Q]);
  seq_printf(seq, "total_data_bk_pkt_queued:  %8d\t", skb_queue_len(&common->tx_queue[BK_Q]));
  seq_printf(seq, "total_bk_pkt_freed: %8d\n", common->tx_stats.total_tx_pkt_freed[BK_Q]);

  seq_puts(seq, "\n");
  return 0;
}

/**
 * rsi_stats_open() - This function calls single open function of seq_file to
 *		      open file and read contents from it.
 * @inode: Pointer to the inode structure.
 * @file: Pointer to the file structure.
 *
 * Return: Pointer to the opened file status: 0 on success, ENOMEM on failure.
 */
static int rsi_stats_open(struct inode *inode, struct file *file)
{
  return single_open(file, rsi_stats_read, inode->i_private);
}

/**
 * rsi_debug_zone_read() - This function display the currently
 *			enabled debug zones.
 * @seq: Pointer to the sequence file structure.
 * @data: Pointer to the data.
 *
 * Return: 0 on success, -1 on failure.
 */
static int rsi_debug_zone_read(struct seq_file *seq, void *data)
{
  rsi_dbg(FSM_ZONE, "%x: rsi_enabled zone", rsi_zone_enabled);
  seq_printf(seq, "The zones available are %#x\n", rsi_zone_enabled);
  return 0;
}

/**
 * rsi_debug_read() - This function calls single open function of seq_file to
 *		      open file and read contents from it.
 * @inode: Pointer to the inode structure.
 * @file: Pointer to the file structure.
 *
 * Return: Pointer to the opened file status: 0 on success, ENOMEM on failure.
 */
static int rsi_debug_read(struct inode *inode, struct file *file)
{
  return single_open(file, rsi_debug_zone_read, inode->i_private);
}

/**
 * rsi_debug_zone_write() - This function writes into hal queues as per user
 *			    requirement.
 * @filp: Pointer to the file structure.
 * @buff: Pointer to the character buffer.
 * @len: Length of the data to be written into buffer.
 * @data: Pointer to the data.
 *
 * Return: len: Number of bytes read.
 */
static ssize_t rsi_debug_zone_write(struct file *filp, const char __user *buff, size_t len, loff_t *data)
{
  unsigned long dbg_zone;
  int ret;

  if (!len)
    return 0;

  ret = kstrtoul_from_user(buff, len, 16, &dbg_zone);

  if (ret)
    return ret;

  rsi_zone_enabled = dbg_zone;
  return len;
}

/**
 * rsi_bgscan_int_read() - This function display the default bgscan param
 *			   values.
 * @seq: Pointer to the sequence file structure.
 * @data: Pointer to the data.
 *
 * Return: 0 on success, -1 on failure.
 */
static int rsi_bgscan_int_read(struct seq_file *file, void *data)
{
  struct rsi_common *common           = file->private;
  struct bgscan_config_params *params = NULL;
  int cnt;

  if (!common) {
    rsi_dbg(ERR_ZONE, "No Interface\n");
    return -ENODEV;
  }
  if (common->iface_down) {
    rsi_dbg(ERR_ZONE, "Interface Down\n");
    return -ENODEV;
  }
  params = &common->bgscan_info;

  seq_printf(file,
             "%d %d %d %d %d %d %d ",
             common->bgscan_en,
             params->bgscan_threshold,
             params->roam_threshold,
             params->bgscan_periodicity,
             params->active_scan_duration,
             params->passive_scan_duration,
             params->two_probe);

  if (common->debugfs_bgscan) {
    seq_printf(file, "%d\n", params->debugfs_bg_channels);
    for (cnt = 0; cnt < params->debugfs_bg_channels; cnt++) {
      if (params->debugfs_channels[cnt] & (DFS_CHANNEL))
        seq_printf(file, "%d[DFS] ", (params->debugfs_channels[cnt] & 0x7FFF));
      else
        seq_printf(file, "%d ", params->debugfs_channels[cnt]);
    }
  } else {
    seq_printf(file, "%d\n", params->num_bg_channels);
    for (cnt = 0; cnt < params->num_bg_channels; cnt++) {
      if (params->channels2scan[cnt] & (DFS_CHANNEL))
        seq_printf(file, "%d[DFS] ", (params->channels2scan[cnt] & 0x7FFF));
      else
        seq_printf(file, "%d ", params->channels2scan[cnt]);
    }
  }
  seq_printf(file, "\n");

  return 0;
}

static int rsi_bgscan_read(struct inode *inode, struct file *file)
{
  return single_open(file, rsi_bgscan_int_read, inode->i_private);
}

/**
 * rsi_validate_debugfs_bgscan_channels() - This function is used to validate
 *				bgscan channels provided through debugfs.
 * @common : Pointer to the rsi_common structure.
 * Return : O on success and -1 on Failure.
 */
int rsi_validate_debugfs_bgscan_channels(struct rsi_common *common)
{
  struct bgscan_config_params *bgscan_info = &common->bgscan_info;
  u16 ii, jj, ch_num, num_valid_channels = 0;
  u16 valid_debugfs_channels[MAX_BG_CHAN_FROM_USER];

  for (ii = 0; ii < bgscan_info->debugfs_bg_channels; ii++) {
    ch_num = bgscan_info->debugfs_channels[ii];
    if (ch_num & DFS_CHANNEL)
      ch_num &= ~DFS_CHANNEL;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0))
    for (jj = 0; jj < common->user_channels_count; jj++) {
      if (ch_num == common->user_channels_list[jj]) {
#else
    for (jj = 0; jj < bgscan_info->num_user_channels; jj++) {
      if (ch_num == bgscan_info->user_channels[jj]) {
#endif
        valid_debugfs_channels[num_valid_channels] = ch_num;
        num_valid_channels++;
        break;
      }
    }
  }
  if (num_valid_channels > 0) {
    for (ii = 0; ii < num_valid_channels; ii++) {
      bgscan_info->debugfs_channels[ii] = valid_debugfs_channels[ii];
      if ((common->bgscan_info.debugfs_channels[ii] >= 52) && (common->bgscan_info.debugfs_channels[ii] <= 140))
        common->bgscan_info.debugfs_channels[ii] |= DFS_CHANNEL;
    }
    bgscan_info->debugfs_bg_channels = num_valid_channels;
    return 0;
  } else
    return -1;
}

/**
 * rsi_validate_bgscan_params() - This function is used to validate bgscan
 *				  params provided by user.
 * @common : Pointer to the rsi_common structure.
 * @bgscan_vals : Pointer to user buffer.
 * @user_params_cnt : user_params_cnt
 * Return : O on success and -1 on Failure.
 */
static int rsi_validate_bgscan_params(struct rsi_common *common, int *bgscan_vals, int user_params_cnt)
{
  struct bgscan_config_params *bgscan_info = &common->bgscan_info;
  int cnt                                  = 0;

  bgscan_info->bgscan_threshold   = bgscan_vals[0];
  bgscan_info->roam_threshold     = bgscan_vals[1];
  bgscan_info->bgscan_periodicity = bgscan_vals[2];
  if ((bgscan_info->bgscan_threshold < 0) || (bgscan_info->roam_threshold < 0)
      || (bgscan_info->bgscan_periodicity < 0)) {
    rsi_dbg(ERR_ZONE, "Bg scan arguments can't be negative vals\n");
    return -EINVAL;
  }
  bgscan_info->active_scan_duration = bgscan_vals[3];
  if (!bgscan_info->active_scan_duration || (bgscan_info->active_scan_duration > 255)
      || (bgscan_info->active_scan_duration < 0)) {
    rsi_dbg(ERR_ZONE,
            "Active scan duration should be greater"
            " than 0 and less than 256\n");
    return -EINVAL;
  }

  bgscan_info->passive_scan_duration = bgscan_vals[4];
  if (!bgscan_info->passive_scan_duration || (bgscan_info->passive_scan_duration > 255)
      || (bgscan_info->passive_scan_duration < 0)) {
    rsi_dbg(ERR_ZONE,
            "Passive scan duration should be greater"
            " than 0 and less than 256\n");
    return -EINVAL;
  }
  bgscan_info->two_probe = bgscan_vals[5];
  if (!((bgscan_info->two_probe && (bgscan_info->two_probe == 1)) || (bgscan_info->two_probe == 0))) {
    rsi_dbg(ERR_ZONE, "value for two_probe is either 0 or 1\n");
    return -EINVAL;
  }

  if ((user_params_cnt - 7) != bgscan_vals[6]) {
    rsi_dbg(ERR_ZONE,
            "channels count is more than"
            " Provided Channels\n");
    return -EINVAL;
  }
  bgscan_info->debugfs_bg_channels = bgscan_vals[6];
  if (bgscan_info->debugfs_bg_channels > MAX_BG_CHAN_FROM_USER) {
    rsi_dbg(ERR_ZONE,
            "User provided channels  are more"
            " than MAX_BG_CHAN_FROM_USER\n");
    return -EINVAL;
  }
  memset(bgscan_info->debugfs_channels, 0, (MAX_BG_CHAN_FROM_USER * 2));
  for (cnt = 0; cnt < bgscan_info->debugfs_bg_channels; cnt++)
    bgscan_info->debugfs_channels[cnt] = bgscan_vals[7 + cnt];
  if (rsi_validate_debugfs_bgscan_channels(common)) {
    rsi_dbg(ERR_ZONE, "User provided invalid channels\n");
    return -EINVAL;
  }
  return 0;
}

/**
 * rsi_bgscan_write() - This function gets the bgscan params from user
 *			    and configures to device.
 * @file: Pointer to the file structure.
 * @user_buff: user buffer.
 * @count: Length of the data written in buffer.
 * @ppos: offset.
 *
 * Return: Number of bytes read.
 */
static ssize_t rsi_bgscan_write(struct file *file, const char __user *user_buff, size_t count, loff_t *ppos)

{
  struct rsi_common *common = file->f_inode->i_private;
  struct rsi_hw *adapter    = NULL;
#ifdef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = NULL;
#endif
  struct ieee80211_bss_conf *bss = NULL;
  int bgscan_vals[64]            = { 0 };
  int bytes_read                 = 0, t_bytes;
  int ret, bgscan_enable = 0;
  int total_bytes, cnt = 0, user_params_cnt = 0;
  char *bgscan_buf = kmalloc(count + 1, GFP_KERNEL);

  if (!common) {
    rsi_dbg(ERR_ZONE, "No Interface\n");
    return -ENODEV;
  }
  if (common->iface_down) {
    rsi_dbg(ERR_ZONE, "Interface is Down: Must be Up\n");
    return -ENODEV;
  }
  adapter = common->priv;
#ifndef CONFIG_STA_PLUS_AP
  bss = &adapter->vifs[adapter->sc_nvifs - 1]->bss_conf;
#else
  vif = rsi_get_sta_vif(adapter);
  if (!vif)
    return -ENODEV;
  bss = &vif->bss_conf;
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  if (bss && !bss->assoc) {
#else
  if (bss && !adapter->vifs[adapter->sc_nvifs - 1]->cfg.assoc) {
#endif
    rsi_dbg(ERR_ZONE, "Send Bgscan params after connection\n");
    return -ENODEV;
  }

  total_bytes = simple_write_to_buffer(bgscan_buf, count, ppos, user_buff, count);
  if (total_bytes < 1)
    return -EINVAL;

  /* make sure that buf is null terminated */
  bgscan_buf[count] = '\0';

  ret = sscanf(bgscan_buf, "%d%n", (int *)&bgscan_enable, &t_bytes);
  if (ret <= 0)
    return -EINVAL;

  if (!bgscan_enable) {
    /* return here if bgscan is already disabled */
    if (!common->bgscan_en) {
      rsi_dbg(ERR_ZONE, "bgscan already disabled\n");
      return total_bytes;
    }
    common->debugfs_bgscan = true;
    mutex_lock(&common->bgscan_lock);
    if (common->bgscan_in_prog) {
      common->debugfs_bgscan_en   = true;
      common->debugfs_stop_bgscan = true;
      mutex_unlock(&common->bgscan_lock);
      return total_bytes;
    } else {
      rsi_send_bgscan_params(common, 0);
      common->debugfs_bgscan_en = false;
      common->debugfs_bgscan    = false;
      common->bgscan_en         = false;
      init_bgscan_params(common);
    }
    mutex_unlock(&common->bgscan_lock);
    return total_bytes;
  }

  bytes_read += t_bytes;
  while (1) {
    ret = sscanf(bgscan_buf + bytes_read, "%d%n", &bgscan_vals[cnt++], &t_bytes);
    if (ret <= 0)
      break;
    bytes_read += t_bytes;
    user_params_cnt++;

    if ((bgscan_vals[6] > 0) && (cnt > (6 + bgscan_vals[6])))
      break;
  }
  kfree(bgscan_buf);
  if (rsi_validate_bgscan_params(common, bgscan_vals, user_params_cnt)) {
    rsi_dbg(ERR_ZONE, "User provided invalid bgscan parameters\n");
    return -1;
  }
  rsi_dbg(INFO_ZONE,
          "bgscan_count = %d, roam_count = %d, periodicity = %d\n",
          common->bgscan_info.bgscan_threshold,
          common->bgscan_info.roam_threshold,
          common->bgscan_info.bgscan_periodicity);
  rsi_dbg(INFO_ZONE,
          "active_scan_dur = %d, passive_scan_dur = %d, two_probe = %d\n",
          common->bgscan_info.active_scan_duration,
          common->bgscan_info.passive_scan_duration,
          common->bgscan_info.two_probe);
  rsi_dbg(INFO_ZONE, "Number of scan channels = %d\n", common->bgscan_info.debugfs_bg_channels);
  rsi_hex_dump(INFO_ZONE,
               "bgscan channels",
               (u8 *)common->bgscan_info.debugfs_channels,
               common->bgscan_info.debugfs_bg_channels * 2);

  common->debugfs_bgscan = true;
  mutex_lock(&common->bgscan_lock);
  if (common->bgscan_in_prog) {
    common->debugfs_bgscan_en = true;
    mutex_unlock(&common->bgscan_lock);
    return total_bytes;
  } else
    rsi_send_bgscan_params(common, 1);
  mutex_unlock(&common->bgscan_lock);

  rsi_dbg(INFO_ZONE, "bgscan params update complete\n");

  return total_bytes;
}

#define FOPS(fopen)                                                               \
  {                                                                               \
    .owner = THIS_MODULE, .open = (fopen), .read = seq_read, .llseek = seq_lseek, \
  }

#define FOPS_RW(fopen, fwrite)                                                                       \
  {                                                                                                  \
    .owner = THIS_MODULE, .open = (fopen), .read = seq_read, .llseek = seq_lseek, .write = (fwrite), \
  }

#if defined(CONFIG_RSI_11K) && defined(RSI_DEBUG_RRM)
static ssize_t rsi_write_chload_meas_req(struct file *file, const char __user *user_buff, size_t count, loff_t *ppos)
{
  struct rsi_common *common = file->f_inode->i_private;
  char in_buf[200];
  int chan_load_vals[64] = { 0 };
  int total_bytes, bytes_read = 0;
  int ret, i;
  struct rsi_hw *adapter = common->priv;
#ifndef CONFIG_STA_PLUS_AP
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  struct ieee80211_bss_conf *bss = &adapter->vifs[adapter->sc_nvifs - 1]->bss_conf;
#endif
#else
  struct ieee80211_vif *vif      = NULL;
  struct ieee80211_bss_conf *bss = NULL;

  vif = rsi_get_sta_vif(adapter);
  if (!vif)
    return -ENODEV;
  bss = &vif->bss_conf;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  if (!bss->assoc) {
#else
  if (!adapter->vifs[adapter->sc_nvifs - 1]->cfg.assoc) {
#endif
    rsi_dbg(ERR_ZONE, "unable to send channelload in non connected state\n");
    return -EINVAL;
  }

  total_bytes = simple_write_to_buffer(in_buf, sizeof(in_buf) - 1, ppos, user_buff, count);
  if (total_bytes < 1)
    return -EINVAL;
  in_buf[sizeof(in_buf) - 1] = '\0';
  ret                        = sscanf(in_buf + bytes_read,
               "%x%x%x%x%x%x%d%d%d%d%d%d",
               &chan_load_vals[0],
               &chan_load_vals[1],
               &chan_load_vals[2],
               &chan_load_vals[3],
               &chan_load_vals[4],
               &chan_load_vals[5],
               &chan_load_vals[6],
               &chan_load_vals[7],
               &chan_load_vals[8],
               &chan_load_vals[9],
               &chan_load_vals[10],
               &chan_load_vals[11]);
  rsi_hex_dump(INFO_ZONE, "Parsed values", (u8 *)chan_load_vals, 32);
  for (i = 0; i < ETH_ALEN; i++)
    common->rrm_chload_params.macid[i] = chan_load_vals[i];

  common->rrm_chload_params.regulatory_class = chan_load_vals[6];
  common->rrm_chload_params.channel_num      = chan_load_vals[7];
  common->rrm_chload_params.rand_interval    = chan_load_vals[8];
  common->rrm_chload_params.meas_duration    = chan_load_vals[9];
  common->rrm_chload_params.meas_req_mode    = chan_load_vals[10];
  common->rrm_chload_params.meas_type        = chan_load_vals[11];

  if (!rsi_rrm_send_channel_load_req(common))
    rsi_dbg(ERR_ZONE, "Sent channel load measurement request\n");
  else
    rsi_dbg(ERR_ZONE, "Failed sending channel load measurement req\n");

  return total_bytes;
}

static int rsi_int_read_chload_params(struct seq_file *file, void *data)
{
  struct rsi_common *common                 = file->private;
  struct rsi_chload_meas_req_params *params = &common->rrm_chload_params;

  seq_printf(file,
             "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x %d %d %d %d %d %d\n",
             params->macid[0],
             params->macid[1],
             params->macid[2],
             params->macid[3],
             params->macid[4],
             params->macid[5],
             params->regulatory_class,
             params->channel_num,
             params->rand_interval,
             params->meas_duration,
             params->meas_req_mode,
             params->meas_type);
  seq_puts(file, "\n");

  return 0;
}

static int rsi_read_chload_meas_req(struct inode *inode, struct file *file)
{
  return single_open(file, rsi_int_read_chload_params, inode->i_private);
}

static ssize_t rsi_write_frame_meas_req(struct file *file, const char __user *user_buff, size_t count, loff_t *ppos)
{
  struct rsi_common *common = file->f_inode->i_private;
  char in_buf[200];
  int frame_load_vals[64] = { 0 };
  int total_bytes, bytes_read = 0;
  int ret, i;
  struct rsi_hw *adapter = common->priv;
#ifndef CONFIG_STA_PLUS_AP
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  struct ieee80211_bss_conf *bss = &adapter->vifs[adapter->sc_nvifs - 1]->bss_conf;
#endif
#else
  struct ieee80211_vif *vif      = NULL;
  struct ieee80211_bss_conf *bss = NULL;

  vif = rsi_get_sta_vif(adapter);
  if (!vif)
    return -ENODEV;
  bss = &vif->bss_conf;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  if (!bss->assoc) {
#else
  if (!adapter->vifs[adapter->sc_nvifs - 1]->cfg.assoc) {
#endif
    rsi_dbg(ERR_ZONE, "unable to send frame req in non connected state\n");
    return -EINVAL;
  }
  total_bytes = simple_write_to_buffer(in_buf, sizeof(in_buf) - 1, ppos, user_buff, count);
  if (total_bytes < 1)
    return -EINVAL;
  in_buf[sizeof(in_buf) - 1] = '\0';
  ret                        = sscanf(in_buf + bytes_read,
               "%x%x%x%x%x%x%d%d%d%d%d%d%d\n"
               "%x%x%x%x%x%x",
               &frame_load_vals[0],
               &frame_load_vals[1],
               &frame_load_vals[2],
               &frame_load_vals[3],
               &frame_load_vals[4],
               &frame_load_vals[5],
               &frame_load_vals[6],
               &frame_load_vals[7],
               &frame_load_vals[8],
               &frame_load_vals[9],
               &frame_load_vals[10],
               &frame_load_vals[11],
               &frame_load_vals[12],
               &frame_load_vals[13],
               &frame_load_vals[14],
               &frame_load_vals[15],
               &frame_load_vals[16],
               &frame_load_vals[17],
               &frame_load_vals[18]);
  rsi_hex_dump(INFO_ZONE, "Parsed values", (u8 *)frame_load_vals, 32);

  for (i = 0; i < ETH_ALEN; i++)
    common->rrm_frame_params.destid[i] = frame_load_vals[i];
  common->rrm_frame_params.regulatory_class = frame_load_vals[6];
  common->rrm_frame_params.channel_num      = frame_load_vals[7];
  common->rrm_frame_params.rand_interval    = frame_load_vals[8];
  common->rrm_frame_params.meas_duration    = frame_load_vals[9];
  common->rrm_frame_params.meas_req_mode    = frame_load_vals[10];
  common->rrm_frame_params.meas_type        = frame_load_vals[11];
  common->rrm_frame_params.frame_req_type   = frame_load_vals[12];
  for (i = 0; i < ETH_ALEN; i++)
    common->rrm_frame_params.macid[i] = frame_load_vals[13 + i];

  if (!rsi_rrm_send_frame_req(common))
    rsi_dbg(ERR_ZONE, "Sent frame measurement request\n");
  else
    rsi_dbg(ERR_ZONE, "Failed sending frame measurement req\n");

  return total_bytes;
}

static int rsi_int_read_frame_params(struct seq_file *file, void *data)
{
  struct rsi_common *common                = file->private;
  struct rsi_frame_meas_req_params *params = &common->rrm_frame_params;

  seq_printf(file,
             "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x %d %d %d %d %d %d %d\n"
             "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
             params->destid[0],
             params->destid[1],
             params->destid[2],
             params->destid[3],
             params->destid[4],
             params->destid[5],
             params->regulatory_class,
             params->channel_num,
             params->rand_interval,
             params->meas_duration,
             params->meas_req_mode,
             params->meas_type,
             params->frame_req_type,
             params->macid[0],
             params->macid[1],
             params->macid[2],
             params->macid[3],
             params->macid[4],
             params->macid[5]);
  seq_puts(file, "\n");

  return 0;
}

static int rsi_read_frame_meas_req(struct inode *inode, struct file *file)
{
  return single_open(file, rsi_int_read_frame_params, inode->i_private);
}

static ssize_t rsi_write_beacon_meas_req(struct file *file, const char __user *user_buff, size_t count, loff_t *ppos)
{
  struct rsi_common *common = file->f_inode->i_private;
  char in_buf[200];
  int beacon_load_vals[64] = { 0 };
  int total_bytes, bytes_read = 0;
  int ret;
  char str[32];
  struct rsi_hw *adapter = common->priv;
#ifndef CONFIG_STA_PLUS_AP
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  struct ieee80211_bss_conf *bss = &adapter->vifs[adapter->sc_nvifs - 1]->bss_conf;
#endif
#else
  struct ieee80211_vif *vif      = NULL;
  struct ieee80211_bss_conf *bss = NULL;

  vif = rsi_get_sta_vif(adapter);
  if (!vif)
    return -ENODEV;
  bss = &vif->bss_conf;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  if (!bss->assoc) {
#else
  if (!adapter->vifs[adapter->sc_nvifs - 1]->cfg.assoc) {
#endif
    rsi_dbg(ERR_ZONE, "unable to send beacon req in non connected state\n");
    return -EINVAL;
  }
  total_bytes = simple_write_to_buffer(in_buf, sizeof(in_buf) - 1, ppos, user_buff, count);
  if (total_bytes < 1)
    return -EINVAL;
  in_buf[sizeof(in_buf) - 1] = '\0';
  ret                        = sscanf(in_buf + bytes_read,
               "%x%x%x%x%x%x%d%d%d%d%d%d%d\n"
               "%x%x%x%x%x%x%s",
               &beacon_load_vals[0],
               &beacon_load_vals[1],
               &beacon_load_vals[2],
               &beacon_load_vals[3],
               &beacon_load_vals[4],
               &beacon_load_vals[5],
               &beacon_load_vals[6],
               &beacon_load_vals[7],
               &beacon_load_vals[8],
               &beacon_load_vals[9],
               &beacon_load_vals[10],
               &beacon_load_vals[11],
               &beacon_load_vals[12],
               &beacon_load_vals[13],
               &beacon_load_vals[14],
               &beacon_load_vals[15],
               &beacon_load_vals[16],
               &beacon_load_vals[17],
               &beacon_load_vals[18],
               str);
  rsi_hex_dump(INFO_ZONE, "Parsed values", (u8 *)beacon_load_vals, 32);

  common->rrm_beacon_params.destid[0]        = beacon_load_vals[0];
  common->rrm_beacon_params.destid[1]        = beacon_load_vals[1];
  common->rrm_beacon_params.destid[2]        = beacon_load_vals[2];
  common->rrm_beacon_params.destid[3]        = beacon_load_vals[3];
  common->rrm_beacon_params.destid[4]        = beacon_load_vals[4];
  common->rrm_beacon_params.destid[5]        = beacon_load_vals[5];
  common->rrm_beacon_params.regulatory_class = beacon_load_vals[6];
  common->rrm_beacon_params.channel_num      = beacon_load_vals[7];
  common->rrm_beacon_params.rand_interval    = beacon_load_vals[8];
  common->rrm_beacon_params.meas_duration    = beacon_load_vals[9];
  common->rrm_beacon_params.meas_req_mode    = beacon_load_vals[10];
  common->rrm_beacon_params.meas_type        = beacon_load_vals[11];
  common->rrm_beacon_params.meas_mode        = beacon_load_vals[12];
  common->rrm_beacon_params.bssid[0]         = beacon_load_vals[13];
  common->rrm_beacon_params.bssid[1]         = beacon_load_vals[14];
  common->rrm_beacon_params.bssid[2]         = beacon_load_vals[15];
  common->rrm_beacon_params.bssid[3]         = beacon_load_vals[16];
  common->rrm_beacon_params.bssid[4]         = beacon_load_vals[17];
  common->rrm_beacon_params.bssid[5]         = beacon_load_vals[18];
  memset(common->rrm_beacon_params.str, 0, 32);
  memcpy(common->rrm_beacon_params.str, str, strlen(str));

  rsi_dbg(INFO_ZONE, "regulatory class %d\n", common->rrm_beacon_params.regulatory_class);
  rsi_dbg(INFO_ZONE, "channel num %d\n", common->rrm_beacon_params.channel_num);
  rsi_dbg(INFO_ZONE, "rand_interval %d\n", common->rrm_beacon_params.rand_interval);
  rsi_dbg(INFO_ZONE, "meas_duration %d\n", common->rrm_beacon_params.meas_duration);
  rsi_dbg(INFO_ZONE, "meas_req_mode %d\n", common->rrm_beacon_params.meas_req_mode);
  rsi_dbg(INFO_ZONE, "meas_type %d\n", common->rrm_beacon_params.meas_type);
  rsi_dbg(INFO_ZONE, "meas_mode %d\n", common->rrm_beacon_params.meas_mode);
  rsi_dbg(INFO_ZONE, "ssid %s\n", common->rrm_beacon_params.str);

  if (!rsi_rrm_send_beacon_req(common))
    rsi_dbg(ERR_ZONE, "Sent Beacon measurement request\n");
  else
    rsi_dbg(ERR_ZONE, "Failed sending beacon measurement req\n");

  return total_bytes;
}

static int rsi_int_read_beacon_params(struct seq_file *file, void *data)
{
  struct rsi_common *common                 = file->private;
  struct rsi_beacon_meas_req_params *params = &common->rrm_beacon_params;

  seq_printf(file,
             "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x %d %d %d %d %d %d %d "
             "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x %s\n",
             params->destid[0],
             params->destid[1],
             params->destid[2],
             params->destid[3],
             params->destid[4],
             params->destid[5],
             params->regulatory_class,
             params->channel_num,
             params->rand_interval,
             params->meas_duration,
             params->meas_req_mode,
             params->meas_type,
             params->meas_mode,
             params->bssid[0],
             params->bssid[1],
             params->bssid[2],
             params->bssid[3],
             params->bssid[4],
             params->bssid[5],
             params->str);
  seq_puts(file, "\n");

  return 0;
}

static int rsi_read_beacon_meas_req(struct inode *inode, struct file *file)
{
  return single_open(file, rsi_int_read_beacon_params, inode->i_private);
}
#endif

/**
 * rsi_read_ps_params_from_buf() - This function display the power save param
 *                         values.
 * @seq: Pointer to the sequence file structure.
 * @data: Pointer to the data.
 *
 * Return: 0 on success, -1 on failure.
 */

static int rsi_read_ps_params_from_buf(struct seq_file *file, void *data)
{
  struct rsi_common *common   = file->private;
  struct rsi_hw *adapter      = common->priv;
  struct rsi_ps_info *ps_info = &adapter->ps_info;

  if (!common) {
    rsi_dbg(ERR_ZONE, "No Interface\n");
    return -ENODEV;
  }
  seq_printf(file,
             "%d %d %d %d %d %d %d %d %d %d %d %d\n",
             ps_info->sleep_type,
             ps_info->tx_threshold,
             ps_info->rx_threshold,
             ps_info->tx_hysterisis,
             ps_info->rx_hysterisis,
             ps_info->monitor_interval,
             ps_info->listen_interval_duration,
             ps_info->num_bcns_per_lis_int,
             ps_info->dtim_interval_duration,
             ps_info->num_dtims_per_sleep,
             ps_info->deep_sleep_wakeup_period,
             ps_info->uapsd_wakeup_period);
  seq_printf(file, "\n");

  return 0;
}

static int rsi_read_ps_params(struct inode *inode, struct file *file)
{
  return single_open(file, rsi_read_ps_params_from_buf, inode->i_private);
}
/**
 * rsi_validate_ps_params() - This function is used to validate ps params
 *				provided by user.
 * @common : Pointer to the rsi_common structure.
 * @ps_params_vals : Pointer to user buffer.
 * Retunr : O on success and -1 on Failure.
 */
int rsi_validate_ps_params(struct rsi_common *common, int *ps_params_vals)
{

  struct rsi_hw *adapter      = common->priv;
  struct rsi_ps_info *ps_info = &adapter->ps_info;

  ps_info->sleep_type = ps_params_vals[0];
  if ((ps_info->sleep_type != 1) && (ps_info->sleep_type != 2)) {
    rsi_dbg(ERR_ZONE, "Entered wrong value for sleep_type\n");
    rsi_dbg(ERR_ZONE, "Please Enter 1- For LP mode\n 2-ULP Mode\n");
    return -EINVAL;
  }
  ps_info->tx_threshold = ps_params_vals[1];
  ps_info->rx_threshold = ps_params_vals[2];
  if (ps_info->tx_threshold > 10) {
    rsi_dbg(ERR_ZONE, "Making tx threshold  to default value of 1Mbps\n");
    rsi_dbg(ERR_ZONE, "Supported TX threshold is 0 to 10Mbps\n");
    ps_info->tx_threshold = 1;
  }
  if (ps_info->rx_threshold > 10) {
    rsi_dbg(ERR_ZONE, "Making rx threshold  to default value of 1Mbps\n");
    rsi_dbg(ERR_ZONE, "Supported RX threshold is 0 to 10Mbps\n");
    ps_info->rx_threshold = 1;
  }
  ps_info->tx_hysterisis = ps_params_vals[3];
  ps_info->rx_hysterisis = ps_params_vals[4];
  if (ps_info->rx_threshold && (ps_info->rx_hysterisis > ps_info->rx_threshold)) {
    rsi_dbg(ERR_ZONE,
            "Making RX Hysterisis value to RX threshold "
            "as hysterisis value should not be greater than "
            "rx threshold\n");
    ps_info->rx_hysterisis = ps_info->rx_threshold;
  }
  if (ps_info->tx_threshold && (ps_info->tx_hysterisis > ps_info->tx_threshold)) {
    rsi_dbg(ERR_ZONE,
            "Making TX Hysterisis value to TX threshold "
            "as hysterisis value should not be greater than "
            "tx threshold\n");
    ps_info->tx_hysterisis = ps_info->tx_threshold;
  }
  ps_info->monitor_interval = ps_params_vals[5];
  if ((ps_info->monitor_interval && !((ps_info->monitor_interval >= 10) && (ps_info->monitor_interval <= 29999)))) {
    rsi_dbg(ERR_ZONE,
            "Supported Monitor Interval Range"
            "is 10 to 30000ms\n");
    rsi_dbg(ERR_ZONE,
            "Making monitor interval to default value"
            "of 50 milliseconds Which is recommended ***\n");
    ps_info->monitor_interval = 50;
  }
  ps_info->listen_interval_duration = ps_params_vals[6];
  if (ps_info->listen_interval_duration > 65535) {
    rsi_dbg(ERR_ZONE, "Please enter a valid value for listen_interval\n");
    rsi_dbg(ERR_ZONE, "Supported Range is 0 to 65535\n");
    return -EINVAL;
  }
  ps_info->num_bcns_per_lis_int = ps_params_vals[7];
  if ((ps_info->num_bcns_per_lis_int > 4095) || (ps_info->num_bcns_per_lis_int < 0)) {
    rsi_dbg(ERR_ZONE, "Please enter valid value for num_bcns_per_lis_int\n");
    rsi_dbg(ERR_ZONE, "Supported Range is 0 to 4095 beacons\n");
    return -EINVAL;
  }
  ps_info->dtim_interval_duration = ps_params_vals[8];
  if ((ps_info->dtim_interval_duration > 10000) || (ps_info->dtim_interval_duration < 0)) {
    rsi_dbg(ERR_ZONE,
            "Please enter a valid value for"
            "dtim_interval_duration\n");
    rsi_dbg(ERR_ZONE, "Supported Range is 0 to 10000ms\n");
    return -EINVAL;
  }
  ps_info->num_dtims_per_sleep = ps_params_vals[9];
  if ((ps_info->num_dtims_per_sleep > 10) || (ps_info->num_dtims_per_sleep < 0)) {
    rsi_dbg(ERR_ZONE, "Please enter valid value for num_dtims_per_sleep\n");
    rsi_dbg(ERR_ZONE, "Supported Range is 0 to 10 dtim intervals\n");
    return -EINVAL;
  }
  ps_info->deep_sleep_wakeup_period = ps_params_vals[10];
  if (ps_info->deep_sleep_wakeup_period > 65535) {
    rsi_dbg(ERR_ZONE,
            "Please enter a valid value for"
            "sleep_duration\n");
    rsi_dbg(ERR_ZONE, "Supported Range is 0 to 65535\n");
    return -EINVAL;
  }
  if ((ps_info->sleep_type == 2) && !ps_info->deep_sleep_wakeup_period) {
    ps_info->deep_sleep_wakeup_period = 10;
  }
  if ((!ps_info->listen_interval_duration) && (!ps_info->num_bcns_per_lis_int) && (!ps_info->dtim_interval_duration)
      && (!ps_info->num_dtims_per_sleep)) {
    rsi_dbg(ERR_ZONE,
            "Listen_interval and num_bcns_per_lis_int"
            "and dtim_interval_duration and num_dtims_per_sleep for"
            " are not provided!!\n");
    rsi_dbg(ERR_ZONE, "Wakeup period is set as one dtim!!\n");
    ps_info->num_dtims_per_sleep = 1;
  }
  ps_info->uapsd_wakeup_period = ps_params_vals[11];
  if ((ps_info->uapsd_wakeup_period) && ((ps_info->uapsd_wakeup_period < 10) || (ps_info->uapsd_wakeup_period > 100))) {
    rsi_dbg(ERR_ZONE,
            "UAPSD wakeup period value should be in"
            "between (10 to 100), So defaulting the value to 10\n");
    ps_info->uapsd_wakeup_period = 10;
  }
  return 0;
}

/**
 * rsi_write_ps_params() - This function gets the ps params from user
 *                          and configures to device.
 * @file: Pointer to the file structure.
 * @user_buff: user buffer.
 * @count: Length of the data written in buffer.
 >* @ppos: offset.
 *
 * Return: Number of bytes read.
 */

static ssize_t rsi_write_ps_params(struct file *file, const char __user *user_buff, size_t count, loff_t *ppos)
{
  struct rsi_common *common = file->f_inode->i_private;
  struct rsi_hw *adapter    = common->priv;
  int ps_params_vals[64];
  int total_bytes, cnt = 0;
  int bytes_read = 0, t_bytes, ret = 0;
  char *ps_param_buf = kmalloc(count + 1, GFP_KERNEL);

  if (!ps_param_buf)
    return -ENOMEM;
  total_bytes = simple_write_to_buffer(ps_param_buf, count, ppos, user_buff, count);
  if (total_bytes != count) {
    rsi_dbg(ERR_ZONE, "Invalid Power Save Parameter passed\n");
    kfree(ps_param_buf);
    return -EINVAL;
  }
  /* make sure that buf is null terminated */
  ps_param_buf[count] = '\0';
  while (1) {
    ret = sscanf(ps_param_buf + bytes_read, "%d%n", &ps_params_vals[cnt++], &t_bytes);
    if (ret <= 0)
      break;
    bytes_read += t_bytes;
  }
  kfree(ps_param_buf);
  if (rsi_validate_ps_params(common, ps_params_vals))
    return -EINVAL;
  if (adapter->user_ps_en)
    rsi_enable_ps(adapter);
  return total_bytes;
}

static ssize_t rsi_set_bgscan_ssid(struct file *file, const char __user *user_buff, size_t count, loff_t *ppos)
{
  struct rsi_common *common = file->f_inode->i_private;
  int total_bytes;

  common->bgscan_ssid_len = count - 1;
  if (common->bgscan_ssid_len > 32) {
    rsi_dbg(ERR_ZONE, "The ssid should be <= to 32 characters\n");
    return -EINVAL;
  }
  total_bytes = simple_write_to_buffer(common->bgscan_ssid, count, ppos, user_buff, count);
  if (total_bytes < 1)
    return -EINVAL;
  common->bgscan_ssid[count] = '\0';
  return count;
}

static int rsi_read_bgscan_ssid(struct seq_file *file, void *data)
{
  struct rsi_common *common = file->private;

  if (!common) {
    rsi_dbg(ERR_ZONE, "No Interface\n");
    return -ENODEV;
  }
  seq_printf(file, "%s\n", common->bgscan_ssid);
  return 0;
}
static int rsi_get_bgscan_ssid(struct inode *inode, struct file *file)
{
  return single_open(file, rsi_read_bgscan_ssid, inode->i_private);
}
static const struct rsi_dbg_files dev_debugfs_files[] = {
  {
    "version",
    0644,
    FOPS(rsi_version_open),
  },
  {
    "stats",
    0644,
    FOPS(rsi_stats_open),
  },
  {
    "debug_zone",
    0666,
    FOPS_RW(rsi_debug_read, rsi_debug_zone_write),
  },
  {
    "bgscan",
    0666,
    FOPS_RW(rsi_bgscan_read, rsi_bgscan_write),
  },
  {
    "ps_params",
    0666,
    FOPS_RW(rsi_read_ps_params, rsi_write_ps_params),
  },
  {
    "bgscan_ssid",
    0666,
    FOPS_RW(rsi_get_bgscan_ssid, rsi_set_bgscan_ssid),
  },
//__9117_CODE_START
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
  {
    "ax_rate",
    0666,
    FOPS_RW(rsi_get_ax_rate, rsi_set_ax_rate),
  },
#endif
  {
    "GI_LTF",
    0666,
    FOPS_RW(rsi_get_gi_ltf, rsi_set_gi_ltf),
  },
  {
    "edca_susp",
    0666,
    FOPS_RW(rsi_get_edca_susp, rsi_set_edca_susp),
  },
  {
    "config_er_su",
    0666,
    FOPS_RW(rsi_get_config_er_su, rsi_set_config_er_su),
  },
//__9117_CODE_END
#if defined(CONFIG_RSI_11K) && defined(RSI_DEBUG_RRM)
  {
    "rrm_chan_load_req",
    0666,
    FOPS_RW(rsi_read_chload_meas_req, rsi_write_chload_meas_req),
  },
  {
    "rrm_frame_req",
    0666,
    FOPS_RW(rsi_read_frame_meas_req, rsi_write_frame_meas_req),
  },
  {
    "rrm_beacon_req",
    0666,
    FOPS_RW(rsi_read_beacon_meas_req, rsi_write_beacon_meas_req),
  },
#endif
  {
    "sdio_stats",
    0644,
    FOPS(rsi_sdio_stats_open),
  },
};

//__9117_CODE_START
int rsi_init_917_dbgfs(struct rsi_hw *adapter)
{
  struct rsi_common *common = adapter->priv;
  struct rsi_debugfs_917 *dev_dbgfs;
  char devdir[6];
  int ii;
  const struct rsi_dbg_files *files;

  dev_dbgfs = kzalloc(sizeof(*dev_dbgfs), GFP_KERNEL);
  if (!dev_dbgfs)
    return -ENOMEM;

  adapter->dfsentry_917 = dev_dbgfs;

  snprintf(devdir, sizeof(devdir), "%s", wiphy_name(adapter->hw->wiphy));

  dev_dbgfs->subdir = debugfs_create_dir(devdir, NULL);

  if (!dev_dbgfs->subdir) {
    kfree(dev_dbgfs);
    return -ENOMEM;
  }

  for (ii = 0; ii < adapter->num_debugfs_entries; ii++) {
    files                    = &dev_debugfs_files[ii];
    dev_dbgfs->rsi_files[ii] = debugfs_create_file(files->name, files->perms, dev_dbgfs->subdir, common, &files->fops);
  }
  return 0;
}
EXPORT_SYMBOL_GPL(rsi_init_917_dbgfs);

void rsi_remove_917_dbgfs(struct rsi_hw *adapter)
{
  struct rsi_debugfs_917 *dev_dbgfs = adapter->dfsentry_917;

  if (!dev_dbgfs)
    return;

  debugfs_remove_recursive(dev_dbgfs->subdir);
}
EXPORT_SYMBOL_GPL(rsi_remove_917_dbgfs);
//__9117_CODE_END

/**
 * rsi_init_dbgfs() - This function initializes the dbgfs entry.
 * @adapter: Pointer to the adapter structure.
 *
 * Return: 0 on success, -1 on failure.
 */
int rsi_init_dbgfs(struct rsi_hw *adapter)
{
  struct rsi_common *common = adapter->priv;
  struct rsi_debugfs *dev_dbgfs;
  char devdir[6];
  int ii;
  const struct rsi_dbg_files *files;

  dev_dbgfs = kzalloc(sizeof(*dev_dbgfs), GFP_KERNEL);
  if (!dev_dbgfs)
    return -ENOMEM;

  adapter->dfsentry = dev_dbgfs;

  snprintf(devdir, sizeof(devdir), "%s", wiphy_name(adapter->hw->wiphy));

  dev_dbgfs->subdir = debugfs_create_dir(devdir, NULL);

  if (!dev_dbgfs->subdir) {
    kfree(dev_dbgfs);
    return -ENOMEM;
  }

  for (ii = 0; ii < adapter->num_debugfs_entries; ii++) {
    files                    = &dev_debugfs_files[ii];
    dev_dbgfs->rsi_files[ii] = debugfs_create_file(files->name, files->perms, dev_dbgfs->subdir, common, &files->fops);
  }
  return 0;
}
EXPORT_SYMBOL_GPL(rsi_init_dbgfs);

/**
 * rsi_remove_dbgfs() - Removes the previously created dbgfs file entries
 *			in the reverse order of creation.
 * @adapter: Pointer to the adapter structure.
 *
 * Return: None.
 */
void rsi_remove_dbgfs(struct rsi_hw *adapter)
{
  struct rsi_debugfs *dev_dbgfs = adapter->dfsentry;

  if (!dev_dbgfs)
    return;

  debugfs_remove_recursive(dev_dbgfs->subdir);
}
EXPORT_SYMBOL_GPL(rsi_remove_dbgfs);
