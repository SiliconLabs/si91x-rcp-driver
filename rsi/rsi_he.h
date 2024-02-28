/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#ifndef __RSI_HE_H__
#define __RSI_HE_H__

#include <linux/sort.h>
#include "rsi_boot_params.h"
#include "rsi_main.h"

//__9117_CODE_START
/*11ax defines*/

#define _11AX_MCS0 0x400
#define _11AX_MCS1 0x401
#define _11AX_MCS2 0x402
#define _11AX_MCS3 0x403
#define _11AX_MCS4 0x404
#define _11AX_MCS5 0x405
#define _11AX_MCS6 0x406
#define _11AX_MCS7 0x407

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
#ifdef CONFIG_TWT_SUPPORT
static const u8 rsi_ext_capa[] = {
  [9] = WLAN_EXT_CAPA10_TWT_REQUESTER_SUPPORT,
};
#endif
#endif
extern u16 rsi_he_rates[9];
extern const u16 er_su_rates[9];
int rsi_ax_rate_read(struct seq_file *seq, void *data);
int rsi_get_ax_rate(struct inode *inode, struct file *file);
int rsi_read_edca_susp(struct seq_file *seq, void *data);
int rsi_get_edca_susp(struct inode *inode, struct file *file);
ssize_t rsi_set_edca_susp(struct file *filp, const char __user *buff, size_t len, loff_t *data);
ssize_t rsi_set_config_er_su(struct file *filp, const char __user *buff, size_t len, loff_t *data);
int rsi_read_config_er_su(struct seq_file *seq, void *data);
int rsi_get_config_er_su(struct inode *inode, struct file *file);
ssize_t rsi_set_ax_rate(struct file *filp, const char __user *buff, size_t len, loff_t *data);
int rsi_gi_ltf_read(struct seq_file *seq, void *data);
int rsi_get_gi_ltf(struct inode *inode, struct file *file);
ssize_t rsi_set_gi_ltf(struct file *filp, const char __user *buff, size_t len, loff_t *data);
void rsi_update_ax_capabilities(struct rsi_hw *adapter, struct ieee80211_supported_band *sbands);
//__9117_CODE_END
#endif
