// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <linux/etherdevice.h>
#include <linux/if.h>
#include <linux/version.h>
//__9117_CODE_START
#include <linux/ieee80211.h>
#include <net/cfg80211.h>
#include "rsi_he.h"
//__9117_CODE_END
#include "rsi_debugfs.h"
#include "rsi_mgmt.h"
#include "rsi_common.h"
#include "rsi_ps.h"
#include "rsi_hal.h"

static const struct ieee80211_channel rsi_2ghz_channels[] = {
  { .band = NL80211_BAND_2GHZ, .center_freq = 2412, .hw_value = 1 },  /* Channel 1 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2417, .hw_value = 2 },  /* Channel 2 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2422, .hw_value = 3 },  /* Channel 3 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2427, .hw_value = 4 },  /* Channel 4 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2432, .hw_value = 5 },  /* Channel 5 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2437, .hw_value = 6 },  /* Channel 6 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2442, .hw_value = 7 },  /* Channel 7 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2447, .hw_value = 8 },  /* Channel 8 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2452, .hw_value = 9 },  /* Channel 9 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2457, .hw_value = 10 }, /* Channel 10 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2462, .hw_value = 11 }, /* Channel 11 */
#ifndef CONFIG_CARACALLA_BOARD
  { .band = NL80211_BAND_2GHZ, .center_freq = 2467, .hw_value = 12 }, /* Channel 12 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2472, .hw_value = 13 }, /* Channel 13 */
#else
  { .band = NL80211_BAND_2GHZ, .center_freq = 2467, .max_power = 15, .hw_value = 12 }, /* Channel 12 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2472, .max_power = 7, .hw_value = 13 },  /* Channel 13 */
#endif
  { .band = NL80211_BAND_2GHZ, .center_freq = 2484, .hw_value = 14 }, /* Channel 14 */
};

static const struct ieee80211_channel rsi_5ghz_channels[] = {
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5180,
    .hw_value    = 36,
  }, /* Channel 36 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5200,
    .hw_value    = 40,
  }, /* Channel 40 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5220,
    .hw_value    = 44,
  }, /* Channel 44 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5240,
    .hw_value    = 48,
  }, /* Channel 48 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5260,
    .hw_value    = 52,
  }, /* Channel 52 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5280,
    .hw_value    = 56,
  }, /* Channel 56 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5300,
    .hw_value    = 60,
  }, /* Channel 60 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5320,
    .hw_value    = 64,
  }, /* Channel 64 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5500,
    .hw_value    = 100,
  }, /* Channel 100 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5520,
    .hw_value    = 104,
  }, /* Channel 104 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5540,
    .hw_value    = 108,
  }, /* Channel 108 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5560,
    .hw_value    = 112,
  }, /* Channel 112 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5580,
    .hw_value    = 116,
  }, /* Channel 116 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5600,
    .hw_value    = 120,
  }, /* Channel 120 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5620,
    .hw_value    = 124,
  }, /* Channel 124 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5640,
    .hw_value    = 128,
  }, /* Channel 128 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5660,
    .hw_value    = 132,
  }, /* Channel 132 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5680,
    .hw_value    = 136,
  }, /* Channel 136 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5700,
    .hw_value    = 140,
  }, /* Channel 140 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5745,
    .hw_value    = 149,
  }, /* Channel 149 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5765,
    .hw_value    = 153,
  }, /* Channel 153 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5785,
    .hw_value    = 157,
  }, /* Channel 157 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5805,
    .hw_value    = 161,
  }, /* Channel 161 */
  {
    .band        = NL80211_BAND_5GHZ,
    .center_freq = 5825,
    .hw_value    = 165,
  }, /* Channel 165 */
};

struct ieee80211_rate rsi_rates[12] = {
  { .bitrate = STD_RATE_01 * 5, .hw_value = RSI_RATE_1 },    { .bitrate = STD_RATE_02 * 5, .hw_value = RSI_RATE_2 },
  { .bitrate = STD_RATE_5_5 * 5, .hw_value = RSI_RATE_5_5 }, { .bitrate = STD_RATE_11 * 5, .hw_value = RSI_RATE_11 },
  { .bitrate = STD_RATE_06 * 5, .hw_value = RSI_RATE_6 },    { .bitrate = STD_RATE_09 * 5, .hw_value = RSI_RATE_9 },
  { .bitrate = STD_RATE_12 * 5, .hw_value = RSI_RATE_12 },   { .bitrate = STD_RATE_18 * 5, .hw_value = RSI_RATE_18 },
  { .bitrate = STD_RATE_24 * 5, .hw_value = RSI_RATE_24 },   { .bitrate = STD_RATE_36 * 5, .hw_value = RSI_RATE_36 },
  { .bitrate = STD_RATE_48 * 5, .hw_value = RSI_RATE_48 },   { .bitrate = STD_RATE_54 * 5, .hw_value = RSI_RATE_54 },
};

u16 rsi_mcsrates[8] = { RSI_RATE_MCS0, RSI_RATE_MCS1, RSI_RATE_MCS2, RSI_RATE_MCS3,
                        RSI_RATE_MCS4, RSI_RATE_MCS5, RSI_RATE_MCS6, RSI_RATE_MCS7 };

//__9117_CODE_START
u16 rsi_he_rates[9] = { _11AX_MCS0, _11AX_MCS0, _11AX_MCS1, _11AX_MCS2, _11AX_MCS3,
                        _11AX_MCS4, _11AX_MCS5, _11AX_MCS6, _11AX_MCS7 };

const u16 er_su_rates[9] = { _11AX_MCS0, _11AX_MCS0, _11AX_MCS0, _11AX_MCS0, _11AX_MCS0,
                             _11AX_MCS0, _11AX_MCS0, _11AX_MCS1, _11AX_MCS2 };
//__9117_CODE_END
static const u32 rsi_max_ap_stas[16] = {
  32, /* 1 - Wi-Fi alone */
  0,  /* 2 */
  0,  /* 3 */
  0,  /* 4 - BT EDR alone */
  4,  /* 5 - STA + BT EDR */
  32, /* 6 - AP + BT EDR */
  0,  /* 7 */
  0,  /* 8 - BT LE alone */
  4,  /* 9 - STA + BE LE */
  32, /* 10 - AP + BE LE */
  0,  /* 11 */
  0,  /* 12 */
  1,  /* 13 - STA + BT Dual */
  4,  /* 14 - AP + BT Dual */
};

#ifdef CONFIG_RSI_P2P
static const struct ieee80211_iface_limit rsi_iface_limits[] = {
  {
    .max   = 1,
    .types = BIT(NL80211_IFTYPE_STATION),
  },
  {
    .max   = 1,
    .types = BIT(NL80211_IFTYPE_AP) | BIT(NL80211_IFTYPE_P2P_CLIENT) | BIT(NL80211_IFTYPE_P2P_GO),
  },
  {
    .max   = 1,
    .types = BIT(NL80211_IFTYPE_P2P_DEVICE),
  },
};

static const struct ieee80211_iface_combination rsi_iface_combinations[] = {
  {
    .num_different_channels = 1,
    .max_interfaces         = 3,
    .limits                 = rsi_iface_limits,
    .n_limits               = ARRAY_SIZE(rsi_iface_limits),
  },
};
#else
static const struct ieee80211_iface_limit rsi_iface_limits[] = {
  {
    .max = 1,
    .types = BIT(NL80211_IFTYPE_STATION),
  },
  {
    .max = RSI_MAX_VIFS - 1,
    .types = BIT(NL80211_IFTYPE_AP),
  },
};

static const struct ieee80211_iface_combination rsi_iface_combinations[] = {
  {
    .num_different_channels = 1,
    .max_interfaces = RSI_MAX_VIFS,
    .limits = rsi_iface_limits,
    .n_limits = ARRAY_SIZE(rsi_iface_limits),
    .beacon_int_infra_match = true,
  },
};
#endif

#ifdef CONFIG_CARACALLA_BOARD
struct reg_map {
  char country_code[2];
  u8 region_code;
};

static struct reg_map rsi_caracalla_reg_db[MAX_REG_COUNTRIES] = {
  { "AU", NL80211_DFS_ETSI },  { "AT", NL80211_DFS_ETSI },  { "BE", NL80211_DFS_ETSI },  { "BR", NL80211_DFS_WORLD },
  { "CA", NL80211_DFS_FCC },   { "CL", NL80211_DFS_WORLD }, { "CN", NL80211_DFS_WORLD }, { "CO", NL80211_DFS_FCC },
  { "CZ", NL80211_DFS_ETSI },  { "DK", NL80211_DFS_ETSI },  { "FI", NL80211_DFS_ETSI },  { "FR", NL80211_DFS_ETSI },
  { "DE", NL80211_DFS_ETSI },  { "HK", NL80211_DFS_WORLD }, { "IN", NL80211_DFS_WORLD }, { "ID", NL80211_DFS_WORLD },
  { "IE", NL80211_DFS_ETSI },  { "IL", NL80211_DFS_ETSI },  { "IT", NL80211_DFS_ETSI },  { "JP", NL80211_DFS_JP },
  { "KR", NL80211_DFS_WORLD }, { "LU", NL80211_DFS_ETSI },  { "MY", NL80211_DFS_WORLD }, { "MX", NL80211_DFS_FCC },
  { "MA", NL80211_DFS_WORLD }, { "NL", NL80211_DFS_ETSI },
};
#endif

/**
 * rsi_start_ap() - This function is used to configure the AP params 
 *                  check the validity.
 * @hw: Pointer to the hw structure.
 * @vif: Pointer to virtual interface structure.
 *
 * Return: Status of validity of params.
 */
int rsi_start_ap(struct ieee80211_hw *hw,
                 struct ieee80211_vif *vif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
                 ,
                 struct ieee80211_bss_conf *link_conf
#endif
)
{
  if ((vif->bss_conf.beacon_int < MIN_BEACON_INTVL) || (vif->bss_conf.beacon_int > MAX_BEACON_INTVL)) {
    rsi_dbg(ERR_ZONE,
            "%s: Please configure beacon interval within the supported range from 56ms to 1000ms\n",
            __func__);
    return -EINVAL;
  }
  return 0;
}

static int rsi_validate_mac_addr(struct rsi_common *common, u8 *addr_t)
{
  u8 addr[ETH_ALEN] = { 0 };

  if (!memcmp(addr, addr_t, ETH_ALEN)) {
    rsi_dbg(ERR_ZONE, "%s: MAC addr is NULL \n", __func__);
    return -1;
  } else if (memcmp(common->mac_addr, addr_t, ETH_ALEN)) {
    memcpy(common->mac_addr, addr_t, ETH_ALEN);
  }
  return 0;
}

static int rsi_mac80211_get_chan_survey(struct ieee80211_hw *hw, int idx, struct survey_info *survey)
{
  int ret                = 0;
  struct rsi_hw *adapter = hw->priv;

  if (!idx)
    rsi_dbg(INFO_ZONE, "Copying ACS survey results\n");

  if (idx >= adapter->idx)
    return -ENOENT;

  memcpy(survey, &adapter->rsi_survey[idx], sizeof(struct survey_info));
  memset(&adapter->rsi_survey[idx], 0, sizeof(struct survey_info));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
  survey->filled = SURVEY_INFO_NOISE_DBM | SURVEY_INFO_TIME | SURVEY_INFO_TIME_BUSY;
#else
  survey->filled = SURVEY_INFO_NOISE_DBM | SURVEY_INFO_CHANNEL_TIME | SURVEY_INFO_CHANNEL_TIME_BUSY;
#endif
  return ret;
}
#ifdef CONFIG_STA_PLUS_AP
struct ieee80211_vif *rsi_get_vif_using_vap_id(struct rsi_hw *adapter, u8 vap_id)
{
  u8 i;

  for (i = 0; i < RSI_MAX_VIFS; i++) {
    if (!adapter->vifs[i])
      continue;
    if (((struct vif_priv *)adapter->vifs[i]->drv_priv)->vap_id == vap_id)
      return adapter->vifs[i];
  }
  return NULL;
}
struct ieee80211_vif *rsi_get_sta_vif(struct rsi_hw *adapter)
{
  struct ieee80211_vif *vif;
  u8 i;

  for (i = 0; i < RSI_MAX_VIFS; i++) {
    vif = adapter->vifs[i];
    if (!vif)
      continue;
    else if (vif->type == NL80211_IFTYPE_STATION)
      return vif;
  }
  return NULL;
}
struct ieee80211_vif *rsi_get_first_valid_vif(struct rsi_hw *adapter)
{
  struct ieee80211_vif *vif;
  u8 i;

  for (i = 0; i < RSI_MAX_VIFS; i++) {
    vif = adapter->vifs[i];
    if (!vif)
      continue;
    else
      return vif;
  }
  return NULL;
}
#endif
struct ieee80211_vif *rsi_get_vif(struct rsi_hw *adapter, u8 *mac)
{
  u8 i;

  for (i = 0; i < RSI_MAX_VIFS; i++) {
    if (!adapter->vifs[i])
      continue;
    if (!(memcmp(adapter->vifs[i]->addr, mac, ETH_ALEN)))
      return adapter->vifs[i];
  }
  return NULL;
}

/**
 * rsi_is_cipher_wep() -  This function determines if the cipher is WEP or not.
 * @common: Pointer to the driver private structure.
 *
 * Return: If cipher type is WEP, a value of 1 is returned, else 0.
 */
bool rsi_is_cipher_wep(struct rsi_common *common)
{
  if (((common->secinfo.gtk_cipher == WLAN_CIPHER_SUITE_WEP104)
       || (common->secinfo.gtk_cipher == WLAN_CIPHER_SUITE_WEP40))
      && (!common->secinfo.ptk_cipher))
    return true;
  else
    return false;
}

/**
 * rsi_register_rates_channels() - This function registers channels and rates.
 * @adapter: Pointer to the adapter structure.
 * @band: Operating band to be set.
 *
 * Return: None.
 */
static void rsi_register_rates_channels(struct rsi_hw *adapter, int band)
{
  struct ieee80211_supported_band *sbands = &adapter->sbands[band];
  struct rsi_common *common               = adapter->priv;
  void *channels                          = NULL;
  memset(&sbands->ht_cap, 0, sizeof(struct ieee80211_sta_ht_cap));
  sbands->ht_cap.ht_supported = true;

  if (band == NL80211_BAND_2GHZ) {
    channels = kzalloc(sizeof(rsi_2ghz_channels), GFP_KERNEL);
    memcpy(channels, rsi_2ghz_channels, sizeof(rsi_2ghz_channels));
    sbands->band       = NL80211_BAND_2GHZ;
    sbands->n_channels = ARRAY_SIZE(rsi_2ghz_channels);
    sbands->bitrates   = rsi_rates;
    sbands->n_bitrates = ARRAY_SIZE(rsi_rates);
    sbands->ht_cap.cap = (IEEE80211_HT_CAP_SGI_20 | ENABLE_RX_STBC);
  } else {
    channels = kzalloc(sizeof(rsi_5ghz_channels), GFP_KERNEL);
    memcpy(channels, rsi_5ghz_channels, sizeof(rsi_5ghz_channels));
    sbands->band       = NL80211_BAND_5GHZ;
    sbands->n_channels = ARRAY_SIZE(rsi_5ghz_channels);
    sbands->bitrates   = &rsi_rates[4];
    sbands->n_bitrates = ARRAY_SIZE(rsi_rates) - 4;
    sbands->ht_cap.cap = (IEEE80211_HT_CAP_SGI_20 | ENABLE_RX_STBC);
    if (common->enable_40mhz) {
      sbands->ht_cap.cap |= (IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_40 | ENABLE_RX_STBC);
    }
  }

  sbands->channels              = channels;
  sbands->ht_cap.ampdu_factor   = IEEE80211_HT_MAX_AMPDU_16K;
  sbands->ht_cap.ampdu_density  = IEEE80211_HT_MPDU_DENSITY_16;
  sbands->ht_cap.mcs.rx_mask[0] = 0xff;
  sbands->ht_cap.mcs.tx_params  = IEEE80211_HT_MCS_TX_DEFINED;
  /* sbands->ht_cap.mcs.rx_highest = 0x82; */
//__9117_CODE_START
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
  rsi_update_ax_capabilities(adapter, sbands);
#endif
  //__9117_CODE_END
}

#ifdef CONFIG_STA_PLUS_AP
static void rsi_set_min_rate(struct ieee80211_hw *hw,
                             struct ieee80211_sta *sta,
                             struct rsi_common *common,
                             struct ieee80211_vif *vif)
#else
static void rsi_set_min_rate(struct ieee80211_hw *hw, struct ieee80211_sta *sta, struct rsi_common *common)
#endif
{
#ifdef CONFIG_STA_PLUS_AP
  u8 band = hw->conf.chandef.chan->band;
#else
  struct rsi_hw *adapter = common->priv;
  struct ieee80211_vif *vif = adapter->vifs[adapter->sc_nvifs - 1];
  u8 band = hw->conf.chandef.chan->band;
#endif
  u8 ii;
  u32 rate_bitmap;
  bool matched = false;
#ifdef CONFIG_STA_PLUS_AP
  struct vif_priv *vif_info = (struct vif_priv *)vif->drv_priv;
#endif

  if (vif->type == NL80211_IFTYPE_AP) {
    common->bitrate_mask[band] = common->fixedrate_mask[band];
    rate_bitmap                = common->bitrate_mask[band];
  } else {
    common->bitrate_mask[band] = SUPP_RATES[band];
    rate_bitmap                = (common->fixedrate_mask[band] & SUPP_RATES[band]);
  }
  rsi_dbg(INFO_ZONE, "bitrate_mask = %x\n", common->bitrate_mask[band]);
  rsi_dbg(INFO_ZONE, "rate_bitmap = %x\n", rate_bitmap);

  if (rate_bitmap & 0xfff) {
    /* Find out the min rate */
    for (ii = 0; ii < ARRAY_SIZE(rsi_rates); ii++) {
      if (rate_bitmap & BIT(ii)) {
        common->min_rate = rsi_rates[ii].hw_value;
        matched          = true;
        break;
      }
    }
  }

  if (vif->type == NL80211_IFTYPE_STATION) {
#ifndef CONFIG_STA_PLUS_AP
    common->vif_info[0].is_ht = HT_CAP.ht_supported;
#else
    vif_info->is_ht = HT_CAP.ht_supported;
#endif
    if ((HT_CAP.cap & IEEE80211_HT_CAP_SGI_20) || (HT_CAP.cap & IEEE80211_HT_CAP_SGI_40))
#ifndef CONFIG_STA_PLUS_AP
      common->vif_info[0].sgi = true;
#else
      vif_info->sgi = true;
#endif
  }

#ifndef CONFIG_STA_PLUS_AP
  if ((common->vif_info[0].is_ht) && (rate_bitmap >> 12)) {
#else
  if ((vif_info->is_ht) && (rate_bitmap >> 12)) {
#endif
    for (ii = 0; ii < ARRAY_SIZE(rsi_mcsrates); ii++) {
      if ((rate_bitmap >> 12) & BIT(ii)) {
        common->min_rate = rsi_mcsrates[ii];
        matched          = true;
        break;
      }
    }
  }

  if (!matched)
    common->min_rate = 0xffff;

  rsi_dbg(INFO_ZONE, "Min Rate = %d\n", common->min_rate);
}

static void rsi_trigger_antenna_change(struct rsi_common *common)
{
  common->rsi_scan_count++;
  if (common->rsi_scan_count > MAX_SCAN_PER_ANTENNA) {
    common->rsi_scan_count = 0;
    if (common->ant_in_use == ANTENNA_SEL_INT)
      common->obm_ant_sel_val = ANTENNA_SEL_UFL;
    else
      common->obm_ant_sel_val = ANTENNA_SEL_INT;
    if (rsi_set_antenna(common, common->obm_ant_sel_val)) {
      rsi_dbg(ERR_ZONE, "Failed to change antenna to %d\n", common->obm_ant_sel_val);
    } else {
      rsi_dbg(ERR_ZONE, "Antenna is changed to %d\n", common->obm_ant_sel_val);
      common->ant_in_use = common->obm_ant_sel_val;
    }
  }
  return;
}

#define MAX_HW_SCAN_SSID 1
static int rsi_mac80211_hw_scan_start(struct ieee80211_hw *hw,
                                      struct ieee80211_vif *vif,
                                      struct ieee80211_scan_request *hw_req)
{
  struct cfg80211_scan_request *scan_req = &hw_req->req;
  struct cfg80211_ssid *cfg_ssid;
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;
#ifdef CONFIG_STA_PLUS_AP
  struct ieee80211_bss_conf *bss = &vif->bss_conf;
#else
  struct ieee80211_bss_conf *bss = &adapter->vifs[0]->bss_conf;
#endif
  int ii               = 0, n;
  common->scan_request = scan_req;
  //__9117_CODE_START
  if (scan_req->n_ssids > 0) {
    cfg_ssid = &scan_req->ssids[0];
    memcpy(common->ssid, cfg_ssid->ssid, cfg_ssid->ssid_len);
    common->ssid_len = cfg_ssid->ssid_len;
  }
  //__9117_CODE_END
  rsi_dbg(INFO_ZONE, "***** Hardware scan start *****\n");

  if (common->fsm_state != FSM_MAC_INIT_DONE)
    return -ENODEV;
#ifdef CONFIG_RSI_WOW
  if (common->wow_flags & RSI_WOW_ENABLED)
    return -ENETDOWN;
#endif
#ifndef CONFIG_STA_PLUS_AP
  if (!BSS_ASSOC) {
#else
  if (!BSS_ASSOC && (common->vap_mode != CONCURRENT)) {
#endif
    if (common->antenna_diversity)
      rsi_trigger_antenna_change(common);
    if (vif->type == NL80211_IFTYPE_AP) {
      if (adapter->auto_chan_sel == ACS_DISABLE) {
        adapter->auto_chan_sel = ACS_ENABLE;
        adapter->n_channels    = scan_req->n_channels;
        rsi_dbg(MGMT_DEBUG_ZONE, "Auto Channel selection scan start\n");
      }
      adapter->idx = 0;
    }
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 20, 17) || defined(OFFLOAD_SCAN_TO_DEVICE)
    if (scan_req->n_channels == 0)
      return -EINVAL;
    /* Scan already in progress. So return */
    if (common->scan_in_prog)
      return -EBUSY;

    if (rsi_validate_mac_addr(common, vif->addr))
      return -ENODEV;
    cancel_work_sync(&common->scan_work);
    mutex_lock(&common->mutex);
    common->scan_vif     = vif;
    common->scan_in_prog = false;
    common->bgscan_en    = false;
    common->hwscan_en    = false;
    queue_work(common->scan_workqueue, &common->scan_work);
#else
    /* If STA is not connected, return with special value 1,
		 * in order to start sw_scan in mac80211
		 */
    return 1;
#endif
  } else {
    /* Upon connection, make scan count to 0 */
    common->rsi_scan_count = 0;

    /* Wait for EAPOL4 completion before starting bg scan */
    if ((bss->assoc_capability & BIT(4))) {
#ifndef CONFIG_STA_PLUS_AP
      if (!common->start_bgscan) {
#else
      if (!common->start_bgscan && common->vap_mode != CONCURRENT) {
#endif
        mutex_unlock(&common->mutex);
        return -EBUSY;
      }
    }
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0))
    if (common->num_supp_bands > 1) {
      if (common->hw_scan_count < 2) {
        for (ii = 0; ii < scan_req->n_channels; ii++) {
          common->user_channels_list[ii + common->user_channels_count] = scan_req->channels[ii]->hw_value;
        }
        common->user_channels_count += scan_req->n_channels;
        common->hw_scan_count++;
      }
    } else {
      common->user_channels_count = scan_req->n_channels;
      for (ii = 0; ii < scan_req->n_channels; ii++) {
        common->user_channels_list[ii] = scan_req->channels[ii]->hw_value;
      }
    }
#endif
    common->bgscan_info.num_user_channels = scan_req->n_channels;
    for (ii = 0; ii < scan_req->n_channels; ii++) {
      common->bgscan_info.user_channels[ii] = scan_req->channels[ii]->hw_value;
    }

    if (common->efuse_map.module_type == ACX_MODULE)
      update_bgscan_channel_for_acx_module(common);

    common->hwscan_en = true;
    if (!rsi_send_bgscan_params(common, 1)) {
      mutex_lock(&common->bgscan_lock);
      common->bgscan_in_prog = true;
      mutex_unlock(&common->bgscan_lock);
      if (scan_req->n_ssids > MAX_HW_SCAN_SSID) {
        n        = 0;
        cfg_ssid = &scan_req->ssids[n];
        rsi_dbg(INFO_ZONE,
                "handling only '%s'ssid of %d "
                "hw scan ssid's\n",
                cfg_ssid->ssid,
                scan_req->n_ssids);
      }
      if (!rsi_send_bgscan_probe_req(common)) {
        rsi_dbg(INFO_ZONE, "Background scan started\n");
        common->bgscan_en = 1;
      }
    }
  }

  mutex_unlock(&common->mutex);
  return 0;
}

void rsi_mac80211_hw_scan_cancel(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;
#ifdef CONFIG_STA_PLUS_AP

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  struct ieee80211_bss_conf *bss = &vif->bss_conf;
#endif
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  struct ieee80211_bss_conf *bss = &adapter->vifs[0]->bss_conf;
#endif

#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
  struct cfg80211_scan_info info;
#endif
  mutex_lock(&common->mutex);

  common->hw_scan_cancel = true;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 20, 17) || defined(OFFLOAD_SCAN_TO_DEVICE)
  if (common->scan_in_prog) {
    common->scan_in_prog = false;
    rsi_wait_event(&common->cancel_hw_scan_event, EVENT_WAIT_FOREVER);
    rsi_reset_event(&common->cancel_hw_scan_event);
    common->scan_request = NULL;
  }
#endif
  if (common->bgscan_in_prog) {
    if (BSS_ASSOC) {
      rsi_wait_event(&common->cancel_hw_scan_event, EVENT_WAIT_FOREVER);
      rsi_reset_event(&common->cancel_hw_scan_event);
    } else {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
      info.aborted = false;
      ieee80211_scan_completed(adapter->hw, &info);
#else
      ieee80211_scan_completed(adapter->hw, false);
#endif
#ifdef CONFIG_STA_PLUS_AP
      common->hwscan_en = false;
#endif
    }
  }
  common->hw_scan_cancel = false;
  mutex_unlock(&common->mutex);
}
EXPORT_SYMBOL_GPL(rsi_mac80211_hw_scan_cancel);

/**
 * rsi_mac80211_detach() - This function is used to de-initialize the
 *			   Mac80211 stack.
 * @adapter: Pointer to the adapter structure.
 *
 * Return: None.
 */
void rsi_mac80211_detach(struct rsi_hw *adapter)
{
  struct ieee80211_hw *hw   = adapter->hw;
  struct rsi_common *common = adapter->priv;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
  enum nl80211_band band;
#else
  int band;
#endif

  rsi_dbg(INFO_ZONE, "Detach mac80211...\n");

  flush_workqueue(common->scan_workqueue);
  if (hw) {
    ieee80211_stop_queues(hw);
    ieee80211_unregister_hw(hw);
    ieee80211_free_hw(hw);
    adapter->hw       = NULL;
    adapter->sc_nvifs = 0;
  }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
  for (band = 0; band < NUM_NL80211_BANDS; band++) {
#else
  for (band = 0; band < IEEE80211_NUM_BANDS; band++) {
#endif
    struct ieee80211_supported_band *sband = &adapter->sbands[band];

    kfree(sband->channels);
  }

#ifdef CONFIG_RSI_DEBUGFS
  //__9117_CODE_START
  if (adapter->device_model >= RSI_DEV_9117)
    rsi_remove_917_dbgfs(adapter);
  else
    //__9117_CODE_END
    rsi_remove_dbgfs(adapter);
  kfree(adapter->dfsentry);
#endif
}
EXPORT_SYMBOL_GPL(rsi_mac80211_detach);

/**
 * rsi_indicate_tx_status() - This function indicates the transmit status.
 * @adapter: Pointer to the adapter structure.
 * @skb: Pointer to the socket buffer structure.
 * @status: Status
 *
 * Return: None.
 */
void rsi_indicate_tx_status(struct rsi_hw *adapter, struct sk_buff *skb, int status)
{
  struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
  struct skb_info *tx_params;

  if (!adapter->hw) {
    rsi_dbg(ERR_ZONE, "##### No Hardware #####\n");
    return;
  }

  if (adapter->priv->iface_down) {
    rsi_dbg(ERR_ZONE, "#####Interface is down#####\n");
    return;
  }

  if (!status)
    info->flags |= IEEE80211_TX_STAT_ACK;

  tx_params = (struct skb_info *)info->driver_data;
  skb_pull(skb, tx_params->internal_hdr_size);
  memset(info->driver_data, 0, IEEE80211_TX_INFO_DRIVER_DATA_SIZE);

  ieee80211_tx_status_irqsafe(adapter->hw, skb);
}

/**
 * rsi_mac80211_tx() - This is the handler that 802.11 module calls for each
 *		       transmitted frame.SKB contains the buffer starting
 *		       from the IEEE 802.11 header.
 * @hw: Pointer to the ieee80211_hw structure.
 * @control: Pointer to the ieee80211_tx_control structure
 * @skb: Pointer to the socket buffer structure.
 *
 * Return: None
 */
static void rsi_mac80211_tx(struct ieee80211_hw *hw, struct ieee80211_tx_control *control, struct sk_buff *skb)
{
  struct rsi_hw *adapter         = hw->priv;
  struct rsi_common *common      = adapter->priv;
  struct ieee80211_hdr *wlh      = NULL;
  struct ieee80211_tx_info *info = NULL;
  struct skb_info *tx_params     = NULL;
  struct ethhdr *ehdr            = NULL;
  struct ieee80211_vif *vif      = NULL;
  struct ieee80211_bss_conf *bss = NULL;

  info      = IEEE80211_SKB_CB(skb);
  tx_params = (struct skb_info *)info->driver_data;
  wlh       = (struct ieee80211_hdr *)skb->data;
  ehdr      = (struct ethhdr *)skb->data;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0) && LINUX_VERSION_CODE <= KERNEL_VERSION(5, 9, 16))
  if ((info->control.flags & IEEE80211_TX_CTRL_HW_80211_ENCAP))
    tx_params->flags |= ENCAP_OFFLOAD_EN;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
  if (info->flags & IEEE80211_TX_CTL_HW_80211_ENCAP)
    tx_params->flags |= ENCAP_OFFLOAD_EN;
#endif
#ifdef CONFIG_STA_PLUS_AP
  if (tx_params->flags & ENCAP_OFFLOAD_EN)
    vif = rsi_get_vif(adapter, ehdr->h_source);
  else
    vif = rsi_get_vif(adapter, wlh->addr2);
#else
  vif = adapter->vifs[adapter->sc_nvifs - 1];
#endif

  if (!vif) {
    rsi_dbg(ERR_ZONE, "%s: vif is NULL\n", __func__);
    ieee80211_free_txskb(common->priv->hw, skb);
    return;
  }
  bss = &vif->bss_conf;

  if (!(tx_params->flags & ENCAP_OFFLOAD_EN)) {
    if (rsi_validate_mac_addr(common, wlh->addr2)) {
      ieee80211_free_txskb(common->priv->hw, skb);
      return;
    }
    if (ieee80211_is_beacon(wlh->frame_control)) {
      ieee80211_free_txskb(common->priv->hw, skb);
      return;
    }
  }

#ifdef CONFIG_RSI_WOW
  if (common->wow_flags & RSI_WOW_ENABLED) {
    ieee80211_free_txskb(common->priv->hw, skb);
    return;
  }
#endif
  if (((!BSS_ASSOC) || (vif->type == NL80211_IFTYPE_AP)) && (adapter->ps_state == PS_ENABLED))
    rsi_disable_ps(adapter);

  if ((vif->type == NL80211_IFTYPE_STATION) && (adapter->user_ps_en) && (adapter->ps_info.monitor_interval)) {
    check_traffic_pwr_save(adapter);
  }

#if defined(CONFIG_RSI_BT_ALONE) && !defined(CONFIG_RSI_COEX_MODE)
  ieee80211_free_txskb(common->priv->hw, skb);
#else
  rsi_core_xmit(common, skb);
#endif
}

/**
 * rsi_mac80211_start() - This is first handler that 802.11 module calls, since
 *			  the driver init is complete by then, just
 *			  returns success.
 * @hw: Pointer to the ieee80211_hw structure.
 *
 * Return: 0 as success.
 */
static int rsi_mac80211_start(struct ieee80211_hw *hw)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;

  rsi_dbg(ERR_ZONE, "<==== Interface UP ====>\n");
  mutex_lock(&common->mutex);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 20, 17) || defined(OFFLOAD_SCAN_TO_DEVICE)
  common->scan_in_prog = false;
#endif
  common->iface_down = false;
  wiphy_rfkill_start_polling(hw->wiphy);
  if (common->driver_mode != SNIFFER_MODE)
    rsi_send_rx_filter_frame(common, 0);

  mutex_unlock(&common->mutex);

  return 0;
}

/**
 * rsi_mac80211_stop() - This is the last handler that 802.11 module calls.
 * @hw: Pointer to the ieee80211_hw structure.
 *
 * Return: None.
 */
static void rsi_mac80211_stop(struct ieee80211_hw *hw)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;

  rsi_dbg(ERR_ZONE, "<==== Interface DOWN ====>\n");

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 20, 17) || defined(OFFLOAD_SCAN_TO_DEVICE)
  cancel_work_sync(&common->scan_work);
#endif
  mutex_lock(&common->mutex);

  common->iface_down = true;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 20, 17) || defined(OFFLOAD_SCAN_TO_DEVICE)
  if (common->scan_in_prog)
    common->scan_in_prog = false;
#endif
  wiphy_rfkill_stop_polling(hw->wiphy);

  /* Block all rx frames */
  if (common->driver_mode != SNIFFER_MODE) {
    if (common->fsm_state == FSM_MAC_INIT_DONE)
      rsi_send_rx_filter_frame(common, 0xffff);
  }

  mutex_unlock(&common->mutex);
  /* Let the device be in power save, even if the interface is down.*/
  if (common->default_deep_sleep_enable && !adapter->rx_stats_inprog)
    rsi_enable_ps(adapter);
}

/**
 * rsi_mac80211_add_interface() - This function is called when a netdevice
 *				  attached to the hardware is enabled.
 * @hw: Pointer to the ieee80211_hw structure.
 * @vif: Pointer to the ieee80211_vif structure.
 *
 * Return: ret: 0 on success, negative error code on failure.
 */
static int rsi_mac80211_add_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = NULL;
  enum opmode intf_mode;
  int ret                    = 0;
  struct vif_priv *vif_info  = (struct vif_priv *)vif->drv_priv;
  enum vap_status vap_status = VAP_ADD;

  rsi_dbg(INFO_ZONE, "Add Interface Called\n");

  if (!adapter)
    return -ENODEV;
  common = adapter->priv;

#ifdef CONFIG_STA_PLUS_AP
  if ((vif->type == NL80211_IFTYPE_STATION) && (common->vap_mode & STA_ALONE)) {
    /* Not supporting multiple STA vifs */
    rsi_dbg(ERR_ZONE, "%s: Not supported for now\n", __func__);
    return -EOPNOTSUPP;
  }
#endif

  if (common->enabled_uapsd) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
    vif->driver_flags |= IEEE80211_VIF_SUPPORTS_UAPSD;
#endif
  }

  mutex_lock(&common->mutex);

#ifndef CONFIG_STA_PLUS_AP
  /* Not supporting concurrent mode now */
  if (adapter->sc_nvifs > 0) {
    if ((!common->p2p_enabled) && (vif->type != NL80211_IFTYPE_P2P_DEVICE)) {
      ret = -EINVAL;
      goto out;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
    if (adapter->vifs[0]->bss_conf.assoc) {
#else
    if (adapter->vifs[0]->cfg.assoc) {
#endif
      ret = -EINVAL;
      goto out;
    }
  }
#endif

  switch (vif->type) {
    case NL80211_IFTYPE_STATION:
      rsi_dbg(INFO_ZONE, "Station Mode");
      intf_mode = STA_OPMODE;
      if (common->enable_encap_offload) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0) && LINUX_VERSION_CODE <= KERNEL_VERSION(5, 9, 16))
        ieee80211_set_hw_80211_encap(vif, true);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
        vif->offload_flags = IEEE80211_OFFLOAD_ENCAP_ENABLED;
#endif
      }
      break;
    case NL80211_IFTYPE_AP:
      rsi_dbg(INFO_ZONE, "AP Mode");
      intf_mode = AP_OPMODE;
      break;
    case NL80211_IFTYPE_P2P_DEVICE:
      rsi_dbg(INFO_ZONE, "P2P Device Mode");
      common->p2p_enabled = 1;
      intf_mode           = STA_OPMODE;
      break;
    case NL80211_IFTYPE_P2P_GO:
      rsi_dbg(INFO_ZONE, "P2P GO Mode");
      intf_mode           = P2P_GO_OPMODE;
      common->p2p_enabled = 1;
      break;
    case NL80211_IFTYPE_P2P_CLIENT:
      rsi_dbg(INFO_ZONE, "P2P Client Mode");
      intf_mode           = P2P_CLIENT_OPMODE;
      common->p2p_enabled = 1;
      break;
    case NL80211_IFTYPE_MONITOR:
      rsi_dbg(INFO_ZONE, "Monitor mode ");
      intf_mode = STA_OPMODE;
      break;
    default:
      rsi_dbg(ERR_ZONE, "Unsupported mode");
      ret = -EINVAL;
      goto out;
  }
#ifndef CONFIG_STA_PLUS_AP
  if (adapter->sc_nvifs >= RSI_MAX_VIFS)
    adapter->vifs[adapter->sc_nvifs - 1] = vif;
  else
    adapter->vifs[adapter->sc_nvifs++] = vif;
#endif

  if (!common->p2p_enabled) {
#ifdef CONFIG_STA_PLUS_AP
    int i;

    for (i = 0; i < RSI_MAX_VIFS; i++)
      if (!test_bit(i, (void *)&adapter->vif_bit_map))
        break;
    if (i == RSI_MAX_VIFS) {
      rsi_dbg(ERR_ZONE, "Max vifs reached\n");
      ret = -EPERM;
      goto out;
    } else {
      vif_info->vap_id = i;
      adapter->vifs[i] = vif;
      adapter->sc_nvifs++;
      set_bit(i, (void *)&adapter->vif_bit_map);
      if (vif->type == NL80211_IFTYPE_STATION)
        common->vap_mode |= STA_ALONE;
      else if (vif->type == NL80211_IFTYPE_AP)
        common->vap_mode |= AP_ALONE;
    }
#else
    vif_info->vap_id = 0;
    common->last_vap_id = 0;
#endif
  } else {
#ifdef CONFIG_STA_PLUS_AP
    if (adapter->sc_nvifs >= RSI_MAX_VIFS)
      adapter->vifs[adapter->sc_nvifs - 1] = vif;
    else
      adapter->vifs[adapter->sc_nvifs++] = vif;
#endif
    if ((intf_mode == AP_OPMODE) || (intf_mode == P2P_GO_OPMODE)) {
      vif_info->vap_id = common->last_vap_id + 1;
    } else {
      vif_info->vap_id = common->last_vap_id;
      vap_status       = VAP_UPDATE;
    }
  }
  if (common->driver_mode != SNIFFER_MODE) {
    ret = rsi_set_vap_capabilities(common, intf_mode, vif->addr, vif_info->vap_id, vap_status);
    if (ret) {
      rsi_dbg(ERR_ZONE, "Failed to set VAP capabilities\n");
      goto out;
    }
  } else {
    mutex_unlock(&common->mutex);
    return 0;
  }

  memset(vif_info->rx_bcmc_pn, 0, IEEE80211_CCMP_PN_LEN);
  vif_info->rx_pn_valid = false;
  vif_info->key         = NULL;

  if ((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO)) {
    int i;

    common->bc_mc_seqno = 1;
    rsi_send_rx_filter_frame(common, DISALLOW_BEACONS);
    common->min_rate = 0xffff;
#ifdef CONFIG_STA_PLUS_AP
    for (i = 1; i < common->max_stations; i++)
#else
    for (i = 0; i < common->max_stations; i++)
#endif
      common->stations[i].sta = NULL;
  }

out:
  mutex_unlock(&common->mutex);

  return ret;
}

/**
 * rsi_mac80211_remove_interface() - This function notifies driver that an
 *				     interface is going down.
 * @hw: Pointer to the ieee80211_hw structure.
 * @vif: Pointer to the ieee80211_vif structure.
 *
 * Return: None.
 */
static void rsi_mac80211_remove_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;
  struct vif_priv *vif_info = (struct vif_priv *)vif->drv_priv;
#ifdef CONFIG_STA_PLUS_AP
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
  struct cfg80211_scan_info info;
#endif
#endif
  int i;

  rsi_dbg(INFO_ZONE, "Remove Interface Called\n");

  if (adapter->sc_nvifs <= 0)
    return;

  mutex_lock(&common->mutex);

  for (i = 0; i < RSI_MAX_VIFS; i++) {
    if (adapter->vifs[i] == vif) {
      adapter->sc_nvifs--;
      switch (adapter->vifs[i]->type) {
        case NL80211_IFTYPE_STATION:
          rsi_set_vap_capabilities(common, STA_OPMODE, vif->addr, vif_info->vap_id, VAP_DELETE);
          break;
        case NL80211_IFTYPE_AP:
          rsi_set_vap_capabilities(common, AP_OPMODE, vif->addr, vif_info->vap_id, VAP_DELETE);
#ifdef CONFIG_STA_PLUS_AP
          if (common->vap_mode == CONCURRENT && common->bgscan_in_prog) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
            info.aborted = false;
            ieee80211_scan_completed(adapter->hw, &info);
#else
            ieee80211_scan_completed(adapter->hw, false);
#endif
            mutex_lock(&common->bgscan_lock);
            common->bgscan_in_prog = false;
            mutex_unlock(&common->bgscan_lock);
          }
#endif
          break;
        case NL80211_IFTYPE_P2P_CLIENT:
          rsi_set_vap_capabilities(common, P2P_CLIENT_OPMODE, vif->addr, vif_info->vap_id, VAP_DELETE);
          break;
        case NL80211_IFTYPE_P2P_GO:
          rsi_set_vap_capabilities(common, P2P_GO_OPMODE, vif->addr, vif_info->vap_id, VAP_DELETE);
          break;
        default:
          goto out;
      }
      adapter->vifs[i] = NULL;
#ifdef CONFIG_STA_PLUS_AP
      clear_bit(vif_info->vap_id, (void *)&adapter->vif_bit_map);
      common->vap_mode = NO_VAP;
      for (i = 0; i < RSI_MAX_VIFS; i++) {
        if (!adapter->vifs[i])
          continue;
        else if (adapter->vifs[i]->type == NL80211_IFTYPE_STATION)
          common->vap_mode |= STA_ALONE;
        else if (adapter->vifs[i]->type == NL80211_IFTYPE_AP)
          common->vap_mode |= AP_ALONE;
      }
#endif
      break;
    }
  }

out:
  mutex_unlock(&common->mutex);
}

static int rsi_mac80211_change_interface(struct ieee80211_hw *hw,
                                         struct ieee80211_vif *vif,
                                         enum nl80211_iftype newtype,
                                         bool newp2p)
{
  struct rsi_hw *adapter    = (struct rsi_hw *)hw->priv;
  struct rsi_common *common = (struct rsi_common *)adapter->priv;
  struct vif_priv *vif_info = (struct vif_priv *)vif->drv_priv;
  int status                = 0;
  enum opmode intf_mode;

  rsi_dbg(INFO_ZONE, "Change Interface: New_type = %d, New_p2p = %d\n", newtype, newp2p);

  mutex_lock(&common->mutex);

  vif->type = newtype;
  vif->p2p  = newp2p;

  switch (newtype) {
    case NL80211_IFTYPE_AP:
      if (adapter->ps_state == PS_ENABLED)
        rsi_disable_ps(adapter);
      rsi_dbg(INFO_ZONE, "Change to AP Mode\n");
      intf_mode = AP_OPMODE;
      break;
    case NL80211_IFTYPE_STATION:
      intf_mode = STA_OPMODE;
      rsi_dbg(INFO_ZONE, "Change to Station Mode\n");
      break;
    case NL80211_IFTYPE_P2P_CLIENT:
      rsi_dbg(INFO_ZONE, "Change to P2P Client Mode\n");
      intf_mode = P2P_CLIENT_OPMODE;
      break;
    case NL80211_IFTYPE_P2P_GO:
      rsi_dbg(INFO_ZONE, "Change to P2P Go Mode\n");
      intf_mode = P2P_GO_OPMODE;
      break;
    default:
      status = -EINVAL;
      goto out;
  }
  //	if (intf_mode == AP_OPMODE) {
  rsi_set_vap_capabilities(common,
                           //STA_OPMODE,
                           common->last_vap_type,
                           vif->addr,
                           vif_info->vap_id,
                           VAP_DELETE);
  status = rsi_set_vap_capabilities(common, intf_mode, vif->addr, vif_info->vap_id, VAP_ADD);
#if 0
	} else if ((intf_mode != common->last_vap_type) ||
		   (!ether_addr_equal(vif->addr, common->last_vap_addr))) {
		status = rsi_set_vap_capabilities(common,
						  intf_mode,
						  vif->addr,
						  vif_info->vap_id,
						  VAP_UPDATE);
	}
#endif
  if ((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO)) {
    common->bc_mc_seqno = 1;
    rsi_send_rx_filter_frame(common, DISALLOW_BEACONS);
    common->min_rate = 0xffff;
  }

out:
  mutex_unlock(&common->mutex);
  return status;
}

/**
 * rsi_channel_change() - This function is a performs the checks
 *			  required for changing a channel and sets
 *			  the channel accordingly.
 * @hw: Pointer to the ieee80211_hw structure.
 *
 * Return: 0 on success, negative error code on failure.
 */
#ifdef CONFIG_STA_PLUS_AP
static int rsi_channel_change(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
#else
static int rsi_channel_change(struct ieee80211_hw *hw)
#endif
{
  struct rsi_hw *adapter            = hw->priv;
  struct rsi_common *common         = adapter->priv;
  int status                        = -EOPNOTSUPP;
  struct ieee80211_channel *curchan = hw->conf.chandef.chan;
  u16 channel                       = curchan->hw_value;
  struct ieee80211_bss_conf *bss    = NULL;
#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = adapter->vifs[adapter->sc_nvifs - 1];
#endif

  if (adapter->sc_nvifs <= 0) {
    rsi_dbg(ERR_ZONE, "%s: No virtual interface found\n", __func__);
    return -EINVAL;
  }
  bss = &vif->bss_conf;

  rsi_dbg(INFO_ZONE,
          "%s: Set channel: %d MHz type: %d channel_no %d\n",
          __func__,
          curchan->center_freq,
          curchan->flags,
          channel);

  if ((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO)) {
    rsi_dbg(INFO_ZONE, "Configure channel %d for AP\n", channel);
    if (rsi_band_check(common, curchan->band)) {
      rsi_dbg(ERR_ZONE, "Failed to set band\n");
      return -EINVAL;
    }
#ifdef CONFIG_STA_PLUS_AP
    if (rsi_set_channel(common, curchan, vif)) {
#else
    if (rsi_set_channel(common, curchan)) {
#endif
      rsi_dbg(ERR_ZONE, "Failed to set the channel\n");
      return -EINVAL;
    }
    common->ap_channel = curchan;
    return 0;
  }
  common->ap_channel           = curchan;
  common->mac80211_cur_channel = channel;
  if (BSS_ASSOC) {
    rsi_dbg(INFO_ZONE, "%s: connected\n", __func__);

    if (common->bgscan_en)
      return 0;

    if (!common->hw_data_qs_blocked && (rsi_get_connected_channel(adapter) != channel)) {
      rsi_dbg(INFO_ZONE, "blk data q %d\n", channel);
      if (!rsi_send_block_unblock_frame(common, true))
        common->hw_data_qs_blocked = true;
    }
  } else {
    rsi_dbg(INFO_ZONE, "assoc status:%d channel:%d\n", BSS_ASSOC, channel);
  }

  status = rsi_band_check(common, curchan->band);
  if (!status)
#ifdef CONFIG_STA_PLUS_AP
    status = rsi_set_channel(adapter->priv, curchan, vif);
#else
    status = rsi_set_channel(adapter->priv, curchan);
#endif

  if (BSS_ASSOC) {
    if (common->hw_data_qs_blocked && (rsi_get_connected_channel(adapter) == channel)) {
      rsi_dbg(INFO_ZONE, "unblk data q %d\n", channel);
      if (!rsi_send_block_unblock_frame(common, false))
        common->hw_data_qs_blocked = false;
    }
  } else {
#if 0
		if (common->hw_data_qs_blocked) {
			rsi_dbg(INFO_ZONE, "unblk data q %d\n", channel);
			if (!rsi_send_block_unblock_frame(common, false))
				common->hw_data_qs_blocked = false;
		}
#endif
  }

  return status;
}

/**
 * rsi_config_power() - This function configures tx power in device
 * @hw: Pointer to the ieee80211_hw structure.
 *
 * Return: 0 on success, negative error code on failure.
 */
static int rsi_config_power(struct ieee80211_hw *hw)
{
  struct rsi_hw *adapter      = hw->priv;
  struct rsi_common *common   = adapter->priv;
  struct ieee80211_conf *conf = &hw->conf;
  int status;

  if (adapter->sc_nvifs <= 0) {
    rsi_dbg(ERR_ZONE, "%s: No virtual interface found\n", __func__);
    return -EINVAL;
  }

  rsi_dbg(INFO_ZONE, "%s: Set tx power: %d dBM\n", __func__, conf->power_level);

  if (conf->power_level == common->tx_power)
    return 0;

  common->tx_power = conf->power_level;

  status = rsi_send_radio_params_update(common);

  return status;
}

/**
 * rsi_mac80211_config() - This function is a handler for configuration
 *			   requests. The stack calls this function to
 *			   change hardware configuration, e.g., channel.
 * @hw: Pointer to the ieee80211_hw structure.
 * @changed: Changed flags set.
 *
 * Return: 0 on success, negative error code on failure.
 */
static int rsi_mac80211_config(struct ieee80211_hw *hw, u32 changed)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;
#ifdef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif      = rsi_get_sta_vif(adapter); // Get vifs[STA] always in concurrent mode
  struct ieee80211_bss_conf *bss = NULL;
#else
  struct ieee80211_vif *vif = adapter->vifs[adapter->sc_nvifs - 1];
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  struct ieee80211_bss_conf *bss = &vif->bss_conf;
#endif

#endif
  struct ieee80211_conf *conf = &hw->conf;
  int status                  = -EOPNOTSUPP;

  if (common->fsm_state != FSM_MAC_INIT_DONE)
    return -ENODEV;
#ifdef CONFIG_STA_PLUS_AP
  if (!vif) {
    rsi_dbg(ERR_ZONE, "vlm: %s[%d] We are running AP alone mode...\n", __func__, __LINE__);
    vif = rsi_get_first_valid_vif(adapter);
  }
  bss = &vif->bss_conf;
#endif

  mutex_lock(&common->mutex);

  /* channel */
  if (changed & IEEE80211_CONF_CHANGE_CHANNEL) {
    if (!BSS_ASSOC && (adapter->ps_state == PS_ENABLED))
      rsi_disable_ps(adapter);
#ifdef CONFIG_STA_PLUS_AP
    status = rsi_channel_change(hw, vif);
#else
    status = rsi_channel_change(hw);
#endif
  }

  /* listen interval */
  if (changed & IEEE80211_CONF_CHANGE_LISTEN_INTERVAL) {
    rsi_dbg(INFO_ZONE, "listen_int = %d\n", conf->listen_interval);
  }

  /* tx power */
  if (changed & IEEE80211_CONF_CHANGE_POWER) {
    rsi_dbg(INFO_ZONE, "%s: Configuring Power\n", __func__);
    status = rsi_config_power(hw);
  }

  /* retry limit */
  if (changed & IEEE80211_CONF_CHANGE_RETRY_LIMITS) {
    /* FIXME */
  }

  /* Power save parameters */
  if ((changed & IEEE80211_CONF_CHANGE_PS) && (vif->type == NL80211_IFTYPE_STATION)) {
    unsigned long flags;

    spin_lock_irqsave(&adapter->ps_lock, flags);
    if ((conf->flags & IEEE80211_CONF_PS) && (adapter->user_ps_en == 0)) {
      rsi_enable_ps(adapter);
      adapter->user_ps_en = 1;
    } else if (adapter->ps_state == PS_ENABLED) {
      rsi_disable_ps(adapter);
      adapter->user_ps_en = 0;
    } else if (adapter->ps_state == PS_NONE) {
      rsi_dbg(INFO_ZONE, "Device already Power Save Disabled State\n");
      adapter->user_ps_en = 0;
    }
    spin_unlock_irqrestore(&adapter->ps_lock, flags);
  }

  /* RTS threshold */
  if (changed & WIPHY_PARAM_RTS_THRESHOLD) {
    rsi_dbg(INFO_ZONE, "RTS threshold\n");
    if ((common->rts_threshold) < IEEE80211_MAX_RTS_THRESHOLD) {
      rsi_dbg(INFO_ZONE, "%s: Sending vap updates....\n", __func__);
      status = rsi_send_vap_dynamic_update(common);
    }
  }

  mutex_unlock(&common->mutex);

  return status;
}

/**
 * rsi_get_connected_channel() - This function is used to get the current
 *				 connected channel number.
 * @adapter: Pointer to the adapter structure.
 *
 * Return: Current connected AP's channel number is returned.
 */
u16 rsi_get_connected_channel(struct rsi_hw *adapter)
{
#ifdef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = NULL;
  u8 i;
#else
  struct ieee80211_vif *vif = adapter->vifs[adapter->sc_nvifs - 1];
#endif
  struct rsi_common *common = adapter->priv;

  if (common->iface_down)
    return -EINVAL;
#ifdef CONFIG_STA_PLUS_AP
  /* Return a valid channel of AP or else running STA */
  for (i = 0; i < RSI_MAX_VIFS; i++) {
    vif = adapter->vifs[i];
    if (vif) {
      struct ieee80211_bss_conf *bss    = &vif->bss_conf;
      struct ieee80211_channel *channel = bss->chandef.chan;
      if ((!BSS_ASSOC) && (vif->type != NL80211_IFTYPE_AP))
        continue;

      if (!channel)
        return 0;
      return channel->hw_value;
    }
  }
  return -ENODATA;
#else
  if ((!adapter->vifs[0]) || (!vif))
    return -EINVAL;
  if (vif) {
    struct ieee80211_bss_conf *bss = &vif->bss_conf;
    struct ieee80211_channel *channel = bss->chandef.chan;

    if (!channel)
      return 0;
    return channel->hw_value;
  }

  return 0;
#endif
}

void rsi_resume_conn_channel(struct rsi_hw *adapter, struct ieee80211_vif *vif)
{
  struct rsi_common *common      = adapter->priv;
  struct ieee80211_bss_conf *bss = NULL;

  if (common->iface_down)
    return;
#ifdef CONFIG_STA_PLUS_AP
  if (!vif)
#else
  if ((!adapter->vifs[0]) || (!vif))
#endif
    return;

  bss = &vif->bss_conf;
  if ((BSS_ASSOC) || (vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO)) {
    struct ieee80211_channel *channel = bss->chandef.chan;

    if (!channel)
      return;

    rsi_band_check(common, channel->band);

#ifdef CONFIG_STA_PLUS_AP
    rsi_set_channel(common, channel, vif);
#else
    rsi_set_channel(common, channel);
#endif
    rsi_dbg(INFO_ZONE, "resumed to channel %d\n", channel->hw_value);
  }
}

/**
 * rsi_mac80211_bss_info_changed() - This function is a handler for config
 *				     requests related to BSS parameters that
 *				     may vary during BSS's lifespan.
 * @hw: Pointer to the ieee80211_hw structure.
 * @vif: Pointer to the ieee80211_vif structure.
 * @bss_conf: Pointer to the ieee80211_bss_conf structure.
 * @changed: Changed flags set.
 *
 * Return: None.
 */
static void rsi_mac80211_bss_info_changed(struct ieee80211_hw *hw,
                                          struct ieee80211_vif *vif,
                                          struct ieee80211_bss_conf *bss_conf,
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
                                          u32 changed)
#else
                                          u64 changed)
#endif
{
  struct rsi_hw *adapter         = hw->priv;
  struct rsi_common *common      = adapter->priv;
  struct ieee80211_bss_conf *bss = &vif->bss_conf;
  struct ieee80211_conf *conf    = &hw->conf;
  u16 rx_filter_word             = 0;
  //__9117_CODE_START
  u8 update_heparams_dynamic = 0;
#ifdef CONFIG_UORA_SUPPORT
  u8 uora_eocw_min, uora_eocw_max;
#endif
  //__9117_CODE_END
  u8 addr[ETH_ALEN] = { 0 };
  u8 basic_rate;

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  rsi_dbg(INFO_ZONE, "%s: BSS status changed; changed=%08x\n", __func__, changed);
#else
  rsi_dbg(INFO_ZONE, "%s: BSS status changed; changed=%08llx\n", __func__, changed);
#endif

  if (common->fsm_state != FSM_MAC_INIT_DONE)
    return;

  mutex_lock(&common->mutex);

  if ((changed & BSS_CHANGED_BSSID) && (changed & BSS_CHANGED_BASIC_RATES)) {
    basic_rate = ffs(vif->bss_conf.basic_rates) - 1;
    if (common->band == NL80211_BAND_2GHZ)
      common->minimum_basic_rate = rsi_rates[basic_rate].hw_value;

    else
      common->minimum_basic_rate = rsi_rates[basic_rate + 4].hw_value;
  }
  if ((changed & BSS_CHANGED_ASSOC) && (vif->type == NL80211_IFTYPE_STATION)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
    rsi_dbg(INFO_ZONE, "%s: Changed Association status: %d\n", __func__, bss_conf->assoc);
    bss->assoc = bss_conf->assoc;
#else
    rsi_dbg(INFO_ZONE, "%s: Changed Association status: %d\n", __func__, vif->cfg.assoc);
#endif
    if (BSS_ASSOC) {
//__9117_CODE_START
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
      adapter->ax_params._11ax_enabled = bss->he_support;
      rsi_dbg(ERR_ZONE, "HE support: %d\n", bss->he_support);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0))
      common->enable_er_su =
        bss->he_support ? !(bss_conf->he_operation.he_oper_params & IEEE80211_HE_OPERATION_ER_SU_DISABLE) : 0;
#else
      common->enable_er_su = bss->he_support ? !(bss_conf->he_oper.params & IEEE80211_HE_OPERATION_ER_SU_DISABLE) : 0;
#endif
#ifdef CONFIG_TWT_SUPPORT
      adapter->twt_current_status = TWT_TEARDOWN_ERR_NOACTIVE_SESS;
#endif
#endif
      //__9117_CODE_END
      /* Send the RX filter frame */
      rx_filter_word = (ALLOW_DATA_ASSOC_PEER | ALLOW_CTRL_ASSOC_PEER | ALLOW_MGMT_ASSOC_PEER | 0);
      if (DEV_MODEL_9116)
        rx_filter_word |= DISALLOW_BEACONS;
      rsi_send_rx_filter_frame(common, rx_filter_word);
      //__9117_CODE_START
    } else {
      adapter->ax_params._11ax_enabled = 0;
      //__9117_CODE_END
    }
    rsi_dbg(INFO_ZONE, "assoc_status=%d, qos=%d, aid=%d\n", BSS_ASSOC, bss->qos, AID);
    rsi_dbg(INFO_ZONE,
            "bssid=%02x:%02x:%02x:%02x:%02x:%02x",
            bss->bssid[0],
            bss->bssid[1],
            bss->bssid[2],
            bss->bssid[3],
            bss->bssid[4],
            bss->bssid[5]);
//__9117_CODE_START
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
    if (BSS_ASSOC) {
      adapter->ax_params.bss_color         = bss->he_bss_color.color;
      adapter->ax_params.spatial_reuse_en  = bss->he_obss_pd.enable;
      adapter->ax_params.mbssid_mask_value = bss->bssid_indicator;
      adapter->ax_params.obss_pd_min       = bss->he_obss_pd.min_offset;
      adapter->ax_params.obss_pd_max       = bss->he_obss_pd.max_offset;
      if (adapter->ax_params._11ax_enabled) {
        common->txop_dur_rts_threshold = bss_conf->frame_time_rts_th;
        rsi_send_vap_dynamic_update(common);
        common->txop_dur_rts_threshold = MIN_TX_OP_RTS_TH;
      }
#ifdef CONFIG_UORA_SUPPORT
      if (bss->uora_exists) {
        uora_eocw_min        = (bss->uora_ocw_range & 0x7);
        common->uora_ocw_min = (1 << uora_eocw_min) - 1;
        uora_eocw_max        = ((bss->uora_ocw_range >> 3) & 0x7);
        common->uora_ocw_max = (1 << uora_eocw_max) - 1; /* calculating 2^n - 1 value */
      }
#endif
      rsi_dbg(INFO_ZONE, "\nBss color:%d, Spatial reuse:%d\n", bss->he_bss_color.color, bss->he_obss_pd.enable);
#ifdef CONFIG_TWT_SUPPORT
      if (bss->he_support && (changed & BSS_CHANGED_TWT)) {
        adapter->ap_support_twt = bss_conf->twt_requester;
        rsi_dbg(ERR_ZONE, "AP support TWT :%d\n", adapter->ap_support_twt);
      }
#endif
    } else {
      memset(&adapter->ax_params, 0, sizeof(struct rsi_ax_params));
    }
#endif
    //__9117_CODE_END

    /* Send peer notify to device */
    rsi_dbg(INFO_ZONE, "Indicate bss status to device\n");
    rsi_inform_bss_status(common,
                          STA_OPMODE,
                          BSS_ASSOC,
                          bss->bssid,
                          bss->qos,
                          AID,
                          NULL,
                          0,
#ifdef CONFIG_STA_PLUS_AP
                          bss->assoc_capability,
                          vif);
#else
                          bss->assoc_capability);
#endif

    //__9117_CODE_START
    update_heparams_dynamic = 1;
    //__9117_CODE_END
    rsi_dbg(INFO_ZONE,
            "Beacon_Int = %d Lis_Int = %d Dtim = %d\n",
            bss->beacon_int,
            conf->listen_interval,
            bss->dtim_period);

    /* If UAPSD is updated send ps params */
    if (BSS_ASSOC) {
      if (common->uapsd_bitmap) {
        rsi_dbg(INFO_ZONE, "Configuring UAPSD\n");
        rsi_conf_uapsd(adapter);
      }
    } else
      common->uapsd_bitmap = 0;
  }

  if ((vif->type == NL80211_IFTYPE_STATION) && changed & BSS_CHANGED_BSSID) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
    if (common->peer_notify_state == true && !bss_conf->assoc && !(changed & BSS_CHANGED_ASSOC)
#else
    if (common->peer_notify_state == true && !vif->cfg.assoc && !(changed & BSS_CHANGED_ASSOC)
#endif
        && !(memcmp(bss_conf->bssid, addr, ETH_ALEN))) {
      rsi_send_sta_notify_frame(common,
                                STA_OPMODE,
                                STA_DISCONNECTED,
                                common->sta_bssid,
                                bss_conf->qos,
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
                                bss_conf->aid,
#else
                                vif->cfg.aid,
#endif
#ifdef CONFIG_STA_PLUS_AP
                                0,
                                vif);
#else
                                0);
#endif
    }
  }
//__9117_CODE_START
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
  if (!update_heparams_dynamic) {
    if (BSS_ASSOC && (changed & BSS_CHANGED_HE_BSS_COLOR)) {
      adapter->ax_params.bss_color        = bss->he_bss_color.color;
      adapter->ax_params.spatial_reuse_en = bss->he_obss_pd.enable;
      rsi_load_radio_caps(common);
    }
  }

  if ((vif->type == NL80211_IFTYPE_STATION) && bss->he_support && (changed & BSS_CHANGED_QOS)) {
    rsi_dbg(INFO_ZONE, "%s: Update MU-EDCA Parameters \n", __func__);
    rsi_load_radio_caps(common);
  }
#endif
  //__9117_CODE_END

  if ((vif->type == NL80211_IFTYPE_STATION) && changed & BSS_CHANGED_CQM) {
    rsi_dbg(INFO_ZONE, "%s: Changed CQM\n", __func__);
    common->cqm_info.last_cqm_event_rssi = 0;
    common->cqm_info.rssi_thold          = bss_conf->cqm_rssi_thold;
    common->cqm_info.rssi_hyst           = bss_conf->cqm_rssi_hyst;
    rsi_dbg(INFO_ZONE,
            "RSSI throld & hysteresis are: %d %d\n",
            common->cqm_info.rssi_thold,
            common->cqm_info.rssi_hyst);
  }

  if (changed & BSS_CHANGED_TXPOWER) {
    rsi_dbg(INFO_ZONE, "%s: Changed TX power: %d\n", __func__, bss_conf->txpower);
  }

  if (changed & BSS_CHANGED_BEACON_INT) {
    rsi_dbg(INFO_ZONE, "%s: Changed Beacon interval: %d\n", __func__, bss_conf->beacon_int);
    if (common->beacon_interval != bss->beacon_int) {
      common->beacon_interval = bss->beacon_int;
      if (vif->type == NL80211_IFTYPE_AP)
        rsi_set_vap_capabilities(common, AP_OPMODE, vif->addr, ((struct vif_priv *)vif->drv_priv)->vap_id, VAP_UPDATE);
    }
  }

  if ((changed & BSS_CHANGED_BEACON_ENABLED)
      && ((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO))) {
    if (bss->enable_beacon) {
      rsi_dbg(INFO_ZONE, "===> BEACON ENABLED <===\n");
      common->beacon_enabled = 1;
    } else {
      rsi_dbg(INFO_ZONE, "===> BEACON DISABLED <===\n");
      common->beacon_enabled = 0;
    }
  }
  if ((vif->type == NL80211_IFTYPE_AP) && common->beacon_enabled && adapter->auto_chan_sel) {
    rsi_dbg(MGMT_DEBUG_ZONE, "ACS Final Channel selection\n");
    adapter->auto_chan_sel = ACS_DISABLE;
  }
  if (changed & BSS_CHANGED_ERP_CTS_PROT) {
    rsi_dbg(ERR_ZONE, "%s: Change of ERP INFO: %d\n", __func__, bss_conf->use_cts_prot);
    common->use_protection = bss_conf->use_cts_prot;
    rsi_dbg(ERR_ZONE, "%s: Sending vap updates....\n", __func__);
    rsi_send_vap_dynamic_update(common);
  }

  mutex_unlock(&common->mutex);
}

/**
 * rsi_mac80211_conf_filter() - This function configure the device's RX filter.
 * @hw: Pointer to the ieee80211_hw structure.
 * @changed: Changed flags set.
 * @total_flags: Total initial flags set.
 * @multicast: Multicast.
 *
 * Return: None.
 */
static void rsi_mac80211_conf_filter(struct ieee80211_hw *hw, u32 changed_flags, u32 *total_flags, u64 multicast)
{
#if 0
	struct rsi_hw *adapter = hw->priv;
	struct rsi_common *common = adapter->priv;
	u16 rx_filter_word = 0;
#endif

  /* Not doing much here as of now */
  *total_flags &= RSI_SUPP_FILTERS;

  //	rsi_send_rx_filter_frame(common, rx_filter_word);
}

/**
 * rsi_mac80211_conf_tx() - This function configures TX queue parameters
 *			    (EDCF (aifs, cw_min, cw_max), bursting)
 *			    for a hardware TX queue.
 * @hw: Pointer to the ieee80211_hw structure
 * @vif: Pointer to the ieee80211_vif structure.
 * @queue: Queue number.
 * @params: Pointer to ieee80211_tx_queue_params structure.
 *
 * Return: 0 on success, negative error code on failure.
 */
static int rsi_mac80211_conf_tx(struct ieee80211_hw *hw,
                                struct ieee80211_vif *vif,
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
                                u16 queue,
#else
                                unsigned int link_id,
                                u16 queue,
#endif
                                const struct ieee80211_tx_queue_params *params)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;
  u8 idx                    = 0;
  bool wmm_config_complete  = 0;

  if (queue >= IEEE80211_NUM_ACS)
    return 0;

  rsi_dbg(INFO_ZONE,
          "[Conf] queue:%d, aifs:%d, cwmin:%d cwmax:%d, txop:%d uapsd:%d\n",
          queue,
          params->aifs,
          params->cw_min,
          params->cw_max,
          params->txop,
          params->uapsd);
//__9117_CODE_START
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
  rsi_dbg(INFO_ZONE,
          "[Conf] queue:%d, aifsn:%x, cwmin:%x cwmax:%x, edca_timer:%x \n",
          queue,
          (params->mu_edca_param_rec.aifsn & 0xF),
          (params->mu_edca_param_rec.ecw_min_max & 0xF),
          (params->mu_edca_param_rec.ecw_min_max >> 4),
          (params->mu_edca_param_rec.mu_edca_timer));
#endif
  //__9117_CODE_END

  mutex_lock(&common->mutex);
  /* Map into the way the f/w expects */
  switch (queue) {
    case IEEE80211_AC_VO:
      idx = VO_Q;
      break;
    case IEEE80211_AC_VI:
      idx = VI_Q;
      break;
    case IEEE80211_AC_BE:
      idx = BE_Q;
      break;
    case IEEE80211_AC_BK:
      idx = BK_Q;
      break;
    default:
      idx = BE_Q;
      break;
  }

#ifdef CONFIG_STA_PLUS_AP
  if ((vif->type == NL80211_IFTYPE_AP)) {
    idx += 4;
  }
#endif

  memcpy(&common->edca_params[idx], params, sizeof(struct ieee80211_tx_queue_params));

  if (params->uapsd)
    common->uapsd_bitmap |= idx;
  else
    common->uapsd_bitmap &= (~idx);
  if (queue == (IEEE80211_NUM_ACS - 1))
    wmm_config_complete = 1;
  if ((common->beacon_enabled && wmm_config_complete))
    rsi_load_radio_caps(common);

  mutex_unlock(&common->mutex);
  return 0;
}

/**
 * rsi_hal_key_config() - This function loads the keys into the firmware.
 * @hw: Pointer to the ieee80211_hw structure.
 * @vif: Pointer to the ieee80211_vif structure.
 * @key: Pointer to the ieee80211_key_conf structure.
 *
 * Return: status: 0 on success, -1 on failure.
 */
static int rsi_hal_key_config(struct ieee80211_hw *hw,
                              struct ieee80211_vif *vif,
                              struct ieee80211_key_conf *key,
                              struct ieee80211_sta *sta)
{
  struct rsi_hw *adapter = hw->priv;
  struct rsi_sta *rsta   = NULL;
  int status;
  u8 key_type;
  s16 sta_id = 0;

  if (key->flags & IEEE80211_KEY_FLAG_PAIRWISE)
    key_type = RSI_PAIRWISE_KEY;
  else
    key_type = RSI_GROUP_KEY;

  rsi_dbg(INFO_ZONE, "%s: Cipher 0x%x key_type: %d key_len: %d\n", __func__, key->cipher, key_type, key->keylen);
  rsi_dbg(INFO_ZONE, "hw_key_idx %d\n", key->hw_key_idx);

  if ((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO)) {
    if (sta) {
      rsta = rsi_find_sta(adapter->priv, sta->addr);

      if (rsta)
        sta_id = rsta->sta_id;
    }
    adapter->priv->key = key;
  } else {
    if ((key->cipher == WLAN_CIPHER_SUITE_WEP104) || (key->cipher == WLAN_CIPHER_SUITE_WEP40)) {
#ifdef CONFIG_STA_PLUS_AP
      status =
        rsi_load_key(adapter->priv, key->key, key->keylen, RSI_PAIRWISE_KEY, key->keyidx, key->cipher, sta_id, vif);
#else
      status = rsi_load_key(adapter->priv, key->key, key->keylen, RSI_PAIRWISE_KEY, key->keyidx, key->cipher, sta_id);
#endif
      if (status)
        return status;
    }
  }

#ifdef CONFIG_STA_PLUS_AP
  status = rsi_load_key(adapter->priv, key->key, key->keylen, key_type, key->keyidx, key->cipher, sta_id, vif);
#else
  status = rsi_load_key(adapter->priv, key->key, key->keylen, key_type, key->keyidx, key->cipher, sta_id);
#endif

  if ((vif->type == NL80211_IFTYPE_STATION) && (key->key)
      && ((key->cipher == WLAN_CIPHER_SUITE_WEP104) || (key->cipher == WLAN_CIPHER_SUITE_WEP40))) {
    adapter->priv->start_bgscan = 1;
    if (!rsi_send_block_unblock_frame(adapter->priv, false))
      adapter->priv->hw_data_qs_blocked = false;
  }
  return status;
}

/**
 * rsi_mac80211_set_key() - This function sets type of key to be loaded.
 * @hw: Pointer to the ieee80211_hw structure.
 * @cmd: enum set_key_cmd.
 * @vif: Pointer to the ieee80211_vif structure.
 * @sta: Pointer to the ieee80211_sta structure.
 * @key: Pointer to the ieee80211_key_conf structure.
 *
 * Return: status: 0 on success, negative error code on failure.
 */
static int rsi_mac80211_set_key(struct ieee80211_hw *hw,
                                enum set_key_cmd cmd,
                                struct ieee80211_vif *vif,
                                struct ieee80211_sta *sta,
                                struct ieee80211_key_conf *key)
{
  struct rsi_hw *adapter        = hw->priv;
  struct rsi_common *common     = adapter->priv;
  struct security_info *secinfo = &common->secinfo;
  struct vif_priv *vif_info     = (struct vif_priv *)vif->drv_priv;
  int status;

  mutex_lock(&common->mutex);
  switch (cmd) {
    case SET_KEY:
      rsi_dbg(MGMT_DEBUG_ZONE,
              "<==== SENDING INSTALL %s KEY FRAME ====>\n",
              (key->flags & IEEE80211_KEY_FLAG_PAIRWISE) ? "PAIR_WISE" : "GROUP");
      status = rsi_hal_key_config(hw, vif, key, sta);
      if (status) {
        mutex_unlock(&common->mutex);
        return status;
      }

      if (key->flags & IEEE80211_KEY_FLAG_PAIRWISE) {
        secinfo->ptk_cipher = key->cipher;
      } else {
        struct ieee80211_key_seq seq;

        secinfo->gtk_cipher = key->cipher;
        ieee80211_get_key_rx_seq(key, 0, &seq);
        switch (key->cipher) {
          case WLAN_CIPHER_SUITE_CCMP:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
          case WLAN_CIPHER_SUITE_CCMP_256:
#endif
            memcpy(vif_info->rx_bcmc_pn, seq.ccmp.pn, IEEE80211_CCMP_PN_LEN);
            vif_info->rx_pn_valid = true;
            vif_info->key         = key;
            break;
          case WLAN_CIPHER_SUITE_TKIP:
            vif_info->rx_bcmc_pn[5] = seq.tkip.iv16 & 0xff;
            vif_info->rx_bcmc_pn[4] = (seq.tkip.iv16 >> 8) & 0xff;
            vif_info->rx_bcmc_pn[3] = seq.tkip.iv32 & 0xff;
            vif_info->rx_bcmc_pn[2] = (seq.tkip.iv32 >> 8) & 0xff;
            vif_info->rx_bcmc_pn[1] = (seq.tkip.iv32 >> 16) & 0xff;
            vif_info->rx_bcmc_pn[0] = (seq.tkip.iv32 >> 24) & 0xff;
            vif_info->rx_pn_valid   = true;
            vif_info->key           = key;
            break;
          case WLAN_CIPHER_SUITE_AES_CMAC:
            memcpy(vif_info->rx_bcmc_pn, seq.aes_cmac.pn, 6);
            vif_info->rx_pn_valid = true;
            vif_info->key         = key;
            break;
        }
        rsi_dbg(INFO_ZONE,
                "%s: RX PN: %02x-%02x-%02x-%02x-%02x-%02x",
                __func__,
                vif_info->rx_bcmc_pn[5],
                vif_info->rx_bcmc_pn[4],
                vif_info->rx_bcmc_pn[3],
                vif_info->rx_bcmc_pn[2],
                vif_info->rx_bcmc_pn[1],
                vif_info->rx_bcmc_pn[0]);
      }
      key->hw_key_idx = key->keyidx;
      key->flags |= IEEE80211_KEY_FLAG_GENERATE_IV;
      break;

    case DISABLE_KEY:
      rsi_dbg(MGMT_DEBUG_ZONE,
              "<==== SENDING DELETE %s KEY FRAME ====>\n",
              (key->flags & IEEE80211_KEY_FLAG_PAIRWISE) ? "PAIR_WISE" : "GROUP");
      if (vif_info->key) {
        vif_info->prev_keyid = vif_info->key->keyidx;
        memcpy(vif_info->rx_bcmc_pn_prev, vif_info->rx_bcmc_pn, IEEE80211_CCMP_PN_LEN);
      }
      memset(vif_info->rx_bcmc_pn, 0, IEEE80211_CCMP_PN_LEN);
      vif_info->rx_pn_valid = false;
      vif_info->key         = NULL;
      status                = rsi_hal_key_config(hw, vif, key, sta);
      break;

    default:
      status = -EOPNOTSUPP;
      break;
  }

  mutex_unlock(&common->mutex);
  return status;
}

/**
 * rsi_mac80211_ampdu_action() - This function selects the AMPDU action for
 *				 the corresponding mlme_action flag and
 *				 informs the f/w regarding this.
 * @hw: Pointer to the ieee80211_hw structure.
 * @vif: Pointer to the ieee80211_vif structure.
 * @action: ieee80211_ampdu_mlme_action enum.
 * @sta: Pointer to the ieee80211_sta structure.
 * @tid: Traffic identifier.
 * @ssn: Pointer to ssn value.
 * @buf_size: Buffer size (for kernel version > 2.6.38).
 *
 * Return: status: 0 on success, negative error code on failure.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
static int rsi_mac80211_ampdu_action(struct ieee80211_hw *hw,
                                     struct ieee80211_vif *vif,
                                     enum ieee80211_ampdu_mlme_action action,
                                     struct ieee80211_sta *sta,
                                     u16 tid,
                                     u16 *ssn,
                                     u8 buf_size)
#elif (((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 69))) \
       || ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0))))
static int rsi_mac80211_ampdu_action(struct ieee80211_hw *hw,
                                     struct ieee80211_vif *vif,
                                     enum ieee80211_ampdu_mlme_action action,
                                     struct ieee80211_sta *sta,
                                     u16 tid,
                                     u16 *ssn,
                                     u8 buf_size,
                                     bool amsdu)
#else
static int rsi_mac80211_ampdu_action(struct ieee80211_hw *hw,
                                     struct ieee80211_vif *vif,
                                     struct ieee80211_ampdu_params *params)
#endif
{
  int status                = 1;
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;
  struct rsi_sta *rsta      = NULL;
  u16 seq_no = 0, seq_start = 0;
  u8 sta_id = 0;
#ifndef CONFIG_STA_PLUS_AP
  u8 ii = 0;
#else
  struct vif_priv *vif_info = (struct vif_priv *)vif->drv_priv;
#endif

#if (((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 69)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0))) \
     || (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 6, 0)))
  u16 tid                                 = params->tid;
  u8 buf_size                             = params->buf_size;
  enum ieee80211_ampdu_mlme_action action = params->action;
  struct ieee80211_sta *sta               = params->sta;
#endif

#ifndef CONFIG_STA_PLUS_AP
  for (ii = 0; ii < RSI_MAX_VIFS; ii++) {
    if (vif == adapter->vifs[ii])
      break;
  }
#endif

  mutex_lock(&common->mutex);

#if (((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 69)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0))) \
     || (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 6, 0)))
  seq_no = params->ssn;
#else
  if (ssn != NULL)
    seq_no = *ssn;
#endif
  if ((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO)) {
    rsta = rsi_find_sta(common, sta->addr);

    if (!rsta) {
      rsi_dbg(ERR_ZONE, "No station mapped\n");
      mutex_unlock(&common->mutex);
      return 0;
    }
    sta_id = rsta->sta_id;
  }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
  rsi_dbg(INFO_ZONE, "%s: AMPDU action tid=%d ssn=0x%x, buf_size=%d\n", __func__, tid, seq_no, buf_size);
#endif

  switch (action) {
    case IEEE80211_AMPDU_RX_START:
      rsi_dbg(INFO_ZONE, "AMPDU action RX_START (%d)\n", action);
      status = rsi_send_aggr_params_frame(common, tid, seq_no, buf_size, STA_RX_ADDBA_DONE, sta_id);
      break;

    case IEEE80211_AMPDU_RX_STOP:
      rsi_dbg(INFO_ZONE, "AMPDU action RX_STOP (%d) called\n", action);
      status = rsi_send_aggr_params_frame(common, tid, 0, buf_size, STA_RX_DELBA, sta_id);
      break;

    case IEEE80211_AMPDU_TX_START:
      rsi_dbg(INFO_ZONE, "AMPDU action TX_START (%d) called\n", action);
      if ((vif->type == NL80211_IFTYPE_STATION) || (vif->type == NL80211_IFTYPE_P2P_CLIENT))
#ifndef CONFIG_STA_PLUS_AP
        common->vif_info[ii].seq_start = seq_no;
#else
        vif_info->seq_start = seq_no;
#endif
      else if ((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO))
#ifndef CONFIG_STA_PLUS_AP
        rsta->seq_start[ii] = seq_no;
#else
        rsta->seq_start[TID_TO_WME_AC(tid)] = seq_no;
#endif
      ieee80211_start_tx_ba_cb_irqsafe(vif, sta->addr, tid);
      status = 0;
      break;

    case IEEE80211_AMPDU_TX_STOP_CONT:
    case IEEE80211_AMPDU_TX_STOP_FLUSH:
    case IEEE80211_AMPDU_TX_STOP_FLUSH_CONT:
      rsi_dbg(INFO_ZONE,
              "AMPDU action TX_STOP_CONT / TX_STOP_FLUSH /"
              " TX_STOP_FLUSH_CONT (%d) called\n",
              action);
      status = rsi_send_aggr_params_frame(common, tid, seq_no, buf_size, STA_TX_DELBA, sta_id);
      if (!status)
        ieee80211_stop_tx_ba_cb_irqsafe(vif, sta->addr, tid);
      break;

    case IEEE80211_AMPDU_TX_OPERATIONAL:
      rsi_dbg(INFO_ZONE, "AMPDU action TX_OPERATIONAL(%d) called\n", action);
      if ((vif->type == NL80211_IFTYPE_STATION) || (vif->type == NL80211_IFTYPE_P2P_CLIENT))
#ifndef CONFIG_STA_PLUS_AP
        seq_start = common->vif_info[ii].seq_start;
#else
        seq_start = vif_info->seq_start;
#endif
      else if ((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO))
#ifndef CONFIG_STA_PLUS_AP
        seq_start = rsta->seq_start[ii];
#else
        seq_start = rsta->seq_start[TID_TO_WME_AC(tid)];
#endif
      status = rsi_send_aggr_params_frame(common, tid, seq_start, buf_size, STA_TX_ADDBA_DONE, sta_id);
      break;

    default:
      rsi_dbg(ERR_ZONE, "%s: Uknown AMPDU action\n", __func__);
      break;
  }

  mutex_unlock(&common->mutex);
  return status;
}

/**
 * rsi_mac80211_set_frag_threshold() - This function sets fragmentation threshold value.
 * @hw: Pointer to the ieee80211_hw structure.
 * @value: Rts threshold value.
 *
 * Return: 0 on success.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0) && LINUX_VERSION_CODE <= KERNEL_VERSION(5, 9, 16))
static int rsi_mac80211_set_frag_threshold(struct ieee80211_hw *hw, u32 value)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;

  mutex_lock(&common->mutex);
  common->frag_threshold = value;
  rsi_send_vap_dynamic_update(common);
  mutex_unlock(&common->mutex);

  return 0;
}
#endif
/**
 * rsi_mac80211_set_rts_threshold() - This function sets rts threshold value.
 * @hw: Pointer to the ieee80211_hw structure.
 * @value: Rts threshold value.
 *
 * Return: 0 on success.
 */
static int rsi_mac80211_set_rts_threshold(struct ieee80211_hw *hw, u32 value)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;

  mutex_lock(&common->mutex);
  common->rts_threshold = value;
  rsi_send_vap_dynamic_update(common);
  mutex_unlock(&common->mutex);

  return 0;
}

/**
 * rsi_get_rate_code() - This function parse the bitmap to find the rate code.
 * @bitmap: bitmap containing rate indices.
 * @band: band info of the current state
 * @mcs: 1 -> bitmap is containing mcs rate indices.
 *       0 -> bitmap is containing legacy rate indices.
 * Return: returns rate code.
 */
static u16 rsi_get_rate_code(struct rsi_hw *adapter, u16 bitmap, enum nl80211_band band, u8 rate_type)
{
  int i = 0;
  if (bitmap & (bitmap - 1)) /*Making sure only 1 bit is set in bitmap*/
    return 0xFF;

  while (!(bitmap & BIT(i)) && (i < 16)) {
    i++;
  }

  if (rate_type == MCS_RATE)
    return (u16)rsi_mcsrates[i];
//__9117_CODE_START
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
  else if (rate_type == HE_MCS_RATE)
    return (u16)(rsi_he_rates[i] | adapter->rate_field_params);
#endif
  //__9117_CODE_END
  else {
    if (band == NL80211_BAND_5GHZ)
      i += 4;
    return (u16)(rsi_rates[i].hw_value);
  }
}

/**
 * rsi_mac80211_set_rate_mask() - This function sets bitrate_mask to be used.
 * @hw: Pointer to the ieee80211_hw structure
 * @vif: Pointer to the ieee80211_vif structure.
 * @mask: Pointer to the cfg80211_bitrate_mask structure.
 *
 * Return: 0 on success.
 */
static int rsi_mac80211_set_rate_mask(struct ieee80211_hw *hw,
                                      struct ieee80211_vif *vif,
                                      const struct cfg80211_bitrate_mask *mask)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;
  enum nl80211_band band    = hw->conf.chandef.chan->band;
  u16 legacy = 0, rate = 0, enable_auto_rate;
  u8 mcs = 0;
  int i, status = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
  u16 he_mcs = mask->control[band].he_mcs[0];
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  struct ieee80211_bss_conf *bss = &vif->bss_conf;
#endif

  if (!BSS_ASSOC) {
    rsi_dbg(ERR_ZONE, "Can't set any rate before connection\n");
    return -EINVAL;
  }

  mutex_lock(&common->mutex);
  common->fixedrate_mask[band] = 0;

#if 0
	if (mask->control[band].legacy == 0xfff) {
		common->fixedrate_mask[band] =
			(mask->control[band].ht_mcs[0] << 12);
	} else {
		common->fixedrate_mask[band] = mask->control[band].legacy;
	}
#endif

  mcs    = mask->control[band].ht_mcs[0];
  legacy = mask->control[band].legacy;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
  enable_auto_rate = (mcs == MCS_RATE_MAP) && (legacy == (band ? _5G_LEGACY_RATE_MAP : _2G_LEGACY_RATE_MAP))
                     && (he_mcs == HE_MCS_RATE_MAP || he_mcs == HE_DISABLE);
#else
  enable_auto_rate = (mcs == MCS_RATE_MAP) && (legacy == (band ? _5G_LEGACY_RATE_MAP : _2G_LEGACY_RATE_MAP));
#endif

//__9117_CODE_START
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
  if (adapter->ax_params._11ax_enabled && !enable_auto_rate) {
    rsi_dbg(ERR_ZONE, "Can't set any rate for lower kernel in 11ax mode,use debugfs to set 11ax rate\n");
    mutex_unlock(&common->mutex);
    return -EINVAL;
  }
#endif
  //__9117_CODE_END

  if (enable_auto_rate) {
    /* Disabling the fixed rate */
    common->fixed_rate_en = 0;
  } else { /* Selecting the fixed rate */
    if (mcs != MCS_RATE_MAP)
      rate = rsi_get_rate_code(adapter, (u16)mcs, band, MCS_RATE);
//__9117_CODE_START
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    else if (adapter->ax_params._11ax_enabled && he_mcs != HE_MCS_RATE_MAP)
      rate = rsi_get_rate_code(adapter, (u16)he_mcs, band, HE_MCS_RATE);
    else if (!adapter->ax_params._11ax_enabled && (he_mcs != HE_MCS_RATE_MAP && he_mcs != HE_DISABLE)) {
      rsi_dbg(ERR_ZONE, "11AX rate not supported in non 11AX Mode \n");
      mutex_unlock(&common->mutex);
      return -EINVAL;
    }
#endif
    //__9117_CODE_END
    else {
      for (i = 11; i >= 0; i--) {
        if (legacy & BIT(i)) {
          legacy = BIT(i);
          break;
        }
      }
      rate = rsi_get_rate_code(adapter, legacy, band, LEGACY_RATE);
    }

    if (rate == 0xFF) {
      mutex_unlock(&common->mutex);
      return -EINVAL;
    }

    common->fixed_rate_en = 1;
    common->fixed_rate    = rate;
  }
  status = rsi_send_vap_dynamic_update(common);

  mutex_unlock(&common->mutex);

  return status;
}

/**
 * rsi_perform_cqm() - This function performs cqm.
 * @common: Pointer to the driver private structure.
 * @bssid: pointer to the bssid.
 * @rssi: RSSI value.
 */
#ifdef CONFIG_STA_PLUS_AP
static void rsi_perform_cqm(struct rsi_common *common, u8 *bssid, s8 rssi, struct ieee80211_vif *vif)
{
#else
static void rsi_perform_cqm(struct rsi_common *common, u8 *bssid, s8 rssi)
{
  struct rsi_hw *adapter = common->priv;
#endif
  s8 last_event = common->cqm_info.last_cqm_event_rssi;
  int thold     = common->cqm_info.rssi_thold;
  u32 hyst      = common->cqm_info.rssi_hyst;
  enum nl80211_cqm_rssi_threshold_event event;

  if (common->iface_down)
    return;

  if (rssi < thold && (last_event == 0 || rssi < (last_event - hyst)))
    event = NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW;
  else if (rssi > thold && (last_event == 0 || rssi > (last_event + hyst)))
    event = NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH;
  else
    return;

  common->cqm_info.last_cqm_event_rssi = rssi;
  rsi_dbg(INFO_ZONE, "CQM: Notifying event: %s\n", (event == NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW) ? "LOW" : "HIGH");

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#ifdef CONFIG_STA_PLUS_AP
  ieee80211_cqm_rssi_notify(vif, event, rssi, GFP_KERNEL);
#else
  ieee80211_cqm_rssi_notify(adapter->vifs[adapter->sc_nvifs - 1], event, rssi, GFP_KERNEL);
#endif
#else
#ifdef CONFIG_STA_PLUS_AP
  ieee80211_cqm_rssi_notify(vif, event, GFP_KERNEL);
#else
  ieee80211_cqm_rssi_notify(adapter->vifs[adapter->sc_nvifs - 1], event, GFP_KERNEL);
#endif
#endif
}

void rsi_indicate_bcnmiss(struct rsi_common *common)
{
  struct rsi_hw *adapter = common->priv;

  rsi_dbg(INFO_ZONE, "CQM: Notifying beacon miss\n");

  if (common->iface_down)
    return;

#ifdef CONFIG_STA_PLUS_AP
  ieee80211_beacon_loss(rsi_get_sta_vif(adapter));
#else
  ieee80211_beacon_loss(adapter->vifs[0]);
#endif
  return;
}

struct dot11_ccmp_hdr {
  u8 pn5;
  u8 pn4;
  u8 reserved;
  u8 keyid_info;
  u8 pn3;
  u8 pn2;
  u8 pn1;
  u8 pn0;
};
struct dot11_tkip_hdr {
  u8 tsc1;
  u8 wep_seed;
  u8 tsc0;
  u8 keyid_info;
  u8 tsc2;
  u8 tsc3;
  u8 tsc4;
  u8 tsc5;
};

int rsi_validate_pn(struct rsi_hw *adapter, struct ieee80211_hdr *hdr)
{
  struct ieee80211_vif *vif;
  struct ieee80211_bss_conf *bss;
  struct vif_priv *vif_info = NULL;
  u8 cur_pn[IEEE80211_CCMP_PN_LEN];
  u8 *last_pn;
  int i, hdrlen, keyid;

  if (!is_broadcast_ether_addr(hdr->addr1) && !is_multicast_ether_addr(hdr->addr1))
    return 1;

  hdrlen = ieee80211_hdrlen(hdr->frame_control);
  for (i = 0; i < adapter->sc_nvifs; i++) {
    vif = adapter->vifs[i];

    if (!vif)
      continue;
    if ((vif->type != NL80211_IFTYPE_STATION) && (vif->type != NL80211_IFTYPE_P2P_CLIENT))
      continue;
    bss = &vif->bss_conf;
    if (!BSS_ASSOC)
      continue;
    if (!ether_addr_equal(bss->bssid, hdr->addr2))
      continue;
    vif_info = (struct vif_priv *)vif->drv_priv;
    if (!vif_info->key) {
      vif_info = NULL;
      continue;
    }
    if (!vif_info->rx_pn_valid) {
      vif_info = NULL;
      continue;
    }
  }
  if (!vif_info)
    return 1;
  last_pn = vif_info->rx_bcmc_pn;
  if (vif_info->key->cipher == WLAN_CIPHER_SUITE_CCMP) {
    struct dot11_ccmp_hdr *ccmp = (struct dot11_ccmp_hdr *)&((u8 *)hdr)[hdrlen];

    keyid     = KEYID_BITMASK(ccmp->keyid_info);
    cur_pn[0] = ccmp->pn0;
    cur_pn[1] = ccmp->pn1;
    cur_pn[2] = ccmp->pn2;
    cur_pn[3] = ccmp->pn3;
    cur_pn[4] = ccmp->pn4;
    cur_pn[5] = ccmp->pn5;
  } else {
    struct dot11_tkip_hdr *tkip = (struct dot11_tkip_hdr *)&((u8 *)hdr)[hdrlen];

    keyid     = KEYID_BITMASK(tkip->keyid_info);
    cur_pn[5] = tkip->tsc0;
    cur_pn[4] = tkip->tsc1;
    cur_pn[3] = tkip->tsc2;
    cur_pn[2] = tkip->tsc3;
    cur_pn[1] = tkip->tsc4;
    cur_pn[0] = tkip->tsc5;
  }

  if (keyid == vif_info->key->keyidx) {
    i = memcmp(cur_pn, last_pn, IEEE80211_CCMP_PN_LEN);
    if (i <= 0) {
      rsi_dbg(ERR_ZONE,
              "Packet Dropped as RX PN is less than last "
              "received PN\n");
      return -1;
    }
    memcpy(vif_info->rx_bcmc_pn, cur_pn, IEEE80211_CCMP_PN_LEN);
  } else {
    if (keyid == vif_info->prev_keyid) {
      i = memcmp(cur_pn, vif_info->rx_bcmc_pn_prev, IEEE80211_CCMP_PN_LEN);
      if (i <= 0) {
        rsi_dbg(ERR_ZONE,
                "Packet Dropped as RX PN is less than "
                "last  received PN\n");
        return -1;
      }
      memcpy(vif_info->rx_bcmc_pn_prev, cur_pn, IEEE80211_CCMP_PN_LEN);
    } else {
      rsi_dbg(ERR_ZONE,
              "Packet Dropped as Key ID not matched with both "
              "current and previous Key ID\n");
      return -1;
    }
  }

  return 0;
}

/**
 * rsi_fill_rx_status() - This function fills rx status in
 *			  ieee80211_rx_status structure.
 * @hw: Pointer to the ieee80211_hw structure.
 * @skb: Pointer to the socket buffer structure.
 * @common: Pointer to the driver private structure.
 * @rxs: Pointer to the ieee80211_rx_status structure.
 *
 * Return: None.
 */
static int rsi_fill_rx_status(struct ieee80211_hw *hw,
                              struct sk_buff *skb,
                              struct rsi_common *common,
                              struct ieee80211_rx_status *rxs)
{
  struct rsi_hw *adapter = common->priv;
#ifdef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = NULL;
#else
  struct ieee80211_vif *vif = adapter->vifs[adapter->sc_nvifs - 1];
#endif
  struct ieee80211_bss_conf *bss = NULL;
  struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
  struct skb_info *rx_params     = (struct skb_info *)info->driver_data;
  struct ieee80211_hdr *hdr;
  char rssi  = rx_params->rssi;
  u8 hdrlen  = 0;
  u8 channel = rx_params->channel;
  s32 freq;

  hdr    = ((struct ieee80211_hdr *)(skb->data));
  hdrlen = ieee80211_hdrlen(hdr->frame_control);

#ifdef CONFIG_STA_PLUS_AP
  vif = rsi_get_vif(adapter, hdr->addr1);
#endif
  memset(info, 0, sizeof(struct ieee80211_tx_info));

  rxs->signal = -(rssi);

  if (channel >= 1 && channel <= 14)
    rxs->band = NL80211_BAND_2GHZ;
  else if (channel >= 32 && channel <= 173)
    rxs->band = NL80211_BAND_5GHZ;

  freq = ieee80211_channel_to_frequency(channel, rxs->band);

  if (freq)
    rxs->freq = freq;

  if (ieee80211_has_protected(hdr->frame_control)) {
    if (rsi_is_cipher_wep(common)) {
      memmove(skb->data + 4, skb->data, hdrlen);
      skb_pull(skb, 4);
    } else {
      if (common->driver_mode != SNIFFER_MODE) {
        if (skb->len < (hdrlen + IEEE80211_CCMP_HDR_LEN)) {
          rsi_dbg(ERR_ZONE, "Invalid encrypted packet\n");
          dev_kfree_skb(skb);
          return -EINVAL;
        }
        if (rsi_validate_pn(adapter, hdr) < 0) {
          rsi_dbg(INFO_ZONE, "Invalid RX PN; Dropping\n");
          dev_kfree_skb(skb);
          return -EINVAL;
        }
        memmove(skb->data + IEEE80211_CCMP_HDR_LEN, skb->data, hdrlen);
        skb_pull(skb, IEEE80211_CCMP_HDR_LEN);
        rxs->flag |= RX_FLAG_MMIC_STRIPPED;
      }
    }
    rxs->flag |= RX_FLAG_DECRYPTED;
    rxs->flag |= RX_FLAG_IV_STRIPPED;
  }

  if (!vif) {
    rsi_dbg(INFO_ZONE, "No virtual interface\n");
    return 0;
  }
  bss = &vif->bss_conf;

  /* CQM only for connected AP beacons, the RSSI is a weighted avg */
  if ((vif->type == NL80211_IFTYPE_STATION) && BSS_ASSOC && ether_addr_equal(bss->bssid, hdr->addr2)) {
    if (ieee80211_is_beacon(hdr->frame_control))
#ifdef CONFIG_STA_PLUS_AP
      rsi_perform_cqm(common, hdr->addr2, rxs->signal, vif);
#else
      rsi_perform_cqm(common, hdr->addr2, rxs->signal);
#endif
  }
  return 0;
}

/**
 * rsi_indicate_pkt_to_os() - This function sends received packet to mac80211.
 * @common: Pointer to the driver private structure.
 * @skb: Pointer to the socket buffer structure.
 *
 * Return: None.
 */
void rsi_indicate_pkt_to_os(struct rsi_common *common, struct sk_buff *skb)
{
  struct rsi_hw *adapter                = common->priv;
  struct ieee80211_hw *hw               = adapter->hw;
  struct ieee80211_rx_status *rx_status = IEEE80211_SKB_RXCB(skb);

  if ((common->iface_down) || (!adapter->sc_nvifs)) {
    dev_kfree_skb(skb);
    return;
  }

  /* filling in the ieee80211_rx_status flags */
  if (rsi_fill_rx_status(hw, skb, common, rx_status))
    return;

  if (common->driver_mode == SNIFFER_MODE) {
    if (skb->len <= 4) {
      rsi_dbg(ERR_ZONE, "Dropping pkt of length less than or equal to FCS\n");
      dev_kfree_skb(skb);
      return;
    }
  }
  rsi_dbg(MGMT_RX_ZONE, "RX Packet Type: %s\n", dot11_pkt_type(skb->data[0]));
  rsi_hex_dump(DATA_RX_ZONE, "802.11 RX packet", skb->data, skb->len);
  ieee80211_rx_irqsafe(hw, skb);
}

/**
 * rsi_mac80211_sta_add() - This function notifies driver about a peer getting
 *			    connected.
 * @hw: pointer to the ieee80211_hw structure.
 * @vif: Pointer to the ieee80211_vif structure.
 * @sta: Pointer to the ieee80211_sta structure.
 *
 * Return: 0 on success, -1 on failure.
 */
static int rsi_mac80211_sta_add(struct ieee80211_hw *hw, struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;
  bool sta_exist            = 0;
  struct rsi_sta *rsta;
#ifdef CONFIG_STA_PLUS_AP
  struct vif_priv *vif_info = (struct vif_priv *)vif->drv_priv;
#endif
  u8 i;

  rsi_hex_dump(INFO_ZONE, "Station Add", sta->addr, ETH_ALEN);

  mutex_lock(&common->mutex);

//__9117_CODE_START
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
  if (vif->type == NL80211_IFTYPE_STATION && (HE_CAP.he_cap_elem.mac_cap_info[2] & IEEE80211_HE_MAC_CAP2_BSR))
    common->bsr_support = true;
#endif
  //__9117_CODE_END

  if ((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO)) {
    u8 j;
    int free_index = -1;

    /* Check if max stations reached */
    if (common->num_stations >= common->max_stations) {
      rsi_dbg(ERR_ZONE, "Reject: Max Stations exists\n");
      mutex_unlock(&common->mutex);
      return -EOPNOTSUPP;
    }

    /* Send peer notify to device */
    rsi_dbg(INFO_ZONE, "Indicate bss status to device\n");
#ifdef CONFIG_STA_PLUS_AP
    for (i = 1; i < common->max_stations; i++) {
#else
    for (i = 0; i < common->max_stations; i++) {
#endif
      if (!common->stations[i].sta) {
        if (free_index < 0)
          free_index = i;
        continue;
      }
      if (!memcmp(common->stations[i].sta->addr, sta->addr, ETH_ALEN)) {
        rsi_dbg(INFO_ZONE, "Station exists\n");
        sta_exist = 1;
        break;
      }
    }
    if (!sta_exist) {
      rsi_dbg(INFO_ZONE, "New Station\n");
      if (free_index >= 0)
        i = free_index;
      common->stations[i].sta    = sta;
      common->stations[i].sta_id = i;
#ifdef CONFIG_STA_PLUS_AP
      rsi_inform_bss_status(common, AP_OPMODE, 1, sta->addr, sta->wme, sta->aid, sta, i, 0, vif);
#else
      rsi_inform_bss_status(common, AP_OPMODE, 1, sta->addr, sta->wme, sta->aid, sta, i, 0);
#endif

      if (common->key
          && ((common->key->cipher == WLAN_CIPHER_SUITE_WEP104) || (common->key->cipher == WLAN_CIPHER_SUITE_WEP40))) {
        rsi_load_key(adapter->priv,
                     common->key->key,
                     common->key->keylen,
                     RSI_PAIRWISE_KEY,
                     common->key->keyidx,
                     common->key->cipher,
#ifdef CONFIG_STA_PLUS_AP
                     i,
                     vif);
#else
                     i);
#endif
      }
      for (j = 0; j < IEEE80211_NUM_ACS; j++)
        common->stations[i].seq_no[j] = 1;
      for (j = 0; j < IEEE80211_NUM_TIDS; j++)
        common->stations[i].start_tx_aggr[j] = false;
      common->num_stations++;
    } else {
      common->stations[i].sta    = sta;
      common->stations[i].sta_id = i;
      for (j = 0; j < IEEE80211_NUM_ACS; j++)
        common->stations[i].seq_no[j] = 1;
      for (j = 0; j < IEEE80211_NUM_TIDS; j++)
        common->stations[i].start_tx_aggr[j] = false;
    }
  }

  if ((HT_CAP.cap & IEEE80211_HT_CAP_SGI_20) || (HT_CAP.cap & IEEE80211_HT_CAP_SGI_40)) {
#ifndef CONFIG_STA_PLUS_AP
    common->vif_info[0].sgi = true;
#else
    vif_info->sgi = true;
#endif
  }
  if (vif->type == NL80211_IFTYPE_STATION) {
#ifdef CONFIG_STA_PLUS_AP
    rsta = &common->stations[STA_PEER];
#else
    rsta = &common->stations[RSI_MAX_ASSOC_STAS];
#endif
    rsta->sta = sta;
#ifdef CONFIG_STA_PLUS_AP
    rsta->sta_id = STA_PEER;
#else
    rsta->sta_id = 0;
#endif
    for (i = 0; i < IEEE80211_NUM_TIDS; i++)
      rsta->start_tx_aggr[i] = false;
    for (i = 0; i < IEEE80211_NUM_ACS; i++)
      rsta->seq_start[i] = 0;
#ifdef CONFIG_STA_PLUS_AP
    rsi_set_min_rate(hw, sta, common, vif);
#else
    rsi_set_min_rate(hw, sta, common);
#endif
  }

  if (((vif->type == NL80211_IFTYPE_STATION) || (vif->type == NL80211_IFTYPE_P2P_CLIENT)) && (HT_CAP.ht_supported)) {
#ifndef CONFIG_STA_PLUS_AP
    common->vif_info[0].is_ht = true;
#else
    vif_info->is_ht = true;
#endif
    common->bitrate_mask[NL80211_BAND_2GHZ] = SUPP_RATES[NL80211_BAND_2GHZ];
    if ((HT_CAP.cap & IEEE80211_HT_CAP_SGI_20) || (HT_CAP.cap & IEEE80211_HT_CAP_SGI_40))
#ifndef CONFIG_STA_PLUS_AP
      common->vif_info[0].sgi = true;
#else
      vif_info->sgi = true;
#endif
    ieee80211_start_tx_ba_session(sta, 0, 0);
  }

  mutex_unlock(&common->mutex);

  return 0;
}

/**
 * rsi_mac80211_sta_remove() - This function notifies driver about a peer
 *			       getting disconnected.
 * @hw: Pointer to the ieee80211_hw structure.
 * @vif: Pointer to the ieee80211_vif structure.
 * @sta: Pointer to the ieee80211_sta structure.
 *
 * Return: 0 on success, -1 on failure.
 */
static int rsi_mac80211_sta_remove(struct ieee80211_hw *hw, struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
  struct rsi_hw *adapter         = hw->priv;
  struct rsi_common *common      = adapter->priv;
  struct ieee80211_bss_conf *bss = &vif->bss_conf;
  struct rsi_sta *rsta;
#ifdef CONFIG_STA_PLUS_AP
  struct vif_priv *vif_info = (struct vif_priv *)vif->drv_priv;
#endif
  u8 i;

  rsi_hex_dump(INFO_ZONE, "Station Removed", sta->addr, ETH_ALEN);

  mutex_lock(&common->mutex);

//__9117_CODE_START
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
  if ((vif->type == NL80211_IFTYPE_STATION) && common->bsr_support)
    common->bsr_support = false;
#endif
#ifdef CONFIG_TWT_SUPPORT
  if (adapter->rsi_twt_config.twt_enable == 1) {
    adapter->twt_current_status = TWT_INACTIVE_DUETO_DISCONNECT;
  }
#endif
  //__9117_CODE_END

  if ((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO)) {
    u8 j;

    /* Send peer notify to device */
    rsi_dbg(INFO_ZONE, "Indicate bss status to device\n");
#ifdef CONFIG_STA_PLUS_AP
    for (i = 1; i < common->max_stations; i++) {
#else
    for (i = 0; i < common->max_stations; i++) {
#endif
      if (!common->stations[i].sta)
        continue;
      if (!memcmp(common->stations[i].sta->addr, sta->addr, ETH_ALEN)) {
#ifdef CONFIG_STA_PLUS_AP
        rsi_inform_bss_status(common, AP_OPMODE, 0, sta->addr, sta->wme, sta->aid, sta, i, 0, vif);
#else
        rsi_inform_bss_status(common, AP_OPMODE, 0, sta->addr, sta->wme, sta->aid, sta, i, 0);
#endif
        common->stations[i].sta    = NULL;
        common->stations[i].sta_id = -1;
        for (j = 0; j < IEEE80211_NUM_ACS; j++)
          common->stations[i].seq_no[j] = 0;
        if (common->num_stations > 0)
          common->num_stations--;
        break;
      }
    }
    if (i >= common->max_stations)
      rsi_dbg(ERR_ZONE, "%s: No station found\n", __func__);
  }

  if (vif->type == NL80211_IFTYPE_STATION) {
#ifdef CONFIG_STA_PLUS_AP
    rsta = &common->stations[STA_PEER];
#else
    rsta = &common->stations[RSI_MAX_ASSOC_STAS];
#endif
    if (rsta->sta) {
      rsta->sta    = NULL;
      rsta->sta_id = -1;
      for (i = 0; i < IEEE80211_NUM_TIDS; i++)
        rsta->start_tx_aggr[i] = false;
    }
    /* Resetting all the fields to default values */
    memcpy((u8 *)bss->bssid, (u8 *)sta->addr, ETH_ALEN);
    bss->qos                                = sta->wme;
    common->bitrate_mask[NL80211_BAND_2GHZ] = 0;
    common->bitrate_mask[NL80211_BAND_5GHZ] = 0;
    common->min_rate                        = 0xffff;
#ifndef CONFIG_STA_PLUS_AP
    common->vif_info[0].is_ht     = false;
    common->vif_info[0].sgi       = false;
    common->vif_info[0].seq_start = 0;
#else
    vif_info->is_ht = false;
    vif_info->sgi = false;
    vif_info->seq_start = 0;
#endif
    common->secinfo.ptk_cipher = 0;
    common->secinfo.gtk_cipher = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0))
    common->hw_scan_count       = 0;
    common->user_channels_count = 0;
#endif
    common->send_initial_bgscan_chan = false;
    common->update_country           = false;
    if (common->bgscan_en) {
      if (!rsi_send_bgscan_params(common, 0))
        common->bgscan_en = 0;
      common->hwscan_en = false;
    }
    if (!common->iface_down)
      rsi_send_rx_filter_frame(common, 0);
  }
  mutex_unlock(&common->mutex);
  return 0;
}

/**
 * rsi_mac80211_set_antenna() - This function is used to configure
 *				tx and rx antennas.
 * @hw: Pointer to the ieee80211_hw structure.
 * @tx_ant: Bitmap for tx antenna
 * @rx_ant: Bitmap for rx antenna
 *
 * Return: 0 on success, -1 on failure.
 */
static int rsi_mac80211_set_antenna(struct ieee80211_hw *hw, u32 tx_ant, u32 rx_ant)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;
  u32 antenna               = 0;

  if (tx_ant > 1 || rx_ant > 1) {
    rsi_dbg(ERR_ZONE, "Invalid antenna selection (tx: %d, rx:%d)\n", tx_ant, rx_ant);
    rsi_dbg(ERR_ZONE, "Use 0 for int_ant, 1 for ext_ant\n");
    return -EINVAL;
  }

  rsi_dbg(INFO_ZONE, "%s: Antenna map Tx %x Rx %d\n", __func__, tx_ant, rx_ant);

  mutex_lock(&common->mutex);

  antenna = tx_ant ? ANTENNA_SEL_UFL : ANTENNA_SEL_INT;
  if (common->ant_in_use != antenna)
    if (rsi_set_antenna(common, antenna))
      goto fail_set_antenna;

  rsi_dbg(INFO_ZONE, "(%s) Antenna path configured successfully\n", tx_ant ? "UFL" : "INT");

  common->ant_in_use = antenna;

  mutex_unlock(&common->mutex);

  return 0;

fail_set_antenna:
  rsi_dbg(ERR_ZONE, "%s: Failed.\n", __func__);
  mutex_unlock(&common->mutex);
  return -EINVAL;
}

/**
 * rsi_mac80211_get_antenna() - This function is used to configure 
 * 				tx and rx antennas.
 *
 * @hw: Pointer to the ieee80211_hw structure.
 * @tx_ant: Bitmap for tx antenna
 * @rx_ant: Bitmap for rx antenna
 * 
 * Return: 0 on success, -1 on failure.
 */
static int rsi_mac80211_get_antenna(struct ieee80211_hw *hw, u32 *tx_ant, u32 *rx_ant)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;

  mutex_lock(&common->mutex);

  *tx_ant = (common->ant_in_use == ANTENNA_SEL_UFL) ? 1 : 0;
  *rx_ant = 0;

  mutex_unlock(&common->mutex);

  return 0;
}

static const char *regdfs_region_str(u8 dfs_region)
{
  switch (dfs_region) {
    case NL80211_DFS_UNSET:
      return "unset";
    case NL80211_DFS_FCC:
      return "FCC";
    case NL80211_DFS_ETSI:
      return "ETSI";
    case NL80211_DFS_JP:
      return "JP";
    case NL80211_DFS_WORLD:
      return "WORLD";
  }
  return "Unknown";
}

static void rsi_reg_notify(struct wiphy *wiphy, struct regulatory_request *request)
{
  struct ieee80211_supported_band *sband;
  struct ieee80211_hw *hw   = wiphy_to_ieee80211_hw(wiphy);
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;
  struct ieee80211_channel *ch;
  struct ieee80211_bss_conf *bss = NULL;
#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = adapter->vifs[0];
#else
  struct ieee80211_vif *vif = rsi_get_first_valid_vif(adapter);
#endif
  int i;
  u8 region_code;

  mutex_lock(&common->mutex);

  rsi_dbg(INFO_ZONE,
          "%s: country = %s dfs_region = %s\n",
          __func__,
          request->alpha2,
          regdfs_region_str(request->dfs_region));

  region_code = request->dfs_region;
#ifdef CONFIG_CARACALLA_BOARD
  for (i = 0; i < ARRAY_SIZE(rsi_caracalla_reg_db); i++) {
    if (!memcmp(rsi_caracalla_reg_db[i].country_code, request->alpha2, 2)) {
      region_code = rsi_caracalla_reg_db[i].region_code;
      break;
    }
  }
  if (!memcmp(request->alpha2, "00", 2))
    region_code = NL80211_DFS_JP;

  rsi_dbg(ERR_ZONE, "%s: Updating regulatory for region %s\n", __func__, regdfs_region_str(region_code));
#endif

  if (vif && vif->type == NL80211_IFTYPE_STATION)
    bss = &vif->bss_conf;

  sband = wiphy->bands[NL80211_BAND_2GHZ];
  for (i = 0; i < sband->n_channels; i++) {
    ch = &sband->channels[i];

#ifdef CONFIG_CARACALLA_BOARD
    if ((region_code == NL80211_DFS_FCC) &&
#else
    if ((region_code == NL80211_DFS_UNSET) &&
#endif
        (ch->hw_value == 12 || ch->hw_value == 13)) {
      if (ch->flags & IEEE80211_CHAN_DISABLED)
        ch->flags &= ~IEEE80211_CHAN_DISABLED;
      if (ch->flags & IEEE80211_CHAN_NO_IR)
        ch->flags &= ~IEEE80211_CHAN_NO_IR;
    }
    if ((ch->flags & IEEE80211_CHAN_DISABLED) || (ch->flags & IEEE80211_CHAN_NO_IR))
      continue;

#ifdef CONFIG_CARACALLA_BOARD
    rsi_apply_carcalla_power_values(adapter, vif, ch);
#endif
  }

  if (common->num_supp_bands > 1) {
    sband = wiphy->bands[NL80211_BAND_5GHZ];

    for (i = 0; i < sband->n_channels; i++) {
      ch = &sband->channels[i];
      if (ch->flags & IEEE80211_CHAN_DISABLED)
        continue;

      if (ch->flags & IEEE80211_CHAN_RADAR)
        ch->flags |= IEEE80211_CHAN_NO_IR;
    }
  }

  /* If DFS region or country is changed configure back ground scan
	 * params to device again */
  if ((adapter->dfs_region != region_code) || (memcmp(adapter->country, request->alpha2, 2))) {
    if (common->bgscan_en) {
      rsi_send_bgscan_params(common, 0);
      common->bgscan_en = 0;
      msleep(10);
      rsi_send_bgscan_params(common, 1);
      common->bgscan_en = 1;
    }
  }

  if (region_code == NL80211_DFS_UNSET)
    region_code = NL80211_DFS_WORLD;

  /* Firmware region code values 1 less than the standard
	 * linux values; At this point DFS_UNSET is not set
	 * as this is set to WORLD (4)
	 */
  if (common->driver_mode == E2E_MODE)
    adapter->dfs_region = region_code - 1;
  adapter->country[0] = request->alpha2[0];
  adapter->country[1] = request->alpha2[1];

  if (bss && BSS_ASSOC && !common->update_country) {
#ifndef CONFIG_STA_PLUS_AP
    rsi_set_channel(common, common->ap_channel);
#else
    rsi_set_channel(common, common->ap_channel, vif);
#endif
    common->update_country = true;
  }

  mutex_unlock(&common->mutex);
}

static void rsi_mac80211_rfkill_poll(struct ieee80211_hw *hw)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;

  if (common->fsm_state != FSM_MAC_INIT_DONE)
    wiphy_rfkill_set_hw_state(hw->wiphy, true);
  else
    wiphy_rfkill_set_hw_state(hw->wiphy, false);
}

#ifdef CONFIG_RSI_WOW
static const struct wiphy_wowlan_support rsi_wowlan_support = {
  .flags = WIPHY_WOWLAN_ANY | WIPHY_WOWLAN_MAGIC_PKT | WIPHY_WOWLAN_DISCONNECT | WIPHY_WOWLAN_GTK_REKEY_FAILURE
           | WIPHY_WOWLAN_SUPPORTS_GTK_REKEY | WIPHY_WOWLAN_EAP_IDENTITY_REQ | WIPHY_WOWLAN_4WAY_HANDSHAKE,
  .n_patterns      = 0,
  .pattern_min_len = 1,
  .pattern_max_len = 0,
};

static u16 rsi_wow_map_triggers(struct rsi_common *common, struct cfg80211_wowlan *wowlan)
{
  u16 wow_triggers = 0;

  rsi_dbg(INFO_ZONE, "Mapping wowlan triggers\n");

  if (wowlan->any)
    wow_triggers |= RSI_WOW_ANY;
  if (wowlan->magic_pkt)
    wow_triggers |= RSI_WOW_MAGIC_PKT;
  if (wowlan->disconnect)
    wow_triggers |= RSI_WOW_DISCONNECT;
  if (wowlan->gtk_rekey_failure || wowlan->eap_identity_req || wowlan->four_way_handshake)
    wow_triggers |= RSI_WOW_GTK_REKEY;

  return wow_triggers;
}
#endif

#ifdef CONFIG_RSI_WOW
int rsi_config_wowlan(struct rsi_hw *adapter, struct cfg80211_wowlan *wowlan)
{
  struct rsi_common *common = adapter->priv;
  u16 triggers              = 0;
  u16 rx_filter_word        = 0;
#ifdef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *sta_vif      = rsi_get_sta_vif(adapter);
  struct ieee80211_bss_conf *sta_bss = NULL;
#else

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  struct ieee80211_bss_conf *bss = &adapter->vifs[0]->bss_conf;
#else
  struct ieee80211_vif *vif = &adapter->vifs[0];
#endif

#endif

  rsi_dbg(INFO_ZONE, "Config WoWLAN to device\n");
#ifdef CONFIG_STA_PLUS_AP
  if (WARN_ON(!sta_vif)) {
    rsi_dbg(ERR_ZONE, "No STA vif found\n");
    return -EINVAL;
  }
  sta_bss = &sta_vif->bss_conf;
#endif

  if (WARN_ON(!wowlan)) {
    rsi_dbg(ERR_ZONE, "WoW triggers not enabled\n");
    return -EINVAL;
  }

  triggers = rsi_wow_map_triggers(common, wowlan);
  if (!triggers) {
    rsi_dbg(ERR_ZONE, "%s:No valid WoW triggers\n", __func__);
    return -EINVAL;
  }
#ifdef CONFIG_STA_PLUS_AP

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  if (!sta_bss->assoc) {
#else
  if (!sta_vif->cfg.assoc) {
#endif

#else
  if (!BSS_ASSOC) {
#endif
    rsi_dbg(ERR_ZONE, "Cannot configure WoWLAN (Station not connected)\n");
    common->wow_flags |= RSI_WOW_NO_CONNECTION;
    return 0;
  }
  rsi_dbg(INFO_ZONE, "TRIGGERS %x\n", triggers);
  rsi_send_wowlan_request(common, triggers, 1);

  /* Send updated vap caps */
  rsi_send_vap_dynamic_update(common);

  rx_filter_word = (ALLOW_DATA_ASSOC_PEER | DISALLOW_BEACONS | 0);
  rsi_send_rx_filter_frame(common, rx_filter_word);

  common->wow_flags |= RSI_WOW_ENABLED;

  return 0;
}
EXPORT_SYMBOL_GPL(rsi_config_wowlan);
#endif

#ifdef CONFIG_PM
static int rsi_mac80211_suspend(struct ieee80211_hw *hw, struct cfg80211_wowlan *wowlan)
{
#ifdef CONFIG_RSI_WOW
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;

  mutex_lock(&common->mutex);
  if ((common->coex_mode > 1) && (adapter->ps_state == PS_ENABLED))
    rsi_disable_ps(adapter);

  if (rsi_config_wowlan(adapter, wowlan)) {
    rsi_dbg(ERR_ZONE, "Failed to configure WoWLAN\n");
    mutex_unlock(&common->mutex);
    return 1;
  }
  /* Cancel back ground scan in suspend */
  if (common->bgscan_en) {
    if (!rsi_send_bgscan_params(common, 0)) {
      rsi_dbg(INFO_ZONE, "Bg scan canceled");
    }
  }
  mutex_unlock(&common->mutex);
#endif

  return 0;
}

static int rsi_mac80211_resume(struct ieee80211_hw *hw)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;
#ifdef CONFIG_RSI_WOW
  u16 rx_filter_word = 0;

  adapter->priv->wow_flags = 0;
#endif

  rsi_dbg(INFO_ZONE, "%s: mac80211 resume\n", __func__);

  if (common->hibernate_resume) {
    if (common->reinit_hw)
      wait_for_completion(&common->wlan_init_completion);
    return 0;
  }

#ifdef CONFIG_RSI_WOW
  mutex_lock(&common->mutex);
  rsi_send_wowlan_request(common, 0, 0);

  rx_filter_word = (ALLOW_DATA_ASSOC_PEER | ALLOW_CTRL_ASSOC_PEER | ALLOW_MGMT_ASSOC_PEER | 0);
  rsi_send_rx_filter_frame(common, rx_filter_word);
  if (common->bgscan_en) {
    if (!rsi_send_bgscan_params(common, 1)) {
      rsi_dbg(INFO_ZONE, "Bg scan enabled");
    }
  }
  mutex_unlock(&common->mutex);
#endif

  return 0;
}
#endif

char *rsi_vif_type_to_name(enum nl80211_iftype vif_type)
{
  switch (vif_type) {
    case NL80211_IFTYPE_STATION:
      return "Station";
    case NL80211_IFTYPE_AP:
      return "AP";
    case NL80211_IFTYPE_P2P_DEVICE:
      return "P2P_Device";
    case NL80211_IFTYPE_P2P_CLIENT:
      return "P2P_Client";
    case NL80211_IFTYPE_P2P_GO:
      return "P2P_GO";
    default:
      return "Unknown";
  }
}

static enum opmode rsi_map_vif_type(enum nl80211_iftype vif_type)
{
  switch (vif_type) {
    case NL80211_IFTYPE_STATION:
      return STA_OPMODE;
    case NL80211_IFTYPE_AP:
      return AP_OPMODE;
    case NL80211_IFTYPE_P2P_DEVICE:
      return STA_OPMODE;
    case NL80211_IFTYPE_P2P_CLIENT:
      return P2P_CLIENT_OPMODE;
    case NL80211_IFTYPE_P2P_GO:
      return P2P_GO_OPMODE;
    default:
      return UNKNOW_OPMODE;
  }
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void rsi_roc_timeout(unsigned long data)
{
  struct rsi_common *common = (struct rsi_common *)data;
#else
void rsi_roc_timeout(struct timer_list *t)
{
  struct rsi_common *common = from_timer(common, t, roc_timer);
#endif
  struct ieee80211_vif *vif = common->roc_vif;
  struct vif_priv *vif_info = (struct vif_priv *)vif->drv_priv;
  enum opmode intf_mode;

  rsi_dbg(INFO_ZONE, "Remain on channel expired\n");

  mutex_lock(&common->mutex);

  ieee80211_remain_on_channel_expired(common->priv->hw);

  if (timer_pending(&common->roc_timer))
    del_timer(&common->roc_timer);

  if ((vif->type == NL80211_IFTYPE_STATION) || (vif->type == NL80211_IFTYPE_AP)
      || (vif->type == NL80211_IFTYPE_P2P_GO)) {
    rsi_dbg(INFO_ZONE, "Resume to connected channel\n");
    rsi_resume_conn_channel(common->priv, vif);
  }

  intf_mode = rsi_map_vif_type(vif->type);
  if ((common->last_vap_type != intf_mode) || (!ether_addr_equal(common->last_vap_addr, vif->addr))) {
    rsi_dbg(INFO_ZONE, "Resume the vap caps to orig mode\n");
    if (rsi_set_vap_capabilities(common, intf_mode, vif->addr, vif_info->vap_id, VAP_UPDATE))
      rsi_dbg(ERR_ZONE, "Failed to update VAP caps\n");
  }

  mutex_unlock(&common->mutex);
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 4, 0))
static int rsi_mac80211_cancel_roc(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;
#else
static int rsi_mac80211_cancel_roc(struct ieee80211_hw *hw)
{
  struct rsi_hw *adapter = hw->priv;
  struct rsi_common *common = adapter->priv;
  struct ieee80211_vif *vif = common->roc_vif;
#endif
  struct vif_priv *vif_info = (struct vif_priv *)vif->drv_priv;
  enum opmode intf_mode;

  rsi_dbg(INFO_ZONE, "Cancel remain on channel\n");

  mutex_lock(&common->mutex);
  if (!timer_pending(&adapter->priv->roc_timer))
    return 0;

  if (timer_pending(&adapter->priv->roc_timer))
    del_timer(&adapter->priv->roc_timer);

  if ((vif->type == NL80211_IFTYPE_STATION) || (vif->type == NL80211_IFTYPE_AP)
      || (vif->type == NL80211_IFTYPE_P2P_GO)) {
    rsi_dbg(INFO_ZONE, "Resume to connected channel\n");
    rsi_resume_conn_channel(adapter, vif);
  }

  intf_mode = rsi_map_vif_type(vif->type);
  if ((common->last_vap_type != intf_mode) || (!ether_addr_equal(common->last_vap_addr, vif->addr))) {
    rsi_dbg(INFO_ZONE, "Resume the vap caps to orig mode\n");
    if (rsi_set_vap_capabilities(common, intf_mode, vif->addr, vif_info->vap_id, VAP_UPDATE))
      rsi_dbg(ERR_ZONE, "Failed to update VAP caps\n");
  }

  mutex_unlock(&common->mutex);

  return 0;
}

static int rsi_mac80211_roc(struct ieee80211_hw *hw,
                            struct ieee80211_vif *vif,
                            struct ieee80211_channel *chan,
                            int duration,
                            enum ieee80211_roc_type type)
{
  struct vif_priv *vif_info = (struct vif_priv *)vif->drv_priv;
  struct rsi_hw *adapter    = (struct rsi_hw *)hw->priv;
  struct rsi_common *common = (struct rsi_common *)adapter->priv;
  int status                = 0;

  rsi_dbg(INFO_ZONE, "***** Remain on channel *****\n");

  if (common->priv->sc_nvifs <= 0) {
    rsi_dbg(ERR_ZONE, "%s: No virtual interface found\n", __func__);
    return -EINVAL;
  }

  mutex_lock(&common->mutex);

  rsi_dbg(INFO_ZONE, "%s: channel_no: %d duration: %dms\n", __func__, chan->hw_value, duration);

  if (timer_pending(&common->roc_timer)) {
    rsi_dbg(INFO_ZONE, "Stop on-going ROC\n");
    del_timer(&common->roc_timer);
  }
  common->roc_timer.expires = msecs_to_jiffies(duration) + jiffies;
  add_timer(&common->roc_timer);

  /* Configure band */
  if (rsi_band_check(common, chan->band)) {
    rsi_dbg(ERR_ZONE, "Failed to set band\n");
    status = -EINVAL;
    goto out;
  }

  /* Configure channel */
#ifdef CONFIG_STA_PLUS_AP
  if (rsi_set_channel(common, chan, vif)) {
#else
  if (rsi_set_channel(common, chan)) {
#endif
    rsi_dbg(ERR_ZONE, "Failed to set the channel\n");
    status = -EINVAL;
    goto out;
  }

  /* For listen phase, configure vap as AP mode */
  if (vif->type == NL80211_IFTYPE_P2P_DEVICE) {
    rsi_dbg(INFO_ZONE, "Update VAP mode to p2p_client mode\n");
    if (rsi_set_vap_capabilities(common, P2P_CLIENT_OPMODE, vif->addr, vif_info->vap_id, VAP_UPDATE)) {
      rsi_dbg(ERR_ZONE, "Failed to update VAP capabilities\n");
      goto out;
    }
  }

  common->roc_vif = vif;
  ieee80211_ready_on_channel(hw);
  rsi_dbg(INFO_ZONE, "%s: Ready on channel :%d\n", __func__, chan->hw_value);

out:
  mutex_unlock(&common->mutex);

  return status;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 1, 0))
static void rsi_mac80211_event_callback(struct ieee80211_hw *hw,
                                        struct ieee80211_vif *vif,
                                        const struct ieee80211_event *event)
{
  struct rsi_hw *adapter    = hw->priv;
  struct rsi_common *common = adapter->priv;
#ifdef CONFIG_STA_PLUS_AP
  struct ieee80211_bss_conf *bss = &vif->bss_conf;
#else
  struct ieee80211_bss_conf *bss = &adapter->vifs[0]->bss_conf;
#endif

  if (event->type == MLME_EVENT && event->u.mlme.data == ASSOC_EVENT
      && (event->u.mlme.status == MLME_TIMEOUT || event->u.mlme.status == MLME_DENIED)) {
    if (event->u.mlme.status == MLME_TIMEOUT) {
      rsi_dbg(ERR_ZONE, "%s: ASSOC Timeout: reason = %d\n", __func__, event->u.mlme.reason);
    } else {
      rsi_dbg(ERR_ZONE, "%s: ASSOC Denied: reason = %d\n", __func__, event->u.mlme.reason);
    }
#ifdef CONFIG_STA_PLUS_AP
    rsi_send_sta_notify_frame(common, STA_OPMODE, STA_DISCONNECTED, bss->bssid, bss->qos, AID, 0, vif);
#else
    rsi_send_sta_notify_frame(common, STA_OPMODE, STA_DISCONNECTED, bss->bssid, bss->qos, AID, 0);
#endif
  }
}
#endif

static struct ieee80211_ops mac80211_ops = {
  .tx = rsi_mac80211_tx,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 2, 0)
  .wake_tx_queue = ieee80211_handle_wake_tx_queue,
#endif
  .start            = rsi_mac80211_start,
  .stop             = rsi_mac80211_stop,
  .add_interface    = rsi_mac80211_add_interface,
  .remove_interface = rsi_mac80211_remove_interface,
  .change_interface = rsi_mac80211_change_interface,
  .config           = rsi_mac80211_config,
  .bss_info_changed = rsi_mac80211_bss_info_changed,
  .conf_tx          = rsi_mac80211_conf_tx,
  .configure_filter = rsi_mac80211_conf_filter,
  .set_key          = rsi_mac80211_set_key,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0) && LINUX_VERSION_CODE <= KERNEL_VERSION(5, 9, 16))
  .set_frag_threshold = rsi_mac80211_set_frag_threshold,
#endif
  .set_rts_threshold = rsi_mac80211_set_rts_threshold,
  .set_bitrate_mask  = rsi_mac80211_set_rate_mask,
  .ampdu_action      = rsi_mac80211_ampdu_action,
  .sta_add           = rsi_mac80211_sta_add,
  .sta_remove        = rsi_mac80211_sta_remove,
  .set_antenna       = rsi_mac80211_set_antenna,
  .get_antenna       = rsi_mac80211_get_antenna,
  .rfkill_poll       = rsi_mac80211_rfkill_poll,
  .get_survey        = rsi_mac80211_get_chan_survey,
#ifdef CONFIG_PM
  .suspend = rsi_mac80211_suspend,
  .resume  = rsi_mac80211_resume,
#endif
  .hw_scan                  = rsi_mac80211_hw_scan_start,
  .cancel_hw_scan           = rsi_mac80211_hw_scan_cancel,
  .remain_on_channel        = rsi_mac80211_roc,
  .cancel_remain_on_channel = rsi_mac80211_cancel_roc,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 1, 0))
  .event_callback = rsi_mac80211_event_callback,
#endif
  .start_ap = rsi_start_ap,
};

/**
 * rsi_mac80211_attach() - This function is used to initialize Mac80211 stack.
 * @common: Pointer to the driver private structure.
 *
 * Return: 0 on success, -1 on failure.
 */
int rsi_mac80211_attach(struct rsi_common *common)
{
  int status              = 0;
  struct ieee80211_hw *hw = NULL;
  struct wiphy *wiphy     = NULL;
  struct rsi_hw *adapter  = common->priv;
  u8 addr_mask[ETH_ALEN]  = { 0x0, 0x0, 0x0, 0xff, 0xff, 0xff };

  rsi_dbg(INIT_ZONE, "%s: Performing mac80211 attach\n", __func__);

  hw = ieee80211_alloc_hw(sizeof(struct rsi_hw), &mac80211_ops);
  if (!hw) {
    rsi_dbg(ERR_ZONE, "%s: ieee80211 hw alloc failed\n", __func__);
    return -ENOMEM;
  }

  wiphy = hw->wiphy;

  SET_IEEE80211_DEV(hw, adapter->device);

  hw->priv    = adapter;
  adapter->hw = hw;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0))
  ieee80211_hw_set(hw, SIGNAL_DBM);
  ieee80211_hw_set(hw, HAS_RATE_CONTROL);
  ieee80211_hw_set(hw, AMPDU_AGGREGATION);
  ieee80211_hw_set(hw, SUPPORTS_PS);
  ieee80211_hw_set(hw, SUPPORTS_DYNAMIC_PS);
  if (DEV_MODEL_9116)
    ieee80211_hw_set(hw, CONNECTION_MONITOR);
  ieee80211_hw_set(hw, SPECTRUM_MGMT);
  ieee80211_hw_set(hw, MFP_CAPABLE);
  ieee80211_hw_set(hw, SINGLE_SCAN_ON_ALL_BANDS);
  if (common->driver_mode == SNIFFER_MODE) {
    ieee80211_hw_set(hw, RX_INCLUDES_FCS);
    ieee80211_hw_set(hw, WANT_MONITOR_VIF);
  }
//__9117_CODE_START
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
  ieee80211_hw_set(hw, SUPPORTS_MULTI_BSSID);
  ieee80211_hw_set(hw, SUPPORTS_ONLY_HE_MULTI_BSSID);
#ifdef CONFIG_TWT_SUPPORT
  wiphy->extended_capabilities     = rsi_ext_capa;
  wiphy->extended_capabilities_len = sizeof(rsi_ext_capa);
#endif
#endif
  //__9117_CODE_END
  if (common->enable_encap_offload) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0) && LINUX_VERSION_CODE <= KERNEL_VERSION(5, 9, 16))
    ieee80211_hw_set(hw, SUPPORTS_TX_FRAG);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    ieee80211_hw_set(hw, SUPPORTS_TX_ENCAP_OFFLOAD);
#endif
  }

#else
  hw->flags = IEEE80211_HW_SIGNAL_DBM | IEEE80211_HW_HAS_RATE_CONTROL | IEEE80211_HW_AMPDU_AGGREGATION
              | IEEE80211_HW_SUPPORTS_PS | IEEE80211_HW_SUPPORTS_DYNAMIC_PS | IEEE80211_HW_SPECTRUM_MGMT
              | IEEE80211_HW_MFP_CAPABLE | 0;
  if (DEV_MODEL_9116)
    hw->flags |= IEEE80211_HW_CONNECTION_MONITOR;
  if (common->driver_mode == SNIFFER_MODE) {
    hw->flags |= IEEE80211_HW_RX_INCLUDES_FCS | IEEE80211_HW_WANT_MONITOR_VIF;
  }

#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 6, 0))
  wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_RRM);
#endif

  hw->queues            = MAX_HW_QUEUES;
  hw->extra_tx_headroom = RSI_NEEDED_HEADROOM;

  hw->max_rates        = 1;
  hw->max_rate_tries   = MAX_RETRIES;
  hw->uapsd_queues     = IEEE80211_MARKALL_UAPSD_QUEUES;
  hw->uapsd_max_sp_len = common->max_sp_len;
  if (DEV_MODEL_9116)
    hw->max_tx_aggregation_subframes = TX_AGGR_LIMIT_FOR_RS9116;
  else
    hw->max_tx_aggregation_subframes = TX_AGGR_LIMIT_FOR_RS9113;

  hw->max_rx_aggregation_subframes = RX_AGGR_LIMIT_FOR_RS9116;

  rsi_register_rates_channels(adapter, NL80211_BAND_2GHZ);
  wiphy->bands[NL80211_BAND_2GHZ] = &adapter->sbands[NL80211_BAND_2GHZ];
  if (common->num_supp_bands > 1) {
    rsi_register_rates_channels(adapter, NL80211_BAND_5GHZ);
    wiphy->bands[NL80211_BAND_5GHZ] = &adapter->sbands[NL80211_BAND_5GHZ];
  }
  hw->rate_control_algorithm = "AARF";
  hw->sta_data_size          = sizeof(struct rsi_sta);
  hw->vif_data_size          = sizeof(struct vif_priv);

  SET_IEEE80211_PERM_ADDR(hw, common->mac_addr);
  ether_addr_copy(hw->wiphy->addr_mask, addr_mask);

  wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_AP) | BIT(NL80211_IFTYPE_MONITOR)
                           | BIT(NL80211_IFTYPE_P2P_DEVICE) | BIT(NL80211_IFTYPE_P2P_CLIENT)
                           | BIT(NL80211_IFTYPE_P2P_GO);

  wiphy->signal_type           = CFG80211_SIGNAL_TYPE_MBM;
  wiphy->retry_short           = RETRY_SHORT;
  wiphy->retry_long            = RETRY_LONG;
  wiphy->frag_threshold        = IEEE80211_MAX_FRAG_THRESHOLD;
  wiphy->rts_threshold         = IEEE80211_MAX_RTS_THRESHOLD;
  wiphy->available_antennas_tx = 1;
  wiphy->available_antennas_rx = 1;

  wiphy->max_ap_assoc_sta = rsi_max_ap_stas[common->oper_mode - 1];
  common->max_stations    = wiphy->max_ap_assoc_sta;
  rsi_dbg(INFO_ZONE, "Max Stations Allowed = %d\n", common->max_stations);

  wiphy->max_scan_ssids   = 16;
  wiphy->max_scan_ie_len  = 256;
  hw->max_listen_interval = 16;
  /* AP Parameters */
  wiphy->flags = WIPHY_FLAG_REPORTS_OBSS;
  wiphy->flags |= WIPHY_FLAG_AP_UAPSD;
  /* wiphy->features |= NL80211_FEATURE_INACTIVITY_TIMER;

	wiphy->regulatory_flags = (REGULATORY_STRICT_REG |
				   REGULATORY_CUSTOM_REG);
	wiphy_apply_custom_regulatory(wiphy,&rsi_regdom);*/
  wiphy->reg_notifier = rsi_reg_notify;

#ifdef CONFIG_RSI_WOW
  wiphy->wowlan = &rsi_wowlan_support;
#endif

#ifdef CONFIG_RSI_P2P
  /* Wi-Fi direct related parameters */
  wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL;
  wiphy->flags |= WIPHY_FLAG_OFFCHAN_TX;
  wiphy->max_remain_on_channel_duration = 10000;
  hw->max_listen_interval               = 10;
  wiphy->iface_combinations             = rsi_iface_combinations;
  wiphy->n_iface_combinations           = ARRAY_SIZE(rsi_iface_combinations);
//	wiphy->features |= (NL80211_FEATURE_P2P_GO_CTWIN |
//			    NL80211_FEATURE_P2P_GO_OPPPS);
#else
  wiphy->iface_combinations = rsi_iface_combinations;
  wiphy->n_iface_combinations = ARRAY_SIZE(rsi_iface_combinations);
#endif

  //  if (common->coex_mode > 1)
  //    wiphy->flags |= WIPHY_FLAG_PS_ON_BY_DEFAULT;

  status = ieee80211_register_hw(hw);
  if (status) {
    rsi_dbg(ERR_ZONE, "Failed to register to mac80211\n");
    return status;
  }
  /* Interface is just enabled, if nothing(station or AP) is running
	 * on this wifi interface, better let it be in power save mode.
	 */
  if (common->default_deep_sleep_enable && common->driver_mode == 1)
    rsi_enable_ps(adapter);
  //__9117_CODE_START
  if (adapter->device_model >= RSI_DEV_9117)
    return rsi_init_917_dbgfs(adapter);
  else
    //__9117_CODE_END
    return rsi_init_dbgfs(adapter);
}
