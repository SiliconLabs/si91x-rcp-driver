/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#ifndef __RSI_DEBUGFS_H__
#define __RSI_DEBUGFS_H__

#include "rsi_main.h"
#include <linux/debugfs.h>

#ifndef CONFIG_RSI_DEBUGFS
static inline int rsi_init_dbgfs(struct rsi_hw *adapter)
{
  return 0;
}

static inline void rsi_remove_dbgfs(struct rsi_hw *adapter)
{
  return;
}
int rsi_validate_ps_params(struct rsi_common *common, int *ps_params_vals);
//__9117_CODE_START
static inline int rsi_init_917_dbgfs(struct rsi_hw *adapter)
{
  return 0;
}

static inline void rsi_remove_917_dbgfs(struct rsi_hw *adapter)
{
  return;
}
//__9117_CODE_END
#else
struct rsi_dbg_files {
  const char *name;
  umode_t perms;
  const struct file_operations fops;
};

struct rsi_debugfs {
  struct dentry *subdir;
  struct rsi_dbg_ops *dfs_get_ops;
  struct dentry *rsi_files[MAX_DEBUGFS_ENTRIES];
};

//__9117_CODE_START
struct rsi_debugfs_917 {
  struct dentry *subdir;
  struct rsi_dbg_ops *dfs_get_ops;
  struct dentry *rsi_files[MAX_DEBUGFS_ENTRIES_917];
};
int rsi_init_917_dbgfs(struct rsi_hw *adapter);
void rsi_remove_917_dbgfs(struct rsi_hw *adapter);
//__9117_CODE_END
int rsi_init_dbgfs(struct rsi_hw *adapter);
void rsi_remove_dbgfs(struct rsi_hw *adapter);
int rsi_validate_ps_params(struct rsi_common *common, int *ps_params_vals);
#endif
#endif
