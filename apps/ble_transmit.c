// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <stdio.h>
#include <string.h>
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
#include "bt_util.h"
#include "per_util.h"
#include <linux/genetlink.h>

#define NO_OF_ARGS   18
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
  int i, rc = 0, sfd, cmd;
  unsigned char ii = 0;
  unsigned int len = 0;
  char *tmp_rate;
  ble_per_params_t ble_per_params;
  struct bb_rf_param_bt_t bb_rf_params;
  struct nlmsghdr *nlh;
  unsigned int num_1;
  unsigned short int num_2;
  char acc_addr[4];

  memset(&ble_per_params, 0, sizeof(ble_per_params_t));
  memset(&bb_rf_params, 0, sizeof(bb_rf_params));

  sfd = socket_creation();
  if (sfd < 0) {
    ONEBOX_PRINT("Socket Creation Failed\n");
    return ONEBOX_STATUS_FAILURE;
  }
  if (argc == NO_OF_ARGS) {
    memset(acc_addr, 0, 4);
    mapconversion(argv[1], acc_addr, 8);
    ble_per_params.enable = 1;

    for (num_1 = 0; num_1 < 4; num_1++) {
      ble_per_params.access_addr[3 - num_1] = acc_addr[num_1];
    }

    ble_per_params.pkt_length        = atoi(argv[2]);
    ble_per_params.ble_rate          = atoi(argv[3]);
    ble_per_params.bt_rx_rf_chnl     = atoi(argv[4]);
    ble_per_params.bt_tx_rf_chnl     = atoi(argv[5]);
    ble_per_params.scrambler_seed    = atoi(argv[6]);
    ble_per_params.num_pkts          = atoi(argv[7]);
    ble_per_params.payload_data_type = atoi(argv[8]);
    ble_per_params.le_chnl           = atoi(argv[9]);
    ble_per_params.tx_pwr_indx       = atoi(argv[10]);
    ble_per_params.transmit_mode     = atoi(argv[11]);
    ble_per_params.hopping_type      = atoi(argv[12]);
    ble_per_params.ant_select        = atoi(argv[13]);
    ble_per_params.inter_pkt_gap     = atoi(argv[14]);
    ble_per_params.pll_mode          = atoi(argv[15]);
    ble_per_params.rf_type           = atoi(argv[16]);
    ble_per_params.rf_chain          = atoi(argv[17]);

    bb_rf_params.value = TX_STATUS;
    if (ble_per_transmit_wrapper(bb_rf_params, BT_PER, sfd) < 0) {
      printf("Unable to perform bt_transmit\n");
      return ONEBOX_STATUS_FAILURE;
    }
    len        = sizeof(int);
    nlh        = common_recv_mesg_wrapper(sfd, len);
    int status = 0;
    if (nlh == NULL) {
      printf("Error receving from bb\n");
      return ONEBOX_STATUS_FAILURE;
    } else {
      memcpy(&status, NLMSG_DATA(nlh), len);
      if (status < 0) {
        printf("======== ALREADY IN TRANSMIT SO STOP TRANSMIT FIRST ============\n");
        return ONEBOX_STATUS_FAILURE;
      }
    }
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
      if (ble_per_params.bt_rx_rf_chnl > BT_LE_END_CHANNEL) {
        printf("Invalid rx_channel in LE opermode\n");
        return ONEBOX_STATUS_FAILURE;
      }
      if (ble_per_params.bt_tx_rf_chnl > BT_LE_END_CHANNEL) {
        printf("Invalid tx_channel in LE oper mode\n");
        return ONEBOX_STATUS_FAILURE;
      }
    } else {
      printf("Invalid Driver mode\n");
      return ONEBOX_STATUS_FAILURE;
    }

    if (ble_per_params.transmit_mode > 2) {
      printf("Invalid transmit Mode\n");
      return ONEBOX_STATUS_FAILURE;
    }

#if 0
    if(((ble_per_params.tx_pwr_indx > BT_LP_CHAIN_THRESHOLD) && (ble_per_params.rf_chain == BT_LP_CHAIN))
      ||((ble_per_params.tx_pwr_indx == BT_LP_CHAIN_MIN_INVALID_TX_POWER_IDX || ble_per_params.tx_pwr_indx == BT_LP_CHAIN_MAX_INVALID_TX_POWER_IDX) 
        && (ble_per_params.rf_chain == BT_LP_CHAIN))) 
    {
      printf("Invalid transmit power\n");
      return ONEBOX_STATUS_FAILURE;
    }
#endif
    if ((ble_per_params.rf_chain == BT_LP_CHAIN)
        && ((ble_per_params.tx_pwr_indx > BT_LP_CHAIN_THRESHOLD)
            || (ble_per_params.tx_pwr_indx == BT_LP_CHAIN_MIN_INVALID_TX_POWER_IDX)
            || (ble_per_params.tx_pwr_indx == BT_LP_CHAIN_MAX_INVALID_TX_POWER_IDX))) {
      printf("Invalid transmit power\n");
      return ONEBOX_STATUS_FAILURE;
    }
    if (!((ble_per_params.ble_rate == BLE_1MBPS) || (ble_per_params.ble_rate == BLE_2MBPS)
          || (ble_per_params.ble_rate == BLR_500KBPS) || (ble_per_params.ble_rate == BLR_125KBPS))) {
      printf("Invalid ble_rate \n");
      return ONEBOX_STATUS_FAILURE;
    }

    if (ble_per_params.le_chnl > 2) {
      printf("Invalid le_chnl type \n");
      return ONEBOX_STATUS_FAILURE;
    }

    if (!((ble_per_params.hopping_type >= FREQ_NO_HOP) && (ble_per_params.hopping_type <= FREQ_RAND_HOP))) {
      printf("Invalid Hopping Type\n");
      return ONEBOX_STATUS_FAILURE;
    }

    if (!((ble_per_params.pll_mode >= PLL_MODE0) && (ble_per_params.pll_mode <= PLL_MODE2))) {
      printf("Invalid pll mode\n");
      return ONEBOX_STATUS_FAILURE;
    }

    if (ble_per_params.rf_type > INTERNAL_RF) {
      printf("Invalid rf_type\n");
      return ONEBOX_STATUS_FAILURE;
    }

    if ((ble_per_params.rf_chain > BT_LP_CHAIN) || (ble_per_params.rf_chain == WLAN_LP_CHAIN)) {
      printf("Invalid rf chain \n");
      return ONEBOX_STATUS_FAILURE;
    }

    printf("the packet length is %d \n", ble_per_params.pkt_length);

    bb_rf_params.value        = PER_BLE_TRANSMIT;
    bb_rf_params.no_of_values = sizeof(ble_per_params_t);

    memcpy(&bb_rf_params.Data[0], &ble_per_params, sizeof(ble_per_params_t));

    usleep(500);
    if (ble_per_transmit_wrapper(bb_rf_params, BT_PER, sfd) < 0) {
      printf("Unable to perform bt_transmit\n");
      return ONEBOX_STATUS_FAILURE;
    }
    printf("======== SUCCESS ============\n");
  } else if (argc == STOP_OF_ARGS) {
    if (!(strcmp(argv[1], "0"))) {
      ble_per_params.enable = 0;
      //    cmd = (unsigned short)PER_BLE_TRANSMIT ;
      bb_rf_params.value        = PER_BLE_TRANSMIT;
      bb_rf_params.no_of_values = sizeof(ble_per_params_t);

      memcpy(&bb_rf_params.Data[0], &ble_per_params, sizeof(ble_per_params_t));
      usleep(50000);
      if (ble_per_transmit_wrapper(bb_rf_params, BT_PER, sfd) < 0) {
        printf("Unable to perform bt_transmit\n");
        return ONEBOX_STATUS_FAILURE;
      } else {
        printf("============= BLE_TRANSMIT_STOP ==================\n");
      }
    } else {
      printf("Please enter either 0  as an argument, instead of %s to stop..\n", argv[1]);
    }
  } else {
    printf("\nUSAGE to start transmit: %s <Access_Addr> <pkt_length> <ble_rate> <rx_channel_index> <tx_channel_index> "
           "<scrambler_seed> <no_of_packets> <payload_type> <le_channel_type> <tx_power> <tx_mode> <hopping_type> "
           "<ant_sel> <inter_pkt_gap> <pll_mode> <rf_type> <rf_chain>\n",
           argv[0]);
    return ONEBOX_STATUS_SUCCESS;
  }
}
