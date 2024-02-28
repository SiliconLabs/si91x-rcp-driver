// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */
//__9117_CODE_START
#ifndef _DIAG_REPORT_H_
#define _DIAG_REPORT_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/types.h>
#include <linux/if.h>
#include <stdlib.h>
#include <linux/wireless.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "per_util.h"

#define DIAG_TOOL_REG_SPACE_REPORT  DIAG_TOOL_REPORT_DIR "diag_tool_register_report"
#define DIAG_TOOL_RF_FLASH_REPORT   DIAG_TOOL_REPORT_DIR "diag_tool_rf_flash_report.csv"
#define DIAG_TOOL_IPMU_FLASH_REPORT DIAG_TOOL_REPORT_DIR "diag_tool_ipmu_flash_report.csv"

#define BUF_SZ_32     32
#define BUF_SZ_64     64
#define BUF_SZ_128    128
#define BUF_SZ_256    256
#define MAX_LINE_SIZE BUF_SZ_256

#define IPMU_FLASH_DATA_ADDRESS 0x04000561
#define IPMU_FLASH_DATA_SIZE    58

#define RF_FLASH_DATA_ADDRESS 0x04000521
#define RF_FLASH_DATA_SIZE    64

#define BB_BASE_ADDR                  0x20108000
#define RF_PROG_SPI_IO_BASE_ADDR      0x41144000
#define MODEM_PLL_CONFIG_IO_BASE_ADDR 0x41138000
#define PMU_SPI_BASE_ADDR             0x24050000
#define PMU1_SPI_BASE_ADDR            (PMU_SPI_BASE_ADDR + 0xA000)
#define PMU2_SPI_BASE_ADDR            (PMU_SPI_BASE_ADDR + 0x8000)

#define CHK_LIMIT      1
#define COLOR_CODE_RED 31

#define update_report(fp, buf, node, member, chk_limit)                                                          \
  ({                                                                                                             \
    if (!chk_limit) {                                                                                            \
      printf("%26s %15d\n", #member, node->member);                                                              \
      fprintf(fp, "%26s %15d\n", #member, node->member);                                                         \
    } else {                                                                                                     \
      snprintf(buf,                                                                                              \
               sizeof(buf),                                                                                      \
               "%26s %15d %15d %15d\n",                                                                          \
               #member,                                                                                          \
               node->member,                                                                                     \
               member##_LOW_LIMIT,                                                                               \
               member##_HIGH_LIMIT);                                                                             \
      printf("\033[1;%dm%s\033[1;0m",                                                                            \
             ((node->member < member##_LOW_LIMIT) || (node->member > member##_HIGH_LIMIT)) ? COLOR_CODE_RED : 0, \
             buf);                                                                                               \
      fprintf(fp, "%s\n", buf);                                                                                  \
    }                                                                                                            \
  })
struct reg_data {
  uint32_t reg_addr;
  uint32_t reg_base_addr;
  uint32_t reg_offset_addr;
  int reg_ref_val;
  int reg_cur_val;
  struct reg_data *next;
};

#define trim_0p5na1_LOW_LIMIT                 0 //TODO
#define trim_0p5na1_HIGH_LIMIT                0 //TODO
#define bg_r_vdd_ulp_LOW_LIMIT                15
#define bg_r_vdd_ulp_HIGH_LIMIT               19
#define bg_r_ptat_vdd_ulp_LOW_LIMIT           1
#define bg_r_ptat_vdd_ulp_HIGH_LIMIT          5
#define resbank_trim_LOW_LIMIT                0 //TODO
#define resbank_trim_HIGH_LIMIT               0 //TODO
#define trim_sel_LOW_LIMIT                    0x4f
#define trim_sel_HIGH_LIMIT                   0x63
#define del_2x_sel_LOW_LIMIT                  0x1c
#define del_2x_sel_HIGH_LIMIT                 0x3f
#define freq_trim_LOW_LIMIT                   0
#define freq_trim_HIGH_LIMIT                  0x1e
#define coarse_trim_16k_LOW_LIMIT             0
#define coarse_trim_16k_HIGH_LIMIT            0
#define fine_trim_16k_LOW_LIMIT               0x36
#define fine_trim_16k_HIGH_LIMIT              0x61
#define coarse_trim_64k_LOW_LIMIT             0x6
#define coarse_trim_64k_HIGH_LIMIT            0x6
#define fine_trim_64k_LOW_LIMIT               0x45
#define fine_trim_64k_HIGH_LIMIT              0x6b
#define coarse_trim_32k_LOW_LIMIT             0x4
#define coarse_trim_32k_HIGH_LIMIT            0x4
#define fine_trim_32k_LOW_LIMIT               0x39
#define fine_trim_32k_HIGH_LIMIT              0x67
#define xtal1_trim_32k_LOW_LIMIT              0x0
#define xtal1_trim_32k_HIGH_LIMIT             0x7
#define xtal2_trim_32k_LOW_LIMIT              0x3
#define xtal2_trim_32k_HIGH_LIMIT             0xf
#define trim_ring_osc_LOW_LIMIT               0x0
#define trim_ring_osc_HIGH_LIMIT              0x7f
#define vbatt_status_1_LOW_LIMIT              0    //TODO
#define vbatt_status_1_HIGH_LIMIT             0    //TODO
#define str_temp_slope_LOW_LIMIT              0    //TODO
#define str_temp_slope_HIGH_LIMIT             0    //TODO
#define f2_nominal_LOW_LIMIT                  0    //TODO
#define f2_nominal_HIGH_LIMIT                 0    //TODO
#define str_nominal_temp_LOW_LIMIT            0    //TODO
#define str_nominal_temp_HIGH_LIMIT           0    //TODO
#define str_bjt_temp_sense_off_LOW_LIMIT      0    //TODO
#define str_bjt_temp_sense_off_HIGH_LIMIT     0    //TODO
#define str_bjt_temp_sense_slope_LOW_LIMIT    0    //TODO
#define str_bjt_temp_sense_slope_HIGH_LIMIT   0    //TODO
#define trim_sel_20Mhz_LOW_LIMIT              0x2c //TODO: cross check
#define trim_sel_20Mhz_HIGH_LIMIT             0x38 //TODO: cross check
#define ro_32khz_00_trim_LOW_LIMIT            0x0
#define ro_32khz_00_trim_HIGH_LIMIT           0x1f
#define scdc_dcdc_trim_LOW_LIMIT              0x0
#define scdc_dcdc_trim_HIGH_LIMIT             0x4
#define scdc_hpldo_trim_LOW_LIMIT             0x0
#define scdc_hpldo_trim_HIGH_LIMIT            0x4
#define reserved1_LOW_LIMIT                   0
#define reserved1_HIGH_LIMIT                  0
#define ldo_ctrl_LOW_LIMIT                    0 //TODO
#define ldo_ctrl_HIGH_LIMIT                   0 //TODO
#define vbg_tsbjt_efuse_LOW_LIMIT             0 //TODO
#define vbg_tsbjt_efuse_HIGH_LIMIT            0 //TODO
#define retn_ldo_lptrim_LOW_LIMIT             0
#define retn_ldo_lptrim_HIGH_LIMIT            4
#define reserved2_LOW_LIMIT                   0
#define reserved2_HIGH_LIMIT                  0
#define auxadc_offset_diff_LOW_LIMIT          0x0113
#define auxadc_offset_diff_HIGH_LIMIT         0x023f
#define auxadc_invgain_diff_int_LOW_LIMIT     0x4000 //TODO: auxadc_invgain_diff_int:2, auxadc_invgain_diff_frac:14
#define auxadc_invgain_diff_int_HIGH_LIMIT    0x5666 //TODO: "
#define auxadc_invgain_diff_frac_LOW_LIMIT    0      //TODO: "
#define auxadc_invgain_diff_frac_HIGH_LIMIT   0      //TODO: "
#define auxadc_offset_single_LOW_LIMIT        0x00FA
#define auxadc_offset_single_HIGH_LIMIT       0x0258
#define auxadc_invgain_single_int_LOW_LIMIT   0x4000 //TODO: auxadc_invgain_single_int:2, auxadc_invgain_single_frac:14
#define auxadc_invgain_single_int_HIGH_LIMIT  0x5666 //TODO: "
#define auxadc_invgain_single_frac_LOW_LIMIT  0      //TODO: "
#define auxadc_invgain_single_frac_HIGH_LIMIT 0      //TODO: "
#define set_vref1p3_LOW_LIMIT                 9
#define set_vref1p3_HIGH_LIMIT                15
#define reserved3_LOW_LIMIT                   0
#define reserved3_HIGH_LIMIT                  0
#define trim_r1_resistorladder_LOW_LIMIT      0
#define trim_r1_resistorladder_HIGH_LIMIT     15
#define reserved4_LOW_LIMIT                   0
#define reserved4_HIGH_LIMIT                  0
//retn_ldo_hptrim 0 - 4
#define scale_soc_ldo_vref_LOW_LIMIT    0
#define scale_soc_ldo_vref_HIGH_LIMIT   1
#define ctrl_rf_LOW_LIMIT               0 //TODO
#define ctrl_rf_HIGH_LIMIT              0 //TODO
#define default_mode_LOW_LIMIT          0 //TODO
#define default_mode_HIGH_LIMIT         0 //TODO
#define test_ldopulldown_sel_LOW_LIMIT  0 //TODO
#define test_ldopulldown_sel_HIGH_LIMIT 0 //TODO
#define test_ldopulldown_LOW_LIMIT      0 //TODO
#define test_ldopulldown_HIGH_LIMIT     0 //TODO
#define drive_n_LOW_LIMIT               0 //TODO
#define drive_n_HIGH_LIMIT              0 //TODO
#define drive_p_LOW_LIMIT               0 //TODO
#define drive_p_HIGH_LIMIT              0 //TODO
#define deadtime_ctrl_n2p_LOW_LIMIT     0 //TODO
#define deadtime_ctrl_n2p_HIGH_LIMIT    0 //TODO
#define deadtime_ctrl_p2n_LOW_LIMIT     0 //TODO
#define deadtime_ctrl_p2n_HIGH_LIMIT    0 //TODO
#define revi_offset_prog_LOW_LIMIT      0 //TODO
#define revi_offset_prog_HIGH_LIMIT     0 //TODO
#define tran_lo_ctr_LOW_LIMIT           0 //TODO
#define tran_lo_ctr_HIGH_LIMIT          0 //TODO
#define tran_hi_ctr_LOW_LIMIT           0 //TODO
#define tran_hi_ctr_HIGH_LIMIT          0 //TODO
#define tran_und_shoot_ctr_LOW_LIMIT    0 //TODO
#define tran_und_shoot_ctr_HIGH_LIMIT   0 //TODO
#define dpwm_freq_trim_LOW_LIMIT        0
#define dpwm_freq_trim_HIGH_LIMIT       12

struct efuse_ipmu_s {
  uint32_t trim_0p5na1 : 1;
  uint32_t bg_r_vdd_ulp : 5;
  uint32_t bg_r_ptat_vdd_ulp : 3;
  uint32_t resbank_trim : 2;
  uint32_t trim_sel : 7;
  uint32_t del_2x_sel : 6;
  uint32_t freq_trim : 5;
  uint32_t coarse_trim_16k : 2;
  uint32_t fine_trim_16k : 7;
  uint32_t coarse_trim_64k : 2;
  uint32_t fine_trim_64k : 7;
  uint32_t coarse_trim_32k : 2;
  uint32_t fine_trim_32k : 7;
  uint32_t xtal1_trim_32k : 4;
  uint32_t xtal2_trim_32k : 4;
  uint32_t trim_ring_osc : 7;
  uint32_t vbatt_status_1 : 6;
  uint32_t str_temp_slope : 10;
  uint32_t f2_nominal : 10;
  uint32_t str_nominal_temp : 7;
  uint32_t str_bjt_temp_sense_off : 16;
  uint32_t str_bjt_temp_sense_slope : 16;
  uint32_t trim_sel_20Mhz : 7; // Trim value for 20mzh rc
  uint32_t ro_32khz_00_trim : 5;
  uint32_t scdc_dcdc_trim : 3;
  uint32_t scdc_hpldo_trim : 3;
  uint32_t reserved1 : 2;
  uint32_t ldo_ctrl : 4;
  uint32_t vbg_tsbjt_efuse : 12;
  uint32_t retn_ldo_lptrim : 2;
  uint32_t reserved2 : 2;
  uint32_t auxadc_offset_diff : 12;
  uint32_t auxadc_invgain_diff_int : 2;
  uint32_t auxadc_invgain_diff_frac : 14;
  uint32_t auxadc_offset_single : 12;
  uint32_t auxadc_invgain_single_int : 2;
  uint32_t auxadc_invgain_single_frac : 14;
  uint32_t set_vref1p3 : 4;
  uint32_t reserved3 : 6;
  uint32_t trim_r1_resistorladder : 4;
  uint32_t reserved4 : 19;
  uint32_t scale_soc_ldo_vref : 1;
  uint32_t ctrl_rf : 4;
  uint32_t default_mode : 1;
  uint32_t test_ldopulldown_sel : 1;
  uint32_t test_ldopulldown : 1;
  uint32_t drive_n : 2;
  uint32_t drive_p : 2;
  uint32_t deadtime_ctrl_n2p : 4;
  uint32_t deadtime_ctrl_p2n : 4;
  uint32_t revi_offset_prog : 3;
  uint32_t tran_lo_ctr : 2;
  uint32_t tran_hi_ctr : 2;
  uint32_t tran_und_shoot_ctr : 3;
  uint32_t dpwm_freq_trim : 4;
} __attribute__((packed));

#define xo_cap_trim_LOW_LIMIT            60
#define xo_cap_trim_HIGH_LIMIT           110
#define rf_temp_sens_count_f2_LOW_LIMIT  300
#define rf_temp_sens_count_f2_HIGH_LIMIT 900
#define evm_offset_11b_LOW_LIMIT         0
#define evm_offset_11b_HIGH_LIMIT        7
#define reserved_LOW_LIMIT               0
#define reserved_HIGH_LIMIT              0
#define LNA_F_CTRL_LOW_LIMIT             0
#define LNA_F_CTRL_HIGH_LIMIT            1
#define PADRV_CAP_TUNE_LOW_LIMIT         2
#define PADRV_CAP_TUNE_HIGH_LIMIT        7
#define PA_CS_BIAS_1_LOW_LIMIT           3
#define PA_CS_BIAS_1_HIGH_LIMIT          10
#define PA_CS_BIAS_4_LOW_LIMIT           3
#define PA_CS_BIAS_4_HIGH_LIMIT          10
#define PADDING_LOW_LIMIT                0
#define PADDING_HIGH_LIMIT               0
#define rx_gain_offset_1_LOW_LIMIT       -4
#define rx_gain_offset_1_HIGH_LIMIT      4
#define tx_gain_offset_1_LOW_LIMIT       -8
#define tx_gain_offset_1_HIGH_LIMIT      8
#define evm_offset_11n_mcs0_LOW_LIMIT    0
#define evm_offset_11n_mcs0_HIGH_LIMIT   6
#define tx_dci_1_LOW_LIMIT               -64
#define tx_dci_1_HIGH_LIMIT              64
#define tx_dcq_1_LOW_LIMIT               -64
#define tx_dcq_1_HIGH_LIMIT              64
#define tx_gain_imbalance_1_LOW_LIMIT    -64
#define tx_gain_imbalance_1_HIGH_LIMIT   64
#define tx_phase_imbalance_1_LOW_LIMIT   -64
#define tx_phase_imbalance_1_HIGH_LIMIT  64
#define gdb_est_noise_1_LOW_LIMIT        0x0D4
#define gdb_est_noise_1_HIGH_LIMIT       0x132
#define ph_deg_noise_1_LOW_LIMIT         0x3D1
#define ph_deg_noise_1_HIGH_LIMIT        0x045
#define rx_gain_offset_2_LOW_LIMIT       -8
#define rx_gain_offset_2_HIGH_LIMIT      8
#define evm_offset_11g_6m_LOW_LIMIT      0
#define evm_offset_11g_6m_HIGH_LIMIT     6
#define gdb_est_noise_2_LOW_LIMIT        0x0D4
#define gdb_est_noise_2_HIGH_LIMIT       0x132
#define ph_deg_noise_2_LOW_LIMIT         0x3D1
#define ph_deg_noise_2_HIGH_LIMIT        0x045
#define rx_gain_offset_3_LOW_LIMIT       -8
#define rx_gain_offset_3_HIGH_LIMIT      8
#define evm_offset_11g_54m_LOW_LIMIT     0
#define evm_offset_11g_54m_HIGH_LIMIT    6
#define gdb_est_noise_3_LOW_LIMIT        0x0D4
#define gdb_est_noise_3_HIGH_LIMIT       0x132
#define ph_deg_noise_3_LOW_LIMIT         0x3D1
#define ph_deg_noise_3_HIGH_LIMIT        0x045
#define rx_gain_offset_4_LOW_LIMIT       -4
#define rx_gain_offset_4_HIGH_LIMIT      4
#define tx_gain_offset_4_LOW_LIMIT       -8
#define tx_gain_offset_4_HIGH_LIMIT      8
#define tx_dbm_4_LOW_LIMIT               3
#define tx_dbm_4_HIGH_LIMIT              6
#define tx_dbc_4_LOW_LIMIT               0
#define tx_dbc_4_HIGH_LIMIT              0
#define tx_dci_4_LOW_LIMIT               -64
#define tx_dci_4_HIGH_LIMIT              64
#define tx_dcq_4_LOW_LIMIT               -64
#define tx_dcq_4_HIGH_LIMIT              64
#define tx_gain_imbalance_4_LOW_LIMIT    -64
#define tx_gain_imbalance_4_HIGH_LIMIT   64
#define tx_phase_imbalance_4_LOW_LIMIT   -64
#define tx_phase_imbalance_4_HIGH_LIMIT  64
#define gdb_est_noise_4_LOW_LIMIT        0x0D4
#define gdb_est_noise_4_HIGH_LIMIT       0x132
#define ph_deg_noise_4_LOW_LIMIT         0x3D1
#define ph_deg_noise_4_HIGH_LIMIT        0x045
#define tx_betacos_RU1_LOW_LIMIT         0xC0
#define tx_betacos_RU1_HIGH_LIMIT        0x140
#define tx_betasin_RU1_LOW_LIMIT         0x3C0
#define tx_betasin_RU1_HIGH_LIMIT        0x40
#define tx_betacos_RU2_LOW_LIMIT         0xC0
#define tx_betacos_RU2_HIGH_LIMIT        0x140
#define tx_betasin_RU2_LOW_LIMIT         0x3C0
#define tx_betasin_RU2_HIGH_LIMIT        0x140
#define tx_betacos_RU3_LOW_LIMIT         0xC0
#define tx_betacos_RU3_HIGH_LIMIT        0x140
#define tx_betasin_RU3_LOW_LIMIT         0x3C0
#define tx_betasin_RU3_HIGH_LIMIT        0x40
#define tx_betacos_RU4_LOW_LIMIT         0xC0
#define tx_betacos_RU4_HIGH_LIMIT        0x140
#define tx_betasin_RU4_LOW_LIMIT         0x3C0
#define tx_betasin_RU4_HIGH_LIMIT        0x40
#define tx_betacos_RU6_LOW_LIMIT         0xC0
#define tx_betacos_RU6_HIGH_LIMIT        0x140
#define tx_betasin_RU6_LOW_LIMIT         0x3C0
#define tx_betasin_RU6_HIGH_LIMIT        0x40
#define tx_betacos_RU8_LOW_LIMIT         0xC0
#define tx_betacos_RU8_HIGH_LIMIT        0x140
#define tx_betasin_RU8_LOW_LIMIT         0x3C0
#define tx_betasin_RU8_HIGH_LIMIT        0x40
#define tx_betacos_RU9_LOW_LIMIT         0xC0
#define tx_betacos_RU9_HIGH_LIMIT        0x140
#define tx_betasin_RU9_LOW_LIMIT         0x3C0
#define tx_betasin_RU9_HIGH_LIMIT        0x40

struct irf_calib_s {
  uint32_t xo_cap_trim : 8;
  uint32_t rf_temp_sens_count_f2 : 10;
  uint32_t evm_offset_11b : 8;
  uint32_t reserved : 2;
  uint32_t LNA_F_CTRL : 4;
  uint32_t PADRV_CAP_TUNE : 3;
  uint32_t PA_CS_BIAS_1 : 5;
  uint32_t PA_CS_BIAS_4 : 5;
  uint32_t PADDING : 3;
  uint32_t rx_gain_offset_1 : 4;
  uint32_t tx_gain_offset_1 : 8;
  uint32_t evm_offset_11n_mcs0 : 8;
  uint32_t tx_dci_1 : 10;
  uint32_t tx_dcq_1 : 10;
  uint32_t tx_gain_imbalance_1 : 10;
  uint32_t tx_phase_imbalance_1 : 10;
  uint32_t gdb_est_noise_1 : 10;
  uint32_t ph_deg_noise_1 : 10;
  uint32_t rx_gain_offset_2 : 4;
  uint32_t evm_offset_11g_6m : 8;
  uint32_t gdb_est_noise_2 : 10;
  uint32_t ph_deg_noise_2 : 10;
  uint32_t rx_gain_offset_3 : 4;
  uint32_t evm_offset_11g_54m : 8;
  uint32_t gdb_est_noise_3 : 10;
  uint32_t ph_deg_noise_3 : 10;
  uint32_t rx_gain_offset_4 : 4;
  uint32_t tx_gain_offset_4 : 8;
  uint32_t tx_dbm_4 : 4;
  uint32_t tx_dbc_4 : 4;
  uint32_t tx_dci_4 : 10;
  uint32_t tx_dcq_4 : 10;
  uint32_t tx_gain_imbalance_4 : 10;
  uint32_t tx_phase_imbalance_4 : 10;
  uint32_t gdb_est_noise_4 : 10;
  uint32_t ph_deg_noise_4 : 10;
  //#ifdef ENABLE_HAPIS
  uint32_t tx_betacos_RU1 : 10;
  uint32_t tx_betasin_RU1 : 10;
  uint32_t tx_betacos_RU2 : 10;
  uint32_t tx_betasin_RU2 : 10;
  uint32_t tx_betacos_RU3 : 10;
  uint32_t tx_betasin_RU3 : 10;
  uint32_t tx_betacos_RU4 : 10;
  uint32_t tx_betasin_RU4 : 10;
  uint32_t tx_betacos_RU6 : 10;
  uint32_t tx_betasin_RU6 : 10;
  uint32_t tx_betacos_RU8 : 10;
  uint32_t tx_betasin_RU8 : 10;
  uint32_t tx_betacos_RU9 : 10;
  uint32_t tx_betasin_RU9 : 10;
  uint32_t reserved1 : 20;
  uint8_t reserved2[10];
  //#endif
} __attribute__((packed));
#endif
//__9117_CODE_END
