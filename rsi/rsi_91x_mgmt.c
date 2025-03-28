// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <linux/etherdevice.h>
#include "rsi_mgmt.h"
#include "rsi_common.h"
#include "rsi_ps.h"
#include "rsi_hal.h"
#include "rsi_sdio.h"
#ifdef CONFIG_RSI_COEX_MODE
#include "rsi_coex.h"
#endif
#ifdef CONFIG_RSI_11K
#include "rsi_rrm.h"
#endif
//__9117_CODE_START
#include "rsi_he.h"
#include <net/cfg80211.h>
//__9117_CODE_END

#if 0
static struct rsi_config_vals dev_config_vals[] = {
	{
		.lp_ps_handshake = 0,
		.ulp_ps_handshake = 0,
		.sleep_config_params = 0,
		.ext_pa_or_bt_coex_en = 0,
	},
};
#endif

struct ieee80211_channel rsi_2ghz_acx_channels[] = {
  { .band = NL80211_BAND_2GHZ, .center_freq = 2412, .max_power = 27, .hw_value = 1 },  /* Channel 1 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2417, .max_power = 27, .hw_value = 2 },  /* Channel 2 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2422, .max_power = 27, .hw_value = 3 },  /* Channel 3 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2427, .max_power = 27, .hw_value = 4 },  /* Channel 4 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2432, .max_power = 27, .hw_value = 5 },  /* Channel 5 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2437, .max_power = 27, .hw_value = 6 },  /* Channel 6 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2442, .max_power = 27, .hw_value = 7 },  /* Channel 7 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2447, .max_power = 27, .hw_value = 8 },  /* Channel 8 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2452, .max_power = 27, .hw_value = 9 },  /* Channel 9 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2457, .max_power = 27, .hw_value = 10 }, /* Channel 10 */
  { .band = NL80211_BAND_2GHZ, .center_freq = 2462, .max_power = 27, .hw_value = 11 }, /* Channel 11 */
  { .band        = NL80211_BAND_2GHZ,
    .center_freq = 2467,
    .max_power   = 27,
    .hw_value    = 12,
    .flags       = IEEE80211_CHAN_NO_IR }, /* Channel 12 */
  { .band        = NL80211_BAND_2GHZ,
    .center_freq = 2472,
    .max_power   = 27,
    .hw_value    = 13,
    .flags       = IEEE80211_CHAN_NO_IR }, /* Channel 13 */
};

#ifdef CONFIG_RSI_11K
struct reg_class reg_db[MAX_REGIONS][MAX_REG_CLASS] = {
  { { 1, 20, { 36, 40, 44, 48 } },
    { 2, 20, { 52, 56, 60, 64 } },
    { 3, 20, { 149, 153, 157, 161 } },
    { 4, 20, { 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140 } },
    { 5, 20, { 149, 153, 157, 161, 165 } },
    { 6, 5, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } },
    { 7, 5, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } },
    { 8, 10, { 11, 13, 15, 17, 19 } },
    { 9, 10, { 11, 13, 15, 17, 19 } },
    { 10, 20, { 21, 25 } },
    { 11, 20, { 21, 25 } },
    { 12, 25, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 } },
    { 13, 20, { 133, 137 } },
    { 14, 10, { 132, 134, 136, 138 } },
    { 15, 5, { 131, 132, 133, 134, 135, 136, 137, 138 } },
    { 22, 40, { 36, 44 } },
    { 23, 40, { 52, 60 } },
    { 24, 40, { 100, 108, 116, 124, 132 } },
    { 25, 40, { 149, 157 } },
    { 26, 40, { 149, 157 } },
    { 27, 40, { 40, 48 } },
    { 28, 40, { 56, 64 } },
    { 29, 40, { 104, 112, 120, 128, 136 } },
    { 30, 40, { 153, 161 } },
    { 31, 40, { 153, 161 } },
    { 32, 40, { 1, 2, 3, 4, 5, 6, 7 } },
    { 33, 40, { 5, 6, 7, 8, 9, 10, 11 } } },
  { { 1, 20, { 36, 40, 44, 48 } },
    { 2, 20, { 52, 56, 60, 64 } },
    { 3, 20, { 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140 } },
    { 4, 25, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 } },
    { 5, 40, { 36, 44 } },
    { 6, 40, { 52, 60 } },
    { 7, 40, { 100, 108, 116, 124, 132 } },
    { 8, 40, { 40, 48 } },
    { 9, 40, { 56, 64 } },
    { 10, 40, { 104, 112, 120, 128, 136 } },
    { 11, 40, { 1, 2, 3, 4, 5, 6, 7, 8, 9 } },
    { 12, 40, { 5, 6, 7, 8, 9, 10, 11, 12, 13 } },
    { 16, 20, { 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140 } },
    { 17, 20, { 149, 153, 157, 161, 165, 169 } } },
  {
    { 1, 20, { 34, 38, 42, 46, 36, 40, 44, 48 } },
    { 2, 20, { 8, 12, 16 } },
    { 3, 20, { 8, 12, 16 } },
    { 4, 20, { 8, 12, 16 } },
    { 5, 20, { 8, 12, 16 } },
    { 6, 20, { 8, 12, 16 } },
    { 7, 20, { 184, 188, 192, 196 } },
    { 8, 20, { 184, 188, 192, 196 } },
    { 9, 20, { 184, 188, 192, 196 } },
    { 10, 20, { 184, 188, 192, 196 } },
    { 11, 20, { 184, 188, 192, 196 } },
    { 12, 10, { 7, 8, 9, 11 } },
    { 13, 10, { 7, 8, 9, 11 } },
    { 14, 10, { 7, 8, 9, 11 } },
    { 15, 10, { 7, 8, 9, 11 } },
    { 16, 10, { 183, 184, 185, 187, 188, 189 } },
    { 17, 10, { 183, 184, 185, 187, 188, 189 } },
    { 18, 10, { 183, 184, 185, 187, 188, 189 } },
    { 19, 10, { 183, 184, 185, 187, 188, 189 } },
    { 20, 10, { 183, 184, 185, 187, 188, 189 } },
    { 21, 5, { 6, 7, 8, 9, 10, 11 } },
    { 22, 5, { 6, 7, 8, 9, 10, 11 } },
    { 23, 5, { 6, 7, 8, 9, 10, 11 } },
    { 24, 5, { 6, 7, 8, 9, 10, 11 } },
    { 25, 5, { 182, 183, 184, 185, 186, 187, 188, 189 } },
    { 26, 5, { 182, 183, 184, 185, 186, 187, 188, 189 } },
    { 27, 5, { 182, 183, 184, 185, 186, 187, 188, 189 } },
    { 28, 5, { 182, 183, 184, 185, 186, 187, 188, 189 } },
    { 29, 5, { 182, 183, 184, 185, 186, 187, 188, 189 } },
    { 30, 25, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 } },
    { 31, 25, { 14 } },
    { 32, 20, { 52, 56, 60, 64 } },
    { 33, 20, { 52, 56, 60, 64 } },
    { 34, 20, { 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140 } },
    { 35, 20, { 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140 } },
    { 36, 40, { 36, 44 } },
    { 37, 40, { 52, 60 } },
    { 38, 40, { 52, 60 } },
    { 39, 40, { 100, 108, 116, 124, 132 } },
    { 40, 40, { 100, 108, 116, 124, 132 } },
    { 41, 40, { 40, 48 } },
    { 42, 40, { 56, 64 } },
    { 43, 40, { 56, 64 } },
    { 44, 40, { 104, 112, 120, 128, 136 } },
    { 45, 40, { 104, 112, 120, 128, 136 } },
    { 46, 40, { 184, 192 } },
    { 47, 40, { 184, 192 } },
    { 48, 40, { 184, 192 } },
    { 49, 40, { 184, 192 } },
    { 50, 40, { 184, 192 } },
    { 51, 40, { 188, 196 } },
    { 52, 40, { 188, 196 } },
    { 53, 40, { 188, 196 } },
    { 54, 40, { 188, 196 } },
    { 55, 40, { 188, 196 } },
    { 56, 40, { 1, 2, 3, 4, 5, 6, 7, 8, 9 } },
    { 57, 40, { 5, 6, 7, 8, 9, 10, 11, 12, 13 } },
    { 58, 20, { 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140 } },
  },
  { { 81, 25, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 } },
    { 82, 25, { 14 } },
    { 83, 40, { 1, 2, 3, 4, 5, 6, 7, 8, 9 } },
    { 84, 40, { 5, 6, 7, 8, 9, 10, 11, 12, 13 } },
    { 94, 20, { 133, 137 } },
    { 95, 10, { 132, 134, 136, 138 } },
    { 96, 5, { 131, 132, 133, 134, 135, 136, 137, 138 } },
    { 101, 20, { 21, 25 } },
    { 102, 10, { 11, 13, 15, 17, 19 } },
    { 103, 5, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } },
    { 104, 40, { 184, 192 } },
    { 105, 40, { 188, 196 } },
    { 106, 20, { 191, 195 } },
    { 107, 10, { 189, 191, 193, 195, 197 } },
    { 108, 5, { 188, 189, 190, 191, 192, 193, 194, 195, 196, 197 } },
    { 109, 20, { 184, 188, 192, 196 } },
    { 110, 10, { 183, 184, 185, 186, 187, 188, 189 } },
    { 111, 5, { 183, 184, 185, 186, 187, 188, 189 } },
    { 112, 20, { 8, 12, 16 } },
    { 113, 10, { 7, 8, 9, 10, 11 } },
    { 114, 5, { 6, 7, 8, 9, 10, 11 } },
    { 115, 20, { 36, 40, 44, 48 } },
    { 116, 40, { 36, 44 } },
    { 117, 40, { 40, 48 } },
    { 118, 20, { 52, 56, 60, 64 } },
    {
      119,
      40,
      { 52, 60 },
    },
    {
      120,
      40,
      { 56, 64 },
    },
    { 121, 20, { 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140 } },
    { 122, 40, { 100, 108, 116, 124, 132 } },
    { 123, 40, { 104, 112, 120, 128, 136 } },
    { 124, 20, { 149, 153, 157, 161 } },
    { 125, 20, { 149, 153, 157, 161, 165, 169 } },
    { 126, 40, { 149, 157 } },
    { 127, 40, { 153, 161 } } }
};
#endif

/* Bootup Parameters for 20MHz */
static struct bootup_params boot_params_20 = {
	.magic_number = cpu_to_le16(0x5aa5),
	.crystal_good_time = 0x0,
	.valid = cpu_to_le32(VALID_20),
	.reserved_for_valids = 0x0,
	.bootup_mode_info = 0x0,
	.digital_loop_back_params = 0x0,
	.rtls_timestamp_en = 0x0,
	.host_spi_intr_cfg = 0x0,
	.device_clk_info = {{
		/* WLAN params */
		.pll_config_g = {
			.tapll_info_g = {
				.pll_reg_1 = cpu_to_le16((TAPLL_N_VAL_20 << 8) |
							 (TAPLL_M_VAL_20)),
				.pll_reg_2 = cpu_to_le16(TAPLL_P_VAL_20),
			},
			.pll960_info_g = {
				.pll_reg_1 =
					cpu_to_le16((PLL960_P_VAL_20 << 8) |
						    (PLL960_N_VAL_20)),
				.pll_reg_2 = cpu_to_le16(PLL960_M_VAL_20),
				.pll_reg_3 = 0x0,
			},
			.afepll_info_g = {
				.pll_reg = cpu_to_le16(0x9f0),
			}
		},
		.switch_clk_g = {
			.switch_umac_clk = 0x1,
			.switch_qspi_clk = 0x1,
			.switch_slp_clk_2_32 = 0x0,
			.switch_bbp_lmac_clk_reg = 0x1,
			.switch_mem_ctrl_cfg = 0x0,
			.reserved = 0x0,
			.bbp_lmac_clk_reg_val = cpu_to_le16(0x111),
			.umac_clock_reg_config = cpu_to_le16(0x48),
			.qspi_uart_clock_reg_config = cpu_to_le16(0x1211)
		}
	},
	/* Bluetooth params */
	{
		.pll_config_g = {
			.tapll_info_g = {
				.pll_reg_1 = cpu_to_le16((TAPLL_N_VAL_20 << 8) |
							 (TAPLL_M_VAL_20)),
				.pll_reg_2 = cpu_to_le16(TAPLL_P_VAL_20),
			},
			.pll960_info_g = {
				.pll_reg_1 =
					cpu_to_le16((PLL960_P_VAL_20 << 8) |
						    (PLL960_N_VAL_20)),
				.pll_reg_2 = cpu_to_le16(PLL960_M_VAL_20),
				.pll_reg_3 = 0x0,
			},
			.afepll_info_g = {
				.pll_reg = cpu_to_le16(0x9f0),
			}
		},
		.switch_clk_g = {
			.switch_umac_clk = 0x0,
			.switch_qspi_clk = 0x0,
			.switch_slp_clk_2_32 = 0x0,
			.switch_bbp_lmac_clk_reg = 0x0,
			.switch_mem_ctrl_cfg = 0x0,
			.reserved = 0x0,
			.bbp_lmac_clk_reg_val = 0x0,
			.umac_clock_reg_config = 0x0,
			.qspi_uart_clock_reg_config = 0x0
		}
	},
	/* Zigbee params */
	{
		.pll_config_g = {
			.tapll_info_g = {
				.pll_reg_1 = cpu_to_le16((TAPLL_N_VAL_20 << 8) |
							 (TAPLL_M_VAL_20)),
				.pll_reg_2 = cpu_to_le16(TAPLL_P_VAL_20),
			},
			.pll960_info_g = {
				.pll_reg_1 =
					cpu_to_le16((PLL960_P_VAL_20 << 8) |
						    (PLL960_N_VAL_20)),
				.pll_reg_2 = cpu_to_le16(PLL960_M_VAL_20),
				.pll_reg_3 = 0x0,
			},
			.afepll_info_g = {
				.pll_reg = cpu_to_le16(0x9f0),
			}
		},
		.switch_clk_g = {
			.switch_umac_clk = 0x0,
			.switch_qspi_clk = 0x0,
			.switch_slp_clk_2_32 = 0x0,
			.switch_bbp_lmac_clk_reg = 0x0,
			.switch_mem_ctrl_cfg = 0x0,
			.reserved = 0x0,
			.bbp_lmac_clk_reg_val = 0x0,
			.umac_clock_reg_config = 0x0,
			.qspi_uart_clock_reg_config = 0x0
		}
	}
	},
	/* ULP Params */
	.buckboost_wakeup_cnt = 0x0,
	.pmu_wakeup_wait = 0x0,
	.shutdown_wait_time = 0x0,
	.pmu_slp_clkout_sel = 0x0,
	.wdt_prog_value = 0x0,
	.wdt_soc_rst_delay = 0x0,
	.dcdc_operation_mode = 0x0,
	.soc_reset_wait_cnt = 0x0,
	.waiting_time_at_fresh_sleep = 0x0,
	.max_threshold_to_avoid_sleep = 0x0,
	.beacon_resedue_alg_en = 0,
};

/* Bootup Parameters for 20MHz */
static struct bootup_params_9116 boot_params_9116_20 = {
	.magic_number = cpu_to_le16(0x5aa5),
	.crystal_good_time = 0x0,
	.valid = cpu_to_le32(VALID_20),
	.reserved_for_valids = 0x0,
	.bootup_mode_info = 0x0,
	.digital_loop_back_params = 0x0,
	.rtls_timestamp_en = 0x0,
	.host_spi_intr_cfg = 0x0,
	.device_clk_info_9116 = {{
		/* WLAN params */
		.pll_config_9116_g = {
			.pll_ctrl_set_reg = 0xd518,
			.pll_ctrl_clr_reg = 0x2ae7,
			.pll_modem_conig_reg = 0x2000,
			.soc_clk_config_reg = 0x0C18,
			.adc_dac_strm1_config_reg = 0x1100,
			.adc_dac_strm2_config_reg = 0x6600,
		},
		.switch_clk_9116_g = {
			.switch_clk_info =
				cpu_to_le32((RSI_SWITCH_TASS_CLK |
					    RSI_SWITCH_WLAN_BBP_LMAC_CLK_REG |
					    RSI_SWITCH_BBP_LMAC_CLK_REG)),
			.tass_clock_reg = 0x083C0503,
			.wlan_bbp_lmac_clk_reg_val = 0x01042001,
			.zbbt_bbp_lmac_clk_reg_val = 0x02010001,
			.bbp_lmac_clk_en_val = 0x0000003b,
		}
	},
	},
	/* ULP Params */
	.buckboost_wakeup_cnt = 0x0,
	.pmu_wakeup_wait = 0x0,
	.shutdown_wait_time = 0x0,
	.pmu_slp_clkout_sel = 0x0,
	.wdt_prog_value = 0x0,
	.wdt_soc_rst_delay = 0x0,
	.dcdc_operation_mode = 0x0,
	.soc_reset_wait_cnt = 0x0,
	.waiting_time_at_fresh_sleep = 0x0,
	.max_threshold_to_avoid_sleep = 0x0,
	.beacon_resedue_alg_en = 0,
};

/* Bootup parameters for 40MHz */
static struct bootup_params boot_params_40 = {
	.magic_number = cpu_to_le16(0x5aa5),
	.crystal_good_time = 0x0,
	.valid = cpu_to_le32(VALID_40),
	.reserved_for_valids = 0x0,
	.bootup_mode_info = 0x0,
	.digital_loop_back_params = 0x0,
	.rtls_timestamp_en = 0x0,
	.host_spi_intr_cfg = 0x0,
	.device_clk_info = {{
		/* WLAN params */
		.pll_config_g = {
			.tapll_info_g = {
				.pll_reg_1 = cpu_to_le16((TAPLL_N_VAL_40 << 8) |
							 (TAPLL_M_VAL_40)),
				.pll_reg_2 = cpu_to_le16(TAPLL_P_VAL_40),
			},
			.pll960_info_g = {
				.pll_reg_1 =
					cpu_to_le16((PLL960_P_VAL_40 << 8) |
						    (PLL960_N_VAL_40)),
				.pll_reg_2 = cpu_to_le16(PLL960_M_VAL_40),
				.pll_reg_3 = 0x0,
			},
			.afepll_info_g = {
				.pll_reg = cpu_to_le16(0x9f0),
			}
		},
		.switch_clk_g = {
			.switch_umac_clk = 0x1,
			.switch_qspi_clk = 0x1,
			.switch_slp_clk_2_32 = 0x0,
			.switch_bbp_lmac_clk_reg = 0x1,
			.switch_mem_ctrl_cfg = 0x0,
			.reserved = 0x0,
			.bbp_lmac_clk_reg_val = cpu_to_le16(0x1121),
			.umac_clock_reg_config = cpu_to_le16(0x48),
			.qspi_uart_clock_reg_config = cpu_to_le16(0x1211)
		}
	},
	/* Bluetooth Params */
	{
		.pll_config_g = {
			.tapll_info_g = {
				.pll_reg_1 = cpu_to_le16((TAPLL_N_VAL_40 << 8) |
							 (TAPLL_M_VAL_40)),
				.pll_reg_2 = cpu_to_le16(TAPLL_P_VAL_40),
			},
			.pll960_info_g = {
				.pll_reg_1 =
					cpu_to_le16((PLL960_P_VAL_40 << 8) |
						    (PLL960_N_VAL_40)),
				.pll_reg_2 = cpu_to_le16(PLL960_M_VAL_40),
				.pll_reg_3 = 0x0,
			},
			.afepll_info_g = {
				.pll_reg = cpu_to_le16(0x9f0),
			}
		},
		.switch_clk_g = {
			.switch_umac_clk = 0x0,
			.switch_qspi_clk = 0x0,
			.switch_slp_clk_2_32 = 0x0,
			.switch_bbp_lmac_clk_reg = 0x0,
			.switch_mem_ctrl_cfg = 0x0,
			.reserved = 0x0,
			.bbp_lmac_clk_reg_val = 0x0,
			.umac_clock_reg_config = 0x0,
			.qspi_uart_clock_reg_config = 0x0
		}
	},
	/* Zigbee Params */
	{
		.pll_config_g = {
			.tapll_info_g = {
				.pll_reg_1 = cpu_to_le16((TAPLL_N_VAL_40 << 8) |
							 (TAPLL_M_VAL_40)),
				.pll_reg_2 = cpu_to_le16(TAPLL_P_VAL_40),
			},
			.pll960_info_g = {
				.pll_reg_1 =
					cpu_to_le16((PLL960_P_VAL_40 << 8) |
						    (PLL960_N_VAL_40)),
				.pll_reg_2 = cpu_to_le16(PLL960_M_VAL_40),
				.pll_reg_3 = 0x0,
			},
			.afepll_info_g = {
				.pll_reg = cpu_to_le16(0x9f0),
			}
		},
		.switch_clk_g = {
			.switch_umac_clk = 0x0,
			.switch_qspi_clk = 0x0,
			.switch_slp_clk_2_32 = 0x0,
			.switch_bbp_lmac_clk_reg = 0x0,
			.switch_mem_ctrl_cfg = 0x0,
			.reserved = 0x0,
			.bbp_lmac_clk_reg_val = 0x0,
			.umac_clock_reg_config = 0x0,
			.qspi_uart_clock_reg_config = 0x0
		}
	}
	},
	/* ULP Params */
	.buckboost_wakeup_cnt = 0x0,
	.pmu_wakeup_wait = 0x0,
	.shutdown_wait_time = 0x0,
	.pmu_slp_clkout_sel = 0x0,
	.wdt_prog_value = 0x0,
	.wdt_soc_rst_delay = 0x0,
	.dcdc_operation_mode = 0x0,
	.soc_reset_wait_cnt = 0x0,
	.waiting_time_at_fresh_sleep = 0x0,
	.max_threshold_to_avoid_sleep = 0x0,
	.beacon_resedue_alg_en = 0,
};

/* Bootup parameters for 40MHz - 9116 */
static struct bootup_params_9116 boot_params_9116_40 = {
	.magic_number = cpu_to_le16(0x5aa5),
	.crystal_good_time = 0x0,
	.valid = cpu_to_le32(VALID_40),
	.reserved_for_valids = 0x0,
	.bootup_mode_info = 0x0,
	.digital_loop_back_params = 0x0,
	.rtls_timestamp_en = 0x0,
	.host_spi_intr_cfg = 0x0,
	.device_clk_info_9116 = {{
		/* WLAN params */
		.pll_config_9116_g = {
			.pll_ctrl_set_reg = 0xd518,
			.pll_ctrl_clr_reg = 0x2ae7,
			.pll_modem_conig_reg = 0x3000,
			.soc_clk_config_reg = 0x0C18,
			.adc_dac_strm1_config_reg = 0x0000,
			.adc_dac_strm2_config_reg = 0x6600,
		},
		.switch_clk_9116_g = {
			.switch_clk_info =
				cpu_to_le32((RSI_SWITCH_TASS_CLK |
					    RSI_SWITCH_WLAN_BBP_LMAC_CLK_REG |
					    RSI_SWITCH_BBP_LMAC_CLK_REG |
					    RSI_MODEM_CLK_160MHZ)),
			.tass_clock_reg = 0x083C0503,
#ifdef CONFIG_FPGA_40MHZ
			.wlan_bbp_lmac_clk_reg_val = 0x01081002,
#else
			.wlan_bbp_lmac_clk_reg_val = 0x01042002,
#endif
			.zbbt_bbp_lmac_clk_reg_val = 0x04010002,
			.bbp_lmac_clk_en_val = 0x0000003b,
		}
	},
	},
	/* ULP Params */
	.buckboost_wakeup_cnt = 0x0,
	.pmu_wakeup_wait = 0x0,
	.shutdown_wait_time = 0x0,
	.pmu_slp_clkout_sel = 0x0,
	.wdt_prog_value = 0x0,
	.wdt_soc_rst_delay = 0x0,
	.dcdc_operation_mode = 0x0,
	.soc_reset_wait_cnt = 0x0,
	.waiting_time_at_fresh_sleep = 0x0,
	.max_threshold_to_avoid_sleep = 0x0,
	.beacon_resedue_alg_en = 0,
};

#define UNUSED_GPIO 1
#define USED_GPIO   0
static struct rsi_ulp_gpio_vals unused_ulp_gpio_bitmap = {
  .motion_sensor_gpio_ulp_wakeup = UNUSED_GPIO,
  .sleep_ind_from_device         = UNUSED_GPIO,
  .ulp_gpio_2                    = UNUSED_GPIO,
  .push_button_ulp_wakeup        = UNUSED_GPIO,
};

struct rsi_soc_gpio_vals unused_soc_gpio_bitmap = {
  .pspi_csn_0                          = USED_GPIO,   //GPIO_0
  .pspi_csn_1                          = USED_GPIO,   //GPIO_1
  .host_wakeup_intr                    = USED_GPIO,   //GPIO_2
  .pspi_data_0                         = USED_GPIO,   //GPIO_3
  .pspi_data_1                         = USED_GPIO,   //GPIO_4
  .pspi_data_2                         = USED_GPIO,   //GPIO_5
  .pspi_data_3                         = USED_GPIO,   //GPIO_6
  .i2c_scl                             = USED_GPIO,   //GPIO_7
  .i2c_sda                             = USED_GPIO,   //GPIO_8
  .uart1_rx                            = UNUSED_GPIO, //GPIO_9
  .uart1_tx                            = UNUSED_GPIO, //GPIO_10
  .uart1_rts_i2s_clk                   = UNUSED_GPIO, //GPIO_11
  .uart1_cts_i2s_ws                    = UNUSED_GPIO, //GPIO_12
  .dbg_uart_rx_i2s_din                 = UNUSED_GPIO, //GPIO_13
  .dbg_uart_tx_i2s_dout                = UNUSED_GPIO, //GPIO_14
  .lp_wakeup_boot_bypass               = UNUSED_GPIO, //GPIO_15
  .led_0                               = USED_GPIO,   //GPIO_16
  .btcoex_wlan_active_ext_pa_ant_sel_A = UNUSED_GPIO, //GPIO_17
  .btcoex_bt_priority_ext_pa_ant_sel_B = UNUSED_GPIO, //GPIO_18
  .btcoex_bt_active_ext_pa_on_off      = UNUSED_GPIO, //GPIO_19
  .rf_reset                            = USED_GPIO,   //GPIO_20
  .sleep_ind_from_device               = UNUSED_GPIO,
};

static u16 mcs[] = { 13, 26, 39, 52, 78, 104, 117, 130 };

#ifdef CONFIG_RSI_11K
int ieee80211_chan_to_bw(struct rsi_hw *adapter, u8 op_class, u8 channel_num)
{
  int i, j;
  u8 region = adapter->dfs_region;

  for (i = 0; i < ARRAY_SIZE(reg_db[region - 1]); i++) {
    if (reg_db[region - 1][i].op_class == op_class) {
      for (j = 0; j < ARRAY_SIZE(reg_db[region - 1][i].chans); j++) {
        if (reg_db[region - 1][i].chans[j] == channel_num)
          return reg_db[region - 1][i].bandwidth;
      }
    }
  }
  return -1;
}

int is_dfs_channel(struct rsi_hw *adapter, int channel)
{
  struct ieee80211_supported_band *sband;
  struct ieee80211_channel *ch;
  struct wiphy *wiphy = adapter->hw->wiphy;
  int i;

  sband = wiphy->bands[NL80211_BAND_5GHZ];
  for (i = 0; i < sband->n_channels; i++) {
    ch = &sband->channels[i];
    if (ch->flags & IEEE80211_CHAN_DISABLED)
      continue;
    if ((ch->hw_value == channel) && (ch->flags & IEEE80211_CHAN_RADAR))
      return 1;
  }
  return 0;
}
#endif

/**
 * rsi_set_default_parameters() - This function sets default parameters.
 * @common: Pointer to the driver private structure.
 *
 * Return: none
 */
static void rsi_set_default_parameters(struct rsi_common *common)
{
  common->band                          = NL80211_BAND_2GHZ;
  common->channel_width                 = BW_20MHZ;
  common->rts_threshold                 = IEEE80211_MAX_RTS_THRESHOLD;
  common->channel                       = 1;
  common->min_rate                      = 0xffff;
  common->fsm_state                     = FSM_CARD_NOT_READY;
  common->iface_down                    = true;
  common->endpoint                      = EP_2GHZ_20MHZ;
  common->ta_aggr                       = 3;
  common->skip_fw_load                  = 0;       /* Default disable skipping fw loading */
  common->rf_power_val                  = 0;       /* Default 1.9V */
  common->device_gpio_type              = TA_GPIO; /* Default TA GPIO */
  common->country_code                  = 840;     /* Default US */
  common->wlan_rf_power_mode            = 0;
  common->bt_rf_power_mode              = 0;
  common->tx_power                      = RSI_TXPOWER_MAX;
  common->dtim_cnt                      = 2;
  common->beacon_interval               = 100;
  common->antenna_gain[0]               = 0;
  common->antenna_gain[1]               = 0;
  common->w9116_features.pll_mode       = 0x0;
  common->w9116_features.rf_type        = 1;
  common->w9116_features.wireless_mode  = 0;
  common->w9116_features.enable_ppe     = 0;
  common->w9116_features.afe_type       = 1;
  common->w9116_features.dpd            = 0;
  common->w9116_features.sifs_tx_enable = 0;
  common->hw_bmiss_threshold            = HW_BMISS_THRESHOLD;
  common->keep_alive_period             = DEFAULT_KEEP_ALIVE_PERIOD;
  //__9117_CODE_START
#ifdef CONIFG_UORA_SUPPORT
  common->uora_ocw_min = 0x7;
  common->uora_ocw_max = 0x1F;
#endif
  common->txop_dur_rts_threshold = MAX_TX_OP_RTS_TH;
  //__9117_CODE_END
}

void init_bgscan_params(struct rsi_common *common)
{
  common->bgscan_info.bgscan_threshold      = 10;
  common->bgscan_info.roam_threshold        = 10;
  common->bgscan_info.bgscan_periodicity    = 30;
  common->bgscan_info.num_bg_channels       = 0;
  common->bgscan_info.two_probe             = 0;
  common->bgscan_info.active_scan_duration  = 20;
  common->bgscan_info.passive_scan_duration = 70;
  common->bgscan_info.channels2scan[0]      = 1;
  common->bgscan_info.channels2scan[1]      = 2;
  common->bgscan_info.channels2scan[2]      = 3;
  common->bgscan_info.channels2scan[3]      = 4;
  common->bgscan_info.channels2scan[4]      = 5;
  common->bgscan_info.channels2scan[5]      = 6;
  common->bgscan_info.channels2scan[6]      = 7;
  common->bgscan_info.channels2scan[7]      = 8;
  common->bgscan_info.channels2scan[8]      = 9;
  common->bgscan_info.channels2scan[9]      = 10;
  common->bgscan_info.channels2scan[10]     = 11;
//	common->bgscan_info.channels2scan[11] = 12;
//	common->bgscan_info.channels2scan[12] = 13;
//	common->bgscan_info.channels2scan[13] = 14;
#if 0
	common->bgscan_info.channels2scan[11] = 36;
	common->bgscan_info.channels2scan[12] = 40;
	common->bgscan_info.channels2scan[13] = 44;
	common->bgscan_info.channels2scan[14] = 48;
	common->bgscan_info.channels2scan[15] = 52;
	common->bgscan_info.channels2scan[16] = 56;
	common->bgscan_info.channels2scan[17] = 60;
	common->bgscan_info.channels2scan[18] = 64;
	common->bgscan_info.channels2scan[19] = 100;
	common->bgscan_info.channels2scan[20] = 104;
#endif
}

/**
 * rsi_set_contention_vals() - This function sets the contention values for the
 *			       backoff procedure.
 * @common: Pointer to the driver private structure.
 *
 * Return: None.
 */
static void rsi_set_contention_vals(struct rsi_common *common)
{
  u8 ii = 0;

  for (; ii < NUM_EDCA_QUEUES; ii++) {
    common->tx_qinfo[ii].wme_params =
      (((common->edca_params[ii].cw_min / 2) + (common->edca_params[ii].aifs)) * WMM_SHORT_SLOT_TIME + SIFS_DURATION);
    common->tx_qinfo[ii].weight        = common->tx_qinfo[ii].wme_params;
    common->tx_qinfo[ii].pkt_contended = 0;
  }
}

/**
 * rsi_send_internal_mgmt_frame() - This function sends management frames to
 *				    firmware.Also schedules packet to queue
 *				    for transmission.
 * @common: Pointer to the driver private structure.
 * @skb: Pointer to the socket buffer structure.
 *
 * Return: 0 on success, -1 on failure.
 */
int rsi_send_internal_mgmt_frame(struct rsi_common *common, struct sk_buff *skb)
{
  struct skb_info *tx_params;

  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: SKB is NULL\n", __func__);
    return -EINVAL;
  }
  skb->data[1] |= BIT(7);
  tx_params = (struct skb_info *)&IEEE80211_SKB_CB(skb)->driver_data;
  tx_params->flags |= INTERNAL_MGMT_PKT;
  skb->priority = MGMT_SOFT_Q;
  if (skb->data[2] == PEER_NOTIFY)
    skb_queue_head(&common->tx_queue[MGMT_SOFT_Q], skb);
  else
    skb_queue_tail(&common->tx_queue[MGMT_SOFT_Q], skb);
  rsi_set_event(&common->tx_thread.event);
  return 0;
}

/**
 * rsi_load_radio_caps() - This function is used to send radio capabilities
 *			   values to firmware.
 * @common: Pointer to the driver private structure.
 *
 * Return: 0 on success, corresponding negative error code on failure.
 */
int rsi_load_radio_caps(struct rsi_common *common)
{
  struct rsi_radio_caps *radio_caps;
  struct rsi_hw *adapter = common->priv;
  u16 value;
  u8 ii, jj = 0;
  u8 radio_id = 0;
  struct sk_buff *skb;
//__9117_CODE_START
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
  mu_edca_params_t mu_edca_params[MAX_HW_QUEUES] = {
    /* cw_max, cw_min, edca_en, reserved, edca_timer, aifsn */
    { 1023, 15, EDCA_Q_EN, 0, 0, 7 }, /*Parameters for Q0*/
    { 1023, 15, EDCA_Q_EN, 0, 0, 3 }, /*Parameters for Q1*/
    { 15, 7, EDCA_Q_EN, 0, 0, 2 },    /*Parameters for Q2*/
    { 7, 3, EDCA_Q_EN, 0, 0, 2 },     /*Parameters for Q3*/
    { 1023, 15, EDCA_Q_EN, 0, 0, 7 }, /*Parameters for Q4*/
    { 1023, 15, EDCA_Q_EN, 0, 0, 3 }, /*Parameters for Q5*/
    { 15, 7, EDCA_Q_EN, 0, 0, 2 },    /*Parameters for Q6*/
    { 7, 3, EDCA_Q_EN, 0, 0, 2 },     /*Parameters for Q7*/
    { 7, 3, EDCA_Q_EN, 0, 0, 2 },     /*Parameters for Q8*/
    { 7, 3, EDCA_Q_EN, 0, 0, 2 },     /*Parameters for Q9*/
    { 7, 3, EDCA_Q_EN, 0, 0, 2 },     /*Parameters for Q10*/
    { 7, 3, EDCA_Q_EN, 0, 0, 2 }      /*Parameters for Q11*/
  };
#endif
  //__9117_CODE_END
  rsi_dbg(INT_MGMT_ZONE, "<===== SENDING RADIO_CAPABILITIES =====>\n");

  skb = dev_alloc_skb(sizeof(struct rsi_radio_caps));
  if (!skb)
    return -ENOMEM;

  memset(skb->data, 0, sizeof(struct rsi_radio_caps));
  radio_caps = (struct rsi_radio_caps *)skb->data;

  radio_caps->desc_word[1] = cpu_to_le16(RADIO_CAPABILITIES);
  radio_caps->desc_word[4] = cpu_to_le16(common->channel);
  radio_caps->desc_word[4] |= cpu_to_le16(RSI_RF_TYPE << 8);

  radio_caps->desc_word[7] |= cpu_to_le16(RSI_LMAC_CLOCK_80MHZ);
  if (common->channel_width == BW_40MHZ) {
    radio_caps->desc_word[7] |= cpu_to_le16(RSI_ENABLE_40MHZ);
    if (common->driver_mode != RF_EVAL_MODE_ON) {
      if (common->fsm_state == FSM_MAC_INIT_DONE) {
        struct ieee80211_hw *hw     = adapter->hw;
        struct ieee80211_conf *conf = &hw->conf;

        if (conf_is_ht40_plus(conf)) {
          radio_caps->desc_word[5] = cpu_to_le16(LOWER_20_ENABLE);
          radio_caps->desc_word[5] |= cpu_to_le16(LOWER_20_ENABLE >> 12);
        } else if (conf_is_ht40_minus(conf)) {
          radio_caps->desc_word[5] = cpu_to_le16(UPPER_20_ENABLE);
          radio_caps->desc_word[5] |= cpu_to_le16(UPPER_20_ENABLE >> 12);
        } else {
          radio_caps->desc_word[5] = cpu_to_le16(BW_40MHZ << 12);
          radio_caps->desc_word[5] |= cpu_to_le16(FULL40M_ENABLE);
        }
      }
    } else {
      if (adapter->per_params.per_ch_bw) {
        radio_caps->desc_word[5] = cpu_to_le16(adapter->per_params.per_ch_bw << 12);
        radio_caps->desc_word[5] |= cpu_to_le16(FULL40M_ENABLE);
      }
    }
  }

  if (adapter->device_model == RSI_DEV_9116) {
    if (common->channel_width == BW_20MHZ) {
      value                    = le16_to_cpu(radio_caps->desc_word[7]);
      value                    = (value & (~0x3));
      radio_caps->desc_word[7] = cpu_to_le16(value);
    }
  }

  radio_caps->sifs_tx_11n   = cpu_to_le16(SIFS_TX_11N_VALUE);
  radio_caps->sifs_tx_11b   = cpu_to_le16(SIFS_TX_11B_VALUE);
  radio_caps->slot_rx_11n   = cpu_to_le16(SHORT_SLOT_VALUE);
  radio_caps->ofdm_ack_tout = cpu_to_le16(OFDM_ACK_TOUT_VALUE);
  radio_caps->cck_ack_tout  = cpu_to_le16(CCK_ACK_TOUT_VALUE);
  radio_caps->preamble_type = cpu_to_le16(LONG_PREAMBLE);

  radio_caps->desc_word[7] |= cpu_to_le16(radio_id << 8);
  for (ii = 0; ii < (MAX_HW_QUEUES - 4); ii++) {
    if (ii > 5)
      jj = 1;
    radio_caps->qos_params[ii].cont_win_min_q = cpu_to_le16(common->edca_params[ii % 4].cw_min + jj);
    radio_caps->qos_params[ii].cont_win_max_q = cpu_to_le16(common->edca_params[ii % 4].cw_max + jj);
    radio_caps->qos_params[ii].aifsn_val_q    = cpu_to_le16(common->edca_params[ii % 4].aifs);
    radio_caps->qos_params[ii].txop_q         = cpu_to_le16(common->edca_params[ii % 4].txop << 5);
  }

  for (ii = 0; ii < MAX_HW_QUEUES; ii++) {
    if (!(radio_caps->qos_params[ii].cont_win_min_q)) {
      radio_caps->qos_params[ii].cont_win_min_q = cpu_to_le16(7);
      radio_caps->qos_params[ii].cont_win_max_q = cpu_to_le16(0x3f);
      radio_caps->qos_params[ii].aifsn_val_q    = cpu_to_le16(2);
      radio_caps->qos_params[ii].txop_q         = 0;
    }
  }
  radio_caps->qos_params[BROADCAST_HW_Q].txop_q = 0xffff;
  radio_caps->qos_params[MGMT_HW_Q].txop_q      = 0;
  radio_caps->qos_params[BEACON_HW_Q].txop_q    = 0xffff;
//__9117_CODE_START
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
  if (common->priv->device_model >= RSI_DEV_9117) {
    if (adapter->ax_params.mbssid_mask_value > 0) {
      radio_caps->ax_feature_valids = MBSSID_MASK_EN | SPR_EN | BSS_COLOR_EN | MU_EDCA_EN;
    } else
      radio_caps->ax_feature_valids = SPR_EN | BSS_COLOR_EN | MU_EDCA_EN;
    if (adapter->ax_params._11ax_enabled && !common->beacon_enabled) {
      for (ii = 0; ii < (MAX_HW_QUEUES - 4); ii++) {
        radio_caps->mu_edca_params[ii].en_mu_edca_q = EDCA_Q_EN;
        if (adapter->ax_params.edca_susp && (ii < 4))
          radio_caps->mu_edca_params[ii].en_mu_edca_q |= EDCA_Q_SUSP;
        radio_caps->mu_edca_params[ii].mu_edca_aifsn =
          cpu_to_le16(common->edca_params[ii % 4].mu_edca_param_rec.aifsn & 0xF);
        radio_caps->mu_edca_params[ii].cw_min_lsb =
          cpu_to_le16(((1 << (common->edca_params[ii % 4].mu_edca_param_rec.ecw_min_max & 0xF)) - 1) & 0xFF);
        radio_caps->mu_edca_params[ii].cw_min_msb =
          cpu_to_le16((((1 << (common->edca_params[ii % 4].mu_edca_param_rec.ecw_min_max & 0xF)) - 1) >> 8) & 0xFF);
        radio_caps->mu_edca_params[ii].cw_max =
          cpu_to_le16((1 << (common->edca_params[ii % 4].mu_edca_param_rec.ecw_min_max >> 4)) - 1);
        radio_caps->mu_edca_params[ii].mu_edca_timer =
          cpu_to_le16(common->edca_params[ii % 4].mu_edca_param_rec.mu_edca_timer);
      }

      for (ii = MGMT_SW_Q; ii < MAX_HW_QUEUES; ii++) {
        radio_caps->mu_edca_params[ii].en_mu_edca_q = EDCA_Q_EN;
        radio_caps->mu_edca_params[ii].mu_edca_aifsn =
          cpu_to_le16(common->edca_params[3].mu_edca_param_rec.aifsn & 0xF);
        radio_caps->mu_edca_params[ii].cw_min_lsb =
          cpu_to_le16(((1 << (common->edca_params[3].mu_edca_param_rec.ecw_min_max & 0xF)) - 1) & 0xFF);
        radio_caps->mu_edca_params[ii].cw_min_msb =
          cpu_to_le16((((1 << (common->edca_params[3].mu_edca_param_rec.ecw_min_max & 0xF)) - 1) >> 8) & 0xFF);
        radio_caps->mu_edca_params[ii].cw_max =
          cpu_to_le16((1 << (common->edca_params[3].mu_edca_param_rec.ecw_min_max >> 4)) - 1);
        radio_caps->mu_edca_params[ii].mu_edca_timer =
          cpu_to_le16(common->edca_params[3].mu_edca_param_rec.mu_edca_timer);
      }
      radio_caps->bss_color_spr_value_mbssid_mask = adapter->ax_params.ipps_valid << 14;
    } else {
      memcpy((u8 *)(radio_caps->mu_edca_params), (u8 *)mu_edca_params, sizeof(mu_edca_params_t) * MAX_HW_QUEUES);
    }
    radio_caps->bss_color_spr_value_mbssid_mask |= adapter->ax_params.bss_color;
    radio_caps->bss_color_spr_value_mbssid_mask |= SPR_VALUE << 6;
    radio_caps->bss_color_spr_value_mbssid_mask |= adapter->ax_params.mbssid_mask_value << 10;
    if (adapter->ax_params.spatial_reuse_en)
      radio_caps->ax_feature_valids |= SRG_VALID;
    radio_caps->spatial_reuse_enable = OBSS_PD_SR_EN;
    radio_caps->srg_obss_pd_value    = adapter->ax_params.obss_pd_min;
    radio_caps->srg_obss_pd_value |= adapter->ax_params.obss_pd_max << 8;
    /* Hardcoding values for bitmap because mac is not passing the values */
    radio_caps->srg_bss_color_bitmap[0]     = 0xffffffff;
    radio_caps->srg_bss_color_bitmap[1]     = 0xffffffff;
    radio_caps->srg_partial_bssid_bitmap[0] = 0;
    radio_caps->srg_partial_bssid_bitmap[1] = 0;
#ifdef CONFIG_UORA_SUPPORT
    if (common->enable_uora)
      radio_caps->ax_feature_valids |= UORA_EN;
    radio_caps->bss_color_spr_value_mbssid_mask |= common->enable_uora << 15;
    radio_caps->ocw_range_value = common->uora_ocw_min;
    radio_caps->ocw_range_value |= common->uora_ocw_max << 8;
#endif
    radio_caps->trigger_response = common->trigger_resp_ind;
    if (common->bsr_support)
      radio_caps->trigger_response |= BSR_EN;
  }
#endif
  //__9117_CODE_END
  radio_caps->desc_word[0] = cpu_to_le16((sizeof(struct rsi_radio_caps) - FRAME_DESC_SZ) | (RSI_WIFI_MGMT_Q << 12));

  skb_put(skb, (sizeof(struct rsi_radio_caps)));
  rsi_hex_dump(INT_MGMT_ZONE, "RADIO-CAP FRAME", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * rsi_mgmt_pkt_to_core() - This function is the entry point for Mgmt module.
 * @common: Pointer to the driver private structure.
 * @msg: Pointer to received packet.
 * @msg_len: Length of the received packet.
 * @type: Type of received packet.
 *
 * Return: 0 on success, -1 on failure.
 */
static int rsi_mgmt_pkt_to_core(struct rsi_common *common, u8 *msg, s32 msg_len)
{
  struct rsi_hw *adapter = common->priv;
  struct ieee80211_tx_info *info;
  struct skb_info *rx_params;
  u8 pad_bytes = msg[4];
  u8 pkt_recv;
  struct sk_buff *skb;
  char *buffer;
  struct ieee80211_hdr *wlh;
#ifdef CONFIG_RSI_11K
  u8 *data;
#endif
  if (!adapter->sc_nvifs)
    return -ENOLINK;

  if (common->iface_down)
    return -ENODEV;

  msg_len -= pad_bytes;
  if ((msg_len <= 0) || (!msg)) {
    rsi_dbg(MGMT_RX_ZONE, "%s: Invalid rx msg of len = %d\n", __func__, msg_len);
    return -EINVAL;
  }
#ifdef CONFIG_RSI_11K
  wlh = (struct ieee80211_hdr *)&msg[FRAME_DESC_SZ + pad_bytes];
  rsi_hex_dump(ERR_ZONE, "RX RRM packet", (u8 *)wlh, msg_len);

  if (ieee80211_is_action(wlh->frame_control)) {
    data = (u8 *)wlh + MIN_802_11_HDR_LEN;
    if (data[0] == WLAN_ACTION_RADIO_MEASUREMENT) {
      rsi_dbg(MGMT_RX_ZONE, "%s: Received Radio Measurement Request\n", __func__);
      if (!rsi_rrm_parse_radio_action_frame(common, (u8 *)wlh, msg_len))
        return 0;
    } else if (data[0] == WLAN_ACTION_SPECTRUM_MANAGEMENT) {
      rsi_dbg(MGMT_RX_ZONE, "%s: Received spectrum Measurement Request\n", __func__);
      rsi_rrm_parse_spectrum_action_frame(common, wlh, data);
      return 0;
    } else {
      rsi_dbg(MGMT_RX_ZONE, "%s: Received invalid category code\n", __func__);
      return 0;
    }
  }
#endif
  skb = dev_alloc_skb(msg_len);
  if (!skb)
    return -ENOMEM;

  buffer = skb_put(skb, msg_len);

  memcpy(buffer, (u8 *)(msg + FRAME_DESC_SZ + pad_bytes), msg_len);

  pkt_recv = buffer[0];

  info               = IEEE80211_SKB_CB(skb);
  rx_params          = (struct skb_info *)info->driver_data;
  rx_params->rssi    = rsi_get_rssi(msg);
  rx_params->channel = rsi_get_channel(msg);
  rsi_dbg(MGMT_RX_ZONE, "%s: rssi=%d channel=%d\n", __func__, rx_params->rssi, rx_params->channel);
  wlh = (struct ieee80211_hdr *)skb->data;
  rsi_dbg(MGMT_RX_ZONE, "RX Dot11 Mgmt Pkt Type: %s\n", dot11_pkt_type(wlh->frame_control));
  if (ieee80211_is_auth(wlh->frame_control)) {
    rsi_dbg(MGMT_DEBUG_ZONE, "<==== Recvd AUTHENTICATION Packet ====>\n");
    rsi_hex_dump(MGMT_DEBUG_ZONE, "AUTH-FRAME", skb->data, skb->len);
  } else if (ieee80211_is_assoc_resp(wlh->frame_control)) {
    rsi_dbg(MGMT_DEBUG_ZONE, "<==== Recvd ASSOCIATION RESPONSE Packet ====>\n");
    rsi_hex_dump(MGMT_DEBUG_ZONE, "ASSOC-RESP", skb->data, skb->len);
  } else if (ieee80211_is_deauth(wlh->frame_control)) {
    rsi_dbg(MGMT_DEBUG_ZONE, "<==== Recvd DE-AUTH Packet ====>\n");
    rsi_hex_dump(MGMT_DEBUG_ZONE, "DE-AUTH FRAME", skb->data, skb->len);
  }
  //__9117_CODE_START
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
  else if (ieee80211_is_beacon(wlh->frame_control)) {
    process_rx_mgmt_beacon(adapter, skb);
  }
#endif
#ifdef CONFIG_TWT_SUPPORT
  else if (ieee80211_is_action(wlh->frame_control)) {
    if (skb->data[MIN_802_11_HDR_LEN] == WLAN_CATEGORY_S1G) {
      twt_setup_frame_t *twt_setup_frame = (twt_setup_frame_t *)&skb->data[MIN_802_11_HDR_LEN];
      rsi_twt_config_t *user_twt_config  = &adapter->rsi_twt_config;
      if (twt_setup_frame->action_type == WLAN_S1G_TWT_SETUP) {
        rsi_dbg(MGMT_DEBUG_ZONE, "<==== TWT setup response received ====>\n");
        rsi_hex_dump(MGMT_DEBUG_ZONE, "Action Frame", skb->data, skb->len);
        if (adapter->twt_session_in_progress) {
          rsi_mgmt_process_twt_setup_resp(adapter, skb);
          if (adapter->twt_ps_disable == 1) {
            rsi_dbg(INFO_ZONE, "%s: Sending Enable PS after TWT Setup\n", __func__);
            rsi_enable_ps(adapter);
            adapter->twt_ps_disable = 0;
          }
        } else if ((adapter->ap_support_twt)
                   && (twt_setup_frame->twt_element.req_type_twt_setup_command == TWT_SETUP_CMD_ACCEPT)) {
          if ((adapter->twt_session_active == 0)) { //unsolicited TWT response processing
            if (!adapter->rsi_twt_config.twt_enable) {
              if (!validate_unsupported_twt_resp_params(adapter, twt_setup_frame)) {
                rsi_dbg(MGMT_DEBUG_ZONE, "Accepted unsolicated TWT response\n");
                send_twt_session_details_to_lmac(adapter,
                                                 1,
                                                 0,
                                                 twt_setup_frame,
                                                 user_twt_config); // For informing to LMAC
                adapter->twt_current_status = TWT_UNSOL_SESSION_SUCC;
                if (adapter->twt_session_active == 1) {
                  adapter->twt_status_to_app.twt_flow_id        = twt_setup_frame->twt_element.req_type_twt_flow_id;
                  adapter->twt_status_to_app.wake_duration      = twt_setup_frame->twt_element.nom_min_wake_duration;
                  adapter->twt_status_to_app.implicit_twt       = twt_setup_frame->twt_element.req_type_implicit_twt;
                  adapter->twt_status_to_app.triggered_twt      = twt_setup_frame->twt_element.req_type_trigger;
                  adapter->twt_status_to_app.req_type           = twt_setup_frame->twt_element.req_type_twt_req;
                  adapter->twt_status_to_app.wake_duration_unit = user_twt_config->twt_wake_duration_unit;
                  adapter->twt_status_to_app.un_announced_twt   = twt_setup_frame->twt_element.req_type_flow_type;
                  adapter->twt_status_to_app.wake_int_exp       = twt_setup_frame->twt_element.req_type_twt_wi_exp;
                  adapter->twt_status_to_app.wake_int_mantissa  = twt_setup_frame->twt_element.wake_interval_mantisa;
                }
              } else { // Send teardown to AP if unsupported TWT parameters are received
                send_twt_teardown_frame(adapter,
                                        twt_setup_frame->twt_element.req_type_twt_flow_id,
                                        twt_setup_frame->twt_element.control_negotiation_type);
                adapter->twt_current_status = TWT_SETUP_UNSUPPORTED_RSP;
              }
            } else { // If user configuration is available validate received unsolicited ACCEPT_TWT against user given params.
              rsi_mgmt_process_twt_setup_resp(adapter, skb);
            }
          } else {
            if (adapter->twt_active_session_flow_id != (twt_setup_frame->twt_element.req_type_twt_flow_id & 0x7)) {
              rsi_dbg(ERR_ZONE, "Unsolicated TWT accept rcvd when an active session present. Tear Down\n");
              send_twt_teardown_frame(adapter,
                                      twt_setup_frame->twt_element.req_type_twt_flow_id,
                                      twt_setup_frame->twt_element.control_negotiation_type);
            } else { //Update the existing TWT session with newly received unsolicited ACCEPT_TWT param corresponding to same flowid.
              rsi_mgmt_process_twt_setup_resp(adapter, skb);
            }
          }
        } else {
          rsi_dbg(ERR_ZONE, "Unexpected case\n");
        }
      } else if (twt_setup_frame->action_type == WLAN_S1G_TWT_TEARDOWN) {
        twt_teardown_frame_t *twt_teardown_frame = (twt_teardown_frame_t *)&skb->data[MIN_802_11_HDR_LEN];
        if (adapter->twt_session_active) {
          u8 teardown_flow_id;
          if (twt_teardown_frame->teardown_all) {
            teardown_flow_id = 0xFF;
            rsi_dbg(MGMT_DEBUG_ZONE, "\nTWT tear down all from AP\n");
            send_twt_session_details_to_lmac(adapter, 0, teardown_flow_id, twt_setup_frame, user_twt_config);
            adapter->twt_current_status = TWT_AP_TEARDOWN_SUCC;
          } else {
            teardown_flow_id = twt_teardown_frame->flow_id;
            if (adapter->twt_active_session_flow_id == teardown_flow_id) {
              send_twt_session_details_to_lmac(adapter, 0, teardown_flow_id, twt_setup_frame, user_twt_config);
              rsi_dbg(MGMT_DEBUG_ZONE, "\n TWT teardown from AP ,flow ID:%d\n", teardown_flow_id);
              adapter->twt_current_status = TWT_AP_TEARDOWN_SUCC;
            } else {
              rsi_dbg(MGMT_DEBUG_ZONE, "\n TWT teardown from AP ,flow ID not matched with active TWT ID\n");
            }
          }
        } else {
          rsi_dbg(MGMT_DEBUG_ZONE, "\nNo TWT session active\n");
        }
      }
      dev_kfree_skb(skb); /* Don't sent TWT Rx Pkt to MAC80211 */
      rsi_dbg(MGMT_DEBUG_ZONE, " Don't send TWT Rx pkt to MAC80211 \n ");
      return 0;
    }
  }
#endif
  //__9117_CODE_END
  rsi_indicate_pkt_to_os(common, skb);

  return 0;
}

/**
 * rsi_send_sta_notify_frame() - This function sends the station notify
 *				     frame to firmware.
 * @common: Pointer to the driver private structure.
 * @opmode: Operating mode of device.
 * @notify_event: Notification about station connection.
 * @bssid: bssid.
 * @qos_enable: Qos is enabled.
 * @aid: Aid (unique for all STA).
 *
 * Return: status: 0 on success, corresponding negative error code on failure.
 */
int rsi_send_sta_notify_frame(struct rsi_common *common,
                              enum opmode opmode,
                              u8 notify_event,
                              const unsigned char *bssid,
                              u8 qos_enable,
                              u16 aid,
#ifndef CONFIG_STA_PLUS_AP
                              u16 sta_id)
#else
                              u16 sta_id,
                              struct ieee80211_vif *vif)
#endif
{
  struct rsi_hw *adapter = common->priv;
#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = adapter->vifs[adapter->sc_nvifs - 1];
#endif
  struct sk_buff *skb = NULL;
  struct rsi_peer_notify *peer_notify;
  int status;
  u16 vap_id    = ((struct vif_priv *)vif->drv_priv)->vap_id;
  int frame_len = sizeof(*peer_notify);
  struct ieee80211_sta *sta;
  u8 mpdu_density_map[] = { 0, 1, 1, 1, 2, 4, 8, 16 };

  rsi_dbg(INT_MGMT_ZONE, "<==== Sending Peer Notify Packet ====>\n");

  skb = dev_alloc_skb(frame_len);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }
  memset(skb->data, 0, frame_len);

  peer_notify = (struct rsi_peer_notify *)skb->data;

#ifndef CONFIG_STA_PLUS_AP
  sta = common->stations[RSI_MAX_ASSOC_STAS].sta;
#else
  sta = common->stations[STA_PEER].sta;
#endif
  if (opmode == STA_OPMODE)
    peer_notify->command = cpu_to_le16(PEER_TYPE_AP << 1);
  else if (opmode == AP_OPMODE) {
    peer_notify->command = cpu_to_le16(PEER_TYPE_STA << 1);
    sta                  = common->stations[sta_id].sta;
  }

  switch (notify_event) {
    case STA_CONNECTED:
      peer_notify->command |= cpu_to_le16(RSI_ADD_PEER);
      common->peer_notify_state = true;
      if (sta != NULL) {
        peer_notify->mpdu_density = cpu_to_le16(mpdu_density_map[HT_CAP.ampdu_density]);
        peer_notify->sta_flags    = cpu_to_le32((HT_CAP.ampdu_factor << 2) | ((qos_enable) ? 1 : 0));
      }
//__9117_CODE_START
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
      if ((opmode == STA_OPMODE) && adapter->ax_params._11ax_enabled)
        peer_notify->desc_word[2] = cpu_to_le16(CONNECTION_TYPE << 8);

#endif
      //__9117_CODE_END
      break;
    case STA_DISCONNECTED:
      peer_notify->command |= cpu_to_le16(RSI_DELETE_PEER);
      common->peer_notify_state = false;
      break;
    default:
      break;
  }
  adapter->peer_notify = true;
  peer_notify->command |= cpu_to_le16((aid & 0xfff) << 4);
  ether_addr_copy(peer_notify->mac_addr, bssid);
  ether_addr_copy(common->sta_bssid, bssid);
  peer_notify->desc_word[0] = cpu_to_le16((frame_len - FRAME_DESC_SZ) | (RSI_WIFI_MGMT_Q << 12));
  peer_notify->desc_word[1] = cpu_to_le16(PEER_NOTIFY);
  //__9117_CODE_START
  peer_notify->desc_word[4] =
    cpu_to_le16(adapter->ax_params.connected_nt_profile_index | (adapter->ax_params.profile_periodicity << 8));
  peer_notify->desc_word[5] =
    cpu_to_le16(adapter->ax_params.mbssid_mask_value | (adapter->ax_params.trasmitter_bssid[5] << 8));
  //__9117_CODE_END
  peer_notify->desc_word[7] |= cpu_to_le16(sta_id | vap_id << 8);

  skb_put(skb, frame_len);
  if (notify_event == STA_CONNECTED) {
    rsi_hex_dump(INT_MGMT_ZONE, "ADD PEER FRAME", skb->data, skb->len);
  } else {
    rsi_hex_dump(INT_MGMT_ZONE, "DELETE PEER FRAME", skb->data, skb->len);
  }
  status = rsi_send_internal_mgmt_frame(common, skb);

  if ((vif->type == NL80211_IFTYPE_STATION) && (!status) && qos_enable) {
    rsi_set_contention_vals(common);
    mdelay(1);
    status = rsi_load_radio_caps(common);
  }

  return status;
}

/**
 * rsi_send_aggr_params_frame() - This function sends the ampdu
 *					 indication frame to firmware.
 * @common: Pointer to the driver private structure.
 * @tid: traffic identifier.
 * @ssn: ssn.
 * @buf_size: buffer size.
 * @event: notification about station connection.
 *
 * Return: 0 on success, corresponding negative error code on failure.
 */
int rsi_send_aggr_params_frame(struct rsi_common *common, u16 tid, u16 ssn, u8 buf_size, u8 event, u8 sta_id)
{
  struct sk_buff *skb = NULL;
  struct rsi_mac_frame *mgmt_frame;
  u8 peer_id = 0;
  //	u8 window_size = 1;

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, FRAME_DESC_SZ);
  mgmt_frame = (struct rsi_mac_frame *)skb->data;

  rsi_dbg(INT_MGMT_ZONE, "<===== SENDING AMPDU_IND FRAME =====>\n");

  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  mgmt_frame->desc_word[1] = cpu_to_le16(AMPDU_IND);

  if (event == STA_TX_ADDBA_DONE) {
    mgmt_frame->desc_word[4] = cpu_to_le16(ssn);
    //__9117_CODE_START
    if (common->priv->ax_params._11ax_enabled)
      mgmt_frame->desc_word[5] = cpu_to_le16(buf_size);
    else
      //__9117_CODE_END
      mgmt_frame->desc_word[5] = cpu_to_le16(TX_AGGR_LIMIT_FOR_11N);
    //mgmt_frame->desc_word[5] = cpu_to_le16(window_size);
    mgmt_frame->desc_word[7] = cpu_to_le16((tid | (START_AMPDU_AGGR << 4) | (peer_id << 8)));
  } else if (event == STA_RX_ADDBA_DONE) {
    mgmt_frame->desc_word[4] = cpu_to_le16(ssn);
    mgmt_frame->desc_word[7] = cpu_to_le16(tid | (START_AMPDU_AGGR << 4) | (RX_BA_INDICATION << 5) | (peer_id << 8));
  } else if (event == STA_TX_DELBA) {
    mgmt_frame->desc_word[7] = cpu_to_le16(tid | (STOP_AMPDU_AGGR << 4) | (peer_id << 8));
#ifndef CONFIG_STA_PLUS_AP
    common->stations[RSI_MAX_ASSOC_STAS].start_tx_aggr[tid] = false;
#else
    common->stations[STA_PEER].start_tx_aggr[tid] = false;
#endif
  } else if (event == STA_RX_DELBA) {
    mgmt_frame->desc_word[7] = cpu_to_le16(tid | (STOP_AMPDU_AGGR << 4) | (RX_BA_INDICATION << 5) | (peer_id << 8));
  }

  skb_put(skb, FRAME_DESC_SZ);
  rsi_hex_dump(INT_MGMT_ZONE, "AMPDU_IND FRAME", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * rsi_program_bb_rf() - This function starts base band and RF programming.
 *			 This is called after initial configurations are done.
 * @common: Pointer to the driver private structure.
 *
 * Return: 0 on success, corresponding negative error code on failure.
 */
int rsi_program_bb_rf(struct rsi_common *common)
{
  struct sk_buff *skb;
  struct rsi_mac_frame *mgmt_frame;

  rsi_dbg(INT_MGMT_ZONE, "<===== SENDING BBP_PROG_IN_TA FRAME =====>\n");

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, FRAME_DESC_SZ);
  mgmt_frame = (struct rsi_mac_frame *)skb->data;

  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  mgmt_frame->desc_word[1] = cpu_to_le16(BBP_PROG_IN_TA);
  mgmt_frame->desc_word[4] = cpu_to_le16(common->endpoint);
  mgmt_frame->desc_word[3] = cpu_to_le16(common->rf_pwr_mode);

  if (common->rf_reset) {
    mgmt_frame->desc_word[7] = cpu_to_le16(RF_RESET_ENABLE);
    rsi_dbg(MGMT_TX_ZONE, "%s: ===> RF RESET REQUEST SENT <===\n", __func__);
    common->rf_reset = 0;
  }
  common->bb_rf_prog_count = 1;
  mgmt_frame->desc_word[7] |= cpu_to_le16(PUT_BBP_RESET | BBP_REG_WRITE | (RSI_RF_TYPE << 4));

  skb_put(skb, FRAME_DESC_SZ);
  rsi_hex_dump(INT_MGMT_ZONE, "BBP_PROG_IN_TA", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * rsi_set_vap_capabilities() - This function send vap capability to firmware.
 * @common: Pointer to the driver private structure.
 * @opmode: Operating mode of device.
 *
 * Return: 0 on success, corresponding negative error code on failure.
 */
int rsi_set_vap_capabilities(struct rsi_common *common, enum opmode mode, u8 *mac_addr, u8 vap_id, u8 vap_status)
{
  struct sk_buff *skb = NULL;
  struct rsi_vap_caps *vap_caps;
  struct rsi_hw *adapter      = common->priv;
  struct ieee80211_hw *hw     = adapter->hw;
  struct ieee80211_conf *conf = &hw->conf;

  rsi_dbg(INT_MGMT_ZONE, "<===== SENDING VAP_CAPABILITIES FRAME =====>\n");

  rsi_dbg(INFO_ZONE, "Config VAP: id=%d mode=%d status=%d ", vap_id, mode, vap_status);
  rsi_hex_dump(INFO_ZONE, "mac", mac_addr, 6);
  skb = dev_alloc_skb(sizeof(struct rsi_vap_caps));
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, sizeof(struct rsi_vap_caps));
  vap_caps = (struct rsi_vap_caps *)skb->data;

  vap_caps->desc_word[0] = cpu_to_le16((sizeof(struct rsi_vap_caps) - FRAME_DESC_SZ) | (RSI_WIFI_MGMT_Q << 12));
  vap_caps->desc_word[1] = cpu_to_le16(VAP_CAPABILITIES);
  vap_caps->desc_word[2] = cpu_to_le16(vap_status << 8);
  vap_caps->desc_word[4] = cpu_to_le16(mode | (common->channel_width << 8));
  vap_caps->desc_word[7] = cpu_to_le16((vap_id << 8) | (common->mac_id << 4) | common->radio_id);

  memcpy(vap_caps->mac_addr, mac_addr, IEEE80211_ADDR_LEN);
  vap_caps->keep_alive_period = cpu_to_le16(90);
  vap_caps->frag_threshold    = cpu_to_le16(IEEE80211_MAX_FRAG_THRESHOLD);

  vap_caps->rts_threshold = cpu_to_le16(common->rts_threshold);
  //__9117_CODE_START
  vap_caps->txop_dur_rts_threshold = cpu_to_le16(common->txop_dur_rts_threshold);
  //__9117_CODE_END
  vap_caps->default_mgmt_rate = cpu_to_le32(RSI_RATE_6);

  if (common->band == NL80211_BAND_5GHZ) {
    vap_caps->default_ctrl_rate = cpu_to_le32(RSI_RATE_6);
  } else {
    if (common->p2p_enabled) {
      vap_caps->default_ctrl_rate = cpu_to_le32(RSI_RATE_6);
    } else {
      vap_caps->default_ctrl_rate = cpu_to_le32(RSI_RATE_1);
    }
  }

  if (conf_is_ht40(conf)) {
    if (conf_is_ht40_minus(conf))
      vap_caps->default_ctrl_rate |= cpu_to_le32(UPPER_20_ENABLE << 16);
    else if (conf_is_ht40_plus(conf))
      vap_caps->default_ctrl_rate |= cpu_to_le32(LOWER_20_ENABLE << 16);
    else
      vap_caps->default_ctrl_rate |= cpu_to_le32(FULL40M_ENABLE << 16);
  }

  vap_caps->default_data_rate     = 0;
  vap_caps->beacon_interval       = cpu_to_le16(common->beacon_interval);
  vap_caps->dtim_period           = cpu_to_le16(common->dtim_cnt);
  vap_caps->beacon_miss_threshold = cpu_to_le16(HW_BMISS_THRESHOLD);

  skb_put(skb, sizeof(*vap_caps));
  rsi_hex_dump(INT_MGMT_ZONE, "VAP-CAP FRAME", skb->data, skb->len);

  common->last_vap_type = mode;
  ether_addr_copy(common->last_vap_addr, mac_addr);
  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * rsi_load_key() - This function is used to load keys within the firmware.
 * @common: Pointer to the driver private structure.
 * @data: Pointer to the key data.
 * @key_len: Key length to be loaded.
 * @key_type: Type of key: GROUP/PAIRWISE.
 * @key_id: Key index.
 * @cipher: Type of cipher used.
 *
 * Return: 0 on success, -1 on failure.
 */
#ifndef CONFIG_STA_PLUS_AP
int rsi_load_key(struct rsi_common *common, u8 *data, u16 key_len, u8 key_type, u8 key_id, u32 cipher, s16 sta_id)
{
  struct ieee80211_vif *vif = common->priv->vifs[common->priv->sc_nvifs - 1];
#else
int rsi_load_key(struct rsi_common *common,
                 u8 *data,
                 u16 key_len,
                 u8 key_type,
                 u8 key_id,
                 u32 cipher,
                 s16 sta_id,
                 struct ieee80211_vif *vif)
{
#endif
  struct sk_buff *skb = NULL;
  struct rsi_set_key *set_key;
  u16 key_descriptor = 0;
  u8 key_t1          = 0;
  u8 vap_id          = ((struct vif_priv *)vif->drv_priv)->vap_id;

  skb = dev_alloc_skb(sizeof(struct rsi_set_key));
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, sizeof(struct rsi_set_key));
  set_key = (struct rsi_set_key *)skb->data;

  switch (key_type) {
    case RSI_GROUP_KEY:
      key_t1 = 1 << 1;
      if ((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO))
        key_descriptor = BIT(7);
      break;
    case RSI_PAIRWISE_KEY:
      if (((vif->type == NL80211_IFTYPE_AP) || (vif->type == NL80211_IFTYPE_P2P_GO))
          && (sta_id >= common->max_stations)) {
        rsi_dbg(INFO_ZONE, "Invalid Sta_id %d\n", sta_id);
        return -1;
      }
      key_t1 = 0 << 1;
      if ((cipher != WLAN_CIPHER_SUITE_WEP40) && (cipher != WLAN_CIPHER_SUITE_WEP104))
        key_id = 0;
      break;
  }
  if ((cipher == WLAN_CIPHER_SUITE_WEP40) || (cipher == WLAN_CIPHER_SUITE_WEP104)) {
    key_descriptor |= BIT(2);
    if (key_len >= 13) {
      key_descriptor |= BIT(3);
    }
  } else if (cipher != KEY_TYPE_CLEAR) {
    key_descriptor |= BIT(4);
    if (cipher == WLAN_CIPHER_SUITE_TKIP)
      key_descriptor |= BIT(5);
  }
  key_descriptor |= (key_t1 | BIT(13) | (key_id << 14));

  set_key->desc_word[0] = cpu_to_le16((sizeof(struct rsi_set_key) - FRAME_DESC_SZ) | (RSI_WIFI_MGMT_Q << 12));
  set_key->desc_word[1] = cpu_to_le16(SET_KEY_REQ);
  set_key->desc_word[4] = cpu_to_le16(key_descriptor);
  set_key->desc_word[7] = cpu_to_le16(sta_id | (vap_id << 8));

  if (data) {
    if ((cipher == WLAN_CIPHER_SUITE_WEP40) || (cipher == WLAN_CIPHER_SUITE_WEP104)) {
      memcpy(&set_key->key[key_id][1], data, key_len * 2);
    } else {
      memcpy(&set_key->key[0][0], data, key_len);
    }
    memcpy(set_key->tx_mic_key, &data[16], 8);
    memcpy(set_key->rx_mic_key, &data[24], 8);
  } else {
    memset(&set_key[FRAME_DESC_SZ], 0, sizeof(struct rsi_set_key) - FRAME_DESC_SZ);
  }

  skb_put(skb, sizeof(struct rsi_set_key));
  rsi_hex_dump(INT_MGMT_ZONE, "KEY FRAME", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * rsi_send_common_dev_params() - This function send the common device
 *				configuration parameters to device.
 * @common: Pointer to the driver private structure.
 *
 * Return: 0 on success, -1 on failure.
 */
int rsi_send_common_dev_params(struct rsi_common *common)
{
  struct sk_buff *skb              = NULL;
  u32 frame_len                    = 0;
  struct rsi_config_vals *dev_cfgs = NULL;
  struct rsi_hw *adapter           = common->priv;

  frame_len = sizeof(struct rsi_config_vals);

  rsi_dbg(INT_MGMT_ZONE, "<==== Sending common device config params =====>\n");
  skb = dev_alloc_skb(frame_len);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Unable to allocate skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, frame_len);

  dev_cfgs = (struct rsi_config_vals *)&skb->data[0];
  memset(dev_cfgs, 0, (sizeof(struct rsi_config_vals)));

  dev_cfgs->desc_word[0] = cpu_to_le16((frame_len - FRAME_DESC_SZ) | (RSI_COEX_Q << 12));
  dev_cfgs->desc_word[1] = cpu_to_le16(COMMON_DEV_CONFIG);

  dev_cfgs->lp_ps_handshake  = common->lp_ps_handshake_mode;
  dev_cfgs->ulp_ps_handshake = common->ulp_ps_handshake_mode;

  dev_cfgs->unused_ulp_gpio        = *(u8 *)&unused_ulp_gpio_bitmap;
  dev_cfgs->unused_soc_gpio_bitmap = cpu_to_le32(*(u32 *)&unused_soc_gpio_bitmap);

  /*
	 * For zigb functionality, send DEV_OPMODE_ZB_ALONE
	 * as opermode to firmware, irrespective of router or
	 * coordinator
	 */
  if (common->oper_mode == DEV_OPMODE_ZB_COORDINATOR || common->oper_mode == DEV_OPMODE_ZB_ROUTER)
    dev_cfgs->opermode = DEV_OPMODE_ZB_ALONE;
  else
    dev_cfgs->opermode = common->oper_mode;

  dev_cfgs->wlan_rf_pwr_mode      = common->wlan_rf_power_mode;
  dev_cfgs->driver_mode           = common->driver_mode;
  dev_cfgs->region_code           = adapter->dfs_region;
  dev_cfgs->antenna_sel_val       = common->obm_ant_sel_val;
  dev_cfgs->dev_peer_dist         = common->peer_dist;
  dev_cfgs->dev_bt_feature_bitmap = common->bt_feature_bitmap;
  dev_cfgs->uart_dbg              = common->uart_debug;
  dev_cfgs->xtal_good_time        = common->xtal_good_time;
  if (COMMON_DEV_MODEL_9116) {
    /*
		 * In 9116_feature_bitmap,
		 * BITS(3:0)  are used for module type selection,
		 * BIT(4)     is used for host interface on demand feature option,
		 * BIT(5)     is used for selecting sleep clock source,
		 * BIT(8)     is used to Disable updated rate params,
		 * BIT(9)     is used to select 1.8V operation,
		 * BIT(11)    is used for Airplane mode,
		 * BIT(12)    is used for AARF_RATE_BLOCK_EN,
		 * BIT(14:13) are used for PTA config.
		 */
    dev_cfgs->features_9116 = (common->ext_opt & 0xF) | (common->host_intf_on_demand << 4)
                              | ((common->crystal_as_sleep_clk & 0x3) << 5) | (common->sleep_ind_gpio_sel << 7)
                              | (common->feature_bitmap_9116 << 8) | (common->pta_config << 13);
    dev_cfgs->dev_ble_roles = common->ble_roles;
    /* In bt_bdr, bt_bdr_mode used a byte[0:7],
		 * three_wire_coex use one bit [8]. Module param of 2 byte
		 * size.
		 */
    dev_cfgs->bt_bdr               = ((common->three_wire_coex << 8) | common->bt_bdr_mode);
    dev_cfgs->dev_anchor_point_gap = common->anchor_point_gap;
  }
  if (common->rx_data_inactive_interval)
    dev_cfgs->xtal_good_time |= EN_ENHANCED_MAX_PSP;

  skb_put(skb, frame_len);
  rsi_hex_dump(INT_MGMT_ZONE, "COMMON-DEV PARAM", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/*
 * rsi_load_bootup_params() - This function send bootup params to the firmware.
 * @common: Pointer to the driver private structure.
 *
 * Return: 0 on success, corresponding error code on failure.
 */
int rsi_load_bootup_params(struct rsi_common *common)
{
  struct sk_buff *skb;
  struct rsi_boot_params *boot_params           = NULL;
  struct rsi_boot_params_9116 *boot_params_9116 = NULL;
  struct rsi_hw *adapter                        = common->priv;

  rsi_dbg(INT_MGMT_ZONE, "<===== SENDING BOOTUP_PARAMS_REQUEST FRAME =====>\n");

  if (DEV_MODEL_9116)
    skb = dev_alloc_skb(sizeof(struct rsi_boot_params_9116));
  else
    skb = dev_alloc_skb(sizeof(struct rsi_boot_params));

  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }
  if (DEV_MODEL_9116) {
    memset(skb->data, 0, sizeof(struct rsi_boot_params_9116));
    boot_params_9116 = (struct rsi_boot_params_9116 *)skb->data;
  } else {
    memset(skb->data, 0, sizeof(struct rsi_boot_params));
    boot_params = (struct rsi_boot_params *)skb->data;
  }

  if (common->channel_width == BW_40MHZ) {
    if (DEV_MODEL_9116) {
      memcpy(&boot_params_9116->bootup_params, &boot_params_9116_40, sizeof(struct bootup_params_9116));
      rsi_dbg(INT_MGMT_ZONE, "%s: Packet 40MHZ UMAC_CLK =%d\n", __func__, UMAC_CLK_40BW);
      boot_params_9116->desc_word[7] = cpu_to_le16(UMAC_CLK_40BW);
    } else {
      memcpy(&boot_params->bootup_params, &boot_params_40, sizeof(struct bootup_params));
      rsi_dbg(INT_MGMT_ZONE, "%s: Packet 40MHZ UMAC_CLK = %d\n", __func__, UMAC_CLK_40BW);
      boot_params->desc_word[7] = cpu_to_le16(UMAC_CLK_40BW);
    }
  } else {
    //__9117_CODE_START
    if (common->priv->device_model >= RSI_DEV_9117) {
      boot_params_9116_20.device_clk_info_9116[0].switch_clk_9116_g.wlan_bbp_lmac_clk_reg_val = cpu_to_le32(0x01041001);
      memcpy(&boot_params_9116->bootup_params, &boot_params_9116_20, sizeof(struct bootup_params_9116));
    } else
      //__9117_CODE_END
      if (common->priv->device_model == RSI_DEV_9116)
      memcpy(&boot_params_9116->bootup_params, &boot_params_9116_20, sizeof(struct bootup_params_9116));
    else
      memcpy(&boot_params->bootup_params, &boot_params_20, sizeof(struct bootup_params));

    if (DEV_MODEL_9116) {
      if (boot_params_9116_20.valid != cpu_to_le32(VALID_20)) {
        boot_params_9116->desc_word[7] = cpu_to_le16(UMAC_CLK_20BW);
        rsi_dbg(INT_MGMT_ZONE, "Packet 20MHZ WITH UMAC_CLK = %d\n", UMAC_CLK_20BW);
      } else {
        boot_params_9116->desc_word[7] = cpu_to_le16(UMAC_CLK_40MHZ);
        rsi_dbg(INT_MGMT_ZONE, "Packet 20MHZ WITH UMAC_CLK = %d\n", UMAC_CLK_40MHZ);
      }
    } else {
      if (boot_params_20.valid != cpu_to_le32(VALID_20)) {
        boot_params->desc_word[7] = cpu_to_le16(UMAC_CLK_20BW);
        rsi_dbg(INT_MGMT_ZONE, "Packet 20MHZ WITH UMAC_CLK = %d\n", UMAC_CLK_20BW);
      } else {
        boot_params->desc_word[7] = cpu_to_le16(UMAC_CLK_40MHZ);
        rsi_dbg(INT_MGMT_ZONE, "Packet 20MHZ WITH UMAC_CLK = %d\n", UMAC_CLK_40MHZ);
      }
    }
  }

  /**
	 * Bit{0:11} indicates length of the Packet
	 * Bit{12:15} indicates host queue number
	 */
  if (DEV_MODEL_9116) {
    boot_params_9116->desc_word[0] = cpu_to_le16(sizeof(struct bootup_params_9116) | (RSI_WIFI_MGMT_Q << 12));
    boot_params_9116->desc_word[1] = cpu_to_le16(BOOTUP_PARAMS_REQUEST);

    skb_put(skb, sizeof(struct rsi_boot_params_9116));

    rsi_dbg(MGMT_TX_ZONE, "%s: Boot params- %p\n", __func__, boot_params_9116);
  } else {
    boot_params->desc_word[0] = cpu_to_le16(sizeof(struct bootup_params) | (RSI_WIFI_MGMT_Q << 12));
    boot_params->desc_word[1] = cpu_to_le16(BOOTUP_PARAMS_REQUEST);
    skb_put(skb, sizeof(struct rsi_boot_params));
    rsi_dbg(MGMT_TX_ZONE, "%s: Boot params- %p\n", __func__, boot_params);
  }
  rsi_hex_dump(INT_MGMT_ZONE, "BOOT-UP PARAM", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * rsi_send_reset_mac() - This function prepares reset MAC request and sends an
 *			  internal management frame to indicate it to firmware.
 * @common: Pointer to the driver private structure.
 *
 * Return: 0 on success, corresponding error code on failure.
 */
static int rsi_send_reset_mac(struct rsi_common *common)
{
  struct sk_buff *skb;
  struct rsi_mac_frame *mgmt_frame;

  rsi_dbg(INT_MGMT_ZONE, "<===== SENDING RESET_MAC_REQUEST FRAME =====>\n");

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, FRAME_DESC_SZ);
  mgmt_frame = (struct rsi_mac_frame *)skb->data;

  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  mgmt_frame->desc_word[1] = cpu_to_le16(RESET_MAC_REQ);
  mgmt_frame->desc_word[4] |= cpu_to_le16(RETRY_COUNT << 8);

  /*TA level aggregation of pkts to host */
  if (common->driver_mode == SNIFFER_MODE) {
    mgmt_frame->desc_word[3] = cpu_to_le16(SNIFFER_ENABLE);
    mgmt_frame->desc_word[3] |= cpu_to_le16(DSBL_TA_AGGR << 8);
  } else {
    mgmt_frame->desc_word[3] |= common->ta_aggr << 8;
  }
  if (common->driver_mode == RF_EVAL_MODE_ON) {
    mgmt_frame->desc_word[5] = PER_MODE_EN;
  }

  if (common->antenna_diversity)
    mgmt_frame->desc_word[6] = common->antenna_diversity;

  //__9117_CODE_START
  if (common->enable_ng_cb) {
    mgmt_frame->desc_word[7] = cpu_to_le16(common->value_of_ng_cb);
    mgmt_frame->desc_word[7] |= cpu_to_le16(common->enable_ng_cb << 8);
  }
  //__9117_CODE_END

  skb_put(skb, FRAME_DESC_SZ);
  rsi_hex_dump(INT_MGMT_ZONE, "RESET_MAC FRAME", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * rsi_band_check() - This function programs the band
 * @common: Pointer to the driver private structure.
 *
 * Return: 0 on success, corresponding error code on failure.
 */
int rsi_band_check(struct rsi_common *common, enum nl80211_band band)
{
  struct rsi_hw *adapter  = common->priv;
  struct ieee80211_hw *hw = adapter->hw;
  u8 prev_bw              = common->channel_width;
  u8 prev_ep              = common->endpoint;
  int status              = 0;
  bool skip               = 0;

  if (common->band != band) {
    common->rf_reset = 1;
    common->band     = band;
  }
  if (common->driver_mode == RF_EVAL_MODE_ON) {
    switch (adapter->per_params.per_ch_bw) {
      case 0:
        common->channel_width = BW_20MHZ;
        break;
      case 1:
      case 2:
      case 4:
      case 6:
        common->channel_width = BW_40MHZ;
        break;
      case 5:
        common->channel_width = BW_5MHZ;
        break;
      case 7:
        common->channel_width = BW_10MHZ;
        break;
      default:
        rsi_dbg(ERR_ZONE, "%s line %d Invalid BW settings\n", __func__, __LINE__);
        return -1;
    }
    if (DEV_MODEL_9116) {
      rsi_dbg(INFO_ZONE, "%s: skipping this for 9116 \n", __func__);
      skip = 1;
    }
  } else {
    if ((hw->conf.chandef.width == NL80211_CHAN_WIDTH_20_NOHT) || (hw->conf.chandef.width == NL80211_CHAN_WIDTH_20))
      common->channel_width = BW_20MHZ;
    else
      common->channel_width = BW_40MHZ;
  }
  if (common->band == NL80211_BAND_2GHZ) {
    if (common->channel_width)
      common->endpoint = EP_2GHZ_40MHZ;
    else
      common->endpoint = EP_2GHZ_20MHZ;
  } else {
    if (common->channel_width)
      common->endpoint = EP_5GHZ_40MHZ;
    else
      common->endpoint = EP_5GHZ_20MHZ;
  }
  if (skip)
    return 0;

  if (common->endpoint != prev_ep) {
    status = rsi_program_bb_rf(common);
    if (status)
      return status;
  }

  if (common->channel_width != prev_bw) {
    status = rsi_load_bootup_params(common);
    if (status)
      return status;

    status = rsi_load_radio_caps(common);
    if (status)
      return status;
  }

  return status;
}

#ifdef CONFIG_CARACALLA_BOARD
void rsi_apply_carcalla_power_values(struct rsi_hw *adapter,
                                     struct ieee80211_vif *vif,
                                     struct ieee80211_channel *channel)
{
  u16 max_power = 20;

#ifdef CONFIG_STA_PLUS_AP
  if (!vif)
    return -ENODEV;
#endif
  if (!conf_is_ht(&adapter->hw->conf)) {
    if (vif->bss_conf.basic_rates == 0xf) {
      if (channel->hw_value == 12)
        max_power = 15;
      else if (channel->hw_value == 13)
        max_power = 7;
      else
        return;
    } else {
      if (channel->hw_value == 12)
        max_power = 8;
      else if (channel->hw_value == 13)
        max_power = 7;
      else
        return;
    }
  } else if (conf_is_ht20(&adapter->hw->conf)) {
    if (channel->hw_value == 12)
      max_power = 7;
    else if (channel->hw_value == 13)
      max_power = 5;
    else
      return;
  } else {
    if (channel->hw_value == 6)
      max_power = 9;
    else if (channel->hw_value == 9)
      max_power = 5;
    else if (channel->hw_value == 10)
      max_power = 4;
    else
      return;
  }
  channel->max_power        = max_power;
  channel->max_antenna_gain = 0;
}
#endif

/**
 * rsi_set_channel() - This function programs the channel.
 * @common: Pointer to the driver private structure.
 * @channel: Channel value to be set.
 *
 * Return: 0 on success, corresponding error code on failure.
 */
#ifndef CONFIG_STA_PLUS_AP
int rsi_set_channel(struct rsi_common *common, struct ieee80211_channel *channel)
#else
int rsi_set_channel(struct rsi_common *common, struct ieee80211_channel *channel, struct ieee80211_vif *vif)
#endif
{
  struct rsi_hw *adapter = common->priv;
#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = adapter->vifs[adapter->sc_nvifs - 1];
#endif
  struct sk_buff *skb = NULL;
  struct rsi_mac_frame *mgmt_frame;

  rsi_dbg(INT_MGMT_ZONE, "<===== SENDING SCAN_REQUEST FRAME =====>\n");

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  if (!channel) {
    dev_kfree_skb(skb);
    return 0;
  }

  if ((common->driver_mode == E2E_MODE) && (common->acx_module == true) && (vif->type == NL80211_IFTYPE_AP)) {
    if (channel->hw_value > 11) {
      rsi_dbg(ERR_ZONE, "%s : Channel = %d not Supported for ACx Modules\n ", __func__, channel->hw_value);
      return -EOPNOTSUPP;
    }
  }

  memset(skb->data, 0, FRAME_DESC_SZ);
  mgmt_frame = (struct rsi_mac_frame *)skb->data;

  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  mgmt_frame->desc_word[1] = cpu_to_le16(SCAN_REQUEST);
  mgmt_frame->desc_word[4] = cpu_to_le16(channel->hw_value);

#if 0
	channel->max_antenna_gain = common->antenna_gain;
	mgmt_frame->desc_word[4] |=
		cpu_to_le16(((char)(channel->max_antenna_gain)) << 8);
	mgmt_frame->desc_word[5] =
		cpu_to_le16((char)(channel->max_antenna_gain));
#endif
  if ((vif && vif->type == NL80211_IFTYPE_AP && adapter->auto_chan_sel) && (adapter->idx < adapter->n_channels)) {
    mgmt_frame->desc_word[2] = cpu_to_le16(TIMER_ENABLE);
    mgmt_frame->desc_word[3] = cpu_to_le16(ACS_TIMEOUT_TYPE);
    mgmt_frame->desc_word[3] |= cpu_to_le16(ACS_TIMEOUT_TIME << 8);
    adapter->rsi_survey[adapter->idx].channel = channel;
  }
  mgmt_frame->desc_word[4] |= cpu_to_le16(common->antenna_gain[0] << 8);
  mgmt_frame->desc_word[5] = cpu_to_le16(common->antenna_gain[1]);

  mgmt_frame->desc_word[7] = cpu_to_le16(PUT_BBP_RESET | BBP_REG_WRITE | (RSI_RF_TYPE << 4));

  if (common->efuse_map.module_type != ACX_MODULE
      && ((channel->flags & IEEE80211_CHAN_NO_IR) || (channel->flags & IEEE80211_CHAN_RADAR))) {
    mgmt_frame->desc_word[4] |= BIT(15);
  } else {
    if (common->tx_power < channel->max_power)
      mgmt_frame->desc_word[6] = cpu_to_le16(common->tx_power);
    else
      mgmt_frame->desc_word[6] = cpu_to_le16(channel->max_power);
  }
  if (common->driver_mode == RF_EVAL_MODE_ON)
    mgmt_frame->desc_word[6] = cpu_to_le16(adapter->per_params.power);
  if (common->driver_mode == SNIFFER_MODE)
    mgmt_frame->desc_word[6] = cpu_to_le16(TX_PWR_IN_SNIFFER_MODE);
#ifdef CONFIG_CARACALLA_BOARD
  rsi_apply_carcalla_power_values(adapter, vif, channel);
  mgmt_frame->desc_word[6] = cpu_to_le16(channel->max_power);

  if ((channel->hw_value == 12) || (channel->hw_value == 13))
    mgmt_frame->desc_word[7] = cpu_to_le16(TARGET_BOARD_CARACALLA);
  if (conf_is_ht40(&adapter->hw->conf)) {
    if ((channel->hw_value == 6) || (channel->hw_value == 9) || (channel->hw_value == 10))
      mgmt_frame->desc_word[7] = cpu_to_le16(TARGET_BOARD_CARACALLA);
  }
  mgmt_frame->desc_word[7] |= cpu_to_le16(adapter->dfs_region);
#else
  mgmt_frame->desc_word[7] = cpu_to_le16(adapter->dfs_region);
#endif
  rsi_dbg(INFO_ZONE, "reg_domain = %d, TX power = %d\n", adapter->dfs_region, mgmt_frame->desc_word[6]);

  if (common->channel_width == BW_40MHZ)
    mgmt_frame->desc_word[5] |= cpu_to_le16(0x1 << 8);

  common->channel = channel->hw_value;

  skb_put(skb, FRAME_DESC_SZ);
  rsi_hex_dump(INT_MGMT_ZONE, "SCAN-REQ FRAME", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * rsi_send_radio_params_update() - This function sends the radio
 *				parameters update to device
 * @common: Pointer to the driver private structure.
 * @channel: Channel value to be set.
 *
 * Return: 0 on success, corresponding error code on failure.
 */
int rsi_send_radio_params_update(struct rsi_common *common)
{
  struct rsi_mac_frame *mgmt_frame;
  struct sk_buff *skb = NULL;

  rsi_dbg(INT_MGMT_ZONE, "<===== Sending Radio Params update frame ====>\n");

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, FRAME_DESC_SZ);
  mgmt_frame = (struct rsi_mac_frame *)skb->data;

  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  mgmt_frame->desc_word[1] = cpu_to_le16(RADIO_PARAMS_UPDATE);
  mgmt_frame->desc_word[3] = cpu_to_le16(BIT(0));

  mgmt_frame->desc_word[3] |= cpu_to_le16(common->tx_power << 8);

  skb_put(skb, FRAME_DESC_SZ);
  rsi_hex_dump(INT_MGMT_ZONE, "RADIO-CAP UPDATE", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * rsi_send_vap_dynamic_update() - This function programs the threshold.
 * @common: Pointer to the driver private structure.
 *
 * Return: 0 on success, corresponding error code on failure.
 */
int rsi_send_vap_dynamic_update(struct rsi_common *common)
{
  struct sk_buff *skb                 = NULL;
  struct rsi_dynamic_s *dynamic_frame = NULL;

  rsi_dbg(INT_MGMT_ZONE, "<==== Sending vap update indication frame ====>\n");

  skb = dev_alloc_skb(sizeof(struct rsi_dynamic_s));
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, sizeof(struct rsi_dynamic_s));
  dynamic_frame = (struct rsi_dynamic_s *)skb->data;

  dynamic_frame->desc_word[0] = cpu_to_le16((sizeof(dynamic_frame->frame_body)) | (RSI_WIFI_MGMT_Q << 12));
  dynamic_frame->desc_word[1] = cpu_to_le16(VAP_DYNAMIC_UPDATE);
  dynamic_frame->desc_word[4] = cpu_to_le16(common->rts_threshold);
#if 0
	dynamic_frame->desc_word[5] = cpu_to_le16(common->frag_threshold);
	dynamic_frame->desc_word[5] = cpu_to_le16(2352);
#endif

#ifdef CONFIG_RSI_WOW
  //	if (common->suspend_flag) {
  if (1) {
    dynamic_frame->desc_word[6]                 = cpu_to_le16(24); /* bmiss_threshold */
    dynamic_frame->frame_body.keep_alive_period = cpu_to_le16(5);
  } else
    dynamic_frame->frame_body.keep_alive_period = cpu_to_le16(90);
#else
  dynamic_frame->frame_body.keep_alive_period = cpu_to_le16(common->keep_alive_period);
  dynamic_frame->desc_word[6] = cpu_to_le16(common->hw_bmiss_threshold); /* bmiss_threshold */
#endif

  dynamic_frame->desc_word[3] = cpu_to_le32(common->use_protection ? BIT(1) : 0); /* Self cts enable */
#if 0
	dynamic_frame->frame_body.mgmt_rate = cpu_to_le32(RSI_RATE_6);

	dynamic_frame->desc_word[2] |= cpu_to_le32(BIT(1));/* Self cts enable */
#endif

  if (common->fixed_rate_en) {
    dynamic_frame->desc_word[3] |= cpu_to_le16(BIT(0)); /* fixed rate */
    dynamic_frame->frame_body.data_rate = cpu_to_le16(common->fixed_rate);
  }

  //__9117_CODE_START
  dynamic_frame->desc_word[5] = cpu_to_le16(common->txop_dur_rts_threshold);
  //__9117_CODE_END

  dynamic_frame->desc_word[7] |= cpu_to_le16((0 << 8)); /* vap id */

  skb_put(skb, sizeof(struct rsi_dynamic_s));
  rsi_hex_dump(INT_MGMT_ZONE, "VAP UPDATE", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * rsi_flash_read() - This function sends the frash read frame to device
 * @adapter: Pointer to the hardware structure.
 *
 * Return: status: 0 on success, -1 on failure.
 */
int rsi_flash_read(struct rsi_hw *adapter)
{
  struct rsi_common *common       = adapter->priv;
  struct rsi_mac_frame *cmd_frame = NULL;
  struct sk_buff *skb;

  rsi_dbg(INT_MGMT_ZONE, "<==== Sending flash read frame ====>\n");

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb)
    return -ENOMEM;

  memset(skb->data, 0, FRAME_DESC_SZ);
  cmd_frame = (struct rsi_mac_frame *)skb->data;

  /* FrameType */
  cmd_frame->desc_word[1] = cpu_to_le16(EEPROM_READ);

  /* Format of length and offset differs for
	 * autoflashing and swbl flashing
	 */
  cmd_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);

  /* Number of bytes to read */
  rsi_dbg(INFO_ZONE, " eeprom length  0x%x, %d\n", adapter->eeprom.length, adapter->eeprom.length);
  cmd_frame->desc_word[3] = cpu_to_le16(adapter->eeprom.length << 4);

  cmd_frame->desc_word[2] |= cpu_to_le16(3 << 8);
  if (adapter->eeprom_init) {
    rsi_dbg(INFO_ZONE, "spi init sent");
    cmd_frame->desc_word[2] |= cpu_to_le16(BIT(13));
  }

  /* Address to read */
  cmd_frame->desc_word[4] = cpu_to_le16(adapter->eeprom.offset);
  cmd_frame->desc_word[5] = cpu_to_le16(adapter->eeprom.offset >> 16);
  cmd_frame->desc_word[6] = cpu_to_le16(0); //delay = 0

  skb_put(skb, FRAME_DESC_SZ);
  rsi_hex_dump(INT_MGMT_ZONE, "Internal Mgmt Pkt", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * rsi_compare() - This function is used to compare two integers
 * @a: pointer to the first integer
 * @b: pointer to the second integer
 *
 * Return: 0 if both are equal, -1 if the first is smaller, else 1
 */
static int rsi_compare(const void *a, const void *b)
{
  u16 _a = *(const u16 *)(a);
  u16 _b = *(const u16 *)(b);

  if (_a > _b)
    return -1;

  if (_a < _b)
    return 1;

  return 0;
}

/**
 * rsi_map_rates() - This function is used to map selected rates to hw rates.
 * @rate: The standard rate to be mapped.
 * @offset: Offset that will be returned.
 *
 * Return: 0 if it is a mcs rate, else 1
 */
static bool rsi_map_rates(u16 rate, int *offset)
{
  int kk;

  for (kk = 0; kk < ARRAY_SIZE(rsi_mcsrates); kk++) {
    if (rate == mcs[kk]) {
      *offset = kk;
      return false;
    }
  }

  for (kk = 0; kk < ARRAY_SIZE(rsi_rates); kk++) {
    if (rate == rsi_rates[kk].bitrate / 5) {
      *offset = kk;
      break;
    }
  }
  return true;
}

/**
 * evaluate_moderate_rateindex() - This function is to find the moderate rate index
 *
 * @a: Pointer to an Array of rate values to be searched for moderate rate.
 * @b: Required moderate rate
 * @c: Rate Table size
 * Return: modearate rate index as per the rate table being sent to Firmware.
 */
static u32 evaluate_moderate_rateindex(u16 *bitrate, u32 moderate_rate, u32 rate_table_sz)
{
  int ii;
  for (ii = 0; ii < rate_table_sz; ii++) {
    if (moderate_rate >= bitrate[ii])
      return ii;
  }
  return (ii - 1);
}

/**
 * rsi_send_auto_rate_request() - This function is to set rates for connection
 *				  and send autorate request to firmware.
 * @common: Pointer to the driver private structure.
 *
 * Return: 0 on success, corresponding error code on failure.
 */
#ifndef CONFIG_STA_PLUS_AP
int rsi_send_auto_rate_request(struct rsi_common *common, struct ieee80211_sta *sta, u16 sta_id)
#else
int rsi_send_auto_rate_request(struct rsi_common *common,
                               struct ieee80211_sta *sta,
                               u16 sta_id,
                               struct ieee80211_vif *vif)
#endif
{
#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = common->priv->vifs[0];
#endif
  struct sk_buff *skb;
  struct rsi_auto_rate *auto_rate;
  int ii = 0, jj = 0, kk = 0, m = 0, n = 0, last_non_aggr_rate = 0;
  struct ieee80211_hw *hw = common->priv->hw;
  //__9117_CODE_START
  struct rsi_hw *adapter          = common->priv;
  struct rsi_ax_params *ax_params = &adapter->ax_params;
  bool is_he                      = false;
  //__9117_CODE_END
  u8 band          = hw->conf.chandef.chan->band;
  u8 rate_table_sz = 0;
  u8 bg_mode = 0, g_mode = 0, b_mode = 0, n_only_mode = 0;
  u16 moderate_rate_inx = 0;
  u32 rate_bitmap       = 0;
  u16 *selected_rates, *bitrate, min_rate;
  bool is_ht = false, is_sgi = false;
#ifdef CONFIG_STA_PLUS_AP
  struct vif_priv *vif_info = (struct vif_priv *)vif->drv_priv;
#endif
  struct rsi_sta *sta_info = NULL;

  rsi_dbg(INT_MGMT_ZONE, "<===== SENDING AUTO_RATE_IND FRAME =====>\n");

  if (!sta) {
#ifdef CONFIG_STA_PLUS_AP
    sta = common->stations[STA_PEER].sta;
#else
    sta = common->stations[RSI_MAX_ASSOC_STAS].sta;
#endif
  }
  sta_info = (struct rsi_sta *)sta->drv_priv;
  skb      = dev_alloc_skb(MAX_MGMT_PKT_SIZE);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, MAX_MGMT_PKT_SIZE);

  selected_rates = kzalloc(2 * RSI_TBL_SZ, GFP_KERNEL);
  bitrate        = kzalloc(RSI_TBL_SZ, GFP_KERNEL);
  if (!selected_rates) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of mem\n", __func__);
    dev_kfree_skb(skb);
    return -ENOMEM;
  }
  memset(selected_rates, 0, 2 * RSI_TBL_SZ);

  auto_rate = (struct rsi_auto_rate *)skb->data;

  auto_rate->aarf_rssi = cpu_to_le16(65); /*Keeping auto rate rssi to 65dbm*/
  //auto_rate->aarf_rssi = cpu_to_le16(((u16)3 << 6) | (u16)(18 & 0x3f));
  if (common->feature_bitmap_9116 & DISABLE_UPDATED_AUTORATE_PARAMS) {
    auto_rate->collision_tolerance = cpu_to_le16(3);
    auto_rate->failure_limit       = cpu_to_le16(3);
    auto_rate->initial_boundary    = cpu_to_le16(3);
    auto_rate->max_threshold_limt  = cpu_to_le16(27);
    auto_rate->desc_word[7]        = cpu_to_le16(BIT(4)); /* Disable autorate enhancements */
  } else {
    auto_rate->collision_tolerance = cpu_to_le16(9);
    auto_rate->failure_limit       = cpu_to_le16(2);
    auto_rate->initial_boundary    = cpu_to_le16(9);
    auto_rate->max_threshold_limt  = cpu_to_le16(9 * 9);
    rsi_mcsrates[0]                = RSI_RATE_6;
    rsi_mcsrates[1]                = RSI_RATE_12;
    //__9117_CODE_START
    rsi_he_rates[0] = RSI_RATE_6;
    rsi_he_rates[1] = RSI_RATE_12;
    //__9117_CODE_END
  }
  if (common->feature_bitmap_9116 & DISABLE_AARF_RATE_BLOCKING)
    auto_rate->desc_word[7] |= cpu_to_le16(BIT(5));

  auto_rate->desc_word[1] = cpu_to_le16(AUTO_RATE_IND);

  if (common->channel_width == BW_40MHZ)
    auto_rate->desc_word[7] = cpu_to_le16(1);
  auto_rate->desc_word[7] |= cpu_to_le16(sta_id << 8);

  if (vif->type == NL80211_IFTYPE_STATION) {
    rate_bitmap = common->bitrate_mask[band];
#ifndef CONFIG_STA_PLUS_AP
    is_ht  = common->vif_info[0].is_ht;
    is_sgi = common->vif_info[0].sgi;
#else
    is_ht = vif_info->is_ht;
    is_sgi = vif_info->sgi;
#endif
  } else {
    rate_bitmap = SUPP_RATES[band];
    is_ht       = HT_CAP.ht_supported;
    if ((HT_CAP.cap & IEEE80211_HT_CAP_SGI_20) || (HT_CAP.cap & IEEE80211_HT_CAP_SGI_40))
      is_sgi = true;
  }
  //__9117_CODE_START
  is_he = ax_params->_11ax_enabled;
  //__9117_CODE_END
  if (band == NL80211_BAND_5GHZ) {
    rate_bitmap <<= 4;
  }
  rsi_dbg(INFO_ZONE, "rate_bitmap = %x\n", rate_bitmap);
  rsi_dbg(INFO_ZONE, "is_ht = %d\n", is_ht);

  if (band == NL80211_BAND_2GHZ) {
    if ((rate_bitmap == 0) && (is_ht))
      min_rate = RSI_RATE_MCS0;
    else
      min_rate = RSI_RATE_1;
  } else {
    if ((rate_bitmap == 0) && (is_ht))
      min_rate = RSI_RATE_MCS0;
    else
      min_rate = RSI_RATE_6;
  }
  b_mode  = rate_bitmap & _11B_RATE_MAP;
  g_mode  = rate_bitmap & _11G_RATE_MAP;
  bg_mode = g_mode && b_mode;

  /*Rejecting 9 MBPS as we don't use it*/
  rate_bitmap &= ~(RSI_RATE_9_INX);

  /*Select table size and moderate rate based on the mode*/
  if (bg_mode && is_ht) {
    /*BGN MODE*/
    rate_table_sz = 11;
    rate_bitmap &= ~(RSI_RATE_5_5_INX | RSI_RATE_11_INX);
    moderate_rate_inx = STD_RATE_36;
  } else if (bg_mode) {
    /*BG MODE*/
    rate_table_sz = 9;
    rate_bitmap &= ~(RSI_RATE_5_5_INX | RSI_RATE_11_INX);
    moderate_rate_inx = STD_RATE_36;
  } else if (g_mode && is_ht) {
    /*GN MODE*/
    rate_table_sz     = 9;
    moderate_rate_inx = STD_RATE_36;
  } else if (g_mode) {
    /*GONLY MODE*/
    rate_table_sz     = 7;
    moderate_rate_inx = STD_RATE_36;
  } else if (b_mode) {
    /*BONLY MODE*/
    rate_table_sz = 4;
    if (strstr(adapter->country, "JP") && common->mac80211_cur_channel == 14) {
      moderate_rate_inx = STD_RATE_01;
      rate_bitmap &= ~(RSI_RATE_5_5_INX | RSI_RATE_11_INX);
    } else
      moderate_rate_inx = STD_RATE_5_5;
  } else if (is_ht) {
    /*NONLY MODE*/
    rate_table_sz     = 9;
    n_only_mode       = 1;
    moderate_rate_inx = RSI_RATE_MCS4;
  } else {
    rsi_dbg(ERR_ZONE, "No Rates supported\n");
  }

  //__9117_CODE_START
  if (is_he)
    rate_table_sz++;
  //__9117_CODE_END
  /*Extracting rates from the Bitmap*/
  for (ii = 0, jj = 0; ii < ARRAY_SIZE(rsi_rates); ii++) {
    if (rate_bitmap & BIT(ii))
      selected_rates[jj++] = (rsi_rates[ii].bitrate / 5);
  }

  sort(selected_rates, jj, sizeof(u16), &rsi_compare, NULL);

  ii = 0;
  n  = 0;
  kk = ARRAY_SIZE(rsi_mcsrates) - 1;
  /*Filling 2 rates of ht mode*/
  if (is_ht) {
    if (is_sgi) {
      auto_rate->supported_rates[ii++] = cpu_to_le16(rsi_mcsrates[kk] | BIT(9));
      bitrate[n++]                     = rsi_mcsrates[kk] | BIT(9);
    }
    auto_rate->supported_rates[ii++] = cpu_to_le16(rsi_mcsrates[kk]);
    bitrate[n++]                     = rsi_mcsrates[kk];
    kk--;
  }
  /* mapping the rates to RSI rates */
  if (n_only_mode) {
    for (; (ii < rate_table_sz) && (kk >= 0); ii++, kk--) {
      auto_rate->supported_rates[ii] = cpu_to_le16(rsi_mcsrates[kk]);
      bitrate[n++]                   = rsi_mcsrates[kk];
    }
  } else /*BGN mode*/ {
    for (m = 0; m < jj; m++) {
      if (rsi_map_rates(selected_rates[m], &kk)) {
        auto_rate->supported_rates[ii] = cpu_to_le16(rsi_rates[kk].hw_value);
        //__9117_CODE_START
        if (is_he && (cpu_to_le16(rsi_rates[kk].hw_value) == RSI_RATE_12))
          auto_rate->supported_rates[++ii] = cpu_to_le16(rsi_rates[kk].hw_value);
        //__9117_CODE_END
        ii++;
        bitrate[n++] = selected_rates[m];
      }
    }
  }
  last_non_aggr_rate = ii - 1;
  /*Stuffing the unfilled entries with min rate*/
  for (; ii < rate_table_sz; ii++, n++) {
    auto_rate->supported_rates[ii] = cpu_to_le16(auto_rate->supported_rates[ii - 1]);
    bitrate[n]                     = bitrate[n - 1];
  }
  sta_info->min_supported_rate = auto_rate->supported_rates[last_non_aggr_rate];
  last_non_aggr_rate -= 1; /* hold last two unaggregated rates to fill the vacancies in the end of aggregation table*/
  /* loading HT rates in the bottom half of the auto rate table */
  //__9117_CODE_START
  if (is_he && (vif->type == NL80211_IFTYPE_STATION)) {
    kk = ARRAY_SIZE(rsi_he_rates) - 1;
    if (is_sgi) {
      if (common->enable_er_su && common->config_er_su == ER_SU_ONLY)
        auto_rate->supported_rates[ii++] = _11AX_MCS2 | (adapter->rate_field_params | ENABLE_ER_SU);
      else
        auto_rate->supported_rates[ii++] = cpu_to_le16(rsi_he_rates[kk] | (adapter->rate_field_params & SGI_RATE));
    }
    for (; (ii < 2 * rate_table_sz) && (kk >= 0); ii++, kk--) {
      if (!(common->enable_er_su && common->config_er_su)) { /* No ERSU */
        if ((rsi_he_rates[kk] == RSI_RATE_6) || (rsi_he_rates[kk] == RSI_RATE_12))
          auto_rate->supported_rates[ii] = cpu_to_le16(rsi_he_rates[kk]);
        else
          auto_rate->supported_rates[ii] = cpu_to_le16(rsi_he_rates[kk] | adapter->rate_field_params);
      }
      if (common->enable_er_su) {
        if (common->config_er_su == ER_SU_ONLY) { /* Only ERSU supported rates */
          auto_rate->supported_rates[ii] = cpu_to_le16(er_su_rates[kk] | adapter->rate_field_params | ENABLE_ER_SU);
        } else if (common->config_er_su == ERSU_HE_MIXED) { /* HE rates with ERSU rates mixed */
          if ((rsi_he_rates[kk] == RSI_RATE_12) || (rsi_he_rates[kk] == RSI_RATE_6)) {
            auto_rate->supported_rates[ii] = cpu_to_le16(rsi_he_rates[kk]);
          } else if ((rsi_he_rates[kk] == _11AX_MCS2) || (rsi_he_rates[kk] == _11AX_MCS1)) {
            auto_rate->supported_rates[ii] = cpu_to_le16(rsi_he_rates[kk] | adapter->rate_field_params | ENABLE_ER_SU);
          } else {
            auto_rate->supported_rates[ii] = cpu_to_le16(rsi_he_rates[kk] | adapter->rate_field_params);
          }
        }
      }
    }
  } else
    //__9117_CODE_END
    if (is_ht) {
    kk = ARRAY_SIZE(rsi_mcsrates) - 1;
    if (is_sgi) {
      auto_rate->supported_rates[ii++] = cpu_to_le16(rsi_mcsrates[kk] | BIT(9));
    }
    for (; (ii < 2 * rate_table_sz) && (kk >= 0); ii++, kk--) {
      auto_rate->supported_rates[ii] = cpu_to_le16(rsi_mcsrates[kk] /* | BIT(9)*/);
    }
  }
  /*Stuffing the unfilled entries with min rate*/
  for (; ii < 2 * rate_table_sz; ii++) {
    //__9117_CODE_START
    if (is_he && (vif->type == NL80211_IFTYPE_STATION)) {
      if (common->enable_er_su && common->config_er_su == ER_SU_ONLY)
        auto_rate->supported_rates[ii] = cpu_to_le16(_11AX_MCS0 | adapter->rate_field_params | ENABLE_ER_SU);
      else
        auto_rate->supported_rates[ii] = cpu_to_le16(auto_rate->supported_rates[last_non_aggr_rate++]);

    } else
    //__9117_CODE_END
    {
      auto_rate->supported_rates[ii] = cpu_to_le16(auto_rate->supported_rates[last_non_aggr_rate++]);
    }
  }

  auto_rate->num_supported_rates = cpu_to_le16(rate_table_sz * 2);
  auto_rate->moderate_rate_inx   = evaluate_moderate_rateindex(bitrate, moderate_rate_inx, rate_table_sz);
  rsi_dbg(INT_MGMT_ZONE, "moderate rate index is %d\n", auto_rate->moderate_rate_inx);

  for (ii = 0; ii < rate_table_sz * 2; ii++)
    rsi_dbg(INFO_ZONE, "Auto rate code is  ==>  0x%x\n", auto_rate->supported_rates[ii]);

  auto_rate->desc_word[0] = cpu_to_le16((sizeof(*auto_rate) - FRAME_DESC_SZ) | (RSI_WIFI_MGMT_Q << 12));

  skb_put(skb, sizeof(struct rsi_auto_rate));
  kfree(selected_rates);
  kfree(bitrate);
  rsi_hex_dump(INT_MGMT_ZONE, "AUTO_RATE FRAME:", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

void update_bgscan_channel_for_acx_module(struct rsi_common *common)
{
  u8 num_scan_chan, jj, dfs_region;
  struct bgscan_config_params *bgscan_info = &common->bgscan_info;
  struct rsi_hw *adapter                   = common->priv;
  dfs_region                               = adapter->dfs_region + 1;
  if (dfs_region == NL80211_DFS_FCC)
    num_scan_chan = MAX_CHAN_FOR_ACX - 2;
  else
    num_scan_chan = MAX_CHAN_FOR_ACX;

  for (jj = 0; jj < num_scan_chan; jj++) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0))
    common->user_channels_list[jj] = rsi_2ghz_acx_channels[jj].hw_value;
  }
  common->user_channels_count = num_scan_chan;
#else
    bgscan_info->user_channels[jj] = rsi_2ghz_acx_channels[jj].hw_value;
  }
  bgscan_info->num_user_channels = num_scan_chan;
#endif
}

/**
 * rsi_validate_bgscan_channels() - This function is used to validate
 *				the user configured bgscan channels for
 *				current regulatory domain
 * @chn_num: It holds the user or default channel for validation.
 *
 * Return: 0 on success, corresponding error code on failure.
 */
void rsi_validate_bgscan_channels(struct rsi_hw *adapter, struct bgscan_config_params *params)
{
  struct ieee80211_supported_band *sband;
  struct ieee80211_channel *ch;
  struct wiphy *wiphy = adapter->hw->wiphy;
  int ch_num, i;
  int num_valid_chs         = 0, cnt;
  struct rsi_common *common = adapter->priv;

  rsi_dbg(INFO_ZONE, "Final bgscan channels:\n");
  for (cnt = 0; cnt < params->num_user_channels; cnt++) {
    ch_num = params->user_channels[cnt];

    if ((ch_num < 1) || ((ch_num > 14) && (ch_num < 36)) || ((ch_num > 64) && (ch_num < 100))
        || ((ch_num > 140) && (ch_num < 149)) || (ch_num > 165))
      continue;
    if ((ch_num >= 36) && (ch_num < 149) && (ch_num % 4))
      continue;

    if (ch_num > 14)
      sband = wiphy->bands[NL80211_BAND_5GHZ];
    else
      sband = wiphy->bands[NL80211_BAND_2GHZ];

    for (i = 0; i < sband->n_channels; i++) {
      ch = &sband->channels[i];

      if (ch->hw_value == ch_num)
        break;
    }
    if (i >= sband->n_channels)
      continue;

    /* Check channel availability for the current reg domain */
    if (ch->flags & IEEE80211_CHAN_DISABLED)
      continue;

    if (common->num_supp_bands > 1) {
      params->channels2scan[num_valid_chs] = ch_num;
      rsi_dbg(INFO_ZONE, "%d ", ch_num);
      if ((ch->flags & IEEE80211_CHAN_RADAR)) {
        rsi_dbg(INFO_ZONE, "[DFS]");
        params->channels2scan[num_valid_chs] |= (cpu_to_le16(BIT(15))); /* DFS indication */
      }
    } else
      params->channels2scan[num_valid_chs] = ch_num;
    num_valid_chs++;
  }

  params->num_bg_channels = num_valid_chs;
}

/**
 * rsi_send_bgscan_params() - This function sends the background
 *			      scan parameters to firmware.
 * @common: Pointer to the driver private structure.
 * @enable: bgscan enable/disable
 *
 * Return: 0 on success, corresponding error code on failure.
 */
int rsi_send_bgscan_params(struct rsi_common *common, int enable)
{
  struct rsi_bgscan_params *bgscan;
  struct bgscan_config_params *info = &common->bgscan_info;
  struct sk_buff *skb;
  u16 frame_len = sizeof(*bgscan);
  bool status   = 0;

  rsi_dbg(INT_MGMT_ZONE, "<===== Sending bgscan params frame ====>\n");
  if (common->debugfs_bgscan)
    status = rsi_validate_debugfs_bgscan_channels(common);
  else
    rsi_validate_bgscan_channels(common->priv, info);
  if (!info->num_bg_channels || status) {
    common->debugfs_bgscan = false;
    rsi_dbg(ERR_ZONE, "##### No valid bgscan channels #####\n");
    return -1;
  }

  skb = dev_alloc_skb(frame_len);
  if (!skb)
    return -ENOMEM;
  memset(skb->data, 0, frame_len);

  bgscan = (struct rsi_bgscan_params *)skb->data;

  bgscan->desc_word[0] = cpu_to_le16((frame_len - FRAME_DESC_SZ) | (RSI_WIFI_MGMT_Q << 12));
  bgscan->desc_word[1] = cpu_to_le16(BG_SCAN_PARAMS);

  bgscan->bgscan_threshold = cpu_to_le16(info->bgscan_threshold);
  bgscan->roam_threshold   = cpu_to_le16(info->roam_threshold);
  if (enable) {
    bgscan->bgscan_periodicity = cpu_to_le16(info->bgscan_periodicity);
  }
  bgscan->active_scan_duration  = cpu_to_le16(info->active_scan_duration);
  bgscan->passive_scan_duration = cpu_to_le16(info->passive_scan_duration);
  bgscan->two_probe             = info->two_probe;

  if (!enable) {
    bgscan->num_bg_channels = 1;
    memset(bgscan->channels2scan, 0, sizeof(bgscan->channels2scan));
    goto out;
  }

  if (common->debugfs_bgscan) {
    bgscan->num_bg_channels = common->bgscan_info.debugfs_bg_channels;
    memcpy(bgscan->channels2scan, common->bgscan_info.debugfs_channels, bgscan->num_bg_channels * 2);
  } else {
    if (common->bgscan_info.num_user_channels > MAX_BG_CHAN_FROM_USER) {
      if (!common->send_initial_bgscan_chan) {
        memcpy(bgscan->channels2scan, info->channels2scan, MAX_BG_CHAN_FROM_USER * 2);
        bgscan->num_bg_channels          = MAX_BG_CHAN_FROM_USER;
        common->send_initial_bgscan_chan = true;
      } else {
        memcpy(bgscan->channels2scan,
               info->channels2scan + MAX_BG_CHAN_FROM_USER,
               (info->num_bg_channels - MAX_BG_CHAN_FROM_USER) * 2);
        bgscan->num_bg_channels          = (info->num_bg_channels - MAX_BG_CHAN_FROM_USER);
        common->send_initial_bgscan_chan = false;
      }
    } else {
      memcpy(bgscan->channels2scan, info->channels2scan, info->num_bg_channels * 2);
      bgscan->num_bg_channels = info->num_bg_channels;
    }
  }
out:
  skb_put(skb, frame_len);
  rsi_hex_dump(MGMT_TX_ZONE, "Bgscan Params", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * rsi_send_bgscan_probe_req() - This function sends the background
 *                               scan probe request to firmware.
 * @common: Pointer to the driver private structure.
 *
 * Return: 0 on success, corresponding error code on failure.
 */
int rsi_send_bgscan_probe_req(struct rsi_common *common)
{
  struct rsi_bgscan_probe *bgscan;
  struct sk_buff *skb;
  u16 frame_len = sizeof(*bgscan);
  u16 len       = 1500;
  u16 pbreq_len = 0;
  struct sk_buff *probereq_skb;
  void *temp_buf;
#ifndef CONFIG_STA_PLUS_AP
  struct rsi_hw *adapter    = common->priv;
  struct ieee80211_vif *vif = adapter->vifs[adapter->sc_nvifs - 1];
#else
  struct ieee80211_vif *sta_vif = rsi_get_sta_vif(common->priv);
#endif
  struct cfg80211_scan_request *scan_req = common->scan_request;

  rsi_dbg(MGMT_TX_ZONE, "<==== Sending bgscan probe req frame ====>\n");

  skb = dev_alloc_skb(frame_len + len);
  if (!skb)
    return -ENOMEM;
  memset(skb->data, 0, frame_len + len);
#ifndef CONFIG_STA_PLUS_AP
  if (!vif)
#else
  if (!sta_vif)
#endif
    return -EINVAL;

  bgscan = (struct rsi_bgscan_probe *)skb->data;

  bgscan->desc_word[1] = cpu_to_le16(BG_SCAN_PROBE_REQ);
#ifndef CONFIG_STA_PLUS_AP
  bgscan->flags = cpu_to_le16(HOST_BG_SCAN_TRIG);
#else
  if (common->vap_mode == CONCURRENT)
    bgscan->flags = cpu_to_le16(HOST_BG_SCAN_TRIG);
  bgscan->flags |= cpu_to_le16(BG_SCAN_TRIG);
#endif
  if (common->band == NL80211_BAND_5GHZ) {
    bgscan->mgmt_rate   = cpu_to_le16(RSI_RATE_6);
    bgscan->channel_num = cpu_to_le16(40);
  } else {
    bgscan->mgmt_rate   = cpu_to_le16(RSI_RATE_1);
    bgscan->channel_num = cpu_to_le16(11);
  }

  bgscan->channel_scan_time = cpu_to_le16(20);
  if (common->bgscan_info.two_probe && common->bgscan_ssid_len) {
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 18, 140)
    probereq_skb =
#ifndef CONFIG_STA_PLUS_AP
      ieee80211_probereq_get(common->priv->hw, vif, common->bgscan_ssid, common->bgscan_ssid_len, scan_req->ie_len);
#else
      ieee80211_probereq_get(common->priv->hw, sta_vif, common->bgscan_ssid, common->bgscan_ssid_len, scan_req->ie_len);
#endif
#else
    probereq_skb = ieee80211_probereq_get(common->priv->hw,
#ifndef CONFIG_STA_PLUS_AP
                                          vif->addr,
#else
                                          sta_vif->addr,
#endif
                                          common->bgscan_ssid,
                                          common->bgscan_ssid_len,
                                          scan_req->ie_len);
#endif
  } else {
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 18, 140)
#ifndef CONFIG_STA_PLUS_AP
    probereq_skb = ieee80211_probereq_get(common->priv->hw, vif, NULL, 0, scan_req->ie_len);
#else
    probereq_skb = ieee80211_probereq_get(common->priv->hw, sta_vif, NULL, 0, scan_req->ie_len);
#endif
#else
#ifndef CONFIG_STA_PLUS_AP
    probereq_skb = ieee80211_probereq_get(common->priv->hw, vif->addr, NULL, 0, scan_req->ie_len);
#else
    probereq_skb = ieee80211_probereq_get(common->priv->hw, sta_vif->addr, NULL, 0, scan_req->ie_len);
#endif
#endif
  }
  if (!probereq_skb) {
    dev_kfree_skb(skb);
    return -ENOMEM;
  }
  if (scan_req->ie_len) {
    temp_buf = skb_put(probereq_skb, scan_req->ie_len);
    memcpy(temp_buf, scan_req->ie, scan_req->ie_len);
  }
  pbreq_len                = probereq_skb->len;
  bgscan->probe_req_length = pbreq_len;
  memcpy(&skb->data[frame_len], probereq_skb->data, probereq_skb->len);
  dev_kfree_skb(probereq_skb);
  bgscan->desc_word[0] = cpu_to_le16((frame_len - FRAME_DESC_SZ + pbreq_len) | (RSI_WIFI_MGMT_Q << 12));

  skb_put(skb, frame_len + pbreq_len);
  rsi_hex_dump(MGMT_TX_ZONE, "Bgscan Probe_Req", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * rsi_inform_bss_status() - This function informs about bss status with the
 *			     help of sta notify params by sending an internal
 *			     management frame to firmware.
 * @common: Pointer to the driver private structure.
 * @status: Bss status type.
 * @bssid: Bssid.
 * @qos_enable: Qos is enabled.
 * @aid: Aid (unique for all STAs).
 *
 * Return: None.
 */
void rsi_inform_bss_status(struct rsi_common *common,
                           enum opmode opmode,
                           u8 status,
                           const u8 *bssid,
                           u8 qos_enable,
                           u16 aid,
                           struct ieee80211_sta *sta,
                           u16 sta_id,
#ifndef CONFIG_STA_PLUS_AP
                           u16 assoc_cap)
#else
                           u16 assoc_cap,
                           struct ieee80211_vif *vif)
#endif
{
  if (status) {
    if (opmode == STA_OPMODE)
      common->hw_data_qs_blocked = true;
#ifndef CONFIG_STA_PLUS_AP
    rsi_send_sta_notify_frame(common, opmode, STA_CONNECTED, bssid, qos_enable, aid, sta_id);
#else
    rsi_send_sta_notify_frame(common, opmode, STA_CONNECTED, bssid, qos_enable, aid, sta_id, vif);
#endif
    if (common->min_rate == 0xffff) {
      rsi_dbg(INFO_ZONE, "Send auto rate request\n");
#ifndef CONFIG_STA_PLUS_AP
      rsi_send_auto_rate_request(common, sta, sta_id);
#else
      rsi_send_auto_rate_request(common, sta, sta_id, vif);
#endif
    }
    if (opmode == STA_OPMODE) {
      rsi_load_radio_caps(common);
      if (!(assoc_cap & BIT(4))) {
        //__9117_CODE_START
#ifdef CONFIG_TWT_SUPPORT
        if (common->priv->ax_params._11ax_enabled && common->priv->ap_support_twt) {
          if (common->priv->rsi_twt_config.twt_enable) /* Send twt_setup in Open Mode */
            send_twt_setup_frame(common->priv, 0, NULL);
        }
#endif
        //__9117_CODE_END
        rsi_dbg(INFO_ZONE, "unblock data in Open case\n");
        if (!rsi_send_block_unblock_frame(common, false))
          common->hw_data_qs_blocked = false;
      }
    } else if ((opmode == AP_OPMODE && common->hw_data_qs_blocked)) {
      rsi_dbg(INFO_ZONE, "unblock data in AP mode\n");
      if (!rsi_send_block_unblock_frame(common, false))
        common->hw_data_qs_blocked = false;
    }
  } else {
    if (opmode == STA_OPMODE)
      common->hw_data_qs_blocked = true;
    if (common->priv->traffic_timer.function) {
      del_timer(&common->priv->traffic_timer);
      rsi_dbg(INFO_ZONE, "Removed Traffic Timer\n");
    }
#ifdef CONFIG_RSI_WOW
    if (!(common->wow_flags & RSI_WOW_ENABLED)) {
#endif
#ifndef CONFIG_STA_PLUS_AP
      rsi_send_sta_notify_frame(common, opmode, STA_DISCONNECTED, bssid, qos_enable, aid, sta_id);
#else
    rsi_send_sta_notify_frame(common, opmode, STA_DISCONNECTED, bssid, qos_enable, aid, sta_id, vif);
#endif
#ifdef CONFIG_RSI_WOW
    }
#endif
    if (opmode == STA_OPMODE)
      rsi_send_block_unblock_frame(common, true);
  }
}

/**
 * rsi_eeprom_read() - This function sends a frame to read the mac address
 *		       from the eeprom.
 * @common: Pointer to the driver private structure.
 *
 * Return: 0 on success, -1 on failure.
 */
static int rsi_eeprom_read(struct rsi_common *common)
{
  struct rsi_mac_frame *mgmt_frame = NULL;
  struct rsi_hw *adapter           = common->priv;
  struct sk_buff *skb;

  rsi_dbg(MGMT_TX_ZONE, "<==== Sending EEPROM read req frame ====>\n");

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, FRAME_DESC_SZ);
  mgmt_frame = (struct rsi_mac_frame *)skb->data;

  /* FrameType */
  mgmt_frame->desc_word[1] = cpu_to_le16(EEPROM_READ);
  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);

  /* Number of bytes to read */
  mgmt_frame->desc_word[3] = cpu_to_le16(adapter->eeprom.length << 4);
  mgmt_frame->desc_word[2] |= cpu_to_le16(3 << 8);

  /* Address to read*/
  mgmt_frame->desc_word[4] = cpu_to_le16(adapter->eeprom.offset);
  mgmt_frame->desc_word[5] = cpu_to_le16(adapter->eeprom.offset >> 16);
  mgmt_frame->desc_word[6] = cpu_to_le16(0); //delay = 0

  skb_put(skb, FRAME_DESC_SZ);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * This function sends a frame to block/unblock
 * data queues in the firmware
 *
 * @param common Pointer to the driver private structure.
 * @param block event - block if true, unblock if false
 * @return 0 on success, -1 on failure.
 */
int rsi_send_block_unblock_frame(struct rsi_common *common, bool block_event)
{
  struct rsi_mac_frame *mgmt_frame;
  struct sk_buff *skb;

  rsi_dbg(INT_MGMT_ZONE, "<==== Sending block/unblock frame ====>\n");

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, FRAME_DESC_SZ);
  mgmt_frame = (struct rsi_mac_frame *)skb->data;

  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  mgmt_frame->desc_word[1] = cpu_to_le16(BLOCK_HW_QUEUE);
  mgmt_frame->desc_word[3] = cpu_to_le16(0x1);

  if (block_event == true) {
    rsi_dbg(INFO_ZONE, "blocking the data qs\n");
    mgmt_frame->desc_word[4] = cpu_to_le16(0xf);
#ifndef CONFIG_STA_PLUS_AP
    mgmt_frame->desc_word[4] |= cpu_to_le16(0xf << 4);
#endif
  } else {
    rsi_dbg(INFO_ZONE, "unblocking the data qs\n");
    mgmt_frame->desc_word[5] = cpu_to_le16(0xf);
#ifndef CONFIG_STA_PLUS_AP
    mgmt_frame->desc_word[5] |= cpu_to_le16(0xf << 4);
#endif
  }
#ifdef CONFIG_STA_PLUS_AP
  // ! Unblocking AP queues
  mgmt_frame->desc_word[5] |= cpu_to_le16(0xf << 4);
#endif

  skb_put(skb, FRAME_DESC_SZ);
  if (block_event == true) {
    rsi_hex_dump(INT_MGMT_ZONE, "BLOCK-DATA QUEUE:", skb->data, skb->len);
  } else {
    rsi_hex_dump(INT_MGMT_ZONE, "UNBLOCK-DATA QUEUE:", skb->data, skb->len);
  }

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * This function sends a frame to filter the RX packets
 *
 * @param common Pointer to the driver private structure.
 * @param rx_filter_word - Flags of filter packets
 * @return 0 on success, -1 on failure.
 */
int rsi_send_rx_filter_frame(struct rsi_common *common, u16 rx_filter_word)
{
  struct rsi_mac_frame *mgmt_frame;
  struct sk_buff *skb;

  rsi_dbg(INT_MGMT_ZONE, "<==== Sending RX filter frame ====>\n");

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, FRAME_DESC_SZ);
  mgmt_frame = (struct rsi_mac_frame *)skb->data;

  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  mgmt_frame->desc_word[1] = cpu_to_le16(SET_RX_FILTER);
  mgmt_frame->desc_word[4] = cpu_to_le16(rx_filter_word);

  skb_put(skb, FRAME_DESC_SZ);
  rsi_hex_dump(INT_MGMT_ZONE, "RX FILTER FRAME", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}
EXPORT_SYMBOL_GPL(rsi_send_rx_filter_frame);

/**
 * rsi_send_ps_request() - Sends power save request.
 *
 * @adapter: pointer to rsi_hw structure.
 * @enable: enable or disable power save.
 *
 * returns: 0 on success, negative error code on failure
 */
int rsi_send_ps_request(struct rsi_hw *adapter, bool enable)
{
  struct rsi_common *common = adapter->priv;
#ifndef CONFIG_STA_PLUS_AP
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  struct ieee80211_bss_conf *bss;
#endif

#else
  struct ieee80211_vif *sta_vif = rsi_get_sta_vif(common->priv);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  struct ieee80211_bss_conf *sta_bss;
#endif

#endif
  struct rsi_request_ps *ps   = NULL;
  struct rsi_ps_info *ps_info = NULL;
  struct sk_buff *skb         = NULL;
  int frame_len               = sizeof(*ps);
  bool assoc;

  rsi_dbg(INT_MGMT_ZONE, "<===== SENDING %s REQUEST =====>\n", enable ? "PS_ENABLED" : "PS_DISABLED");
  skb = dev_alloc_skb(frame_len);
  if (!skb)
    return -ENOMEM;
  memset(skb->data, 0, frame_len);

  ps      = (struct rsi_request_ps *)&skb->data[0];
  ps_info = &adapter->ps_info;

  ps->desc_word[0] = cpu_to_le16((frame_len - FRAME_DESC_SZ) | (RSI_WIFI_MGMT_Q << 12));
  ps->desc_word[1] = cpu_to_le16(WAKEUP_SLEEP_REQUEST);
  if (enable) {
    ps->ps_sleep.enable = 1;
    ps->desc_word[6]    = SLEEP_REQUEST;
  } else {
    ps->ps_sleep.enable = 0;
    ps->desc_word[0] |= BIT(15);
    ps->desc_word[6] = WAKEUP_REQUEST;
  }

  if (common->uapsd_bitmap) {
    //		ps->ps_mimic_support = 1;
    /* HW enables U-APSD on all ACs. Doesn't support enabling for specific AC.
       Hence it is recommanded to configure uapsd_bitmap to 0x0F always */
    ps->ps_uapsd_acs = common->uapsd_bitmap & IEEE80211_WMM_IE_STA_QOSINFO_AC_MASK;
    ps->ps_uapsd_acs |= (adapter->hw->uapsd_max_sp_len << IEEE80211_WMM_IE_STA_QOSINFO_SP_SHIFT);
    ps->ps_uapsd_wakeup_period = ps_info->uapsd_wakeup_period;
  }
  ps->ps_sleep.sleep_type           = ps_info->sleep_type;
  ps->ps_sleep.num_bcns_per_lis_int = cpu_to_le16(ps_info->num_bcns_per_lis_int);
  ps->ps_sleep.sleep_duration       = cpu_to_le32(ps_info->deep_sleep_wakeup_period);

  if (adapter->sc_nvifs == 0) {
    assoc = false;
#ifdef CONFIG_STA_PLUS_AP
  } else if (!sta_vif) {
    assoc = true;
#endif
  } else {
#ifndef CONFIG_STA_PLUS_AP

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
    bss = &adapter->vifs[0]->bss_conf;
    if (bss->assoc)
#else
    if (adapter->vifs[0]->cfg.assoc)
#endif

#else

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
    sta_bss = &sta_vif->bss_conf;
    if (sta_bss->assoc)
#else
    if (sta_vif->cfg.assoc)
#endif

#endif
      assoc = true;
    else
      assoc = false;
  }
  if (assoc)
    ps->ps_sleep.connected_sleep = CONNECTED_SLEEP;
  else
    ps->ps_sleep.connected_sleep = DEEP_SLEEP;

  ps->ps_listen_interval_duration = cpu_to_le32(ps_info->listen_interval_duration);
  ps->ps_dtim_interval_duration   = cpu_to_le32(ps_info->dtim_interval_duration);

  ps->ps_num_dtim_intervals = cpu_to_le16(ps_info->num_dtims_per_sleep);
  skb_put(skb, frame_len);
  rsi_hex_dump(INT_MGMT_ZONE, "PS-REQ FRAME", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

/**
 * rsi_set_antenna() - This function handles antenna selection functionality.
 *
 * @common: Pointer to the driver private structure.
 * @antenna: bitmap for tx antenna selection
 *
 * Return: 0 on Success, < 0 on failure
 */
int rsi_set_antenna(struct rsi_common *common, u8 antenna)
{
  struct rsi_mac_frame *mgmt_frame;
  struct sk_buff *skb;

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, FRAME_DESC_SZ);
  mgmt_frame = (struct rsi_mac_frame *)skb->data;

  mgmt_frame->desc_word[1] = cpu_to_le16(ANT_SEL_FRAME);
  mgmt_frame->desc_word[2] = cpu_to_le16(1 << 8); /* Antenna selection
							   type */
  mgmt_frame->desc_word[3] = cpu_to_le16(antenna & 0x00ff);
  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);

  skb_put(skb, FRAME_DESC_SZ);

  return rsi_send_internal_mgmt_frame(common, skb);
}

#ifdef CONFIG_RSI_11K
int rsi_get_channel_load_meas(struct rsi_common *common, struct rsi_meas_params chl_meas)
{
  struct rsi_mac_frame *cmd_frame;
  struct sk_buff *skb;
  u8 channel_width;
  u8 dfs = 0;

  rsi_dbg(MGMT_TX_ZONE, " %s: Sending channel load request frame\n", __func__);

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb)
    return -ENOMEM;
  memset(skb->data, 0, FRAME_DESC_SZ);

  cmd_frame = (struct rsi_mac_frame *)skb->data;

  channel_width = ieee80211_chan_to_bw(common->priv, chl_meas.regulatory_class, chl_meas.channel_num);

  dfs = is_dfs_channel(common->priv, chl_meas.channel_num);

  if (dfs == 1)
    rsi_dbg(ERR_ZONE, "Requested channel is DFS\n");
  else
    rsi_dbg(ERR_ZONE, "Requested channel is NON DFS\n");

  rsi_dbg(INFO_ZONE, "Regulatory value %d\n", chl_meas.regulatory_class);
  rsi_dbg(INFO_ZONE, "channel_Num value %d\n", chl_meas.channel_num);
  rsi_dbg(INFO_ZONE, "Meas_duration value %d\n", chl_meas.meas_duration);
  rsi_dbg(INFO_ZONE, "rand_interval value %d\n", chl_meas.rand_interval);
  rsi_dbg(INFO_ZONE, "channel_width value %d\n", chl_meas.channel_width);
  rsi_dbg(INFO_ZONE, "meas_req_mode value %d\n", chl_meas.meas_req_mode);
  rsi_dbg(INFO_ZONE, "meas_type value %d\n", chl_meas.meas_type);

  switch (channel_width) {
    case RSI_BW_5:
      channel_width = 0;
      break;
    case RSI_BW_10:
      channel_width = 1;
      break;
    case RSI_BW_20:
      channel_width = 2;
      break;
    case RSI_BW_25:
      channel_width = 2;
      break;
    case RSI_BW_40:
      channel_width = 3;
      break;
    default:
      channel_width = 4;
      break;
  }
  rsi_dbg(INFO_ZONE, "Channel_width = %d\n", channel_width);

  cmd_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  cmd_frame->desc_word[1] = cpu_to_le16(RADIO_MEASUREMENT_REQ);
  cmd_frame->desc_word[2] = cpu_to_le16(chl_meas.meas_type << 8);
  cmd_frame->desc_word[3] = cpu_to_le16(chl_meas.meas_req_mode);
  cmd_frame->desc_word[3] |= cpu_to_le16(((u16)chl_meas.channel_num) << 8);
  cmd_frame->desc_word[4] = cpu_to_le16(chl_meas.meas_duration);
  cmd_frame->desc_word[5] = cpu_to_le16(chl_meas.rand_interval);
  cmd_frame->desc_word[6] = cpu_to_le16(dfs << 8);
  cmd_frame->desc_word[6] |= cpu_to_le16(channel_width);

  skb_put(skb, FRAME_DESC_SZ);

  return rsi_send_internal_mgmt_frame(common, skb);
}

int rsi_get_frame_meas(struct rsi_common *common, struct rsi_frame_meas_params params)
{
  struct rsi_frame_meas_req *cmd_frame;
  struct sk_buff *skb;
  u8 channel_width, length;
  int ret;

  rsi_dbg(MGMT_TX_ZONE, "%s: Sending frame request frame\n", __func__);

  skb = dev_alloc_skb(sizeof(*cmd_frame));
  if (!skb)
    return -ENOMEM;

  memset(skb->data, 0, sizeof(*cmd_frame));
  cmd_frame = (struct rsi_frame_meas_req *)skb->data;

  channel_width = ieee80211_chan_to_bw(common->priv, params.mp.regulatory_class, params.mp.channel_num);
  if (channel_width == RSI_BW_5)
    channel_width = 0;
  else if (channel_width == RSI_BW_10)
    channel_width = 1;
  else if (channel_width == RSI_BW_20)
    channel_width = 2;
  else if (channel_width == RSI_BW_25)
    channel_width = 2;
  else if (channel_width == RSI_BW_40)
    channel_width = 3;
  else
    channel_width = 4;
  rsi_dbg(INFO_ZONE, "channel_width = %d\n", channel_width);

  ret = is_dfs_channel(common->priv, params.mp.channel_num);
  if (ret == 1)
    rsi_dbg(ERR_ZONE, "Requested channel is DFS\n");
  else
    rsi_dbg(ERR_ZONE, "Requested channel is NON DFS\n");

  cmd_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  cmd_frame->desc_word[1] |= cpu_to_le16(RADIO_MEASUREMENT_REQ);
  cmd_frame->desc_word[2] = cpu_to_le16(params.mp.meas_type << 8);
  cmd_frame->desc_word[3] = cpu_to_le16(params.mp.meas_req_mode);
  cmd_frame->desc_word[3] |= cpu_to_le16(((u16)params.mp.channel_num) << 8);
  cmd_frame->desc_word[4] = cpu_to_le16(params.mp.meas_duration);
  cmd_frame->desc_word[5] = cpu_to_le16(params.mp.rand_interval);
  cmd_frame->desc_word[6] = cpu_to_le16(ret << 8);
  cmd_frame->desc_word[6] |= cpu_to_le16(channel_width);

  memcpy(cmd_frame->bssid, params.mac_addr, IEEE80211_ADDR_LEN);
  cmd_frame->frm_req_type = params.frame_req_type;

  length = FRAME_DESC_SZ + IEEE80211_ADDR_LEN + 1;
  cmd_frame->desc_word[0] |= cpu_to_le16(length - FRAME_DESC_SZ);
  cmd_frame->desc_word[2] |= cpu_to_le16(0);

  skb_put(skb, length);

  return rsi_send_internal_mgmt_frame(common, skb);
}

int rsi_get_beacon_meas(struct rsi_common *common, struct rsi_beacon_meas_params params)
{
  struct rsi_bcn_meas_req *beacon_req = NULL;
  struct sk_buff *skb                 = NULL;
  u8 channel_width                    = 0, dfs;
  u8 index, length = 0;

  rsi_dbg(ERR_ZONE, "%s: Sending request to get beacon measurements\n", __func__);

  skb = dev_alloc_skb(MAX_MGMT_PKT_SIZE);
  if (!skb)
    return -ENOMEM;
  beacon_req    = (struct rsi_bcn_meas_req *)skb->data;
  channel_width = ieee80211_chan_to_bw(common->priv, params.mp.regulatory_class, params.mp.channel_num);
  if (channel_width == RSI_BW_5)
    channel_width = 0;
  else if (channel_width == RSI_BW_10)
    channel_width = 1;
  else if (channel_width == RSI_BW_20)
    channel_width = 2;
  else if (channel_width == RSI_BW_25)
    channel_width = 2;
  else if (channel_width == RSI_BW_40)
    channel_width = 3;
  else
    channel_width = 4;
  rsi_dbg(INFO_ZONE, "channel_width = %d\n", channel_width);
  dfs = is_dfs_channel(common->priv, params.mp.channel_num);
  if (dfs == 1)
    rsi_dbg(INFO_ZONE, "Requested channel is DFS\n");
  else
    rsi_dbg(INFO_ZONE, "Requested channel is NON DFS\n");

  beacon_req->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  beacon_req->desc_word[1] = cpu_to_le16(FLAGS << 8);
  beacon_req->desc_word[1] |= cpu_to_le16(RADIO_MEASUREMENT_REQ);
  beacon_req->desc_word[2] = cpu_to_le16(params.mp.meas_type << 8);
  beacon_req->desc_word[3] = cpu_to_le16(params.mp.meas_req_mode);
  beacon_req->desc_word[3] |= cpu_to_le16(((u16)params.mp.channel_num) << 8);
  beacon_req->desc_word[4] = cpu_to_le16(params.mp.meas_duration);
  beacon_req->desc_word[5] = cpu_to_le16(params.mp.rand_interval);
  beacon_req->desc_word[6] = cpu_to_le16(dfs << 8); /* DFS chan */
  beacon_req->desc_word[6] |= cpu_to_le16(channel_width);

  memcpy(beacon_req->bssid, params.mac_addr, IEEE80211_ADDR_LEN);
  memcpy(beacon_req->ssid, &params.ssid_ie[2], SSID_LEN);
  beacon_req->rep_detail = params.rpt_detail;
  beacon_req->meas_mode  = params.meas_mode;
  index                  = 0;
  rsi_dbg(ERR_ZONE, "BEACON REQ REPORTING DETAILS %d\n", beacon_req->rep_detail);
  rsi_dbg(ERR_ZONE, "BEACON REQ SSID INFORMATION %s\n", beacon_req->ssid);
  rsi_dbg(ERR_ZONE, "beacon_req measurement mode %d\n", beacon_req->meas_mode);
  if (params.meas_mode == 1) {
    rsi_dbg(ERR_ZONE, "active beacon request with bgscan probe\n");
    memcpy(beacon_req->bgscan_probe, common->bgscan_probe_req, MIN_802_11_HDR_LEN);
    index += MIN_802_11_HDR_LEN;

    memcpy(&beacon_req->bgscan_probe[index], (u8 *)&params.ssid_ie, (2 + params.ssid_ie[1]));
    index += (2 + params.ssid_ie[1]);

    memcpy(&beacon_req->bgscan_probe[index],
           common->bgscan_probe_req + MIN_802_11_HDR_LEN + 2,
           (common->bgscan_probe_req_len - MIN_802_11_HDR_LEN - 2));
    index += (common->bgscan_probe_req_len - MIN_802_11_HDR_LEN - 2);
  }

  beacon_req->desc_word[0] |= cpu_to_le16(sizeof(*beacon_req) + index);
  length = FRAME_DESC_SZ + sizeof(*beacon_req) + index;
  rsi_hex_dump(INFO_ZONE, "BEACON FRAME HOST TO LMAC", skb->data, length);
  skb_put(skb, FRAME_DESC_SZ + sizeof(*beacon_req) + index);

  return rsi_send_internal_mgmt_frame(common, skb);
}
#endif

#ifdef CONFIG_RSI_WOW
int rsi_send_wowlan_request(struct rsi_common *common, u16 flags, u16 sleep_status)
{
  struct rsi_wowlan_req *cmd_frame;
  struct sk_buff *skb;
  u8 length;
  u8 sourceid[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  rsi_dbg(ERR_ZONE, "%s: Sending wowlan request frame\n", __func__);

  skb = dev_alloc_skb(sizeof(*cmd_frame));
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }
  memset(skb->data, 0, sizeof(*cmd_frame));
  cmd_frame = (struct rsi_wowlan_req *)skb->data;

  cmd_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  cmd_frame->desc_word[1] |= cpu_to_le16(WOWLAN_CONFIG_PARAMS);

  memcpy(cmd_frame->sourceid, &sourceid, IEEE80211_ADDR_LEN);

  cmd_frame->host_sleep_status = sleep_status;
  if (common->secinfo.gtk_cipher)
    flags |= RSI_WOW_GTK_REKEY;
  if (sleep_status)
    cmd_frame->wow_flags = flags; /* TODO: check for magic packet */
  rsi_dbg(INFO_ZONE, "Host_Sleep_Status : %d Flags : %d\n", cmd_frame->host_sleep_status, cmd_frame->wow_flags);

  length = FRAME_DESC_SZ + IEEE80211_ADDR_LEN + 2 + 2;

  cmd_frame->desc_word[0] |= cpu_to_le16(length - FRAME_DESC_SZ);
  cmd_frame->desc_word[2] |= cpu_to_le16(0);

  skb_put(skb, length);

  return rsi_send_internal_mgmt_frame(common, skb);
}
#endif

#ifndef CONFIG_STA_PLUS_AP
int rsi_send_beacon(struct rsi_common *common)
#else
int rsi_send_beacon(struct rsi_common *common, struct ieee80211_vif *vif)
#endif
{
  struct sk_buff *skb  = NULL;
  u8 dword_align_bytes = 0;

  skb = dev_alloc_skb(MAX_MGMT_PKT_SIZE);
  if (!skb)
    return -ENOMEM;

  memset(skb->data, 0, MAX_MGMT_PKT_SIZE);

  dword_align_bytes = ((unsigned long)skb->data & 0x3f);
  if (dword_align_bytes) {
    skb_pull(skb, (64 - dword_align_bytes));
  }
#ifndef CONFIG_STA_PLUS_AP
  if (rsi_prepare_beacon(common, skb)) {
#else
  if (rsi_prepare_beacon(common, skb, vif)) {
#endif
    rsi_dbg(ERR_ZONE, "Failed to prepare beacon\n");
    return -EINVAL;
  }
  skb_queue_tail(&common->tx_queue[MGMT_BEACON_Q], skb);
  rsi_set_event(&common->tx_thread.event);
  rsi_dbg(DATA_TX_ZONE, "%s: Added to beacon queue\n", __func__);

  return 0;
}

int rsi_update_wlan_gain_table(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload_len)
{
  struct rsi_common *common = adapter->priv;
  struct sk_buff *skb;
  u16 frame_len                    = payload_len;
  struct rsi_mac_frame *gain_table = NULL;

  rsi_dbg(INT_MGMT_ZONE, "<=== Update WLAN Gain table ===>\n");
  if (common->acx_module == true) {
    rsi_dbg(ERR_ZONE, "ERROR : Update WLAN Gain table not supported for ACx module \n");
    return -EINVAL;
  }

  skb = dev_alloc_skb(FRAME_DESC_SZ + frame_len);
  if (!skb)
    return -ENOMEM;

  memset(skb->data, 0, FRAME_DESC_SZ + frame_len);

  gain_table = (struct rsi_mac_frame *)skb->data;

  gain_table->desc_word[0] = cpu_to_le16(frame_len | (RSI_WIFI_MGMT_Q << 12));
  gain_table->desc_word[1] = cpu_to_le16(WLAN_GAIN_TABLE_UPDATE);
  skb->priority            = MGMT_SOFT_Q;
  memcpy(&skb->data[FRAME_DESC_SZ], nlmsg_data(nlh) + FRAME_DESC_SZ, payload_len);
  skb_put(skb, frame_len + FRAME_DESC_SZ);

  rsi_hex_dump(INT_MGMT_ZONE, "GAIN TABLE UPDATE", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 20, 17) || defined(OFFLOAD_SCAN_TO_DEVICE)
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void channel_change_event(unsigned long priv)
{
  struct rsi_hw *adapter    = (struct rsi_hw *)priv;
  struct rsi_common *common = adapter->priv;
#else
static void channel_change_event(struct timer_list *t)
{
  struct rsi_common *common = from_timer(common, t, scan_timer);
#endif

  rsi_set_event(&common->chan_change_event);
  del_timer(&common->scan_timer);
}

static int init_channel_timer(struct rsi_hw *adapter, u32 timeout)
{
  struct rsi_common *common = adapter->priv;

  rsi_reset_event(&common->chan_change_event);
  if (timer_pending(&common->scan_timer)) {
    rsi_dbg(ERR_ZONE, "%s : Timer Pending. This Case Should not occur\n", __func__);
    del_timer(&common->scan_timer);
  }
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
  init_timer(&common->scan_timer);
  common->scan_timer.data     = (unsigned long)adapter;
  common->scan_timer.function = (void *)&channel_change_event;
#else
  timer_setup(&common->scan_timer, channel_change_event, 0);
#endif
  common->scan_timer.expires = msecs_to_jiffies(timeout) + jiffies;

  add_timer(&common->scan_timer);

  return 0;
}
#endif
/**
 * This function prepares Probe request frame and send it to LMAC.
 * @param  Pointer to Adapter structure.
 * @param  Broadcast probe.
 * @param  Pointer to the destination address.
 * @param  Indicates LMAC/UMAC Q number.
 * @return 0 if success else -1.
 */
int rsi_send_probe_request(struct rsi_common *common, struct cfg80211_scan_request *scan_req, u8 n_ssid, u8 channel)
{
  struct cfg80211_ssid *ssid_info;
  struct ieee80211_tx_info *info;
  struct skb_info *tx_params;
  struct sk_buff *skb       = NULL;
  struct ieee80211_hdr *hdr = NULL;
  u8 *pos                   = NULL;
  u32 len                   = 0;
  u8 ie_ssid_len;
  u8 q_num;

#ifdef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *sta_vif = rsi_get_sta_vif(common->priv);
  if (!sta_vif) {
    rsi_dbg(ERR_ZONE, "%s: sta_vif is NULL\n", __func__);
    return 0;
  }
#endif
  if (common->priv->sc_nvifs <= 0)
    return 0;
  if (!scan_req)
    return 0;
  rsi_dbg(INT_MGMT_ZONE, "<==== Sending Probe Request Packet ====>\n");
  ssid_info   = &scan_req->ssids[n_ssid];
  ie_ssid_len = (ssid_info && ssid_info->ssid_len) ? ssid_info->ssid_len + 2 : NULL_SSID;

  len = (MIN_802_11_HDR_LEN + scan_req->ie_len + ie_ssid_len);

  skb = dev_alloc_skb(len + DWORD_ALIGNMENT); /* 64 for dword alignment */
  if (!skb) {
    rsi_dbg(ERR_ZONE, "Failed to alloc probe req\n");
    return -ENOMEM;
  }
  skb_put(skb, len);
  memset(skb->data, 0, skb->len);
  skb_reserve(skb, DWORD_ALIGNMENT);

  pos = skb->data;
  /*
	* probe req frame format
	* ssid
	* supported rates
	* RSN (optional)
	* extended supported rates
	* WPA (optional)
	* user-specified ie's
	*/

  hdr                = (struct ieee80211_hdr *)skb->data;
  hdr->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_PROBE_REQ);
  hdr->duration_id   = 0x0;
  memset(hdr->addr1, 0xff, ETH_ALEN);
  memset(hdr->addr3, 0xFF, 6);
#ifndef CONFIG_STA_PLUS_AP
  memcpy(hdr->addr2, common->mac_addr, ETH_ALEN);
#else
  memcpy(hdr->addr2, sta_vif->addr, ETH_ALEN);
#endif
  hdr->seq_ctrl = 0x00;
  pos += MIN_802_11_HDR_LEN;

  *pos++ = WLAN_EID_SSID;
  *pos++ = ie_ssid_len ? ie_ssid_len - 2 : 0;

  if (ssid_info && (ie_ssid_len != NULL_SSID)) {
    if (scan_req->n_ssids > 0 || common->p2p_enabled) {
      memcpy(pos, ssid_info->ssid, ie_ssid_len - NULL_SSID);
      pos += ie_ssid_len - NULL_SSID;
    } else {
      memcpy(pos, ssid_info->ssid, ie_ssid_len);
      pos += ie_ssid_len;
    }
  }

  if (scan_req->ie_len)
    memcpy(pos, scan_req->ie, scan_req->ie_len);

  if ((common->iface_down == true))
    goto out;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 20, 17) || defined(OFFLOAD_SCAN_TO_DEVICE)
  if (!common->scan_in_prog)
    goto out;
#endif

  info                         = IEEE80211_SKB_CB(skb);
  tx_params                    = (struct skb_info *)info->driver_data;
  tx_params->internal_hdr_size = skb_headroom(skb);
#ifndef CONFIG_RSI_P2P
#ifndef CONFIG_STA_PLUS_AP
  info->control.vif = common->priv->vifs[0];
#else
  info->control.vif = sta_vif;
#endif
  if (!info->control.vif) {
    rsi_dbg(ERR_ZONE, "%s: VIF is NULL\n", __func__);
    return 0;
  }
#else
  info->control.vif = common->priv->vifs[1];
  if (!info->control.vif) {
    rsi_dbg(ERR_ZONE, "%s: VIF is NULL\n", __func__);
    return 0;
  }
  memcpy(hdr->addr2, info->control.vif->addr, ETH_ALEN);
  skb_trim(skb, skb->len - INVALID_DATA);
#endif
  q_num         = MGMT_SOFT_Q;
  skb->priority = q_num;

  if (rsi_prepare_mgmt_desc(common, skb))
    goto out;
  rsi_hex_dump(MGMT_DEBUG_ZONE, "PROBE-REQ FRAME", skb->data, skb->len);
  skb_queue_tail(&common->tx_queue[MGMT_SOFT_Q], skb);
  rsi_set_event(&common->tx_thread.event);

  return 0;

out:
  dev_kfree_skb(skb);
  return 0;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 20, 17) || defined(OFFLOAD_SCAN_TO_DEVICE)
void rsi_scan_start(struct work_struct *work)
{
  struct ieee80211_channel *cur_chan     = NULL;
  struct cfg80211_scan_request *scan_req = NULL;
  struct rsi_common *common              = container_of(work, struct rsi_common, scan_work);
  u8 ii, jj;
  int status = 0, num_scan_chan;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
  struct cfg80211_scan_info info;
#endif
  struct rsi_hw *adapter = common->priv;
#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = adapter->vifs[adapter->sc_nvifs - 1];
#else
  struct ieee80211_vif *vif = common->scan_vif;
#endif

  scan_req = common->scan_request;
  if (!scan_req)
    return;

  common->scan_in_prog = true;
#ifndef CONFIG_RSI_P2P
  if (!adapter->rx_stats_inprog && adapter->ps_state == PS_ENABLED) {
    rsi_disable_ps(common->priv);
    rsi_reset_event(&common->mgmt_cfm_event);
    rsi_wait_event(&common->mgmt_cfm_event, msecs_to_jiffies(2000));
  }
#endif
  if (common->efuse_map.module_type == ACX_MODULE) {
    adapter->dfs_region = NL80211_DFS_FCC - 1;
    num_scan_chan       = MAX_CHAN_FOR_ACX;
  } else
    num_scan_chan = scan_req->n_channels;

  for (ii = 0; ii < num_scan_chan; ii++) {
    if (common->iface_down)
      break;

    if (!common->scan_in_prog)
      break;

    if (common->efuse_map.module_type == ACX_MODULE)
      cur_chan = (struct ieee80211_channel *)&rsi_2ghz_acx_channels[ii];
    else
      cur_chan = scan_req->channels[ii];

    rsi_band_check(common, cur_chan->band);
    if (cur_chan->flags & IEEE80211_CHAN_DISABLED)
      continue;

#ifndef CONFIG_STA_PLUS_AP
    status = rsi_set_channel(common, cur_chan);
#else
    status = rsi_set_channel(common, cur_chan, vif);
#endif
    if (status == -EOPNOTSUPP) {
      rsi_dbg(ERR_ZONE, "Failed to set the channel for ACx\n");
      init_channel_timer(common->priv, ACTIVE_SCAN_DURATION);
      goto SKIP_PROBE;
    } else if (status) {
      rsi_dbg(ERR_ZONE, "Failed to set the channel\n");
      break;
    }
    rsi_reset_event(&common->chan_set_event);
    status = rsi_wait_event(&common->chan_set_event, msecs_to_jiffies(50));
    if (status < 0)
      break;

    if (!common->scan_in_prog)
      break;
    rsi_reset_event(&common->chan_set_event);

    if (vif->type == NL80211_IFTYPE_AP && adapter->auto_chan_sel) {
      init_channel_timer(common->priv, PASSIVE_SCAN_DURATION);
    } else {
      if ((cur_chan->flags & IEEE80211_CHAN_NO_IR) || (cur_chan->flags & IEEE80211_CHAN_RADAR)) {
        /* DFS Channel */
        /* Program passive scan duration */
        init_channel_timer(common->priv, PASSIVE_SCAN_DURATION);
      } else if (!adapter->auto_chan_sel) {
        /* Send probe request */
        for (jj = 0; jj < scan_req->n_ssids; jj++) {
          rsi_send_probe_request(common, scan_req, jj, cur_chan->hw_value);
          if (common->iface_down == true) {
            common->scan_in_prog = false;
            return;
          }
          rsi_reset_event(&common->probe_cfm_event);
          status = rsi_wait_event(&common->probe_cfm_event, msecs_to_jiffies(50));
          if (status < 0) {
            rsi_dbg(ERR_ZONE, "Did not received probe confirm\n");
            common->scan_in_prog = false;
            return;
          }
          rsi_reset_event(&common->probe_cfm_event);
        }
        init_channel_timer(common->priv, ACTIVE_SCAN_DURATION);
      }
    }
SKIP_PROBE:
    if (!common->scan_in_prog)
      break;
    if (common->iface_down)
      break;

    status = rsi_wait_event(&common->chan_change_event, EVENT_WAIT_FOREVER);
    if (status < 0)
      break;
    rsi_reset_event(&common->chan_change_event);
  }

  del_timer(&common->scan_timer);
  common->scan_in_prog = false;
#ifndef CONFIG_RSI_P2P
  if (common->default_deep_sleep_enable && !adapter->rx_stats_inprog && vif->type != NL80211_IFTYPE_AP
      && adapter->ps_state == PS_NONE) {
    rsi_enable_ps(common->priv);
    rsi_reset_event(&common->mgmt_cfm_event);
    rsi_wait_event(&common->mgmt_cfm_event, msecs_to_jiffies(2000));
  }
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
  info.aborted = false;
  ieee80211_scan_completed(common->priv->hw, &info);
#else
  ieee80211_scan_completed(common->priv->hw, false);
#endif

  if (common->hw_scan_cancel) {
    skb_queue_purge(&common->tx_queue[MGMT_SOFT_Q]);
    rsi_set_event(&common->cancel_hw_scan_event);
  }
  return;
}
#endif

int rsi_send_w9116_features(struct rsi_common *common)
{
  struct rsi_wlan_9116_features *w9116_features;
  u16 frame_len = sizeof(struct rsi_wlan_9116_features);
  struct sk_buff *skb;

  rsi_dbg(INT_MGMT_ZONE, "<==== Sending wlan 9116 features ====>\n");

  skb = dev_alloc_skb(frame_len);
  if (!skb)
    return -ENOMEM;
  memset(skb->data, 0, frame_len);

  w9116_features = (struct rsi_wlan_9116_features *)skb->data;

  w9116_features->pll_mode      = common->w9116_features.pll_mode;
  w9116_features->rf_type       = common->w9116_features.rf_type;
  w9116_features->wireless_mode = common->w9116_features.wireless_mode;
  w9116_features->enable_ppe    = common->w9116_features.enable_ppe;
  w9116_features->afe_type      = common->w9116_features.afe_type;
  if (common->w9116_features.dpd)
    w9116_features->feature_enable |= cpu_to_le32(RSI_DPD);
  if (common->w9116_features.sifs_tx_enable)
    w9116_features->feature_enable |= cpu_to_le32(RSI_SIFS_TX_ENABLE);
  if (common->w9116_features.ps_options & RSI_DUTY_CYCLING)
    w9116_features->feature_enable |= cpu_to_le32(RSI_DUTY_CYCLING);
  if (common->w9116_features.ps_options & RSI_END_OF_FRAME)
    w9116_features->feature_enable |= cpu_to_le32(RSI_END_OF_FRAME);
  w9116_features->feature_enable |= cpu_to_le32(common->wlan_pwrsave_options);
  if (w9116_features->feature_enable & LMAC_BCON_DROP_EN) {
    w9116_features->feature_enable |= cpu_to_le32(DROP_BYTES_FEATURE);
    w9116_features->feature_enable |= cpu_to_le32(LMAC_BCON_EN_DIS_THRESHOLD << 28);
  }

  w9116_features->desc_word[0] = cpu_to_le16((frame_len - FRAME_DESC_SZ) | (RSI_WIFI_MGMT_Q << 12));
  w9116_features->desc_word[1] = cpu_to_le16(FEATURES_ENABLE);
  skb_put(skb, frame_len);
  rsi_hex_dump(INT_MGMT_ZONE, "Internal Mgmt Pkt", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(common, skb);
}

int rsi_send_bt_reg_params(struct rsi_common *common)
{
  struct sk_buff *skb;
  struct bt_register_param *bt_reg_param;

  rsi_dbg(ERR_ZONE, "%s: Sending BT reg frame\n", __func__);
  skb = dev_alloc_skb(sizeof(*bt_reg_param));
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }
#define BT_PKT_TYPE 0x55
  memset(skb->data, 0, sizeof(struct bt_register_param));
  bt_reg_param = (struct bt_register_param *)skb->data;

  if (common->is_915 && (common->ble_tx_pwr_inx <= 63)) {
    rsi_dbg(ERR_ZONE, "%s : Unsupported ble_tx_pwr_inx : %d for Si915 module \n", __func__, common->ble_tx_pwr_inx);
    return -EINVAL;
  }

  bt_reg_param->desc_word[7] = cpu_to_le16(BT_PKT_TYPE);
  bt_reg_param->desc_word[0] = (RSI_BT_MGMT_Q << 12);
  if (COMMON_DEV_MODEL_9116) {
    bt_reg_param->desc_word[0] |= cpu_to_le16((sizeof(*bt_reg_param) - FRAME_DESC_SZ));
    bt_reg_param->params[2]          = (common->bt_rf_type & 0x0F);
    bt_reg_param->params[3]          = common->ble_tx_pwr_inx;
    bt_reg_param->params[4]          = (common->ble_pwr_save_options & 0x0F);
    *(u32 *)&bt_reg_param->params[5] = (common->ble_vendor_feature_bitmap);
    skb_put(skb, sizeof(struct bt_register_param));
  } else {
    bt_reg_param->desc_word[0] |= cpu_to_le16((sizeof(*bt_reg_param) - FRAME_DESC_SZ - 3));
    bt_reg_param->params[0] = (common->bt_rf_tx_power_mode & 0x0F);
    bt_reg_param->params[0] |= (common->bt_rf_rx_power_mode & 0xF0);
    skb_put(skb, sizeof(struct bt_register_param) - 3);
  }

  rsi_hex_dump(ERR_ZONE, "BT REG PARAMS", skb->data, skb->len);
#ifdef CONFIG_RSI_COEX_MODE
  return rsi_coex_send_pkt(common, skb, BT_Q);
#else
  return rsi_send_bt_pkt(common, skb);
#endif
}

/**
 * rsi_handle_ta_confirm() - This function handles the confirm frames.
 * @common: Pointer to the driver private structure.
 * @msg: Pointer to received packet.
 *
 * Return: 0 on success, -1 on failure.
 */
static int rsi_handle_ta_confirm(struct rsi_common *common, u8 *msg)
{
  struct rsi_hw *adapter = common->priv;
  u8 sub_type            = (msg[15] & 0xff);
  u32 msg_len            = (((u16 *)msg)[0]) & 0xfff;
  int no_of_fragments;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
  struct cfg80211_scan_info info;
#endif
#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = adapter->vifs[0];
#else
  struct ieee80211_vif *vif = rsi_get_first_valid_vif(adapter);
#endif

  rsi_dbg(MGMT_RX_ZONE, "%s: subtype=%d\n", __func__, sub_type);

  switch (sub_type) {
    case COMMON_DEV_CONFIG:
      rsi_dbg(FSM_ZONE, "Common Dev Config params confirm received\n");
      if (common->fsm_state == FSM_COMMON_DEV_PARAMS_SENT) {
        if (rsi_load_bootup_params(common)) {
          common->fsm_state = FSM_CARD_NOT_READY;
          goto out;
        } else {
          common->fsm_state = FSM_BOOT_PARAMS_SENT;
        }
      } else {
        rsi_dbg(INFO_ZONE, "%s: Received common dev config params cfm in %d state\n", __func__, common->fsm_state);
        return 0;
      }
      break;

    case BOOTUP_PARAMS_REQUEST:
      rsi_dbg(FSM_ZONE, "Bootup params confirmation.\n");
      if (common->fsm_state == FSM_BOOT_PARAMS_SENT) {
        if (DEV_MODEL_9116) {
          if (common->band == NL80211_BAND_5GHZ)
            common->num_supp_bands = 2;
          else
            common->num_supp_bands = 1;

          if (rsi_send_reset_mac(common))
            goto out;
          else
            common->fsm_state = FSM_RESET_MAC_SENT;
        } else {
          adapter->eeprom.length = (IEEE80211_ADDR_LEN + WLAN_MAC_MAGIC_WORD_LEN + WLAN_HOST_MODE_LEN);
          adapter->eeprom.offset = WLAN_MAC_EEPROM_ADDR;
          if (rsi_eeprom_read(common)) {
            common->fsm_state = FSM_CARD_NOT_READY;
            goto out;
          } else
            common->fsm_state = FSM_EEPROM_READ_MAC_ADDR;
        }
      } else {
        rsi_dbg(INFO_ZONE, "%s: Received bootup params cfm in %d state\n", __func__, common->fsm_state);
        return 0;
      }
      break;

    case EEPROM_READ:
      rsi_dbg(FSM_ZONE, "EEPROM READ confirm received\n");
      if (common->fsm_state == FSM_EEPROM_READ_MAC_ADDR) {
        u32 msg_len = ((u16 *)msg)[0] & 0xfff;

        if (msg_len <= 0) {
          rsi_dbg(FSM_ZONE, "%s: [EEPROM_READ] Invalid len %d\n", __func__, msg_len);
          goto out;
        }
        if (msg[16] == MAGIC_WORD) {
          u8 offset = (FRAME_DESC_SZ + WLAN_HOST_MODE_LEN + WLAN_MAC_MAGIC_WORD_LEN);

          memcpy(common->mac_addr, &msg[offset], IEEE80211_ADDR_LEN);
          rsi_hex_dump(INIT_ZONE, "MAC Addr", common->mac_addr, ETH_ALEN);
          adapter->eeprom.length = ((WLAN_MAC_MAGIC_WORD_LEN + 3) & (~3));
          adapter->eeprom.offset = WLAN_EEPROM_RFTYPE_ADDR;
          if (rsi_eeprom_read(common)) {
            rsi_dbg(ERR_ZONE, "%s: Failed reading RF band\n", __func__);
            common->fsm_state = FSM_CARD_NOT_READY;
          } else {
            common->fsm_state = FSM_EEPROM_READ_RF_TYPE;
          }
        } else {
          common->fsm_state = FSM_CARD_NOT_READY;
          break;
        }
      } else if (common->fsm_state == FSM_EEPROM_READ_RF_TYPE) {
        u32 msg_len = ((u16 *)msg)[0] & 0xfff;

        if (msg_len <= 0) {
          rsi_dbg(FSM_ZONE, "%s:[EEPROM_READ_CFM] Invalid len %d\n", __func__, msg_len);
          goto out;
        }
        if (msg[16] == MAGIC_WORD) {
          if ((msg[17] & 0x3) == 0x3) {
            rsi_dbg(INIT_ZONE, "Dual band supported\n");
            common->band           = NL80211_BAND_5GHZ;
            common->num_supp_bands = 2;
          } else if ((msg[17] & 0x3) == 0x1) {
            rsi_dbg(INIT_ZONE, "Only 2.4Ghz band supported\n");
            common->band           = NL80211_BAND_2GHZ;
            common->num_supp_bands = 1;
          }
        } else {
          common->fsm_state = FSM_CARD_NOT_READY;
          break;
        }
        if (rsi_send_reset_mac(common))
          goto out;
        else
          common->fsm_state = FSM_RESET_MAC_SENT;
      } else {
        rsi_dbg(ERR_ZONE, "%s: Received eeprom read in %d state\n", __func__, common->fsm_state);
        return 0;
      }
      break;

    case RESET_MAC_REQ:
      if (common->fsm_state == FSM_RESET_MAC_SENT) {
        rsi_dbg(FSM_ZONE, "Reset MAC confirm\n");

        if (rsi_load_radio_caps(common))
          goto out;
        else
          common->fsm_state = FSM_RADIO_CAPS_SENT;
      } else {
        rsi_dbg(ERR_ZONE, "%s: Received reset mac cfm in %d state\n", __func__, common->fsm_state);
        return 0;
      }
      break;

    case RADIO_CAPABILITIES:
      if (common->fsm_state == FSM_RADIO_CAPS_SENT) {
        common->rf_reset = 1;
        if ((DEV_MODEL_9116) && rsi_send_w9116_features(common)) {
          rsi_dbg(ERR_ZONE, "Failed to send 9116 features\n");
          goto out;
        }
        if (rsi_program_bb_rf(common)) {
          goto out;
        } else {
          common->fsm_state = FSM_BB_RF_PROG_SENT;
          rsi_dbg(FSM_ZONE, "Radio caps confirm\n");
        }
      } else {
        rsi_dbg(INFO_ZONE, "%s: Received radio caps cfm in %d state\n", __func__, common->fsm_state);
        return 0;
      }
      break;

    case BB_BUF_PROG_VALUES_REQ:
      rsi_dbg(INFO_ZONE, "BBP_USING_BUFFER: Utils BUF confirm Received: msg_len : %d \n", msg_len);

      if ((msg_len) > 0) {
        rsi_hex_dump(INFO_ZONE, "Message", &msg[FRAME_DESC_SZ], msg_len);
        memcpy((&adapter->bb_rf_read.Data[0]), &msg[FRAME_DESC_SZ], msg_len);
        adapter->bb_rf_read.no_of_values = msg_len / 2;
        rsi_dbg(FSM_ZONE,
                "BBP PROG STATS: msg_len is : %d no_of_vals is %d \n",
                msg_len,
                adapter->bb_rf_read.no_of_values);
        if ((rsi_bb_prog_data_to_app(adapter)) < 0)
          return -1;
        rsi_dbg(INFO_ZONE, "%s : Success in Performing operation\n", __func__);
      }
      break;

    case SOC_REG_OPS:
      rsi_dbg(FSM_ZONE, "SOC_REG_OPS: SOC REG Read confirm Received: msg_len: %d \n", msg_len);
      if (msg_len > 0) {
        rsi_hex_dump(ERR_ZONE, "Message", &msg[FRAME_DESC_SZ], msg_len);
        memcpy((&adapter->bb_rf_read.Data[0]), &msg[FRAME_DESC_SZ], msg_len);
        adapter->bb_rf_read.no_of_values = msg_len / 2;
        rsi_dbg(FSM_ZONE,
                "SOC_REG_OPS: msg_len is : %d no_of_vals is %d \n",
                msg_len,
                adapter->bb_rf_read.no_of_values);
        if ((rsi_bb_prog_data_to_app(adapter)) < 0)
          return -1;
        rsi_dbg(INFO_ZONE, "%s : Success in Performing operation\n", __func__);
      }
      break;
    case BB_PROG_VALUES_REQUEST:
      rsi_dbg(FSM_ZONE, "BBP PROG STATS: Utils BB confirm Received: msg_len: %d \n", msg_len);
      if (msg_len > 0) {
        rsi_hex_dump(ERR_ZONE, "Message", &msg[FRAME_DESC_SZ], msg_len);
        memcpy((&adapter->bb_rf_read.Data[0]), &msg[FRAME_DESC_SZ], msg_len);
        adapter->bb_rf_read.no_of_values = msg_len / 2;
        rsi_dbg(FSM_ZONE,
                "BBP PROG STATS: msg_len is : %d no_of_vals is %d \n",
                msg_len,
                adapter->bb_rf_read.no_of_values);
        if ((rsi_bb_prog_data_to_app(adapter)) < 0)
          return -1;
        rsi_dbg(INFO_ZONE, "%s : Success in Performing operation\n", __func__);
      }
      break;
    case RF_PROG_VALUES_REQUEST:
    case BBP_PROG_IN_TA:
      rsi_dbg(FSM_ZONE, "BB/RF confirmation.\n");
      if (common->fsm_state == FSM_BB_RF_PROG_SENT) {
        common->bb_rf_prog_count--;
        if (!common->bb_rf_prog_count) {
          common->fsm_state = FSM_MAC_INIT_DONE;
          if (common->reinit_hw) {
            common->hw_data_qs_blocked = false;
            ieee80211_wake_queues(adapter->hw);
            complete(&common->wlan_init_completion);
            common->reinit_hw = false;
          } else {
            if (common->driver_mode != RF_EVAL_MODE_ON)
              return rsi_mac80211_attach(common);
            else
              return 0;
          }
        }
      } else {
        rsi_dbg(INFO_ZONE, "%s: Received bb_rf cfm in %d state\n", __func__, common->fsm_state);
        return 0;
      }
      break;

    case AMPDU_IND:
      rsi_dbg(INFO_ZONE, "AMPDU indication.\n");
      if (common->driver_mode == RF_EVAL_MODE_ON) {
        no_of_fragments = adapter->no_of_per_fragments = adapter->per_params.aggr_count;
        do {
          if (adapter->per_params.enable) {
            rsi_start_per_burst(adapter);
            no_of_fragments--;
          }
        } while (no_of_fragments && adapter->per_params.aggr_enable);
        common->fsm_state   = FSM_MAC_INIT_DONE;
        adapter->tx_running = BURST_RUNNING;
      }
      break;

    case SCAN_REQUEST:
      rsi_dbg(INFO_ZONE, "Scan confirm.\n");
      rsi_hex_dump(INFO_ZONE, "Scan Confirm", &msg[0], 16);
#ifdef CONFIG_STA_PLUS_AP
      if (!vif) {
        /* FIXME: vif can be destroyed just before getting confirm from LMAC */
        rsi_dbg(ERR_ZONE, "Vif is NULL\n");
        break;
      }
#endif
      if ((vif && vif->type == NL80211_IFTYPE_AP && adapter->auto_chan_sel) && (adapter->idx < adapter->n_channels)) {
        u8 id;
        struct acs_stats_s *acs_data = (struct acs_stats_s *)(&msg[FRAME_DESC_SZ]);

        if (adapter->idx < MAX_NUM_CHANS) {
          id = adapter->idx;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
          adapter->rsi_survey[id].time_busy = acs_data->chan_busy_time;
          adapter->rsi_survey[id].time      = ACS_TIMEOUT_TIME;
#else
          adapter->rsi_survey[id].channel_time_busy = acs_data->chan_busy_time;
          adapter->rsi_survey[id].channel_time = ACS_TIMEOUT_TIME;
#endif
          adapter->rsi_survey[id].noise = -abs(acs_data->noise_floor_rssi);
          adapter->idx++;
        }
      }

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 20, 17) || defined(OFFLOAD_SCAN_TO_DEVICE)
      rsi_set_event(&common->chan_set_event);
#else
      if (common->driver_mode == RF_EVAL_MODE_ON)
        rsi_set_event(&common->chan_set_event);
#endif
      break;

    case SET_RX_FILTER:
      rsi_dbg(INFO_ZONE, "RX Filter confirmation.\n");
      break;

    case WAKEUP_SLEEP_REQUEST:
      rsi_dbg(INFO_ZONE, "Wakeup/Sleep confirmation.\n");
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 20, 17) || defined(OFFLOAD_SCAN_TO_DEVICE)
#ifndef CONFIG_RSI_P2P
      rsi_set_event(&common->mgmt_cfm_event);
#endif
#endif
      return rsi_handle_ps_confirm(adapter, msg);

    case BG_SCAN_PROBE_REQ:
      rsi_dbg(INT_MGMT_ZONE, "<==== Received BG Scan Complete Event ===>\n");
      if (common->hwscan_en) {
        if (common->send_initial_bgscan_chan) {
          rsi_send_bgscan_params(common, 1);
          rsi_send_bgscan_probe_req(common);
          common->bgscan_en = 1;
        } else {
          if ((common->bgscan_info.num_user_channels > MAX_BG_CHAN_FROM_USER) && (!common->debugfs_bgscan)) {
            rsi_send_bgscan_params(common, 0);
            common->bgscan_en = 0;
          }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
          info.aborted = false;
          ieee80211_scan_completed(adapter->hw, &info);
#else
          ieee80211_scan_completed(adapter->hw, false);
#endif
          mutex_lock(&common->bgscan_lock);
          common->bgscan_in_prog = false;
          if (common->debugfs_bgscan_en) {
            if (common->debugfs_stop_bgscan) {
              rsi_send_bgscan_params(common, 0);
              common->bgscan_en           = false;
              common->debugfs_stop_bgscan = false;
              common->debugfs_bgscan      = false;
              init_bgscan_params(common);
            } else {
              rsi_send_bgscan_params(common, 1);
              common->bgscan_en = true;
            }
            common->debugfs_bgscan_en = false;
          }
          mutex_unlock(&common->bgscan_lock);
          common->hwscan_en = false;
        }
#ifdef CONFIG_STA_PLUS_AP
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
      } else if (common->vap_mode == CONCURRENT && rsi_get_sta_vif(adapter)->bss_conf.assoc) {
#else
      } else if (common->vap_mode == CONCURRENT && rsi_get_sta_vif(adapter)->cfg.assoc) {
#endif
        mutex_lock(&common->bgscan_lock);
        common->bgscan_in_prog = false;
        rsi_send_bgscan_params(common, 0);
        common->bgscan_en = false;
        init_bgscan_params(common);
        mutex_unlock(&common->bgscan_lock);
#endif
      }
      if (common->hw_scan_cancel)
        rsi_set_event(&common->cancel_hw_scan_event);
      break;
    case STATS_REQUEST:
      rsi_dbg(MGMT_DEBUG_ZONE, "<==== RECEIVED STATS FRAME FROM LMAC ===>\n");
      memset(&adapter->sta_info, 0, msg_len);
      memcpy((&adapter->sta_info), &msg[16], msg_len);
      rsi_process_rx_stats(adapter);
      break;
    case GET_RSSI_FRAME:
      memcpy(&adapter->rx_rssi, &msg[16], 2);
      send_rssi_to_app(adapter);
      break;
      //__9117_CODE_START
    case DIAG_TOOL_CMD:
      send_diag_tool_response_to_app(adapter, msg[17]);
      break;
      //__9117_CODE_END
    default:
      rsi_dbg(INFO_ZONE, "%s: Invalid TA confirm type : %x\n", __func__, sub_type);
      break;
  }
  return 0;

out:
  rsi_dbg(ERR_ZONE, "%s: Unable to send pkt/Invalid frame received\n", __func__);
  return -EINVAL;
}

/**
 *rsi_handle_card_ready() - This function handles the card ready
 *                       indication from firmware.
 *@common: Pointer to the driver private structure.
 *
 *Return: 0 on success, -1 on failure.
 */
int rsi_handle_card_ready(struct rsi_common *common, u8 *msg)
{
  u8 rsi_standard_mac[3] = { 0x00, 0x23, 0xa7 };

  switch (common->fsm_state) {
    case FSM_CARD_NOT_READY:
      if (msg[15] & BIT(0)) {
        rsi_dbg(ERR_ZONE, " %s : ACx Module Detected \n", __func__);
        common->acx_module = true;
        if (msg[15] & BIT(1)) {
          rsi_dbg(ERR_ZONE, "[%s] ERROR : Voltage Out of Range \n", __func__);
          common->common_hal_tx_access = true;
          return -EINVAL;
        }
      }

      if (msg[15] & BIT(2)) {
        rsi_dbg(ERR_ZONE, "%s : Si915 Module Detected \n", __func__);
        common->is_915 = true;
      }

      rsi_dbg(INIT_ZONE, "Card ready indication from Common HAL\n");
      common->common_hal_tx_access = true;
      rsi_set_default_parameters(common);
      if (rsi_send_common_dev_params(common) < 0)
        return -EINVAL;
      common->fsm_state = FSM_COMMON_DEV_PARAMS_SENT;
      break;
    case FSM_COMMON_DEV_PARAMS_SENT:
      rsi_dbg(INIT_ZONE, "Card ready indication from WLAN HAL\n");

      if (COMMON_DEV_MODEL_9116) {
        if ((msg[16] != MAGIC_WORD) || ((msg[16] == MAGIC_WORD) && (msg[21] == 0x00))) {
          memcpy(common->mac_addr, rsi_standard_mac, RSI_MAC_SUB_LEN);
          get_random_bytes(&common->mac_addr[3], RSI_MAC_SUB_LEN);
          common->band = NL80211_BAND_5GHZ;
        } else {
          memcpy(common->mac_addr, &msg[20], ETH_ALEN);
          /* RSI device supports four Virtual ifaces each require a unique MAC addr,
	   * here we check the received MAC addr should be less than the maximum
	   * possible Sub MAC addr[FF:FF:FC]*/
          if (common->mac_addr[3] == 0xff && common->mac_addr[4] == 0xff && common->mac_addr[5] > 0xfc) {
            rsi_dbg(ERR_ZONE, "[%s][%d]ERROR:Sub MAC addr greater than FF:FF:FC\n", __func__, __LINE__);
            return -EINVAL;
          }
          common->band = (msg[27] == RSI_BAND_CHECK ? NL80211_BAND_5GHZ : NL80211_BAND_2GHZ);
        }
        rsi_hex_dump(INIT_ZONE, "MAC Addr", common->mac_addr, ETH_ALEN);

        if ((msg[FRAME_DESC_SZ] == MAGIC_WORD) && (msg[7] & BIT(0))) {
          common->efuse_map.efuse_map_version = msg[EFUSE_MAP_VERSION_OFFSET];
          common->efuse_map.module_version    = msg[MODULE_VERSION_OFFSET];
          common->efuse_map.module_part_no    = *(u16 *)&msg[MODULE_PART_NO_OFFSET];
          common->efuse_map.mfg_sw_version    = msg[MFG_SW_VERSION_OFFSET];
          common->efuse_map.module_type       = msg[MODULE_TYPE_OFFSET];
          common->efuse_map.chip_version      = msg[CHIP_VERSION_OFFSET];
          common->efuse_map.m4sb_cert_en      = msg[HW_CONFIG_BITMAP_OFFSET] & BIT(3);
          common->efuse_map.mfg_sw_subversion = msg[MANF_SW_SUB_VERSION_OFFSET];
          common->efuse_map.chip_id_no        = *(u32 *)&msg[CHIP_ID_NO_OFFSET];
          common->efuse_map.sdb_mode          = *(u16 *)&msg[SDB_MODE_OFFSET];
        }
      }
      /* Get usb buffer status register address */
      common->priv->usb_buffer_status_reg = *(u32 *)&msg[8];
      rsi_dbg(INFO_ZONE, "USB buffer status register = %x\n", common->priv->usb_buffer_status_reg);

      if (rsi_load_bootup_params(common)) {
        common->fsm_state = FSM_CARD_NOT_READY;
        return -EINVAL;
      }
      common->fsm_state = FSM_BOOT_PARAMS_SENT;
      break;
    default:
      rsi_dbg(ERR_ZONE, "%s: card ready indication in invalid state %d.\n", __func__, common->fsm_state);
      return -EINVAL;
  }

  return 0;
}

/*
 * rsi_send_ack_for_ulp_entry() - ULP sleep ack is prepared in this
 * function.
 * @common: Pointer to the device private structure.
 */
int rsi_send_ack_for_ulp_entry(struct rsi_common *common)
{
  struct rsi_ulp_params *ulp_params;
  struct sk_buff *skb;
  int status = -EINVAL;

  skb = dev_alloc_skb(sizeof(*ulp_params));
  if (!skb)
    return -ENOMEM;
  memset(skb->data, 0, sizeof(*ulp_params));
  ulp_params               = (struct rsi_ulp_params *)skb->data;
  ulp_params->desc_word[1] = cpu_to_le16(CONFIRM);
  ulp_params->desc_word[6] = common->ulp_token;
  ulp_params->desc_word[7] = cpu_to_le16(ULP_SLEEP_NOTIFY << 8);
  skb_put(skb, sizeof(struct rsi_ulp_params));
  common->ulp_sleep_ack_sent = true;
  rsi_dbg(INFO_ZONE, "%s: ACK sent for ULP sleep entry \n", __func__);
  status = common->priv->host_intf_ops->write_pkt(common->priv, skb->data, skb->len);
  dev_kfree_skb(skb);
  return status;
}
EXPORT_SYMBOL_GPL(rsi_send_ack_for_ulp_entry);

/*
 * rsi_resolve_ulp_request() - This function serves the ulp request came
 * from lmac and decides wheather to send ack or not.
 * @common: Pointer to the device private structure.
 */
static void rsi_process_ulp_sleep_notify(struct rsi_common *common, u8 *msg)
{
  u8 proto_id = 0;
  if ((msg[12] & BIT(0)) == ULP_SLEEP_ENTRY) {
    rsi_dbg(INFO_ZONE, "%s: ULP sleep entry received\n", __func__);
    down(&common->tx_access_lock);
    common->sleep_entry_received = true;
    common->ulp_token            = ULP_TOKEN;
    if (!protocol_tx_access(common)) {
      common->common_hal_tx_access = false;
      rsi_send_ack_for_ulp_entry(common);
    } else {
      rsi_dbg(INFO_ZONE, "%s:ULP sleep entry without sleep exit\n", __func__);
      common->common_hal_tx_access = true;
      common->ulp_sleep_ack_sent   = false;
      while (proto_id < MAX_IDS) {
        if (common->techs[proto_id].wait_for_tx_access) {
          common->techs[proto_id].wait_for_tx_access = 0;
          rsi_dbg(INFO_ZONE, "Waking up Event for proto %d\n", proto_id);
          wake_up(&common->techs[proto_id].tx_access_event);
        }
        proto_id++;
      }
    }
    up(&common->tx_access_lock);
  } else {
    rsi_dbg(INFO_ZONE, "%s: ULP sleep exit received\n", __func__);
    common->ulp_token = 0xABCD;
    sleep_exit_recvd(common);
  }
}

/**
 * rsi_mgmt_pkt_recv() - This function processes the management packets
 *			 received from the hardware.
 * @common: Pointer to the driver private structure.
 * @msg: Pointer to the received packet.
 *
 * Return: 0 on success, -1 on failure.
 */
int rsi_mgmt_pkt_recv(struct rsi_common *common, u8 *msg)
{
  struct rsi_hw *adapter = common->priv;
  s32 msg_len            = (le16_to_cpu(*(__le16 *)&msg[0]) & 0x0fff);
  u16 msg_type           = msg[2];
  u8 hw_bmiss_for_channel_change;
#ifndef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = adapter->vifs[0];
#else
  struct ieee80211_vif *sta_vif = rsi_get_sta_vif(adapter);
  struct ieee80211_vif *vif = NULL;
  u8 vap_id;
#endif

  switch (msg_type) {
    case TA_CONFIRM_TYPE:
      return rsi_handle_ta_confirm(common, msg);

    case CARD_READY_IND:
      rsi_dbg(INIT_ZONE, "CARD READY INDICATION FROM WLAN.\n");
      return rsi_handle_card_ready(common, msg);

    case TX_STATUS_IND:
      if (msg[15] == PROBEREQ_CONFIRM) {
        rsi_dbg(INT_MGMT_ZONE, "<==== Recv PROBEREQ CONFIRM =====>\n");
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 20, 17) || defined(OFFLOAD_SCAN_TO_DEVICE)
        rsi_set_event(&common->probe_cfm_event);
#endif
      }
      if ((msg[15] & 0xff) == EAPOL4_CONFIRM) {
        u8 status = msg[12];

        if (status) {
#ifndef CONFIG_STA_PLUS_AP
          if (vif->type == NL80211_IFTYPE_STATION) {
#else
          if (sta_vif->type == NL80211_IFTYPE_STATION) {
#endif
            rsi_dbg(ERR_ZONE, "EAPOL 4 confirm\n");
            common->start_bgscan   = 1;
            common->eapol4_confirm = 1;
            if (!rsi_send_block_unblock_frame(common, false))
              common->hw_data_qs_blocked = false;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
            if (adapter->user_ps_en)
              rsi_enable_ps(adapter);
#endif
              //__9117_CODE_START
#ifdef CONFIG_TWT_SUPPORT
            if (adapter->ax_params._11ax_enabled && adapter->ap_support_twt) {
              if (adapter->rsi_twt_config.twt_enable) /*Send twt_setup in Secure Mode */
                send_twt_setup_frame(adapter, 0, NULL);
            }
#endif
            //__9117_CODE_END
          }
        }
      }
      if ((msg[15] & 0xFF) == NULLDATA_CONFIRM) {
        struct sk_buff *skb;
        u8 status = msg[12];
        u8 sta_id = msg[13];
        rsi_dbg(MGMT_DEBUG_ZONE, "NULL DATA CNFM RECVD sta_id = %d\n", sta_id);
#ifndef CONFIG_STA_PLUS_AP
        skb = (vif->type == NL80211_IFTYPE_STATION) ? common->stations[RSI_MAX_ASSOC_STAS].sta_skb
                                                    : common->stations[sta_id].sta_skb;
#else
        skb = common->stations[sta_id].sta_skb;
#endif
        if (status) {
          rsi_indicate_tx_status(common->priv, skb, NULLDATA_SUCCESS);
        } else {
          rsi_indicate_tx_status(common->priv, skb, NULLDATA_FAIL);
        }
      }
//__9117_CODE_START
#ifdef CONFIG_TWT_SUPPORT
      if (((msg[15] & 0xff) == TWT_INFO_FRAME_CONFIRM) && adapter->twt_rescheduling_in_progress) {
        u8 flow_id;
        u16 tokan               = (le16_to_cpu(*(__le16 *)&msg[12]) & 0xffff);
        u16 transmission_status = tokan & 0xF000;
        flow_id                 = tokan & 0x7;
        if (transmission_status)
          adapter->twt_current_status = TWT_RESCHEDULE_SUCC;
        else
          adapter->twt_current_status = TWT_RESCHEDULE_FAIL;

        adapter->twt_rescheduling_in_progress = false;
      }
#endif
      //__9117_CODE_END

      break;

    case PS_NOTIFY_IND:
      rsi_dbg(FSM_ZONE, "Powersave notify indication.\n");
      break;

    case SLEEP_NOTIFY_IND:
      rsi_dbg(FSM_ZONE, "Sleep notify indication.\n");
      if (msg[2] == ULP_SLEEP_NOTIFY)
        rsi_process_ulp_sleep_notify(common, msg);
      break;

    case DECRYPT_ERROR_IND:
      rsi_dbg(INFO_ZONE, "Error in decrypt.\n");
      break;

    case DEBUG_IND:
      rsi_dbg(INFO_ZONE, "Debugging indication.\n");
      break;

    case RX_MISC_IND:
      rsi_dbg(INFO_ZONE, "RX misc indication.\n");
      break;

    case HW_BMISS_EVENT:
      hw_bmiss_for_channel_change = (msg[15] & CHANNEL_CHANGE_EVENT);
      if (hw_bmiss_for_channel_change) {
        rsi_dbg(ERR_ZONE, "<==== Hardware BMISS event due to Channel Change ====>\n");
#ifndef CONFIG_STA_PLUS_AP
        ieee80211_connection_loss(adapter->vifs[adapter->sc_nvifs - 1]);
#else
        ieee80211_connection_loss(rsi_get_sta_vif(adapter));
#endif
      } else {
        rsi_dbg(ERR_ZONE, "<==== Hardware beacon miss event ====>\n");
        rsi_indicate_bcnmiss(common);
#ifndef CONFIG_STA_PLUS_AP
        rsi_resume_conn_channel(common->priv, adapter->vifs[adapter->sc_nvifs - 1]);
#else
        rsi_resume_conn_channel(common->priv, rsi_get_sta_vif(adapter));
#endif
      }
      break;

    case BEACON_EVENT_IND:
#ifdef CONFIG_STA_PLUS_AP
      vap_id = msg[VAP_ID_OFFSET];
      vif    = rsi_get_vif_using_vap_id(adapter, vap_id);
      if (!vif) {
        rsi_dbg(ERR_ZONE, "%s: Vif is NULL", __func__);
        return -ENODEV;
      }

      if (vif->type == NL80211_IFTYPE_STATION)
        return 0;
#endif
      if (!common->init_done)
        return -1;
      if (common->iface_down)
        return -1;
      if (!common->beacon_enabled)
        return -1;
      if (common->acx_stop_beacon == true) {
        return -EINVAL;
      }
      rsi_dbg(MGMT_DEBUG_ZONE, "<==== Beacon Interrupt Received ====>\n");
#ifndef CONFIG_STA_PLUS_AP
      rsi_send_beacon(common);
#else
      rsi_send_beacon(common, vif);
#endif
      break;

    case WOWLAN_WAKEUP_REASON:
      rsi_hex_dump(INFO_ZONE, "WoWLAN Wakeup Trigger Pkt", msg, msg_len);
      rsi_dbg(ERR_ZONE, "\n\nWakeup Type: %x\n", msg[15]);
      switch (msg[15]) {
        case UNICAST_MAGIC_PKT:
          rsi_dbg(ERR_ZONE, "*** Wakeup for Unicast magic packet ***\n");
          break;
        case BROADCAST_MAGICPKT:
          rsi_dbg(ERR_ZONE, "*** Wakeup for Broadcast magic packet ***\n");
          break;
        case EAPOL_PKT:
          rsi_dbg(ERR_ZONE, "*** Wakeup for GTK renewal ***\n");
          break;
        case DISCONNECT_PKT:
          rsi_dbg(ERR_ZONE, "*** Wakeup for Disconnect ***\n");
          break;
        case HW_BMISS_PKT:
          rsi_dbg(ERR_ZONE, "*** Wakeup for HW Beacon miss ***\n");
          break;
        default:
          rsi_dbg(ERR_ZONE, "##### Un-intentional Wakeup #####\n");
          break;
      }
      break;
#ifdef CONFIG_RSI_11K
    case RADIO_MEAS_RPT:
      rsi_hex_dump(ERR_ZONE, "11K RRM RX CMD FRAME", msg, msg_len);
      common->priv->rrm_enq_state = 0;
      common->priv->rrm_state     = RRM_RESP_RCVD;
      rsi_rrm_recv_cmd_frame(common, msg, msg_len);
      rsi_rrm_sched_req(common);
      break;
#endif
    case RX_DOT11_MGMT:
      return rsi_mgmt_pkt_to_core(common, msg, msg_len);

    case FW_ERROR_STATE_IND:
      if (msg[15] == RSI_MAX_BGSCAN_CHANNEL_SUPPORTED) {
        rsi_dbg(ERR_ZONE, "*** Bgscan Channel's  greater than 24 Not Supported ***\n");
        return -1;
      } else if (msg[15] == RSI_MAX_BGSCAN_PROBE_REQ_LEN) {
        rsi_dbg(ERR_ZONE, "*** Bgscan Probe Request Length is greater than Max Size ***\n");
        return -1;
      }
      break;
    case DISABLE_PS_FROM_LMAC:
      rsi_dbg(MGMT_DEBUG_ZONE, "<==== RECEIVED DISABLE PS FRAME FROM LMAC ===>\n");
      if (msg[15] == PAUSE_WIFI) {
        rsi_dbg(INFO_ZONE, "<==== RECEIVED PAUSE_WIFI FROM LMAC ===>\n");
        adapter->wifi_paused = true;
        adapter->due_time    = adapter->var_interval - (jiffies - adapter->old_jiffies);
      } else if (msg[15] == RESUME_WIFI) {
        rsi_dbg(INFO_ZONE, "<==== RECEIVED RESUME_WIFI FROM LMAC ===>\n");
        adapter->wifi_paused = false;
        if (adapter->due_time)
          mod_timer(&adapter->traffic_timer, adapter->due_time + jiffies);
      } else {
        common->disable_ps_from_lmac = true;
        check_traffic_pwr_save(adapter);
      }
      break;
      //__9117_CODE_START
    case REAL_TIME_STATS_RESP_FRAME:
      rsi_dbg(MGMT_DEBUG_ZONE, "<==== RECEIVED REAL_TIME_STATS FRAME FROM LMAC ===>\n");
      rsi_hex_dump(MGMT_DEBUG_ZONE, "REAL_TIME_STATS_FRAME", msg, msg_len);
      adapter->stat_type = msg[16] & 0xf;
      adapter->next_stat = 16;

      while (adapter->next_stat < msg_len) {
        if (adapter->stat_type == TX_STATS) {
          memset(&adapter->tx_stats_info, 0, TX_STAT_LENGTH);
          memcpy((&adapter->tx_stats_info), &msg[adapter->next_stat], TX_STAT_LENGTH);
          adapter->next_stat += TX_STAT_LENGTH;
        } else if (adapter->stat_type == RX_STATS) {
          memset(&adapter->rx_stats_info, 0, RX_STAT_LENGTH);
          memcpy((&adapter->rx_stats_info), &msg[adapter->next_stat], RX_STAT_LENGTH);
          adapter->next_stat += RX_STAT_LENGTH;
        } else {
          rsi_dbg(ERR_ZONE, "INVALID STATS\n");
          break;
        }
        rsi_real_time_stats_to_app(adapter);
        adapter->stat_type = msg[adapter->next_stat] & 0xf;
      }
      break;
      //__9117_CODE_END
    default:
      rsi_dbg(INFO_ZONE, "Cmd Frame Type: %d\n", msg_type);
      break;
  }

  return 0;
}

int rsi_bb_prog_data_to_app(struct rsi_hw *adapter)
{
  struct sk_buff *skb_out = { 0 };
  struct nlmsghdr *nlh;
  int msg_size, res;
  msg_size = sizeof(adapter->bb_rf_read);
  skb_out  = nlmsg_new(msg_size, 0);
  if (!skb_out) {
    rsi_dbg(ERR_ZONE, "%s: Failed to allocate new skb\n", __func__);
    return 0;
  }

  nlh = nlmsg_put(skb_out, adapter->wlan_nl_pid, 0, NLMSG_DONE, msg_size, 0);
  /* NETLINK_CB(skb_out).dst_group = 0; */
  memcpy(nlmsg_data(nlh), &adapter->bb_rf_read, msg_size);
  res = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->wlan_nl_pid);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send stats to App\n", __func__);
    return -1;
  }
  return 0;
}
int send_gaintable_status_to_app(struct rsi_hw *adapter)
{
  struct sk_buff *skb_out = { 0 };
  struct nlmsghdr *nlh;
  int res;
  skb_out = nlmsg_new(2, 0);
  if (!skb_out) {
    rsi_dbg(ERR_ZONE, "%s: Failed to allocate new skb\n", __func__);
    return 0;
  }

  nlh = nlmsg_put(skb_out, adapter->wlan_nl_pid, 0, NLMSG_DONE, 2, 0);
  /* NETLINK_CB(skb_out).dst_group = 0; */
  memcpy(nlmsg_data(nlh), &adapter->gaintable_status, 2);
  res = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->wlan_nl_pid);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send RSSI to App\n", __func__);
    return -1;
  }
  return 0;
}

int send_rssi_to_app(struct rsi_hw *adapter)
{
  struct sk_buff *skb_out = { 0 };
  struct nlmsghdr *nlh;
  int res;
  skb_out = nlmsg_new(2, 0);
  if (!skb_out) {
    rsi_dbg(ERR_ZONE, "%s: Failed to allocate new skb\n", __func__);
    return 0;
  }

  nlh = nlmsg_put(skb_out, adapter->wlan_nl_pid, 0, NLMSG_DONE, 2, 0);
  /* NETLINK_CB(skb_out).dst_group = 0; */
  memcpy(nlmsg_data(nlh), &adapter->rx_rssi, 2);
  res = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->wlan_nl_pid);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send RSSI to App\n", __func__);
    return -1;
  }
  return 0;
}
int rsi_do_master_ops(struct rsi_hw *w_adapter, u16 type)
{
  int status = 0;
  int i;
  u8 *temp_buf;
  struct master_params_s *master = &w_adapter->master_ops;
  u16 msb_address                = (master->address >> 16);
  u16 lsb_address                = (u16)master->address;
  struct master_params_s master_get;
  struct rsi_host_intf_ops *hif_ops = w_adapter->host_intf_ops;
  struct sk_buff *skb_out           = { 0 };
  struct nlmsghdr *nlh;
  int msg_size, res;
  unsigned long write_data = 0;

  memcpy(&master_get, master, sizeof(struct master_params_s));
  temp_buf = kzalloc(1000, GFP_KERNEL);
  if (!temp_buf) {
    rsi_dbg(ERR_ZONE, "ERROR: %s %d Failed to allocate memory\n", __func__, __LINE__);
    return -ENOMEM;
  }

  if (w_adapter->rsi_host_intf == RSI_HOST_INTF_SDIO) {
    if ((hif_ops->master_access_msword(w_adapter, msb_address)) < 0) {
      rsi_dbg(ERR_ZONE, "%s: Unable to set ms word reg\n", __func__);
      goto end_master_ops;
    }
  }

  if (type == MASTER_READ) {
    rsi_dbg(ERR_ZONE, "In %s line %d master_read \n", __func__, __LINE__);
    if (w_adapter->rsi_host_intf == RSI_HOST_INTF_SDIO) {
      status = hif_ops->read_reg_multiple(w_adapter, lsb_address | SD_REQUEST_MASTER, temp_buf, SDIO_BLOCK_SIZE);
    } else {
      status = hif_ops->read_reg_multiple(w_adapter, master_get.address, temp_buf, master_get.no_of_bytes);
    }
    /*: mimic copy to user  functionality here*/
    for (i = 0; i < master_get.no_of_bytes; i++) {
      master_get.data[i] = temp_buf[i];
      rsi_dbg(INFO_ZONE, " Read Data[%d] = 0x%x \n", i, master_get.data[i]);
    }
    msg_size = sizeof(struct master_params_s);
    skb_out  = nlmsg_new(msg_size, 0);
    if (!skb_out) {
      rsi_dbg(ERR_ZONE, "%s: Failed to allocate new skb\n", __func__);
      return 0;
    }

    nlh = nlmsg_put(skb_out, w_adapter->wlan_nl_pid, 0, NLMSG_DONE, msg_size, 0);
    /* NETLINK_CB(skb_out).dst_group = 0; */
    memcpy(nlmsg_data(nlh), &master_get, msg_size);
    res = nlmsg_unicast(w_adapter->nl_sk, skb_out, w_adapter->wlan_nl_pid);
    if (res < 0) {
      rsi_dbg(ERR_ZONE, "%s: Failed to send stats to App\n", __func__);
      status = -1;
    }
  } else if (type == MASTER_WRITE) {
    rsi_dbg(INFO_ZONE, "In %s line %d master_write\n", __func__, __LINE__);
    memset(temp_buf, 0, master_get.no_of_bytes);
    memcpy(temp_buf, (u8 *)&master_get.data, master_get.no_of_bytes);
    memcpy((u8 *)&write_data, (u8 *)&master_get.data, master_get.no_of_bytes);
    for (i = 0; i < master_get.no_of_bytes; i++) {
      temp_buf[i] = master_get.data[i];
      rsi_dbg(INFO_ZONE, " Data to write = 0x%x \n", master_get.data[i]);
    }
    if (w_adapter->rsi_host_intf == RSI_HOST_INTF_SDIO) {
      if ((hif_ops->master_reg_write(w_adapter, master_get.address, write_data, master_get.no_of_bytes)) < 0) {
        goto end_master_ops;
      }
    } else {
      status = hif_ops->write_reg_multiple(w_adapter, master_get.address, temp_buf, master_get.no_of_bytes);
    }
  }

end_master_ops:
  if (w_adapter->rsi_host_intf == RSI_HOST_INTF_SDIO) {
    if (hif_ops->master_access_msword(w_adapter, 0x4105) != 0) {
      rsi_dbg(ERR_ZONE, "%s: Unable to set ms word to common reg\n", __func__);
      status = -1;
    }
  }
  kfree(temp_buf);
  return status;
}

/**
 * This function prepares the Baseband buffer Programming request
 * frame 
 *
 * @param
 *  w_adapter       Pointer to Driver Private Structure
 * @param
 *  bb_buf_vals  Pointer to bb_buf programming values
 * @param
 *  num_of_vals   Number of BB Programming values
 *
 * @return
 *  Returns ONEBOX_STATUS_SUCCESS on success, or ONEBOX_STATUS_FAILURE
 *  on failure
 */
int rsi_bb_buffer_request_direct(struct rsi_hw *w_adapter, u16 *bb_buf_vals, u16 num_of_vals)
{
  struct rsi_mac_frame *mgmt_frame = NULL;
  struct sk_buff *skb              = NULL;
  int status_l;
  u16 frame_len;

  skb = dev_alloc_skb(sizeof(struct rsi_buff_rw));
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }
  memset(skb->data, 0, sizeof(struct rsi_buff_rw));
  mgmt_frame = (struct rsi_mac_frame *)skb->data;

  rsi_dbg(INT_MGMT_ZONE, "===> Sending Buffer Programming Packet DIRECT<===\n");

  if (!w_adapter->bb_rf_rw) {
    /* Preparing BB BUFFER Request Frame Body */
    frame_len = ((num_of_vals * 2));
    /* Prepare the frame descriptor */
    mgmt_frame->desc_word[0] = cpu_to_le16((frame_len) | (RSI_WIFI_MGMT_Q << 12));
    mgmt_frame->desc_word[1] = cpu_to_le16(BB_BUF_PROG_VALUES_REQ);
    mgmt_frame->desc_word[5] = cpu_to_le16(1);

    if (w_adapter->soft_reset & BBP_REMOVE_SOFT_RST_BEFORE_PROG) {
      mgmt_frame->desc_word[7] |= cpu_to_le16(BBP_REMOVE_SOFT_RST_BEFORE_PROG);
    }
    if (w_adapter->soft_reset & BBP_REMOVE_SOFT_RST_AFTER_PROG) {
      mgmt_frame->desc_word[7] |= cpu_to_le16(BBP_REMOVE_SOFT_RST_AFTER_PROG);
    }

    if (w_adapter->bb_rf_rw) {
      mgmt_frame->desc_word[7] |= cpu_to_le16(BBP_REG_READ);
      w_adapter->bb_rf_rw = 0;
    }
    w_adapter->soft_reset = 0;

    mgmt_frame->desc_word[4] = cpu_to_le16(num_of_vals);
    mgmt_frame->desc_word[6] = cpu_to_le16(w_adapter->bb_rf_params.no_of_fields);
#ifdef RF_8230
    mgmt_frame->desc_word[7] |= cpu_to_le16(PUT_BBP_RESET | BBP_REG_WRITE | (NONRSI_RF_TYPE << 4));
#else
    mgmt_frame->desc_word[7] |= cpu_to_le16(PUT_BBP_RESET | BBP_REG_WRITE | (RSI_RF_TYPE << 4));
#endif
  } else {
    frame_len                = 0;
    mgmt_frame->desc_word[0] = cpu_to_le16((frame_len) | (RSI_WIFI_MGMT_Q << 12));
    mgmt_frame->desc_word[1] = cpu_to_le16(BB_BUF_PROG_VALUES_REQ);
    mgmt_frame->desc_word[7] |= cpu_to_le16(BBP_REG_READ);
    mgmt_frame->desc_word[5] = cpu_to_le16(1);
    w_adapter->bb_rf_rw      = 0;
    w_adapter->soft_reset    = 0;
    mgmt_frame->desc_word[4] = cpu_to_le16(num_of_vals);
    mgmt_frame->desc_word[6] = cpu_to_le16(w_adapter->bb_rf_params.no_of_fields);
#ifdef RF_8230
    mgmt_frame->desc_word[7] |= cpu_to_le16(PUT_BBP_RESET | BBP_REG_WRITE | (NONRSI_RF_TYPE << 4));
#else
    mgmt_frame->desc_word[7] |= cpu_to_le16(PUT_BBP_RESET | BBP_REG_WRITE | (RSI_RF_TYPE << 4));
#endif
  }
  skb_put(skb, frame_len + FRAME_DESC_SZ);
  rsi_hex_dump(INFO_ZONE, "DIRECT BUF_RD_WR", (const u8 *)mgmt_frame, skb->len);
  status_l = rsi_send_internal_mgmt_frame(w_adapter->priv, skb);
  return status_l;
}

static int rsi_bb_buffer_request_indirect(struct rsi_hw *w_adapter, u16 *bb_buf_vals, u16 num_of_vals)
{
  struct rsi_buff_rw *mgmt_frame = NULL;
  struct sk_buff *skb            = NULL;
  int status_l;
  u16 frame_len;

  skb = dev_alloc_skb(sizeof(struct rsi_buff_rw));
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }
  memset(skb->data, 0, sizeof(struct rsi_buff_rw));
  mgmt_frame = (struct rsi_buff_rw *)skb->data;
  rsi_dbg(INFO_ZONE, "===> Sending Buffer Programming Packet INDIRECT<===\n");

  /* Preparing BB BUFFER Request Frame Body */
  frame_len = ((num_of_vals * 2 * 3) + 6); //for 1 value there are 3 regs and each 2 bytes
  /* Preparing BB Request Frame Header */
  /*prepare the frame descriptor */
  mgmt_frame->desc_word[0] = cpu_to_le16((frame_len) | (RSI_WIFI_MGMT_Q << 12));
  mgmt_frame->desc_word[1] = cpu_to_le16(BB_BUF_PROG_VALUES_REQ);
  mgmt_frame->desc_word[5] = cpu_to_le16(2);

  if (w_adapter->soft_reset & BBP_REMOVE_SOFT_RST_BEFORE_PROG) {
    mgmt_frame->desc_word[7] |= cpu_to_le16(BBP_REMOVE_SOFT_RST_BEFORE_PROG);
  }
  if (w_adapter->soft_reset & BBP_REMOVE_SOFT_RST_AFTER_PROG) {
    mgmt_frame->desc_word[7] |= cpu_to_le16(BBP_REMOVE_SOFT_RST_AFTER_PROG);
  }

  if (w_adapter->bb_rf_rw) {
    mgmt_frame->desc_word[7] |= cpu_to_le16(BBP_REG_READ);
    w_adapter->bb_rf_rw = 0;
  }
  w_adapter->soft_reset = 0;

  mgmt_frame->desc_word[4] = cpu_to_le16(num_of_vals);
#ifdef RF_8230
  mgmt_frame->desc_word[7] |= cpu_to_le16(PUT_BBP_RESET | BBP_REG_WRITE | (NONRSI_RF_TYPE << 4));
#else
  mgmt_frame->desc_word[7] |= cpu_to_le16(PUT_BBP_RESET | BBP_REG_WRITE | (RSI_RF_TYPE << 4));
#endif
  memcpy(&mgmt_frame->bb_buf_req.bb_buf_vals[0], &bb_buf_vals[1], frame_len);
  skb_put(skb, frame_len + FRAME_DESC_SZ);
  rsi_hex_dump(INFO_ZONE, "BUFFER_READ_WRITE", (char *)mgmt_frame, skb->len);
  status_l = rsi_send_internal_mgmt_frame(w_adapter->priv, skb);
  return status_l;
}

int rsi_mgmt_send_bb_prog_frames(struct rsi_hw *adapter, unsigned short *bb_prog_vals, unsigned short num_of_vals)
{
  struct rsi_bb_prog_params *bb_prog_req;
  struct sk_buff *skb;
  unsigned short count;
  unsigned short frame_len;
  unsigned char ii = 0;

  skb = dev_alloc_skb(sizeof(struct rsi_bb_prog_params));
  if (!skb)
    return -1;

  bb_prog_req = (struct rsi_bb_prog_params *)skb->data;
  memset(skb->data, 0, sizeof(struct rsi_bb_prog_params));

  rsi_dbg(INT_MGMT_ZONE, "====> Sending Baseband Programmimg Packet <====\n");

  /* Preparing BB Request Frame Body */
  for (count = 1; ((count < num_of_vals) && (ii < num_of_vals)); ii++, count += 2) {
    bb_prog_req->bb_params_req[ii].reg_addr     = cpu_to_le16(bb_prog_vals[count]);
    bb_prog_req->bb_params_req[ii].bb_prog_vals = cpu_to_le16(bb_prog_vals[count + 1]);
  }

  if (num_of_vals % 2)
    bb_prog_req->bb_params_req[ii].reg_addr = cpu_to_le16(bb_prog_vals[count]);
  /* Preparing BB Request Frame Header */
  frame_len = ((num_of_vals)*2); /* each 2 bytes */

  /* Preparing the Frame descriptor */

  bb_prog_req->desc_word[0] = cpu_to_le16((frame_len) | (RSI_WIFI_MGMT_Q << 12));
  bb_prog_req->desc_word[1] = cpu_to_le16(BB_PROG_VALUES_REQUEST);

  if (adapter->soft_reset & BBP_REMOVE_SOFT_RST_BEFORE_PROG) {
    bb_prog_req->desc_word[7] |= cpu_to_le16(BBP_REMOVE_SOFT_RST_BEFORE_PROG);
  }

  if (adapter->soft_reset & BBP_REMOVE_SOFT_RST_AFTER_PROG) {
    bb_prog_req->desc_word[7] |= cpu_to_le16(BBP_REMOVE_SOFT_RST_AFTER_PROG);
  }

  if (adapter->bb_rf_rw) {
    bb_prog_req->desc_word[7] |= cpu_to_le16(BBP_REG_READ);
    adapter->bb_rf_rw = 0;
  }
  adapter->soft_reset = 0;

  /* Flag not handled */
  bb_prog_req->desc_word[4] = cpu_to_le16(num_of_vals);
  bb_prog_req->desc_word[7] |= cpu_to_le16(PUT_BBP_RESET | BBP_REG_WRITE | (RSI_RF_TYPE << 4));

  skb_put(skb, (frame_len + FRAME_DESC_SZ));
  rsi_hex_dump(INT_MGMT_ZONE, "BB_PROG_VAUE", skb->data, skb->len);

  rsi_send_internal_mgmt_frame(adapter->priv, skb);

  return 0;
}

int rsi_mgmt_soc_reg_ops_req(struct rsi_hw *adapter, unsigned short *bb_prog_vals, unsigned short type)
{
  struct rsi_bb_prog_params *bb_prog_req;
  struct sk_buff *skb;
  unsigned short frame_len;

  skb = dev_alloc_skb(sizeof(struct rsi_bb_prog_params));
  if (!skb)
    return -1;

  bb_prog_req = (struct rsi_bb_prog_params *)skb->data;
  memset(skb->data, 0, sizeof(struct rsi_bb_prog_params));

  rsi_dbg(INT_MGMT_ZONE, "====> Sending SOC REG OPS Request Packet <====\n");
  frame_len = ((4) * 2); /* each 2 bytes */

  /* Preparing the Frame descriptor */

  bb_prog_req->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  bb_prog_req->desc_word[1] = cpu_to_le16(SOC_REG_OPS);
  bb_prog_req->desc_word[3] = cpu_to_le16(bb_prog_vals[1]);
  bb_prog_req->desc_word[4] = cpu_to_le16(bb_prog_vals[2]);
  if (type == SOC_REG_WRITE) {
    bb_prog_req->desc_word[5] = cpu_to_le16(bb_prog_vals[3]);
    bb_prog_req->desc_word[6] = cpu_to_le16(bb_prog_vals[4]);
    bb_prog_req->desc_word[7] = cpu_to_le16(0); //WRITE INDICATION
  } else {
    bb_prog_req->desc_word[7] = cpu_to_le16(1); //READ INDICATION
  }

  skb_put(skb, (frame_len + FRAME_DESC_SZ));
  rsi_hex_dump(INT_MGMT_ZONE, "SOC_REG_OPS", skb->data, skb->len);

  rsi_send_internal_mgmt_frame(adapter->priv, skb);

  return 0;
}

int send_get_rssi_frame_to_fw(struct rsi_hw *adapter)
{
  struct sk_buff *skb              = NULL;
  struct rsi_mac_frame *mgmt_frame = NULL;
  rsi_dbg(INT_MGMT_ZONE, "<=====  Sending GET_RSSI FRAME =====>\n");
  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }
  memset(skb->data, 0, FRAME_DESC_SZ);
  mgmt_frame               = (struct rsi_mac_frame *)skb->data;
  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  mgmt_frame->desc_word[1] = cpu_to_le16(GET_RSSI_FRAME);
  skb_put(skb, FRAME_DESC_SZ);
  rsi_hex_dump(INT_MGMT_ZONE, "GET_RSSI FRAME", skb->data, skb->len);
  return rsi_send_internal_mgmt_frame(adapter->priv, skb);
}

int send_filter_broadcast_frame_to_fw(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload_len)
{

  struct sk_buff *skb              = NULL;
  struct rsi_mac_frame *mgmt_frame = NULL;

  rsi_dbg(INT_MGMT_ZONE, "<=====  Sending FILTER_BCAST FRAME =====>\n");

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }
  memset(skb->data, 0, FRAME_DESC_SZ);
  mgmt_frame = (struct rsi_mac_frame *)skb->data;

  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  mgmt_frame->desc_word[1] = cpu_to_le16(FILTER_BCAST_FRAME_TYPE);
  memcpy(&skb->data[4], nlmsg_data(nlh) + FRAME_DESC_SZ, payload_len);
  skb_put(skb, FRAME_DESC_SZ);
  rsi_hex_dump(INT_MGMT_ZONE, "FILTER_BCAST FRAME", skb->data, skb->len);
  return rsi_send_internal_mgmt_frame(adapter->priv, skb);
}

int rsi_set_bb_rf_values(struct rsi_hw *adapter)
{
  unsigned char type, i;
  struct rsi_common *common = adapter->priv;
  u16 rw_type;

  if (common->driver_mode == E2E_MODE) {
#ifndef CONFIG_STA_PLUS_AP
    struct ieee80211_vif *vif = adapter->vifs[0];
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
    bool assoc = vif && vif->bss_conf.assoc;
#else
    bool assoc = vif && vif->cfg.assoc;
#endif

#else
    struct ieee80211_vif *sta_vif = rsi_get_sta_vif(adapter);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
    bool assoc = sta_vif && sta_vif->bss_conf.assoc;
#else
    bool assoc = sta_vif && sta_vif->cfg.assoc;
#endif

#endif

    if (!assoc && adapter->ps_state == PS_ENABLED)
      rsi_disable_ps(adapter);
  }

  rsi_dbg(INFO_ZONE,
          "BB_RF_VALUES: Address = %x value = %d \n",
          adapter->bb_rf_params.Data[0],
          adapter->bb_rf_params.value);
  rsi_dbg(INFO_ZONE,
          "No of values = %d No of fields = %d \n",
          adapter->bb_rf_params.no_of_values,
          adapter->bb_rf_params.no_of_fields);
  adapter->bb_rf_params.Data[0] = adapter->bb_rf_params.no_of_values;
  type                          = adapter->bb_rf_params.value;

  if (!(DEV_MODEL_9116)) {
    for (i = 0; i <= adapter->bb_rf_params.no_of_values; i++)
      rsi_dbg(INFO_ZONE, "bb_rf_params.Data[] = %x \n", adapter->bb_rf_params.Data[i]);
  }

  if (type % 2 == 0) {
    adapter->bb_rf_rw = 1; //set_read;
    rsi_dbg(INFO_ZONE, "*** READ *** \n");
  }
  adapter->soft_reset = adapter->bb_rf_params.soft_reset;

  if (type == BB_WRITE_REQ || type == BB_READ_REQ) {
    rsi_dbg(INFO_ZONE, "READ_BUF_VALUES : BB_REQ\n");
    if ((rsi_mgmt_send_bb_prog_frames(adapter, adapter->bb_rf_params.Data, adapter->bb_rf_params.no_of_values)) < 0)
      return -1;
  } else if (type == BUF_READ_REQ || type == BUF_WRITE_REQ) {

    rsi_dbg(INFO_ZONE, "BB_BUFFER r/w started\n");
    rw_type = adapter->bb_rf_params.soft_reset >> 4;
    if (rw_type == 1) {
      if (rsi_bb_buffer_request_direct(adapter, adapter->bb_rf_params.Data, adapter->bb_rf_params.no_of_values) != 0) {
        return -1;
      }
    } else if (rw_type == 2) {
      if (rsi_bb_buffer_request_indirect(adapter, adapter->bb_rf_params.Data, adapter->bb_rf_params.no_of_values)
          != 0) {
        return -1;
      }
    } else {
      printk("Invalid R/W type\n");
    }

  } else if (type == SOC_REG_WRITE || type == SOC_REG_READ) {
    rsi_dbg(INFO_ZONE, "READ_BUF_VALUES : SOC_REG_READ/WRITE\n");
    if (rsi_mgmt_soc_reg_ops_req(adapter, adapter->bb_rf_params.Data, type) != 0) {
      return -1;
    }
  } else {
    rsi_dbg(INFO_ZONE, "Invalid R/W type\n");
    return -1;
  }

  rsi_dbg(INFO_ZONE, "Written Success and trying to read\n");

  return 0;
}

int rsi_process_efuse_content(struct rsi_common *common, struct efuse_map_s efuse_map)
{
  struct sk_buff *skb_out = NULL;
  struct nlmsghdr *nlh    = NULL;
  int msg_size, res;
  struct rsi_hw *adapter = common->priv;
  msg_size               = sizeof(struct efuse_map_s);
  skb_out                = nlmsg_new(msg_size, 0);
  if (!skb_out) {
    rsi_dbg(ERR_ZONE, "%s: Failed to allocate skb\n", __func__);
    return -1;
  }
  nlh = nlmsg_put(skb_out, adapter->wlan_nl_pid, 0, NLMSG_DONE, msg_size, 0);
  memcpy(nlmsg_data(nlh), &efuse_map, msg_size);
  rsi_dbg(INFO_ZONE, "<==== Sending EFUSE MAP content to Application ====>\n");
  res = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->wlan_nl_pid);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send EFUSE MAP content to Application\n", __func__);
    return -1;
  }
  return 0;
}

int rsi_copy_efuse_content(struct rsi_hw *adapter)
{
  struct rsi_common *common = adapter->priv;
  struct efuse_map_s efuse_content;
  rsi_dbg(INFO_ZONE, " Copying EFUSE MAP content \n");
  memcpy(&efuse_content, &common->efuse_map, sizeof(struct efuse_map_s));
  rsi_process_efuse_content(common, efuse_content);
  return 0;
}

//__9117_CODE_START
int send_bmiss_threshold_to_app(struct rsi_hw *adapter, u16 prev_bmiss_threshold)
{
  struct sk_buff *skb_out = { 0 };
  struct nlmsghdr *nlh;
  int res;
  skb_out = nlmsg_new(2, 0);
  if (!skb_out) {
    rsi_dbg(ERR_ZONE, "%s: Failed to allocate new skb\n", __func__);
    return 0;
  }

  nlh = nlmsg_put(skb_out, adapter->wlan_nl_pid, 0, NLMSG_DONE, 2, 0);
  /* NETLINK_CB(skb_out).dst_group = 0; */
  memcpy(nlmsg_data(nlh), &prev_bmiss_threshold, 2);
  res = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->wlan_nl_pid);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send RSSI to App\n", __func__);
    return -1;
  }
  return 0;
}

int rsi_send_real_time_stats_request_frame(struct rsi_hw *adapter, struct nlmsghdr *nlh)
{
  struct rsi_nl_desc *nl_desc = NULL;
  struct sk_buff *skb;
  struct rsi_rl_time_stats *mgmt_frame;
  int rltime_enable;
  unsigned long int rltime_timer;
  nl_desc       = (struct rsi_nl_desc *)nlmsg_data(nlh);
  rltime_enable = nl_desc->desc_word[1];
  rltime_timer  = nl_desc->desc_word[3];
  rltime_timer  = rltime_timer << 16;
  rltime_timer  = rltime_timer | nl_desc->desc_word[2];

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }
  memset(skb->data, 0, FRAME_DESC_SZ);
  mgmt_frame = (struct rsi_rl_time_stats *)skb->data;

  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  mgmt_frame->desc_word[1] = cpu_to_le16(REAL_TIME_STATS_REQ_FRAME);
  mgmt_frame->desc_word[4] = cpu_to_le16(rltime_enable);
  mgmt_frame->desc_word[6] = cpu_to_le16(rltime_timer & 0xffff);
  mgmt_frame->desc_word[7] = cpu_to_le16((rltime_timer >> 16));

  skb_put(skb, FRAME_DESC_SZ);
  rsi_hex_dump(ERR_ZONE, "REAL TIME STATS FRAME", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(adapter->priv, skb);
}

int rsi_real_time_stats_to_app(struct rsi_hw *adapter)
{
  struct sk_buff *skb_out = { 0 };
  struct nlmsghdr *nlh;
  int msg_size, res;

  if (adapter->stat_type == TX_STATS) {
    msg_size = sizeof(adapter->tx_stats_info);
    skb_out  = nlmsg_new(msg_size, 0);
    if (!skb_out) {
      rsi_dbg(ERR_ZONE, "%s: Failed to allocate new skb\n", __func__);
      return 0;
    }
    nlh = nlmsg_put(skb_out, adapter->wlan_nl_pid, 0, NLMSG_DONE, msg_size, 0);
    /* NETLINK_CB(skb_out).dst_group = 0; */
    memcpy(nlmsg_data(nlh), &adapter->tx_stats_info, msg_size);
    nlh->nlmsg_flags = 1;
    res              = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->wlan_nl_pid);
    if (res < 0) {
      rsi_dbg(ERR_ZONE, "%s: Failed to send stats to App\n", __func__);
      return -1;
    }
  } else if (adapter->stat_type == RX_STATS) {
    msg_size = sizeof(adapter->rx_stats_info);
    skb_out  = nlmsg_new(msg_size, 0);
    if (!skb_out) {
      rsi_dbg(ERR_ZONE, "%s: Failed to allocate new skb\n", __func__);
      return 0;
    }
    nlh = nlmsg_put(skb_out, adapter->wlan_nl_pid, 0, NLMSG_DONE, msg_size, 0);
    /* NETLINK_CB(skb_out).dst_group = 0; */
    memcpy(nlmsg_data(nlh), &adapter->rx_stats_info, msg_size);
    nlh->nlmsg_flags = 2;
    res              = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->wlan_nl_pid);
    if (res < 0) {
      rsi_dbg(ERR_ZONE, "%s: Failed to send stats to App\n", __func__);
      return -1;
    }
  }
  return 0;
}

#ifdef CONFIG_TWT_SUPPORT
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void twt_setup_timer_callback(struct rsi_hw *adapter)
{
#else
static void twt_setup_timer_callback(struct timer_list *t)
{
  struct rsi_hw *adapter = from_timer(adapter, t, twt_setup_timer);
#endif
  if (adapter->twt_retry_count_limit < adapter->rsi_twt_config.twt_retry_limit) {
    send_twt_setup_frame(adapter, 0, NULL);
  } else {
    adapter->twt_retry_count_limit   = 0;
    adapter->twt_session_in_progress = 0;
    adapter->twt_current_status      = TWT_SETUP_FAIL_MAX_RETRIES_REACHED;
    del_timer(&adapter->twt_setup_timer);
    if (adapter->twt_ps_disable == 1) {
      rsi_dbg(INFO_ZONE, "%s: Sending Enable PS after TWT Setup\n", __func__);
      rsi_enable_ps(adapter);
      adapter->twt_ps_disable = 0;
    }
  }
}
void init_twt_setup_timer(struct rsi_hw *adapter, unsigned long timeout)
{
  if (timer_pending(&adapter->twt_setup_timer)) {
    rsi_dbg(ERR_ZONE, "%s : Timer Pending. This Case Should not occur\n", __func__);
    del_timer(&adapter->twt_setup_timer);
  }
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
  init_timer(&adapter->twt_setup_timer);
  adapter->twt_setup_timer.data     = (unsigned long)adapter;
  adapter->twt_setup_timer.function = (void *)&twt_setup_timer_callback;
#else
  timer_setup(&adapter->twt_setup_timer, twt_setup_timer_callback, 0);
#endif
  adapter->twt_setup_timer.expires = msecs_to_jiffies(timeout) + jiffies;
  add_timer(&adapter->twt_setup_timer);
}

int process_twt_setup_cmd(struct rsi_hw *adapter, struct nlmsghdr *nlh, int payload_len)
{
  rsi_twt_config_t *user_twt_config = kmalloc(sizeof(rsi_twt_config_t), GFP_KERNEL);

  memcpy(user_twt_config, nlmsg_data(nlh) + FRAME_DESC_SZ, payload_len);
  if (user_twt_config->twt_enable) {
    if ((adapter->twt_session_active != 0) || (adapter->twt_session_in_progress != 0)) {
      rsi_dbg(ERR_ZONE, "Session already active or Inprogress, currently supported only one session\n");
      adapter->twt_current_status = TWT_SETUP_ERR_SESSION_ACTIVE;
      return -1;
    }
  } else {
    if (adapter->twt_session_active) {
      if ((adapter->twt_active_session_flow_id != user_twt_config->twt_flow_id)
          && (user_twt_config->twt_flow_id != 0xff)) {
        rsi_dbg(ERR_ZONE, " Flow ID dosen't match \n");
        adapter->twt_current_status = TWT_TEARDOWN_ERR_FLOWID_NOT_MATCHED;
        return -1;
      }
    } else {
      rsi_dbg(ERR_ZONE, " No Active session \n");
      adapter->twt_current_status = TWT_TEARDOWN_ERR_NOACTIVE_SESS;
      return -1;
    }
  }
  memcpy(&adapter->rsi_twt_config, user_twt_config, sizeof(rsi_twt_config_t));
  rsi_dbg(MGMT_DEBUG_ZONE, "TWT_CONFIG Parsing Success\n");
  kfree(user_twt_config);
  return 0;
}

int send_twt_setup_frame(struct rsi_hw *adapter, u8 twt_setup_update, twt_setup_frame_t *twt_setup_resp)
{
  struct sk_buff *skb                = NULL;
  twt_setup_frame_t *twt_setup_frame = NULL;
  struct ieee80211_hdr *hdr          = NULL;
  rsi_twt_config_t *user_twt_config  = &adapter->rsi_twt_config;
  struct rsi_common *common          = adapter->priv;
  u8 twt_retry_interval;
  u16 seq_num               = 0;
  u8 len                    = MIN_802_11_HDR_LEN + sizeof(twt_setup_frame_t);
  struct ieee80211_vif *vif = adapter->vifs[adapter->sc_nvifs - 1];
  if (vif == NULL) {
    rsi_dbg(ERR_ZONE, " vif is NULL :%s\n ", __func__);
    return -1;
  }
  seq_num = ((struct vif_priv *)vif->drv_priv)->seq_num++;
  skb     = dev_alloc_skb(len + DWORD_ALIGNMENT); /* 64 for dword alignment */
  if (!skb) {
    rsi_dbg(ERR_ZONE, "Failed to alloc twt_setup_req\n");
    return -ENOMEM;
  }
  if (adapter->ps_state == PS_ENABLED) {
    rsi_dbg(INFO_ZONE, "%s: Sending disable PS for TWT Setup\n", __func__);
    rsi_disable_ps(adapter);
    adapter->twt_ps_disable = 1;
  }

  skb_put(skb, len);
  memset(skb->data, 0, skb->len);
  skb_reserve(skb, DWORD_ALIGNMENT);
  hdr                = (struct ieee80211_hdr *)skb->data;
  twt_setup_frame    = (twt_setup_frame_t *)&skb->data[MIN_802_11_HDR_LEN];
  hdr->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
  hdr->duration_id   = 0x0;
  hdr->seq_ctrl      = seq_num << 4;
  memcpy(hdr->addr1, common->sta_bssid, ETH_ALEN);
  memcpy(hdr->addr2, common->mac_addr, ETH_ALEN);
  memcpy(hdr->addr3, common->sta_bssid, ETH_ALEN);
  if (user_twt_config->req_type == 0x2)
    rsi_dbg(MGMT_DEBUG_ZONE, "*** Sending TWT-SETUP-DEMAND Frame ***\n");
  else if (user_twt_config->req_type == 0x1)
    rsi_dbg(MGMT_DEBUG_ZONE, "*** Sending TWT-SETUP-SUGGEST Frame ***\n");
  else
    rsi_dbg(MGMT_DEBUG_ZONE, "*** Sending TWT-SETUP-REQUEST Frame ***\n");
  /* TWT frame preparation */
  twt_setup_frame->action_category                        = WLAN_CATEGORY_S1G;
  twt_setup_frame->action_type                            = WLAN_S1G_TWT_SETUP;
  twt_setup_frame->dialog_token                           = 1;
  twt_setup_frame->twt_element.eid                        = WLAN_EID_S1G_TWT;
  twt_setup_frame->twt_element.elength                    = 15;
  twt_setup_frame->twt_element.control_ndp_pi             = 0;
  twt_setup_frame->twt_element.control_resp_pm_mode       = 0;
  twt_setup_frame->twt_element.control_twt_info_frame_dis = 1; //Disable support for TWT info frames.
  twt_setup_frame->twt_element.control_resereved          = 0;
  twt_setup_frame->twt_element.req_type_twt_req           = 1;
  twt_setup_frame->twt_element.target_wake_time[0]        = 0;
  twt_setup_frame->twt_element.target_wake_time[1]        = 0;
  twt_setup_frame->twt_element.twt_channel                = 0;

  if (user_twt_config->twt_enable) {
    twt_setup_frame->twt_element.control_negotiation_type   = user_twt_config->negotiation_type;
    twt_setup_frame->twt_element.req_type_twt_setup_command = user_twt_config->req_type;
    twt_setup_frame->twt_element.req_type_trigger           = user_twt_config->triggered_twt;
    twt_setup_frame->twt_element.req_type_implicit_twt      = user_twt_config->implicit_twt;
    twt_setup_frame->twt_element.req_type_flow_type         = user_twt_config->un_announced_twt;
    twt_setup_frame->twt_element.req_type_twt_flow_id       = user_twt_config->twt_flow_id;
    twt_setup_frame->twt_element.req_type_twt_protection    = user_twt_config->twt_protection;
    twt_setup_frame->twt_element.control_wake_duration_unit = user_twt_config->twt_wake_duration_unit;
    twt_retry_interval                                      = user_twt_config->twt_retry_interval;
    if (twt_setup_update == 0) {
      twt_setup_frame->twt_element.req_type_twt_wi_exp   = user_twt_config->wake_int_exp;
      twt_setup_frame->twt_element.nom_min_wake_duration = user_twt_config->wake_duration;
      twt_setup_frame->twt_element.wake_interval_mantisa = user_twt_config->wake_int_mantissa;
    } else {
      if (twt_setup_resp == NULL) {
        rsi_dbg(ERR_ZONE, "AP'S TWT response is NULL\n");
        return -1;
      } else {
        twt_setup_frame->twt_element.req_type_twt_wi_exp   = twt_setup_resp->twt_element.req_type_twt_wi_exp;
        twt_setup_frame->twt_element.nom_min_wake_duration = twt_setup_resp->twt_element.nom_min_wake_duration;
        twt_setup_frame->twt_element.wake_interval_mantisa = twt_setup_resp->twt_element.wake_interval_mantisa;
        if ((user_twt_config->req_type == TWT_SETUP_CMD_DEMAND)
            && (twt_setup_resp->twt_element.req_type_twt_setup_command == TWT_SETUP_CMD_ALTERNATE)) {
          twt_setup_frame->twt_element.req_type_twt_setup_command =
            TWT_SETUP_CMD_SUGGEST; /* if Ap send alteranate for demand then send suggest */
          twt_setup_frame->twt_element.target_wake_time[0] = twt_setup_resp->twt_element.target_wake_time[0];
          twt_setup_frame->twt_element.target_wake_time[1] = twt_setup_resp->twt_element.target_wake_time[1];
          twt_setup_frame->twt_element.control_ndp_pi      = 1;
        }
      }
    }
  } else { /* default value */
    user_twt_config->twt_flow_id                            = 1;
    user_twt_config->negotiation_type                       = 0;
    user_twt_config->req_type                               = TWT_SETUP_CMD_REQUEST;
    user_twt_config->twt_retry_limit                        = 5;
    twt_setup_frame->twt_element.control_wake_duration_unit = 0;
    twt_setup_frame->twt_element.control_negotiation_type   = 0;
    twt_setup_frame->twt_element.req_type_twt_setup_command = TWT_SETUP_CMD_REQUEST;
    twt_setup_frame->twt_element.req_type_trigger           = 1;
    twt_setup_frame->twt_element.req_type_implicit_twt      = 1;
    twt_setup_frame->twt_element.req_type_flow_type         = 0;
    twt_setup_frame->twt_element.req_type_twt_flow_id       = 1;
    twt_setup_frame->twt_element.req_type_twt_protection    = 0;
    twt_setup_frame->twt_element.req_type_twt_wi_exp        = 20;
    twt_setup_frame->twt_element.nom_min_wake_duration      = 40;
    twt_setup_frame->twt_element.wake_interval_mantisa      = 5;
  }
  skb->priority = MGMT_SOFT_Q;
  if (rsi_prepare_mgmt_desc(common, skb))
    goto out;

  rsi_hex_dump(MGMT_DEBUG_ZONE, "TWT_SETUP", skb->data, skb->len);
  skb_queue_tail(&common->tx_queue[MGMT_SOFT_Q], skb);
  rsi_set_event(&common->tx_thread.event);
  adapter->twt_retry_count_limit += 1;
  adapter->twt_session_in_progress = 1;
  adapter->twt_current_status      = TWT_SETUP_SESSION_IN_PROGRESS;
  if (adapter->twt_setup_timer.function) {
    mod_timer(&adapter->twt_setup_timer, msecs_to_jiffies(adapter->rsi_twt_config.twt_retry_interval * 1000) + jiffies);
  } else {
    init_twt_setup_timer(adapter, adapter->rsi_twt_config.twt_retry_interval * 1000);
  }
  return 0;
out:
  dev_kfree_skb(skb);
  return 0;
}

int send_twt_teardown_frame(struct rsi_hw *adapter, u8 flow_id, u8 negotiation_type)
{
  struct sk_buff *skb                      = NULL;
  struct ieee80211_hdr *hdr                = NULL;
  twt_teardown_frame_t *twt_teardown_frame = NULL;
  struct rsi_common *common                = adapter->priv;
  u8 len                                   = sizeof(twt_teardown_frame_t) + MIN_802_11_HDR_LEN;
  u16 seq_num                              = 0;
  struct ieee80211_vif *vif                = adapter->vifs[adapter->sc_nvifs - 1];
  if (vif == NULL) {
    rsi_dbg(ERR_ZONE, "vif is NULL:%s\n ", __func__);
    return -1;
  }
  seq_num = ((struct vif_priv *)vif->drv_priv)->seq_num++;
  skb     = dev_alloc_skb(len);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "Failed to alloc twt_teardown_req\n");
    return -ENOMEM;
  }
  rsi_dbg(MGMT_DEBUG_ZONE, "Sending TWT_TEARDOWN Frame\n");
  memset(skb->data, 0, len);
  hdr                = (struct ieee80211_hdr *)skb->data;
  twt_teardown_frame = (twt_teardown_frame_t *)&skb->data[MIN_802_11_HDR_LEN];
  hdr->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
  hdr->duration_id   = 0x0;
  hdr->seq_ctrl      = seq_num << 4;
  memcpy(hdr->addr1, common->sta_bssid, ETH_ALEN);
  memcpy(hdr->addr2, common->mac_addr, ETH_ALEN);
  memcpy(hdr->addr3, common->sta_bssid, ETH_ALEN);
  twt_teardown_frame->action_category  = WLAN_CATEGORY_S1G;
  twt_teardown_frame->action_type      = WLAN_S1G_TWT_TEARDOWN;
  twt_teardown_frame->negotiation_type = negotiation_type;
  if (flow_id != 0xFF)
    twt_teardown_frame->flow_id = flow_id;
  else
    twt_teardown_frame->teardown_all = 1;
  skb_put(skb, len);
  skb->priority = MGMT_SOFT_Q;
  if (rsi_prepare_mgmt_desc(common, skb))
    goto out;
  rsi_hex_dump(MGMT_DEBUG_ZONE, "TWT_TEARDOWN", skb->data, skb->len);
  skb_queue_tail(&common->tx_queue[MGMT_SOFT_Q], skb);
  rsi_set_event(&common->tx_thread.event);
  return 0;
out:
  dev_kfree_skb(skb);
  return 0;
}

int send_twt_session_details_to_lmac(struct rsi_hw *adapter,
                                     u8 setup,
                                     u8 twt_flow_id,
                                     twt_setup_frame_t *twt_setup_resp,
                                     rsi_twt_config_t *twt_user_config)
{
  struct sk_buff *skb              = NULL;
  twt_session_config_t *twt_config = NULL;
  u64 wake_interval;
  u32 rx_latency            = 0;
  struct rsi_common *common = adapter->priv;
  skb                       = dev_alloc_skb(sizeof(twt_session_config_t));
  if (!skb) {
    rsi_dbg(ERR_ZONE, "Failed to alloc twt_session details frame\n");
    return -ENOMEM;
  }
  rsi_dbg(MGMT_DEBUG_ZONE, "Sending TWT session Details Frame to LMAC\n");
  memset(skb->data, 0, sizeof(twt_session_config_t));
  twt_config                = (twt_session_config_t *)skb->data;
  twt_config->frame_desc[0] = cpu_to_le16((sizeof(twt_session_config_t) - FRAME_DESC_SZ) | (RSI_WIFI_MGMT_Q << 12));
  twt_config->frame_desc[1] = cpu_to_le16(TWT_CONFIG);
  twt_config->frame_desc[2] = cpu_to_le16(twt_user_config->restrict_tx_outside_tsp << 8);
  if (setup) {
    wake_interval = twt_setup_resp->twt_element.wake_interval_mantisa;
    wake_interval = wake_interval * (1 << twt_setup_resp->twt_element.req_type_twt_wi_exp);
  }

  if (adapter->twt_auto_config_enable && (setup == 1)) {
    rx_latency = (u32)(div64_u64(((u64)twt_user_config->rx_latency - (div64_u64(wake_interval, 1024))), 100));
    twt_config->frame_desc[4] = twt_user_config->beacon_wake_up_count_after_sp & 0xFF;
    twt_config->frame_desc[3] = ((rx_latency & 0xFF) | (((rx_latency >> 8) & 0xFF) << 8));
    twt_config->frame_desc[5] = (((rx_latency >> 16) & 0xFF) | (((rx_latency >> 24) & 0xFF) << 8));
  }

  if (setup) {
    adapter->twt_session_active         = 1;
    adapter->twt_active_session_flow_id = twt_setup_resp->twt_element.req_type_twt_flow_id;
    adapter->ap_twt_info_frame_support  = !twt_setup_resp->twt_element.control_twt_info_frame_dis;
    twt_config->twt_enable              = 1;
    twt_config->twt_negotiation_type    = twt_setup_resp->twt_element.control_negotiation_type;
    twt_config->twt_req                 = twt_setup_resp->twt_element.req_type_twt_req;
    twt_config->twt_setup_command       = twt_setup_resp->twt_element.req_type_twt_setup_command;
    twt_config->twt_trigger             = twt_setup_resp->twt_element.req_type_trigger;
    twt_config->twt_implicit            = twt_setup_resp->twt_element.req_type_implicit_twt;
    twt_config->twt_flow_type           = twt_setup_resp->twt_element.req_type_flow_type;
    twt_config->twt_flow_id             = twt_setup_resp->twt_element.req_type_twt_flow_id;
    twt_config->twt_wake_inter_expo     = twt_setup_resp->twt_element.req_type_twt_wi_exp;
    twt_config->twt_protection          = twt_setup_resp->twt_element.req_type_twt_protection;
    twt_config->target_wake_time[0]     = twt_setup_resp->twt_element.target_wake_time[0];
    twt_config->target_wake_time[1]     = twt_setup_resp->twt_element.target_wake_time[1];
    if (twt_setup_resp->twt_element.control_wake_duration_unit) {
      twt_config->twt_nom_wake_dur = (twt_setup_resp->twt_element.nom_min_wake_duration * 1024);
    } else {
      twt_config->twt_nom_wake_dur = (twt_setup_resp->twt_element.nom_min_wake_duration * 256);
    }
    twt_config->twt_wake_interval[0] = wake_interval & 0xFFFFFFFF;
    twt_config->twt_wake_interval[1] = (wake_interval >> 32) & 0xFFFFFFFF;
  } else {
    adapter->twt_session_active = 0;
    twt_config->twt_enable      = 0;
    twt_config->twt_flow_id     = twt_flow_id;
  }
  skb_put(skb, sizeof(twt_session_config_t));
  rsi_hex_dump(MGMT_DEBUG_ZONE, "TWT_SESSION_DETAILS", skb->data, skb->len);
  return rsi_send_internal_mgmt_frame(common, skb);
}

int rsi_twt_status_resp_to_app(struct rsi_hw *adapter)
{
  struct sk_buff *skb_out = { 0 };
  struct nlmsghdr *nlh;
  int msg_size, res;
  msg_size = sizeof(adapter->twt_status_to_app);
  skb_out  = nlmsg_new(msg_size, 0);
  if (!skb_out) {
    rsi_dbg(ERR_ZONE, "%s: Failed to allocate new skb\n", __func__);
    return 0;
  }

  nlh = nlmsg_put(skb_out, adapter->wlan_nl_pid, 0, NLMSG_DONE, msg_size, 0);
  /* NETLINK_CB(skb_out).dst_group = 0; */
  memcpy(nlmsg_data(nlh), &adapter->twt_status_to_app, msg_size);
  nlh->nlmsg_flags = adapter->twt_current_status;
  res              = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->wlan_nl_pid);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send stats to App\n", __func__);
    return -1;
  }
  return 0;
}

int validate_unsupported_twt_resp_params(struct rsi_hw *adapter, twt_setup_frame_t *twt_setup_resp)
{

  u64 wake_interval = ((u64)twt_setup_resp->twt_element.wake_interval_mantisa
                       * ((u64)1 << twt_setup_resp->twt_element.req_type_twt_wi_exp));
  u32 wake_duration = ((u32)twt_setup_resp->twt_element.nom_min_wake_duration
                       * (u32)(twt_setup_resp->twt_element.control_wake_duration_unit ? 1024 : 256));

  if ((twt_setup_resp->twt_element.req_type_twt_req == 1)
      && (twt_setup_resp->twt_element.req_type_twt_setup_command < 3)) {
    rsi_dbg(ERR_ZONE, "twt setup requests processing not supported\n");
    return 1;
  } else if ((adapter->twt_auto_config_enable == 1) && (twt_setup_resp->twt_element.req_type_flow_type != 0)) {
    rsi_dbg(ERR_ZONE, "only announced TWT setup is supported when auto config enabled\n");
    return 1;
  } else if (twt_setup_resp->dialog_token != 1) {
    rsi_dbg(ERR_ZONE, "twt setup response dialogue token does not match\n");
    return 1;
  } else if (twt_setup_resp->twt_element.control_negotiation_type) {
    rsi_dbg(ERR_ZONE, "only individual twt setup is supported \n");
    return 1;
  } else if (twt_setup_resp->twt_element.req_type_twt_protection != 0) {
    rsi_dbg(ERR_ZONE, "twt protection is not supported \n");
    return 1;
  } else if ((twt_setup_resp->twt_element.nom_min_wake_duration == 0)
             || (twt_setup_resp->twt_element.wake_interval_mantisa == 0) || (wake_duration > wake_interval)) {
    rsi_dbg(
      ERR_ZONE,
      "wake_duration/wake_interval values should not be zeros or wake_duration can't be greater than wake_interval.\n");
    return 1;
  }
  return 0;
}

static u8 does_twt_response_match_user_requirement(struct rsi_hw *adapter, twt_setup_frame_t *twt_setup_resp)
{
  twt_selection_t *user_config = &adapter->user_twt_auto_config;
  rsi_twt_config_t *twt_config = &adapter->rsi_twt_config;
  uint64_t resp_twt_wi, req_twt_wi, max_twt_wi, max_twt_wi_tol, total_resp_wake_duration_ms,
    min_wake_duration_required_ms;
  uint32_t resp_wake_duration, req_wake_duration, num_twt_service_periods;
  if (twt_setup_resp->twt_element.req_type_flow_type != 0) {
    return 0;
  }
  resp_twt_wi = ((uint64_t)twt_setup_resp->twt_element.wake_interval_mantisa
                 * ((uint64_t)0x1 << twt_setup_resp->twt_element.req_type_twt_wi_exp))
                >> 10;
  resp_wake_duration = (((uint32_t)twt_setup_resp->twt_element.nom_min_wake_duration
                         << (twt_setup_resp->twt_element.control_wake_duration_unit ? 10 : 8)))
                       >> 10;
  req_twt_wi        = ((uint64_t)twt_config->wake_int_mantissa * ((u64)0x1 << twt_config->wake_int_exp)) >> 10;
  req_wake_duration = (((uint32_t)twt_config->wake_duration << (twt_config->twt_wake_duration_unit ? 10 : 8))) >> 10;
  max_twt_wi = 0, max_twt_wi_tol = 0;
  if (user_config->tx_latency != 0) {
    max_twt_wi = MIN_OF_3(user_config->rx_latency, user_config->tx_latency, user_config->default_wake_interval_ms);
  } else {
    max_twt_wi = MIN_OF_2(user_config->rx_latency, user_config->default_wake_interval_ms);
  }
  max_twt_wi_tol = div64_u64((max_twt_wi * (u64)(100 + user_config->twt_tolerable_deviation)), 100);
  // ensuring that AP given TWT Wake interval does not exceed max allowed TWT Wake interval beyond tolerated value.
  if ((resp_twt_wi > max_twt_wi_tol) || (resp_wake_duration > (resp_twt_wi >> 1))) {
    return 0;
  } else {
    //Calculating num of AP given TWT service periods possible within the Max allowed TWT Wake Interval
    num_twt_service_periods = (uint32_t)(div64_u64(max_twt_wi, resp_twt_wi));
    // If calculated number of service periods is zero, assign 1
    num_twt_service_periods = num_twt_service_periods ? num_twt_service_periods : 1;
    // Calculate total AP wake duration for all the number of TWT service periods
    total_resp_wake_duration_ms = (uint64_t)num_twt_service_periods * (uint64_t)resp_wake_duration;
    // min wake dur required by STA
    min_wake_duration_required_ms = (uint64_t)req_wake_duration * (uint64_t)(div64_u64(max_twt_wi, req_twt_wi));
    if ((total_resp_wake_duration_ms
         < (div64_u64((min_wake_duration_required_ms * (u64)(100 - user_config->twt_tolerable_deviation)), 100)))
        || (total_resp_wake_duration_ms
            > (div64_u64((min_wake_duration_required_ms * (u64)(100 + user_config->twt_tolerable_deviation)), 100)))) {
      //        indicate session setup failure if total AP wake duration is not within the tolerable limits.
      return 0;
    }
  }
  return 1;
}

int rsi_mgmt_process_twt_setup_resp(struct rsi_hw *adapter, struct sk_buff *skb)
{
  twt_setup_frame_t *twt_setup_frame = (twt_setup_frame_t *)&skb->data[MIN_802_11_HDR_LEN];
  rsi_twt_config_t *user_twt_config  = &adapter->rsi_twt_config;
  u8 trigger_setup_again             = 0;
  bool send_twt_teardown             = false;

  if (adapter->twt_session_in_progress) {
    adapter->twt_retry_count_limit = 0;
    del_timer(&adapter->twt_setup_timer);
  }
  if (validate_unsupported_twt_resp_params(adapter, twt_setup_frame)) {
    adapter->twt_current_status = TWT_SETUP_UNSUPPORTED_RSP;
    if (twt_setup_frame->twt_element.req_type_twt_setup_command == TWT_SETUP_CMD_ACCEPT)
      send_twt_teardown = true;
    goto RSP_PROC_DONE;
  }

  switch (user_twt_config->req_type) {
    case TWT_SETUP_CMD_REQUEST: {
      switch (twt_setup_frame->twt_element.req_type_twt_setup_command) {
        case TWT_SETUP_CMD_ACCEPT:
        case TWT_SETUP_CMD_ALTERNATE:
        case TWT_SETUP_CMD_DICTATE: {
          rsi_dbg(MGMT_DEBUG_ZONE,
                  "Received response = %x for TWT-SETUP-REQUEST Frame \n",
                  twt_setup_frame->twt_element.req_type_twt_setup_command);
          if (twt_setup_frame->twt_element.req_type_twt_setup_command == TWT_SETUP_CMD_ACCEPT) {
            if (adapter->twt_session_in_progress)
              adapter->twt_current_status = TWT_SESSION_SUCC;
            else
              adapter->twt_current_status = TWT_UNSOL_SESSION_SUCC;
            send_twt_session_details_to_lmac(adapter, 1, 0, twt_setup_frame, user_twt_config);
          } else {
            adapter->twt_retry_count_limit = 0;
            send_twt_setup_frame(adapter, 1, twt_setup_frame);
            trigger_setup_again = 1;
            rsi_dbg(ERR_ZONE, "ALTERNATE or DICATATE TWT not expected with REQUEST TWT  \n");
          }
        } break;
        case TWT_SETUP_CMD_REJECT: {
          adapter->twt_current_status = TWT_SETUP_AP_REJECTED;
          rsi_dbg(ERR_ZONE, "TWT setup Rejected by AP \n");
        } break;
        default: {
          rsi_dbg(ERR_ZONE, "Un supported TWT Response \n");
          adapter->twt_current_status = TWT_SETUP_UNSUPPORTED_RSP;
        } break;
      }
    } break;
    case TWT_SETUP_CMD_SUGGEST: {
      switch (twt_setup_frame->twt_element.req_type_twt_setup_command) {
        case TWT_SETUP_CMD_ALTERNATE:
        case TWT_SETUP_CMD_DICTATE:
        case TWT_SETUP_CMD_ACCEPT: {
          rsi_dbg(MGMT_DEBUG_ZONE,
                  "Received response = %x for TWT-SETUP-SUGGEST Frame \n",
                  twt_setup_frame->twt_element.req_type_twt_setup_command);
          if ((twt_setup_frame->twt_element.req_type_trigger != (user_twt_config->triggered_twt & 1))
              || (twt_setup_frame->twt_element.req_type_implicit_twt != (user_twt_config->implicit_twt & 1))
              || (twt_setup_frame->twt_element.req_type_flow_type != (user_twt_config->un_announced_twt & 1))
              || (((adapter->twt_auto_config_enable == 0)
                   && ((twt_setup_frame->twt_element.nom_min_wake_duration
                        < ((user_twt_config->wake_duration > user_twt_config->wake_duration_tol)
                             ? (user_twt_config->wake_duration - user_twt_config->wake_duration_tol)
                             : 0))
                       || (twt_setup_frame->twt_element.nom_min_wake_duration
                           > (user_twt_config->wake_duration + user_twt_config->wake_duration_tol))
                       || (twt_setup_frame->twt_element.req_type_twt_wi_exp
                           < ((user_twt_config->wake_int_exp > user_twt_config->wake_int_exp_tol)
                                ? (user_twt_config->wake_int_exp - user_twt_config->wake_int_exp_tol)
                                : 0))
                       || (twt_setup_frame->twt_element.req_type_twt_wi_exp
                           > (user_twt_config->wake_int_exp + user_twt_config->wake_int_exp_tol))
                       || (twt_setup_frame->twt_element.wake_interval_mantisa
                           < ((user_twt_config->wake_int_mantissa > user_twt_config->wake_int_mantissa_tol)
                                ? (user_twt_config->wake_int_mantissa - user_twt_config->wake_int_mantissa_tol)
                                : 0))
                       || (twt_setup_frame->twt_element.wake_interval_mantisa
                           > (user_twt_config->wake_int_mantissa + user_twt_config->wake_int_mantissa_tol))))
                  || ((adapter->twt_auto_config_enable == 1)
                      && (does_twt_response_match_user_requirement(adapter, twt_setup_frame) == 0)))) {
            rsi_dbg(ERR_ZONE, "AP TWT parameters not the in tolerance limit provided \n");
            adapter->twt_current_status = TWT_SETUP_RSP_OUTOF_TOL;
            if (twt_setup_frame->twt_element.req_type_twt_setup_command == TWT_SETUP_CMD_ACCEPT) {
              send_twt_teardown = true;
              goto RSP_PROC_DONE;
            }
          } else {
            if (twt_setup_frame->twt_element.req_type_twt_setup_command == TWT_SETUP_CMD_ACCEPT) {
              if (adapter->twt_session_in_progress)
                adapter->twt_current_status = TWT_SESSION_SUCC;
              else
                adapter->twt_current_status = TWT_UNSOL_SESSION_SUCC;
              send_twt_session_details_to_lmac(adapter, 1, 0, twt_setup_frame, user_twt_config);
            } else {
              adapter->twt_retry_count_limit = 0;
              send_twt_setup_frame(adapter, 1, twt_setup_frame);
              trigger_setup_again = 1;
            }
          }
        } break;
        case TWT_SETUP_CMD_REJECT: {
          rsi_dbg(ERR_ZONE, "TWT setup Rejected by AP \n");
          adapter->twt_current_status = TWT_SETUP_AP_REJECTED;
        } break;
        default: {
          rsi_dbg(ERR_ZONE, "Un supported TWT Response \n");
          adapter->twt_current_status = TWT_SETUP_UNSUPPORTED_RSP;
        } break;
      }
    } break;
    case TWT_SETUP_CMD_DEMAND: {
      switch (twt_setup_frame->twt_element.req_type_twt_setup_command) {
        case TWT_SETUP_CMD_ALTERNATE:
        case TWT_SETUP_CMD_DICTATE:
        case TWT_SETUP_CMD_ACCEPT: {
          rsi_dbg(MGMT_DEBUG_ZONE,
                  "Received response = %x for TWT-SETUP-DEMAND Frame \n",
                  twt_setup_frame->twt_element.req_type_twt_setup_command);
          if ((((u32)twt_setup_frame->twt_element.nom_min_wake_duration
                * (u32)(twt_setup_frame->twt_element.control_wake_duration_unit ? 1024 : 256))
               != ((u32)user_twt_config->wake_duration * (u32)(user_twt_config->twt_wake_duration_unit ? 1024 : 256)))
              || (((u64)twt_setup_frame->twt_element.wake_interval_mantisa
                   * ((u64)1 << twt_setup_frame->twt_element.req_type_twt_wi_exp))
                  != ((u64)user_twt_config->wake_int_mantissa * ((u64)1 << user_twt_config->wake_int_exp)))
              || (twt_setup_frame->twt_element.req_type_trigger != (user_twt_config->triggered_twt & 1))
              || (twt_setup_frame->twt_element.req_type_implicit_twt != (user_twt_config->implicit_twt & 1))
              || (twt_setup_frame->twt_element.req_type_flow_type != (user_twt_config->un_announced_twt & 1))) {
            rsi_dbg(ERR_ZONE, "AP TWT parameters not matched with requested params\n");
            adapter->twt_current_status = TWT_SETUP_RSP_NOT_MATCHED;
            if (twt_setup_frame->twt_element.req_type_twt_setup_command == TWT_SETUP_CMD_ACCEPT) {
              send_twt_teardown = true;
              goto RSP_PROC_DONE;
            }
          } else {
            if (twt_setup_frame->twt_element.req_type_twt_setup_command == TWT_SETUP_CMD_ACCEPT) {
              if (adapter->twt_session_in_progress)
                adapter->twt_current_status = TWT_SESSION_SUCC;
              else
                adapter->twt_current_status = TWT_UNSOL_SESSION_SUCC;
              send_twt_session_details_to_lmac(adapter, 1, 0, twt_setup_frame, user_twt_config);
            } else {
              adapter->twt_retry_count_limit = 0;
              if (adapter->twt_demand_resp_retry_limit < user_twt_config->twt_retry_limit) {
                send_twt_setup_frame(adapter, 1, twt_setup_frame);
                adapter->twt_demand_resp_retry_limit += 1;
                trigger_setup_again = 1;
              } else {
                rsi_dbg(ERR_ZONE, "Max TWT-SETUP-DEMAND reached\n");
                adapter->twt_current_status = TWT_SETUP_AP_REJECTED;
              }
            }
          }
        } break;
        case TWT_SETUP_CMD_REJECT: {
          rsi_dbg(ERR_ZONE, "TWT setup Rejected by AP \n");
          adapter->twt_current_status = TWT_SETUP_AP_REJECTED;
        } break;
        default: {
          rsi_dbg(ERR_ZONE, "Un supported TWT Response \n");
          adapter->twt_current_status = TWT_SETUP_UNSUPPORTED_RSP;
        } break;
      }
    } break;
    default: {
      rsi_dbg(ERR_ZONE, "This should not occur \n");
    } break;
  }
  if (adapter->twt_session_active == 1) {
    adapter->twt_status_to_app.twt_flow_id        = twt_setup_frame->twt_element.req_type_twt_flow_id;
    adapter->twt_status_to_app.wake_duration      = twt_setup_frame->twt_element.nom_min_wake_duration;
    adapter->twt_status_to_app.implicit_twt       = twt_setup_frame->twt_element.req_type_implicit_twt;
    adapter->twt_status_to_app.triggered_twt      = twt_setup_frame->twt_element.req_type_trigger;
    adapter->twt_status_to_app.req_type           = twt_setup_frame->twt_element.req_type_twt_req;
    adapter->twt_status_to_app.wake_duration_unit = user_twt_config->twt_wake_duration_unit;
    adapter->twt_status_to_app.un_announced_twt   = twt_setup_frame->twt_element.req_type_flow_type;
    adapter->twt_status_to_app.wake_int_exp       = twt_setup_frame->twt_element.req_type_twt_wi_exp;
    adapter->twt_status_to_app.wake_int_mantissa  = twt_setup_frame->twt_element.wake_interval_mantisa;
  }
RSP_PROC_DONE:
  if (send_twt_teardown) {
    send_twt_teardown_frame(adapter,
                            twt_setup_frame->twt_element.req_type_twt_flow_id,
                            twt_setup_frame->twt_element.control_negotiation_type);
    if ((adapter->twt_session_active == 1)
        && (adapter->twt_active_session_flow_id
            == twt_setup_frame->twt_element
                 .req_type_twt_flow_id)) { // if session is already active Teardown should be given to LMAC as well.
      send_twt_session_details_to_lmac(adapter,
                                       0,
                                       adapter->twt_active_session_flow_id,
                                       twt_setup_frame,
                                       user_twt_config);
      adapter->twt_current_status = TWT_TEARDOWN_SUCC;
    }
  }

  if (trigger_setup_again == 0)
    adapter->twt_session_in_progress = 0;
  return 1;
}

int send_twt_information_frame(struct rsi_hw *adapter, wifi_reschedule_twt_config_t reschedule_twt_config)
{
  struct sk_buff *skb              = NULL;
  struct ieee80211_hdr *hdr        = NULL;
  struct rsi_common *common        = adapter->priv;
  u8 len                           = 0;
  u16 seq_num                      = 0;
  u16 twt_info_frame_size          = 0;
  twt_info_frame_t *twt_info_frame = NULL;
  __le16 *desc                     = NULL;
  struct xtended_desc *extend_desc = NULL;
  struct ieee80211_vif *vif        = adapter->vifs[adapter->sc_nvifs - 1];
  if (vif == NULL) {
    rsi_dbg(ERR_ZONE, "vif is NULL:%s\n ", __func__);
    return -1;
  }
  seq_num = ((struct vif_priv *)vif->drv_priv)->seq_num++;
  if (reschedule_twt_config.twt_action == SUSPEND_INDEFINITELY)
    twt_info_frame_size = sizeof(twt_info_frame_t) - sizeof(twt_info_frame->suspend_duration);
  else
    twt_info_frame_size = sizeof(twt_info_frame_t);
  len = twt_info_frame_size + MIN_802_11_HDR_LEN;

  skb = dev_alloc_skb(len + DWORD_ALIGNMENT); /* 64 for dword alignment */
  if (!skb) {
    rsi_dbg(ERR_ZONE, "Failed to alloc twt_setup_req\n");
    return -ENOMEM;
  }
  skb_put(skb, len);
  memset(skb->data, 0, skb->len);
  skb_reserve(skb, DWORD_ALIGNMENT);
  hdr                = (struct ieee80211_hdr *)skb->data;
  twt_info_frame     = (twt_info_frame_t *)&skb->data[MIN_802_11_HDR_LEN];
  hdr->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
  hdr->duration_id   = 0x0;
  hdr->seq_ctrl      = seq_num << 4;
  memcpy(hdr->addr1, common->sta_bssid, ETH_ALEN);
  memcpy(hdr->addr2, common->mac_addr, ETH_ALEN);
  memcpy(hdr->addr3, common->sta_bssid, ETH_ALEN);
  twt_info_frame->action_category    = WLAN_CATEGORY_S1G;
  twt_info_frame->action_type        = TWT_INFORMATION_ACTION;
  twt_info_frame->flow_id            = reschedule_twt_config.flow_id & 0x7;
  twt_info_frame->response_requested = 0;
  twt_info_frame->next_twt_request   = 0;
  twt_info_frame->all_twt            = 0;
  if (reschedule_twt_config.twt_action == SUSPEND_INDEFINITELY) {
    twt_info_frame->next_twt_subfield_size = 0;
  }
  //In case of SUSPEND_FOR_DURATION and RESUME_IMMEDIATELY actions
  else {
    twt_info_frame->next_twt_subfield_size = NEXT_TWT_SUBFIELD_SIZE;
    twt_info_frame->suspend_duration       = reschedule_twt_config.suspend_duration * 1000;
  }
  adapter->twt_rescheduling_in_progress = true;
  skb->priority                         = MGMT_SOFT_Q;
  if (rsi_prepare_mgmt_desc(common, skb))
    goto out;
  extend_desc = (struct xtended_desc *)&skb->data[FRAME_DESC_SZ];
  desc        = (__le16 *)skb->data;
  desc[1] |= cpu_to_le16(RSI_DESC_REQUIRE_CFM_TO_HOST);
  extend_desc->confirm_frame_type = TWT_INFO_FRAME_CONFIRM;
  extend_desc->retry_cnt          = reschedule_twt_config.flow_id & 0x7;
  extend_desc->reserved           = reschedule_twt_config.twt_action;

  rsi_hex_dump(MGMT_DEBUG_ZONE, "TWT_INFO_FRAME", skb->data, skb->len);
  skb_queue_tail(&common->tx_queue[MGMT_SOFT_Q], skb);
  rsi_set_event(&common->tx_thread.event);
  return 0;
out:
  dev_kfree_skb(skb);
  return 0;
}

static u64 calculate_min_required_wake_dur(twt_selection_t *user_config, u32 max_allowed_wake_interval_ms)
{
  //minimum wake duration required within the max allowable wake interval to achieve expected tx throughput given the fixed device average throughput.
  u64 min_wake_duration_required_ms =
    div64_u64(((u64)user_config->expected_tx_throughput * (u64)max_allowed_wake_interval_ms),
              (u64)user_config->device_avg_throughput);
  min_wake_duration_required_ms *=
    (100
     + user_config
         ->estimated_extra_wake_duration_percent); //Overestimating the time required by estimated_extra_wake_duration_percent
  min_wake_duration_required_ms = (div64_u64(min_wake_duration_required_ms, 100));
  //min_wake_duration_required_ms should be not be less than default minimum wake duration.
  if (min_wake_duration_required_ms < user_config->default_min_wake_duration_ms) {
    min_wake_duration_required_ms = user_config->default_min_wake_duration_ms;
  }
  return min_wake_duration_required_ms;
}

int use_case_based_twt_params_configuration(struct rsi_hw *adapter, twt_selection_t *user_config)
{
  rsi_twt_config_t *twt_config = &adapter->rsi_twt_config;
  uint64_t wake_interval, min_wake_duration_required;
  uint32_t max_allowed_wake_interval, num_twt_service_periods, wake_duration_per_sp, remainder;
  wake_interval = min_wake_duration_required = 0;
  max_allowed_wake_interval = num_twt_service_periods = wake_duration_per_sp = 0;
  //Taking minimum of rx_latency/tx_latency/Default_wake_interval as Maximum allowable Wake interval.
  if (user_config->tx_latency != 0) {
    max_allowed_wake_interval =
      MIN_OF_3(user_config->rx_latency, user_config->tx_latency, user_config->default_wake_interval_ms);
  } else {
    max_allowed_wake_interval = MIN_OF_2(user_config->rx_latency, user_config->default_wake_interval_ms);
  }
  min_wake_duration_required = calculate_min_required_wake_dur(user_config, max_allowed_wake_interval);
  if (min_wake_duration_required > (max_allowed_wake_interval >> 1)) {
    rsi_dbg(ERR_ZONE, "Invalid User Configuration\n");
    return -1;
  }
  /*wake duration unit 1 is allowed only if ratio of expected_tx_throughput to device_avg_throughput greater than 1/4
	  when it is the initial negotiation ie., when re-negotiating always use wake duration = 0 */
  if ((user_config->device_avg_throughput > (user_config->expected_tx_throughput << 2))
      || (twt_config->twt_wake_duration_unit == 1)) {
    wake_duration_per_sp               = (min_wake_duration_required < 64) ? min_wake_duration_required : 63;
    twt_config->twt_wake_duration_unit = 0;
  } else {
    wake_duration_per_sp               = (min_wake_duration_required < 256) ? min_wake_duration_required : 255;
    twt_config->twt_wake_duration_unit = (min_wake_duration_required < 64) ? 0 : 1;
  }
  div_u64_rem(min_wake_duration_required, wake_duration_per_sp, &remainder);
  num_twt_service_periods = div64_u64(min_wake_duration_required, wake_duration_per_sp) + ((remainder != 0) ? 1 : 0);
  num_twt_service_periods = (num_twt_service_periods != 0) ? num_twt_service_periods : 1;
  wake_duration_per_sp    = div64_u64((min_wake_duration_required << 10), num_twt_service_periods);
  twt_config->wake_duration =
    twt_config->twt_wake_duration_unit
      ? (wake_duration_per_sp >> 10)
      : (wake_duration_per_sp
         >> 8); //divide by 256 to convert from 1uS units to 256uS units and 1024 to convert to 1024us
  twt_config->wake_int_mantissa = (uint16_t)((uint64_t)max_allowed_wake_interval / (uint64_t)num_twt_service_periods);
  twt_config->wake_int_exp      = 0xA;
  twt_config->wake_int_exp_tol  = 0xFF;
  twt_config->wake_int_mantissa_tol         = 0xFFFF;
  twt_config->implicit_twt                  = 1;
  twt_config->un_announced_twt              = 0; //only announced TWT for use case based TWT
  twt_config->triggered_twt                 = 0;
  twt_config->twt_channel                   = 0; //twt_channel must be zero
  twt_config->twt_protection                = 0; //twt_protection must be zero
  twt_config->restrict_tx_outside_tsp       = 1;
  twt_config->twt_retry_limit               = 3;
  twt_config->twt_retry_interval            = 10;
  twt_config->req_type                      = 1; //0 - Request TWT; 1 - Suggest TWT; 2 - Demand TWT
  twt_config->negotiation_type              = 0;
  twt_config->twt_flow_id                   = 0;
  twt_config->twt_enable                    = 1;
  twt_config->rx_latency                    = user_config->rx_latency;
  twt_config->beacon_wake_up_count_after_sp = user_config->beacon_wake_up_count_after_sp;
  return 0;
}

#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
int process_rx_mgmt_beacon(struct rsi_hw *adapter, struct sk_buff *skb)
{
  u8 *buf, *ie_start;
  u16 ie_len, baselen;
  const struct element *elem;
  struct ieee80211_mgmt *mgmt;
  struct rsi_common *common;
  struct ieee80211_hdr *wlh;
  common = adapter->priv;
  mgmt   = (struct ieee80211_mgmt *)skb->data;
  wlh    = (struct ieee80211_hdr *)skb->data;
  if (mgmt == NULL || wlh == NULL) {
    rsi_dbg(ERR_ZONE, "mgmt/wlh is NULL \n");
    return -1;
  }
  ie_start = mgmt->u.beacon.variable;
  baselen  = offsetof(struct ieee80211_mgmt, u.beacon.variable);
  ie_len   = skb->len - baselen;
  elem     = (struct element *)cfg80211_find_ie(WLAN_EID_MULTIPLE_BSSID, ie_start, ie_len);
  if (elem == NULL) {
    rsi_dbg(INFO_ZONE, "AP dosen't support MBSSID\n");
    return -1;
  }
  ether_addr_copy(adapter->ax_params.trasmitter_bssid, mgmt->bssid);
  if (elem->data[0] > 0)
    adapter->ax_params.mbssid_mask_value = elem->data[0];

  buf = (u8 *)skb->data;
  if (buf == NULL) {
    rsi_dbg(ERR_ZONE, "buf is NULL \n");
    return -1;
  }
  if ((common->ssid != 0) && common->ssid_len == buf[37] && (memcmp(common->ssid, &buf[38], common->ssid_len) == 0)) {
    adapter->ax_params.mbssid_mask_value          = 0;
    adapter->ax_params.connected_nt_profile_index = 0;
    adapter->ax_params.profile_periodicity        = 1;
    return 0;
  }
  if (rsi_process_mbssid_parameters(adapter, mgmt, ie_start, ie_len)) {
    rsi_dbg(ERR_ZONE, "AP dosen't support MBSSID\n");
    return -1;
  }
  return 0;
}

int rsi_process_mbssid_parameters(struct rsi_hw *adapter, struct ieee80211_mgmt *mgmt, u8 *ie_start, int ie_len)
{
  const struct element *elem, *subelem;
  u8 *nt_profile;
  struct rsi_common *common;
  int nt_profile_len = 0;
  common             = adapter->priv;
  elem = (struct element *)cfg80211_find_ext_elem(WLAN_EID_EXT_MULTIPLE_BSSID_CONFIGURATION, ie_start, ie_len);
  if (elem && elem->datalen >= 3)
    adapter->ax_params.profile_periodicity = elem->data[2];
  else
    adapter->ax_params.profile_periodicity = 1;
  nt_profile = kmalloc(ie_len, GFP_ATOMIC);
  for_each_element_id(elem, WLAN_EID_MULTIPLE_BSSID, ie_start, ie_len)
  {
    if (elem->datalen < 2)
      continue;
    for_each_element(subelem, elem->data + 1, elem->datalen - 1)
    {
      const u8 *index;

      if (subelem->id != 0 || subelem->datalen < 4) {
        /* not a valid BSS profile */
        continue;
      }

      if (subelem->data[0] != WLAN_EID_NON_TX_BSSID_CAP || subelem->data[1] != 2) {
        /* The first element of the
                                 * Nontransmitted BSSID Profile is not
                                 * the Nontransmitted BSSID Capability
                                 * element.
                                 */
        continue;
      }

      memset(nt_profile, 0, ie_len);
      nt_profile_len = cfg80211_merge_profile(ie_start, ie_len, elem, subelem, nt_profile, ie_len);

      /* found a Nontransmitted BSSID Profile */
      index = cfg80211_find_ie(WLAN_EID_MULTI_BSSID_IDX, nt_profile, nt_profile_len);

      if (!index || index[1] < 1 || index[2] == 0) {
        /* Invalid MBSSID Index element */
        continue;
      }

      if (common->ssid == NULL) {
        rsi_dbg(ERR_ZONE, "%s: common->ssid is NULL\n", __func__);
        break;
      } else if (memcmp(common->ssid, &nt_profile[6], nt_profile[5]) == 0) {
        adapter->ax_params.connected_nt_profile_index = index[2];
        break;
      }
    }
  }
  kfree(nt_profile);
  return 0;
}
#endif

int send_diagnostic_report_resp_to_app(struct rsi_hw *adapter, u32 reg_val, int bytes)
{
  int res                 = 0;
  struct sk_buff *skb_out = { 0 };
  struct nlmsghdr *nlh;
  skb_out = nlmsg_new(bytes, 0);
  if (!skb_out) {
    rsi_dbg(ERR_ZONE, "%s: Failed to allocate new skb\n", __func__);
    return 0;
  }
  nlh = nlmsg_put(skb_out, adapter->wlan_nl_pid, 0, NLMSG_DONE, bytes, 0);
  /* NETLINK_CB(skb_out).dst_group = 0; */
  memcpy(nlmsg_data(nlh), &reg_val, bytes);
  res = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->wlan_nl_pid);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send stats to App\n", __func__);
    return -1;
  }
  return 0;
}

int diag_tool_update_dpd_fw(struct rsi_hw *adapter,
                            struct nlmsghdr *nlh,
                            uint16_t dpd_cmd,
                            uint16_t dpd_param_name,
                            uint16_t dpd_param_val)
{
  struct sk_buff *skb              = NULL;
  struct rsi_mac_frame *mgmt_frame = NULL;

  skb = dev_alloc_skb(FRAME_DESC_SZ);
  if (!skb) {
    rsi_dbg(ERR_ZONE, "%s: Failed in allocation of skb\n", __func__);
    return -ENOMEM;
  }

  memset(skb->data, 0, FRAME_DESC_SZ);
  mgmt_frame               = (struct rsi_mac_frame *)skb->data;
  mgmt_frame->desc_word[0] = cpu_to_le16(RSI_WIFI_MGMT_Q << 12);
  mgmt_frame->desc_word[1] = cpu_to_le16(DIAG_TOOL_CMD);
  mgmt_frame->desc_word[2] = cpu_to_le16(dpd_cmd);
  mgmt_frame->desc_word[3] = cpu_to_le16(dpd_param_name);
  mgmt_frame->desc_word[4] = cpu_to_le16(dpd_param_val);

  skb_put(skb, FRAME_DESC_SZ);
  rsi_hex_dump(MGMT_DEBUG_ZONE, "Diagnostic tool param update frame", skb->data, skb->len);

  return rsi_send_internal_mgmt_frame(adapter->priv, skb);
}

int send_diag_tool_response_to_app(struct rsi_hw *adapter, int data)
{
  struct sk_buff *skb_out = { 0 };
  struct nlmsghdr *nlh;
  int res;
  skb_out = nlmsg_new(2, 0);
  if (!skb_out) {
    rsi_dbg(ERR_ZONE, "%s: Failed to allocate new skb\n", __func__);
    return 0;
  }

  nlh = nlmsg_put(skb_out, adapter->wlan_nl_pid, 0, NLMSG_DONE, 2, 0);
  memcpy(nlmsg_data(nlh), &data, 2);
  res = nlmsg_unicast(adapter->nl_sk, skb_out, adapter->wlan_nl_pid);
  if (res < 0) {
    rsi_dbg(ERR_ZONE, "%s: Failed to send DIAG RESP DATA to App\n", __func__);
    return -1;
  }
  return 0;
}
//__9117_CODE_END
