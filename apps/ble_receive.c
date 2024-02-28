// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include "bt_util.h"
#include "per_util.h"
#include <signal.h>
#include <stdlib.h>

#define NO_OF_ARGS   15
#define STOP_OF_ARGS 2

void mapconversion(char *src, char *bitmap, int length)
{
  int ii;
  unsigned char temp[30];
  memset(temp, 0, 30);
  for (ii = 0; *src != '\0'; ii++) {
    if (*src == ':') {
      src++;
    }
    if ((*src >= '0') && (*src <= '9')) {
      *src -= '0';
      temp[ii] = *src;
    } else if ((*src >= 'a') && (*src <= 'f')) {
      *src -= 'a';
      *src += 0xa;
      temp[ii] = *src;
    } else if ((*src >= 'A') && (*src <= 'F')) {
      *src -= 'A';
      *src += 0xa;
      temp[ii] = *src;
    } else {
      ONEBOX_PRINT("Invalid bitmap\n");
      exit(1);
    }
    src++;
  }
  for (ii = 0; ii < length;) {
    *bitmap = (temp[ii] << 4) | (temp[ii + 1]);
    ii      = ii + 2;
    bitmap++;
  }
}

int main(int argc, char *argv[])
{
  int sfd;
  int ret   = 0, num_1;
  int cmd   = -1;
  int count = 0, stop_bit = 0, rc = 0, len;
  int first_time = 1;
  FILE *pFile;
  unsigned char macaddr[4];
  struct bb_rf_param_bt_t bb_rf_params;
  ble_per_recv_params_t ble_per_recv_params;
  struct nlmsghdr *nlh;
  memset(&bb_rf_params, 0, sizeof(bb_rf_params));
  printf("argc = %d\n", argc);
  sfd = socket_creation();
  if (sfd < 0) {
    ONEBOX_PRINT("Socket Creation Failed\n");
    return ONEBOX_STATUS_FAILURE;
  }
  if (argc == NO_OF_ARGS) {
    ble_per_recv_params.enable = 1;
    memset(macaddr, 0, 4);
    mapconversion(argv[1], macaddr, 8);

    for (num_1 = 0; num_1 < 4; num_1++) {
      ble_per_recv_params.access_addr[3 - num_1] = macaddr[num_1];
    }
    ble_per_recv_params.data_length_indication = atoi(argv[2]);
    ble_per_recv_params.scrambler_seed         = atoi(argv[3]);
    ble_per_recv_params.ble_rate               = atoi(argv[4]);
    ble_per_recv_params.bt_rx_rf_chnl          = atoi(argv[5]);
    ble_per_recv_params.bt_tx_rf_chnl          = atoi(argv[6]);
    ble_per_recv_params.le_chnl_type           = atoi(argv[7]);
    ble_per_recv_params.freq_hop_en            = atoi(argv[8]);
    ble_per_recv_params.ant_sel                = atoi(argv[9]);
    ble_per_recv_params.loop_back_mode         = atoi(argv[10]);
    ble_per_recv_params.pwrsave_options        = atoi(argv[11]);
    ble_per_recv_params.pll_mode               = atoi(argv[12]);
    ble_per_recv_params.rf_type                = atoi(argv[13]);
    ble_per_recv_params.rf_chain               = atoi(argv[14]);

    len                = sizeof(int);
    bb_rf_params.value = GET_DRV_COEX_MODE;
    if (ble_per_transmit_wrapper(bb_rf_params, BT_PER, sfd) < 0) {
      printf("Unable to perform bt_transmit\n");
      return ONEBOX_STATUS_FAILURE;
    }
    nlh = common_recv_mesg_wrapper(sfd, len);
    if (nlh == NULL) {
      printf("Error receving from bb\n");
      return ONEBOX_STATUS_FAILURE;
    } else {

      memcpy(&rc, NLMSG_DATA(nlh), len);
    }

    if ((rc == BT_LE_MODE) || (rc == BT_DUAL_MODE)) {
      if (ble_per_recv_params.bt_rx_rf_chnl > BT_LE_END_CHANNEL) {
        printf("Invalid rx_channel in LE oper mode\n");
        return ONEBOX_STATUS_FAILURE;
      }
      if (ble_per_recv_params.bt_tx_rf_chnl > BT_LE_END_CHANNEL) {
        printf("Invalid tx_channel in LE oper mode\n");
        return ONEBOX_STATUS_FAILURE;
      }
    } else {
      printf("Invalid Driver mode\n");
      return ONEBOX_STATUS_FAILURE;
    }

    if (!((ble_per_recv_params.ble_rate == BLE_1MBPS) || (ble_per_recv_params.ble_rate == BLE_2MBPS)
          || (ble_per_recv_params.ble_rate == BLR_500KBPS) || (ble_per_recv_params.ble_rate == BLR_125KBPS)
          || (ble_per_recv_params.ble_rate == BLR_BLE_SIM_RX))) {
      printf("Invalid BLE Rate\n");
      return ONEBOX_STATUS_FAILURE;
    }

    if (ble_per_recv_params.data_length_indication > 2) {
      printf("Invalid data_length_indication\n");
      return ONEBOX_STATUS_FAILURE;
    }

    if (ble_per_recv_params.le_chnl_type > 2) {
      printf("Invalid Channel Type\n");
      return ONEBOX_STATUS_FAILURE;
    }

#if 0
    if(!((ble_per_recv_params.skip_bb_rf_programming >= DISABLE_SKIP_PROG) 
          && (ble_per_recv_params.skip_bb_rf_programming <= ENABLE_SKIP_PROG)))
    {
      printf("Invalid bb_rf pogramming\n");
      return ONEBOX_STATUS_FAILURE;
    }
#endif
    if (!((ble_per_recv_params.loop_back_mode >= DISABLE_LOOP_BACK)
          && (ble_per_recv_params.loop_back_mode <= ENABLE_LOOP_BACK))) {
      printf("Invalid loop back mode\n");
      return ONEBOX_STATUS_FAILURE;
    }

#if 0
    if(!((ble_per_recv_params.pwrsave_options >= DISABLE_DUTY_CYCLE) 
          && (ble_per_recv_params.pwrsave_options <= ENABLE_DUTY_CYCLE)))
    {
      printf("pwrsave_options Selection\n");
      return ONEBOX_STATUS_FAILURE;
    }
#endif

    if (!((ble_per_recv_params.pll_mode >= PLL_MODE0) && (ble_per_recv_params.pll_mode <= PLL_MODE2))) {
      printf("Invalid pll mode\n");
      return ONEBOX_STATUS_FAILURE;
    }

    if (!((ble_per_recv_params.rf_type >= EXTERNAL_RF) && (ble_per_recv_params.rf_type <= INTERNAL_RF))) {
      printf("Invalid rf type\n");
      return ONEBOX_STATUS_FAILURE;
    }

    if ((ble_per_recv_params.rf_chain > BT_LP_CHAIN) || (ble_per_recv_params.rf_chain == WLAN_LP_CHAIN)) {
      printf("Invalid rf chain \n");
      return ONEBOX_STATUS_FAILURE;
    }

    bb_rf_params.value        = PER_BLE_RECEIVE;
    bb_rf_params.no_of_values = sizeof(ble_per_recv_params_t);
    memcpy(&bb_rf_params.Data[0], &ble_per_recv_params, sizeof(ble_per_recv_params_t));
    usleep(500000);
    if (ble_per_transmit_wrapper(bb_rf_params, BT_PER, sfd) < 0) {
      printf("Unable to perform bt_receive\n");
      return ONEBOX_STATUS_FAILURE;
    }
    printf("======== SUCCESS ============\n");
  } else if (argc == STOP_OF_ARGS) {
    ble_per_recv_params.enable = 0;
    bb_rf_params.value         = PER_BLE_RECEIVE;
    bb_rf_params.no_of_values  = sizeof(ble_per_recv_params_t);
    memcpy(&bb_rf_params.Data[0], &ble_per_recv_params, sizeof(ble_per_recv_params_t));
    if (ble_per_transmit_wrapper(bb_rf_params, BT_PER, sfd) < 0) {
      printf("Unable to stop bt_receive\n");
      return ONEBOX_STATUS_FAILURE;
    }
    printf("============= BT_RECEIVE_STOP ==================");

  } else {
    printf("USAGE: %s  <access_addr> <data_legth_indication> <scrambler_seed> <ble_rate> <rx_channel_index> "
           "<tx_channel_index> <le_channel_type> <freq_hop_en> <ant_sel> <loop_back_mode> <pwrsave_options> <pll_mode> "
           "<rf_type> <rf_chain>\n",
           argv[0]);
    return ONEBOX_STATUS_SUCCESS;
  }
}
