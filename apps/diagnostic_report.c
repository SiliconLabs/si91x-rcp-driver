// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */
//__9117_CODE_START
#include "diagnostic_report.h"

int get_column_pos(char *line, char *mode)
{
  char *token = strtok(line, ",");
  int pos     = -1;

  while (token) {
    pos++;
    if (strcmp(token, mode) == 0)
      return pos;
    token = strtok(NULL, ",");
  }

  return -1;
}

int hex_str_to_int(const char *input_str)
{
  int i;
  int value             = 0;
  int dec_val           = 0;
  char hex_s[BUF_SZ_32] = { 0 };

  if (input_str == NULL)
    return ONEBOX_STATUS_FAILURE;

  for (i = 0; input_str[i] != '\0'; i++) {
    if (input_str[i] == '\'' || input_str[i] == ' ') {
      continue;
    }

    if (!isxdigit(input_str[i])) {
      ONEBOX_PRINT("%s: Error parsing register reference value\n", __func__);
      return ONEBOX_STATUS_FAILURE;
    }

    if (isdigit(input_str[i]))
      dec_val = input_str[i] - '0';
    else
      dec_val = toupper(input_str[i]) - 'A' + 10;

    value = (value << 4) | dec_val;
  }

  return value;
}

uint32_t get_reg_base_addr(char *token)
{
  if (strstr(token, "RF_SPI_TOP") || strstr(token, "WLAN_RF_PL") || strstr(token, "BT_RF_PLL"))
    return RF_PROG_SPI_IO_BASE_ADDR;
  if (strstr(token, "SPI_TOP") || strstr(token, "PLL480SS") || strstr(token, "PLL_480") || strstr(token, "FD")
      || strstr(token, "ADC+DAC") || strstr(token, "SPI_XO_BG"))
    return MODEM_PLL_CONFIG_IO_BASE_ADDR;
  if (strstr(token, "ULP ANALOG SPI") || strstr(token, "BOD") || strstr(token, "CLOCK") || strstr(token, "ADC+DAC")
      || strstr(token, "WURX") || strstr(token, "ULP BG"))
    return PMU1_SPI_BASE_ADDR;
  if (strstr(token, "PMU SPI"))
    return PMU2_SPI_BASE_ADDR;
  return ONEBOX_STATUS_FAILURE;
}

uint32_t get_reg_addr_from_offset(uint32_t reg_base_addr, int offset)
{
  if (offset < 0)
    return ONEBOX_STATUS_FAILURE;
  if (reg_base_addr == BB_BASE_ADDR)
    return (BB_BASE_ADDR + (offset << 1));
  if (reg_base_addr == RF_PROG_SPI_IO_BASE_ADDR)
    return (RF_PROG_SPI_IO_BASE_ADDR + ((offset) << 2));
  if (reg_base_addr == MODEM_PLL_CONFIG_IO_BASE_ADDR)
    return (MODEM_PLL_CONFIG_IO_BASE_ADDR + (offset << 2));
  if (reg_base_addr == PMU1_SPI_BASE_ADDR)
    return (PMU1_SPI_BASE_ADDR + (offset << 2));
  if (reg_base_addr == PMU2_SPI_BASE_ADDR)
    return (PMU1_SPI_BASE_ADDR + (offset << 2));
  return ONEBOX_STATUS_FAILURE;
}

ONEBOX_STATUS diag_load_reg_reference_file(char *fname, struct reg_data **head, char *mode)
{
  int pos                   = -1;
  int i                     = 0;
  FILE *file                = NULL;
  int offset                = 0;
  char *token               = NULL;
  uint32_t reg_addr         = 0;
  uint32_t reg_offset_addr  = 0;
  int reg_ref_val           = 0;
  int reg_base_addr         = 0;
  struct reg_data *curr     = NULL;
  struct reg_data *new_node = NULL;
  char line[MAX_LINE_SIZE]  = { 0 };

  file = fopen(fname, "r");
  if (file == NULL) {
    ONEBOX_PRINT("%s: Reg reference file %s access error\n", __func__, fname);
    return ONEBOX_STATUS_FAILURE;
  }

  if (fgets(line, MAX_LINE_SIZE, file) == NULL) {
    ONEBOX_PRINT("%s: Error: Empty file\n", __func__);
    fclose(file);
    return ONEBOX_STATUS_FAILURE;
  }

  pos = get_column_pos(line, mode);
  if (pos < 0) {
    ONEBOX_PRINT("%s: Error: Could not find column\n", __func__);
    fclose(file);
    return ONEBOX_STATUS_FAILURE;
  }

  while (fgets(line, MAX_LINE_SIZE, file) != NULL) {
    token = strtok(line, ",");
    if (strstr(token, "Register")) {
      reg_base_addr = get_reg_base_addr(token);
      continue;
    }

    reg_offset_addr = hex_str_to_int(token);
    reg_addr        = get_reg_addr_from_offset(reg_base_addr, reg_offset_addr);
    if (reg_addr <= 0) {
      ONEBOX_PRINT("%s: Invalid register address\n", __func__);
      continue;
    }

    for (i = 1; i <= pos; i++) {
      token = strtok(NULL, ",");
      if (!token) {
        continue;
      }
      if (i == pos)
        reg_ref_val = hex_str_to_int(token);
    }

    if ((reg_addr > 0) && (reg_ref_val >= 0)) {
      new_node                  = (struct reg_data *)malloc(sizeof(struct reg_data));
      new_node->reg_addr        = reg_addr;
      new_node->reg_offset_addr = reg_offset_addr;
      new_node->reg_ref_val     = reg_ref_val;
      new_node->reg_base_addr   = reg_base_addr;
      new_node->reg_cur_val     = 0;
      new_node->next            = NULL;
      if (*head == NULL) {
        *head = new_node;
        curr  = new_node;
      } else {
        curr->next = new_node;
        curr       = new_node;
      }
    }
  }
  fclose(file);

  return ONEBOX_STATUS_SUCCESS;
}

int drv_query_register(int sfd, uint32_t reg_addr, int no_of_bytes, int *reg_value)
{
  struct nlmsghdr *nlh;

  if (send_query_register_to_drv(sfd, reg_addr, no_of_bytes) < 0)
    return ONEBOX_STATUS_FAILURE;

  nlh = common_recv_mesg_wrapper(sfd, no_of_bytes);
  if (nlh == NULL) {
    printf("RECEVING FAILED\n");
    return ONEBOX_STATUS_FAILURE;
  } else {
    memcpy(reg_value, NLMSG_DATA(nlh), no_of_bytes);
  }
  return ONEBOX_STATUS_SUCCESS;
}

ONEBOX_STATUS diag_validate_reg(int sfd, char *mode, struct reg_data *head)
{
  int i                       = 0;
  int ret                     = 0;
  FILE *fp                    = NULL;
  int no_of_bytes             = 0;
  struct reg_data *curr       = head;
  char report_name[BUF_SZ_64] = { 0 };
  char header[BUF_SZ_256]     = { 0 };
  char reg_info[BUF_SZ_256]   = { 0 };

  snprintf(report_name, sizeof(report_name), "%s_%s.csv", DIAG_TOOL_REG_SPACE_REPORT, mode);
  fp = fopen(report_name, "w");
  if (fp == NULL) {
    ONEBOX_PRINT("%s: Unable to create report file %s", __func__, report_name);
    return ONEBOX_STATUS_FAILURE;
  }
  snprintf(header,
           sizeof(header),
           "%25s,%25s,%25s,%25s,%25s,%25s\n",
           "Register Address",
           "Register Base Address",
           "Register Offset Address",
           "Register Reference Value",
           "Register Current Value",
           "Report");

  fprintf(fp, "%s\n", header);
  printf("%s\n", header);

  while (curr != NULL) {
    /*
     * #define INT_RF_RD(_ADDR_)      *(volatile uint32 *)(RF_PROG_SPI_IO_BASE_ADDR + ((_ADDR_) << 2))
     * #define AFE_READ(ADDR)         *(volatile uint16 *)(MODEM_PLL_CONFIG_IO_BASE_ADDR + (ADDR << 2))
     * #define BB_READ                *(volatile uint16 *)(BB_BASE_ADDR + (ADDR << 1))
     * #define PMU1_DIRECT_ACCESS(_x) *(volatile uint32 *)(PMU_SPI_BASE_ADDR + 0xA000 + ((_x) << 2))
     * #define PMU2_DIRECT_ACCESS(_x) *(volatile uint32 *)(PMU_SPI_BASE_ADDR + 0x8000 + ((_x) << 2))
    */
    if ((curr->reg_base_addr == RF_PROG_SPI_IO_BASE_ADDR) || (curr->reg_base_addr == PMU1_SPI_BASE_ADDR)
        || (curr->reg_base_addr == PMU2_SPI_BASE_ADDR))
      no_of_bytes = 4;
    else
      no_of_bytes = 2;

    ret = drv_query_register(sfd, curr->reg_addr, no_of_bytes, &curr->reg_cur_val);
    if (ret < 0)
      continue;
    snprintf(reg_info,
             sizeof(reg_info),
             "%25x,%25x,%25x,%25x,%25x,%25s\n",
             curr->reg_addr,
             curr->reg_base_addr,
             curr->reg_offset_addr,
             curr->reg_ref_val,
             curr->reg_cur_val,
             (curr->reg_ref_val == curr->reg_cur_val ? "Valid" : "ERROR"));

    fprintf(fp, "%s\n", reg_info);
    printf("\033[1;%dm%s\033[1;0m", curr->reg_ref_val == curr->reg_cur_val ? 0 : COLOR_CODE_RED, reg_info);

    curr = curr->next;
  }

  fclose(fp);
  return ONEBOX_STATUS_SUCCESS;
}

void free_list(struct reg_data *head)
{
  struct reg_data *curr;

  while (head != NULL) {
    curr       = head;
    head       = head->next;
    curr->next = NULL;
    free(curr);
  }
}

ONEBOX_STATUS diag_generate_reg_report(int sfd, char *ref_file, char *mode)
{
  struct reg_data *reg_data_list = NULL;

  if (diag_load_reg_reference_file(ref_file, &reg_data_list, mode) < 0) {
    ONEBOX_PRINT("%s: Failed to parse input register reference file\n", __func__);
    free_list(reg_data_list);
    return ONEBOX_STATUS_FAILURE;
  }

  if (diag_validate_reg(sfd, mode, reg_data_list) < 0) {
    ONEBOX_PRINT("%s: Failed to generate report\n", __func__);
    free_list(reg_data_list);
    return ONEBOX_STATUS_FAILURE;
  }

  free_list(reg_data_list);

  return ONEBOX_STATUS_SUCCESS;
}

void diag_display_ipmu_flash_report(struct efuse_ipmu_s *impu_flash_data)
{
  FILE *fp                = NULL;
  char buf[128]           = { 0 };
  char header[BUF_SZ_256] = { 0 };

  fp = fopen(DIAG_TOOL_IPMU_FLASH_REPORT, "w");

  snprintf(header,
           sizeof(header),
           "%55s\n%26s %15s %15s %15s\n",
           "IPMU Production Calibrated Values",
           "Signal Name",
           "Signal Value",
           "Lower limit",
           "Upper limit");

  fprintf(fp, "%s\n", header);

  printf("--------------------------------------------------------------------------------------\n");
  printf("%s\n", header);
  printf("--------------------------------------------------------------------------------------\n");

  update_report(fp, buf, impu_flash_data, trim_0p5na1, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, bg_r_vdd_ulp, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, bg_r_ptat_vdd_ulp, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, resbank_trim, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, trim_sel, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, del_2x_sel, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, freq_trim, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, coarse_trim_16k, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, fine_trim_16k, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, coarse_trim_64k, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, fine_trim_64k, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, coarse_trim_32k, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, fine_trim_32k, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, xtal1_trim_32k, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, xtal2_trim_32k, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, trim_ring_osc, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, vbatt_status_1, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, str_temp_slope, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, f2_nominal, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, str_nominal_temp, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, str_bjt_temp_sense_off, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, str_bjt_temp_sense_slope, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, trim_sel_20Mhz, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, ro_32khz_00_trim, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, scdc_dcdc_trim, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, scdc_hpldo_trim, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, reserved1, 0);
  update_report(fp, buf, impu_flash_data, ldo_ctrl, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, vbg_tsbjt_efuse, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, retn_ldo_lptrim, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, reserved2, 0);
  update_report(fp, buf, impu_flash_data, auxadc_offset_diff, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, auxadc_invgain_diff_int, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, auxadc_invgain_diff_frac, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, auxadc_offset_single, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, auxadc_invgain_single_int, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, auxadc_invgain_single_frac, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, set_vref1p3, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, reserved3, 0);
  update_report(fp, buf, impu_flash_data, trim_r1_resistorladder, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, reserved4, 0);
  update_report(fp, buf, impu_flash_data, scale_soc_ldo_vref, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, ctrl_rf, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, default_mode, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, test_ldopulldown_sel, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, test_ldopulldown, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, drive_n, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, drive_p, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, deadtime_ctrl_n2p, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, deadtime_ctrl_p2n, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, revi_offset_prog, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, tran_lo_ctr, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, tran_hi_ctr, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, tran_und_shoot_ctr, CHK_LIMIT);
  update_report(fp, buf, impu_flash_data, dpwm_freq_trim, CHK_LIMIT);

  fclose(fp);
}
ONEBOX_STATUS read_flash_data(int sfd, int flash_address, int flash_size, uint8_t *flash_data)
{
  int i;
  struct nlmsghdr *nlh;

  for (i = 0; i < flash_size; i++) {
    if (send_query_register_to_drv(sfd, flash_address + i, 1) < 0)
      return ONEBOX_STATUS_FAILURE;

    nlh = common_recv_mesg_wrapper(sfd, 1);
    if (nlh == NULL) {
      printf("RECEVING FAILED\n");
      return ONEBOX_STATUS_FAILURE;
    } else {
      memcpy(flash_data + i, NLMSG_DATA(nlh), 1);
    }
  }

  return ONEBOX_STATUS_SUCCESS;
}

ONEBOX_STATUS diag_impu_flash_dump(int sfd, char *ifName)
{
  int ret                                       = 0;
  struct efuse_ipmu_s impu_flash_data           = { 0 };
  unsigned int flash_data[IPMU_FLASH_DATA_SIZE] = { 0 };

  ret = read_flash_data(sfd, IPMU_FLASH_DATA_ADDRESS, IPMU_FLASH_DATA_SIZE, (uint8_t *)flash_data);
  if (ret < 0)
    return ONEBOX_STATUS_FAILURE;

  impu_flash_data = *(struct efuse_ipmu_s *)flash_data;
  diag_display_ipmu_flash_report(&impu_flash_data);

  return ONEBOX_STATUS_SUCCESS;
}

void diag_display_rf_flash_report(struct irf_calib_s *node)
{
  FILE *fp                = NULL;
  char buf[128]           = { 0 };
  char header[BUF_SZ_256] = { 0 };

  fp = fopen(DIAG_TOOL_RF_FLASH_REPORT, "w");

  snprintf(header,
           sizeof(header),
           "%55s\n%26s %15s %15s %15s\n",
           "RF Production Calibrated Values",
           "Signal Name",
           "Signal Value",
           "Lower limit",
           "Upper limit");

  fprintf(fp, "%s\n", header);

  printf("--------------------------------------------------------------------------------------\n");
  printf("%s\n", header);
  printf("--------------------------------------------------------------------------------------\n");

  update_report(fp, buf, node, xo_cap_trim, CHK_LIMIT);
  update_report(fp, buf, node, rf_temp_sens_count_f2, CHK_LIMIT);
  update_report(fp, buf, node, evm_offset_11b, CHK_LIMIT);
  update_report(fp, buf, node, reserved, 0);
  update_report(fp, buf, node, LNA_F_CTRL, CHK_LIMIT);
  update_report(fp, buf, node, PADRV_CAP_TUNE, CHK_LIMIT);
  update_report(fp, buf, node, PA_CS_BIAS_1, CHK_LIMIT);
  update_report(fp, buf, node, PA_CS_BIAS_4, CHK_LIMIT);
  update_report(fp, buf, node, PADDING, 0);
  update_report(fp, buf, node, rx_gain_offset_1, CHK_LIMIT);
  update_report(fp, buf, node, tx_gain_offset_1, CHK_LIMIT);
  update_report(fp, buf, node, evm_offset_11n_mcs0, CHK_LIMIT);
  update_report(fp, buf, node, tx_dci_1, CHK_LIMIT);
  update_report(fp, buf, node, tx_dcq_1, CHK_LIMIT);
  update_report(fp, buf, node, tx_gain_imbalance_1, CHK_LIMIT);
  update_report(fp, buf, node, tx_phase_imbalance_1, CHK_LIMIT);
  update_report(fp, buf, node, gdb_est_noise_1, CHK_LIMIT);
  update_report(fp, buf, node, ph_deg_noise_1, CHK_LIMIT);
  update_report(fp, buf, node, rx_gain_offset_2, CHK_LIMIT);
  update_report(fp, buf, node, evm_offset_11g_6m, CHK_LIMIT);
  update_report(fp, buf, node, gdb_est_noise_2, CHK_LIMIT);
  update_report(fp, buf, node, ph_deg_noise_2, CHK_LIMIT);
  update_report(fp, buf, node, rx_gain_offset_3, CHK_LIMIT);
  update_report(fp, buf, node, evm_offset_11g_54m, CHK_LIMIT);
  update_report(fp, buf, node, gdb_est_noise_3, CHK_LIMIT);
  update_report(fp, buf, node, ph_deg_noise_3, CHK_LIMIT);
  update_report(fp, buf, node, rx_gain_offset_4, CHK_LIMIT);
  update_report(fp, buf, node, tx_gain_offset_4, CHK_LIMIT);
  update_report(fp, buf, node, tx_dbm_4, CHK_LIMIT);
  update_report(fp, buf, node, tx_dbc_4, CHK_LIMIT);
  update_report(fp, buf, node, tx_dci_4, CHK_LIMIT);
  update_report(fp, buf, node, tx_dcq_4, CHK_LIMIT);
  update_report(fp, buf, node, tx_gain_imbalance_4, CHK_LIMIT);
  update_report(fp, buf, node, tx_phase_imbalance_4, CHK_LIMIT);
  update_report(fp, buf, node, gdb_est_noise_4, CHK_LIMIT);
  update_report(fp, buf, node, ph_deg_noise_4, CHK_LIMIT);
  update_report(fp, buf, node, tx_betacos_RU1, CHK_LIMIT);
  update_report(fp, buf, node, tx_betasin_RU1, CHK_LIMIT);
  update_report(fp, buf, node, tx_betacos_RU2, CHK_LIMIT);
  update_report(fp, buf, node, tx_betasin_RU2, CHK_LIMIT);
  update_report(fp, buf, node, tx_betacos_RU3, CHK_LIMIT);
  update_report(fp, buf, node, tx_betasin_RU3, CHK_LIMIT);
  update_report(fp, buf, node, tx_betacos_RU4, CHK_LIMIT);
  update_report(fp, buf, node, tx_betasin_RU4, CHK_LIMIT);
  update_report(fp, buf, node, tx_betacos_RU6, CHK_LIMIT);
  update_report(fp, buf, node, tx_betasin_RU6, CHK_LIMIT);
  update_report(fp, buf, node, tx_betacos_RU8, CHK_LIMIT);
  update_report(fp, buf, node, tx_betasin_RU8, CHK_LIMIT);
  update_report(fp, buf, node, tx_betacos_RU9, CHK_LIMIT);
  update_report(fp, buf, node, tx_betasin_RU9, CHK_LIMIT);

  fclose(fp);
}

ONEBOX_STATUS diag_rf_flash_dump(int sfd, char *ifName)
{
  int ret                                     = 0;
  struct irf_calib_s rf_flash_data            = { 0 };
  unsigned int flash_data[RF_FLASH_DATA_SIZE] = { 0 };

  ret = read_flash_data(sfd, RF_FLASH_DATA_ADDRESS, RF_FLASH_DATA_SIZE, (uint8_t *)flash_data);
  if (ret < 0)
    return ONEBOX_STATUS_FAILURE;

  rf_flash_data = *(struct irf_calib_s *)flash_data;
  diag_display_rf_flash_report(&rf_flash_data);

  return ONEBOX_STATUS_SUCCESS;
}
//__9117_CODE_END
