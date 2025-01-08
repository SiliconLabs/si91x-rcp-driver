// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <stdio.h>
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

/******************************************************************************
 * main()
 *****************************************************************************/
int main(int argc, char *argv[])
{
  int sfd;
  int ret   = 0;
  int cmdNo = -1;
  struct bb_rf_param_bt_t bb_rf_params;
  int no_of_packets;
  unsigned int bb_addr = 0, bb_val = 0, len = 0;
  unsigned char ii = 0;
  struct nlmsghdr *nlh;

  memset(&bb_rf_params, 0, sizeof(bb_rf_params));

  if (argc < 2) {
    usage();
  } else if (argc <= 50) {
    cmdNo = get_bt_cmdnumber(argv[1]);
  }

  /* Open socket */
  sfd = socket_creation();
  if (sfd < 0) {
    ONEBOX_PRINT("Socket Creation Failed\n");
    return ONEBOX_STATUS_FAILURE;
  }

  switch (cmdNo) {
    case RSI_GET_BT_E2E_STATS: {
      if (argc == 3) {
        if (bt_e2e_stats(argv[2], sfd) == ONEBOX_STATUS_FAILURE) {
          return ONEBOX_STATUS_FAILURE;
        }
      } else {
        usage();
      }

    } break;
    case RSI_GET_BT_E2E_PERIOD_STATS: {
      if (argc == 4) {
        if (bt_e2e_period_stats(argv[2], atoi(argv[3]), sfd) == ONEBOX_STATUS_FAILURE) {
          return ONEBOX_STATUS_FAILURE;
        }
      } else {
        usage();
      }
    } break;
    case RSI_GET_BT_STATS: {
      if (argc == 4) {
        no_of_packets = atoi(argv[3]);
        if (bt_stats(argv[2], sfd, no_of_packets) == ONEBOX_STATUS_FAILURE) {
          return ONEBOX_STATUS_FAILURE;
        }
      } else {
        usage();
      }
    } break;

    case RSI_SET_BB_WRITE:
      if (argc == 4) {
        bb_addr = strtol(argv[2], NULL, 16);
        bb_val  = strtol(argv[3], NULL, 16);
        ONEBOX_PRINT("BB addr: 0x%x value 0x%x\n", bb_addr, bb_val);
        bb_rf_params.value        = 1;
        bb_rf_params.no_of_values = 2;
        bb_rf_params.soft_reset   = 0;
        bb_rf_params.Data[0]      = bb_addr;
        bb_rf_params.Data[1]      = bb_val;
      } else {
        usage();
        return ONEBOX_STATUS_FAILURE;
      }
      if (send_bb_read_write_frame_to_drv(bb_rf_params, sfd) < 0) {
        printf("Unable to perform BB_WRITE\n");
      } else
        printf("SUCCESS Writing to BB: \n");
      break;
    case RSI_SET_BB_READ:
      len = sizeof(bb_rf_params);
      if (argc == 3) {
        bb_addr = strtol(argv[2], NULL, 16);
        ONEBOX_PRINT("BB addr: 0x%x \n", bb_addr);
        bb_rf_params.value        = 0; //BB_READ_TYPE
        bb_rf_params.no_of_values = 1;
        bb_rf_params.soft_reset   = 0;
        bb_rf_params.Data[0]      = bb_addr;
      } else {
        usage();
        return ONEBOX_STATUS_FAILURE;
      }
      if (send_bb_read_write_frame_to_drv(bb_rf_params, sfd) < 0) {
        printf("Unable to perform BB_READ\n");
      } else
        printf("SUCCESS in Reading BB: \n");
      nlh = common_recv_mesg_wrapper(sfd, len);
      if (nlh == NULL) {
        printf("Error receving from bb\n");
        break;
      } else {
        memcpy(&bb_rf_params, NLMSG_DATA(nlh), len);
        for (ii = 0; ii < bb_rf_params.no_of_values; ii++) {
          printf("BB_read value is 0x%x\n", bb_rf_params.Data[ii]);
        }
      }
      free(nlh);
      break;
    default:
      break;
  }
  return ret;
}

int recv_bt_e2e_stat_from_drv(struct bt_stats_s *sta_info, int sfd)
{
  struct msghdr msg    = { 0 };
  struct nlmsghdr *nlh = NULL;
  struct iovec iov     = { 0 };
  struct sockaddr_nl dest_addr;
  nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(struct bt_stats_s)));
  memset(nlh, 0, NLMSG_SPACE(sizeof(struct bt_stats_s)));
  nlh->nlmsg_len  = NLMSG_SPACE(sizeof(struct bt_stats_s));
  iov.iov_base    = (void *)nlh;
  iov.iov_len     = nlh->nlmsg_len;
  msg.msg_name    = (void *)&dest_addr;
  msg.msg_namelen = sizeof(dest_addr);
  msg.msg_iov     = &iov;
  msg.msg_iovlen  = 1;

  if (recvmsg(sfd, &msg, 0) < 0) {
    close(sfd);
    return ONEBOX_STATUS_FAILURE;
  }
  memcpy(sta_info, NLMSG_DATA(nlh), sizeof(struct bt_stats_s));
  free(nlh);
  return ONEBOX_STATUS_SUCCESS;
}

int send_bt_e2e_stat_frame_to_drv(struct bb_rf_param_bt_t bb_rf_params, int sfd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh        = NULL;
  struct rsi_nl_desc *nl_desc = NULL;
  int ret;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux Kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(struct bb_rf_param_bt_t) + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(sizeof(struct bb_rf_param_bt_t) + NL_DATA_DESC_SZ));
  nlh->nlmsg_len        = NLMSG_SPACE(sizeof(struct bb_rf_param_t) + NL_DATA_DESC_SZ);
  nl_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nl_desc->desc_word[0] = BT_E2E_STAT;
  nl_desc->desc_word[1] = sizeof(struct bb_rf_param_bt_t);
  nlh->nlmsg_type       = BT_PACKET;
  nlh->nlmsg_pid        = getpid();
  nlh->nlmsg_flags      = 0;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, &bb_rf_params, sizeof(struct bb_rf_param_bt_t));
  ret = common_send_mesg_wrapper(sfd, dest_addr, nlh);
  if (ret < 0)
    close(sfd);
  free(nlh);
  return ret;
}

int send_bt_stat_frame_to_drv(struct bb_rf_param_bt_t bb_rf_params, int sfd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh        = NULL;
  struct rsi_nl_desc *nl_desc = NULL;
  int ret;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux Kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(struct bb_rf_param_bt_t) + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(sizeof(struct bb_rf_param_bt_t) + NL_DATA_DESC_SZ));
  nlh->nlmsg_len        = NLMSG_SPACE(sizeof(struct bb_rf_param_bt_t) + NL_DATA_DESC_SZ);
  nl_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nl_desc->desc_word[0] = BT_PER_STATS;
  nl_desc->desc_word[1] = sizeof(struct bb_rf_param_bt_t);
  nlh->nlmsg_type       = (unsigned short)BT_PACKET;
  nlh->nlmsg_pid        = getpid();
  nlh->nlmsg_flags      = 0;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, &bb_rf_params, sizeof(struct bb_rf_param_bt_t));
  ret = common_send_mesg_wrapper(sfd, dest_addr, nlh);
  if (ret < 0)
    close(sfd);
  free(nlh);
  return ret;
}

int send_bb_read_write_frame_to_drv(struct bb_rf_param_bt_t bb_rf_params, int sfd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh        = NULL;
  struct rsi_nl_desc *nl_desc = NULL;
  int ret;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux Kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(struct bb_rf_param_bt_t) + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(sizeof(struct bb_rf_param_bt_t) + NL_DATA_DESC_SZ));
  nlh->nlmsg_len        = NLMSG_SPACE(sizeof(struct bb_rf_param_bt_t) + NL_DATA_DESC_SZ);
  nl_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nl_desc->desc_word[0] = BT_PER;
  nl_desc->desc_word[1] = sizeof(struct bb_rf_param_bt_t);
  nlh->nlmsg_type       = (unsigned short)BT_PACKET;
  nlh->nlmsg_pid        = getpid();
  nlh->nlmsg_flags      = 0;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, &bb_rf_params, sizeof(struct bb_rf_param_bt_t));
  ret = common_send_mesg_wrapper(sfd, dest_addr, nlh);
  if (ret < 0)
    close(sfd);
  free(nlh);
  return ret;
}

int_32 bt_e2e_stats(uint_8 *file_name, int sfd)
{
  int first_time = 1;
  int count = 0, stop_bit = 0;
  int i;
  FILE *pFile;
  struct bt_stats_s *sta_info;
  struct bb_rf_param_bt_t bb_rf_params;
  float rcv_percentage;

  memset(&bb_rf_params, 0, sizeof(bb_rf_params));

  pFile = fopen(file_name, "w");
  if (pFile == NULL) {
    printf("Unable to create a file\n");
    return ONEBOX_STATUS_FAILURE;
  }
  sta_info = (struct bt_stats_s *)malloc(100);

  while (1) {

    if (usleep(SLEEP_TIME) != 0) {
      printf("Unable to sleep\n");
      break;
    }

    bb_rf_params.value = BT_E2E_STATS;

    if (send_bt_e2e_stat_frame_to_drv(bb_rf_params, sfd) < 0) {
      printf("Unable to perform BT E2E STATS request\n");
      free(sta_info);
      fclose(pFile);
      return ONEBOX_STATUS_FAILURE;
    } else {
      if (recv_bt_e2e_stat_from_drv(sta_info, sfd) < 0) {
        printf("Unable to receive BT E2E STATS\n");
        free(sta_info);
        fclose(pFile);
        return ONEBOX_STATUS_FAILURE;
      }
      if (first_time) {
        first_time = 0;
        fflush(pFile);
        continue;
      }

      if (sta_info->crc_fail)
        rcv_percentage = (sta_info->crc_fail * 100.0) / (sta_info->crc_pass + sta_info->crc_fail);
      else
        rcv_percentage = 0;

      if ((count % 20) == 0) {
        printf("%s %9s %9s %9s %9s %9s %9s %9s %9s %8s %5s  %s\n",
               "crc_fail",
               "crc_pass",
               "tx_aborts",
               "cca_stk",
               "fls_rx_start",
               "cca_idle",
               "rx-phy_abort",
               "fifo_occupied",
               "tx_dones",
               "id_pkts_rcvd",
               "rssi",
               "packet_error_rate");
        fprintf(pFile,
                "\n%s %9s %9s %9s %9s %9s %9s %9s %9s %8s %5s  %s\n",
                "crc_fail",
                "crc_pass",
                "tx_aborts",
                "cca_stk",
                "fls_rx_start",
                "cca_idle",
                "rx-phy_abort",
                "fifo_occupied",
                "tx_dones",
                "id_pkts_rcvd",
                "rssi",
                "packet_error_rate");
      }
      printf("%2d %10d %10d %10d %10d %10d %10d %10d %12d %11d %11d %14f\n",
             sta_info->crc_fail,
             sta_info->crc_pass,
             sta_info->tx_aborts,
             sta_info->cca_stk,
             sta_info->fls_rx_start,
             sta_info->cca_idle,
             sta_info->rx_phy_abort,
             sta_info->fifo_occupied,
             sta_info->tx_dones,
             sta_info->id_pkts_rcvd,
             sta_info->rssi,
             rcv_percentage);
      fprintf(pFile,
              "%2d %10d %10d %10d %10d %10d %10d %10d %11d %11d %12d %14f\n",
              sta_info->crc_fail,
              sta_info->crc_pass,
              sta_info->tx_aborts,
              sta_info->cca_stk,
              sta_info->fls_rx_start,
              sta_info->cca_idle,
              sta_info->rx_phy_abort,
              sta_info->fifo_occupied,
              sta_info->tx_dones,
              sta_info->id_pkts_rcvd,
              sta_info->rssi,
              rcv_percentage);
      ++count;
      memset(sta_info, 0, 100);
    }
    fflush(pFile);
  }
  free(sta_info);
  fclose(pFile);
  return ONEBOX_STATUS_SUCCESS;
}

int_32 bt_e2e_period_stats(uint_8 *file_name, int period, int sfd)
{

  int first_time = 1;
  int count = 0, stop_bit = 0;
  int i;
  FILE *pFile;
  struct bt_stats_s *sta_info;
  struct bb_rf_param_bt_t bb_rf_params;

  memset(&bb_rf_params, 0, sizeof(bb_rf_params));

  pFile = fopen(file_name, "w");
  if (pFile == NULL) {
    printf("Unable to create a file\n");
    return ONEBOX_STATUS_FAILURE;
  }
  sta_info = (struct bt_stats_s *)malloc(100);

  while (period) {

    if (usleep(SLEEP_TIME) != 0) {
      printf("Unable to sleep\n");
      break;
    }

    bb_rf_params.value = BT_E2E_STATS;

    if (send_bt_e2e_stat_frame_to_drv(bb_rf_params, sfd) < 0) {
      printf("Unable to perform BT E2E PERIODIC STATS request\n");
      free(sta_info);
      fclose(pFile);
      return ONEBOX_STATUS_FAILURE;
    } else {
      if (recv_bt_e2e_stat_from_drv(sta_info, sfd) < 0) {
        free(sta_info);
        fclose(pFile);
        return ONEBOX_STATUS_FAILURE;
      }
      if (first_time) {
        first_time = 0;
        fflush(pFile);
        continue;
      }

      if ((count % 20) == 0) {
        printf("%s %11s %11s %11s %14s %12s %12s %12s %11s %12s %8s\n",
               "crc_fail",
               "crc_pass",
               "tx_aborts",
               "cca_stk",
               "fls_rx_start",
               "cca_idle",
               "rx_phy_abort",
               "fifo_occupied",
               "tx_dones",
               "id_pkts_rcvd",
               "rssi");
        fprintf(pFile,
                "\n%s %11s %11s %11s %14s %12s %12s %12s %11s %12s %8s\n",
                "crc_fail",
                "crc_pass",
                "tx_aborts",
                "cca_stk",
                "fls_rx_start",
                "cca_idle",
                "rx-phy_abort",
                "fifo_occupied",
                "tx_dones",
                "id_pkts_rcvd",
                "rssi");
      }
      printf("%2d %12d %12d %12d %12d %12d %12d %12d %12d %12d %12d\n",
             sta_info->crc_fail,
             sta_info->crc_pass,
             sta_info->tx_aborts,
             sta_info->cca_stk,
             sta_info->fls_rx_start,
             sta_info->cca_idle,
             sta_info->rx_phy_abort,
             sta_info->fifo_occupied,
             sta_info->tx_dones,
             sta_info->id_pkts_rcvd,
             sta_info->rssi);
      fprintf(pFile,
              "%2d %12d %12d %12d %12d %12d %12d %12d %12d %12d %12d\n",
              sta_info->crc_fail,
              sta_info->crc_pass,
              sta_info->tx_aborts,
              sta_info->cca_stk,
              sta_info->fls_rx_start,
              sta_info->cca_idle,
              sta_info->rx_phy_abort,
              sta_info->fifo_occupied,
              sta_info->tx_dones,
              sta_info->id_pkts_rcvd,
              sta_info->rssi);
      ++count;
      memset(sta_info, 0, 100);
    }
    fflush(pFile);
    period--;
  }
  free(sta_info);
  fclose(pFile);
  return ONEBOX_STATUS_SUCCESS;
}

int recv_bt_stat_from_drv(struct bt_per_stats_s *sta_info, int sfd)
{
  struct msghdr msg    = { 0 };
  struct nlmsghdr *nlh = NULL;
  struct iovec iov     = { 0 };
  struct sockaddr_nl dest_addr;
  nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(struct bt_stats_s)));
  memset(nlh, 0, NLMSG_SPACE(sizeof(struct bt_per_stats_s)));
  nlh->nlmsg_len  = NLMSG_SPACE(sizeof(struct bt_per_stats_s));
  iov.iov_base    = (void *)nlh;
  iov.iov_len     = nlh->nlmsg_len;
  msg.msg_name    = (void *)&dest_addr;
  msg.msg_namelen = sizeof(dest_addr);
  msg.msg_iov     = &iov;
  msg.msg_iovlen  = 1;

  if (recvmsg(sfd, &msg, 0) < 0) {
    close(sfd);
    return ONEBOX_STATUS_FAILURE;
  }
  memcpy(sta_info, NLMSG_DATA(nlh), sizeof(struct bt_stats_s));
  free(nlh);
  return ONEBOX_STATUS_SUCCESS;
}

int_32 bt_stats(uint_8 *file_name, int sfd, int no_of_packets)
{
  int first_time = 1;
  int count = 0, stop_bit = 0, no_pkts = 0;
  int i;
  FILE *pFile;
  FILE *pFile_csv;
  struct bt_per_stats_s *sta_info;
  struct bb_rf_param_bt_t bb_rf_params;
  float rcv_percentage;

  memset(&bb_rf_params, 0, sizeof(bb_rf_params));

  pFile = fopen(file_name, "w");
  if (pFile == NULL) {
    printf("Unable to create a file\n");
    return ONEBOX_STATUS_FAILURE;
  }

  pFile_csv = fopen("bt_ble_data.csv", "w");
  if (pFile_csv == NULL) {
    printf("Unable to create a file\n");
    fclose(pFile);
    return ONEBOX_STATUS_FAILURE;
  }
  printf("\nCSV File Created\n");

  fflush(stdout);
  fflush(pFile_csv);
  fprintf(pFile_csv, "%s,%s,%s,%s\n", "crc_pass", "crc_fail", "rssi", "no_pkts");

  sta_info = (struct bt_per_stats_s *)malloc(100);

  while (1) {

    if (usleep(SLEEP_TIME) != 0) {
      printf("Unable to sleep\n");
      break;
    }

    bb_rf_params.value = BT_PER_STATS;

    if (send_bt_stat_frame_to_drv(bb_rf_params, sfd) < 0) {
      printf("Unable to perform BT E2E STATS request\n");
      free(sta_info);
      fclose(pFile);
      fclose(pFile_csv);
      return ONEBOX_STATUS_FAILURE;
    } else {
      if (recv_bt_stat_from_drv(sta_info, sfd) < 0) {
        printf("Unable to receive BT E2E STATS\n");
        free(sta_info);
        fclose(pFile);
        fclose(pFile_csv);
        return ONEBOX_STATUS_FAILURE;
      }
      if (first_time) {
        first_time = 0;
        fflush(pFile);
        fflush(pFile_csv);
        continue;
      }
      if (sta_info->crc_fail)
        rcv_percentage = (sta_info->crc_fail * 100.0) / (sta_info->crc_pass + sta_info->crc_fail);
      else
        rcv_percentage = 0;

      if ((count % 20) == 0) {
        printf("%12s %12s %12s %12s %12s %12s\n",
               "crc_fail",
               "crc_pass",
               "id_pkts_rcvd",
               "tx_dones",
               "rssi",
               "packet_error_rate");
        fprintf(pFile,
                "\n%12s %12s %12s %12s %12s %12s\n",
                "crc_fail",
                "crc_pass",
                "id_pkts_rcvd",
                "tx_dones",
                "rssi",
                "packet_error_rate");
      }
      printf("%12d %12d %12d %12d %12d %12f\n",
             sta_info->crc_fail,
             sta_info->crc_pass,
             sta_info->id_pkts_rcvd,
             sta_info->tx_dones,
             sta_info->rssi,
             rcv_percentage);
      fprintf(pFile,
              "%12d %12d %12d %12d %12d %12f\n",
              sta_info->crc_fail,
              sta_info->crc_pass,
              sta_info->id_pkts_rcvd,
              sta_info->tx_dones,
              sta_info->rssi,
              rcv_percentage);

      fflush(pFile_csv);
      if (sta_info->crc_pass == 0 && sta_info->crc_fail == 0) {
        no_pkts = 50;
      } else {
        no_pkts = 0;
      }
      fprintf(pFile_csv, "%d,%d,%d,%d\n", sta_info->crc_pass, sta_info->crc_fail, sta_info->rssi, no_pkts);
      ++count;
      memset(sta_info, 0, 100);
    }
    fflush(pFile);
    fflush(pFile_csv);
  }
  free(sta_info);
  fclose(pFile);
  fclose(pFile_csv);
  return ONEBOX_STATUS_SUCCESS;
}

int get_bt_cmdnumber(char *command)
{
  if (!strcmp(command, "bt_e2e_stats"))
    return RSI_GET_BT_E2E_STATS;
  else if (!strcmp(command, "bt_e2e_periodic_stats"))
    return RSI_GET_BT_E2E_PERIOD_STATS;
  else if (!strcmp(command, "bt_stats"))
    return RSI_GET_BT_STATS;
  else if (!strcmp(command, "bb_write"))
    return RSI_SET_BB_WRITE;
  else if (!strcmp(command, "bb_read"))
    return RSI_SET_BB_READ;
  else
    usage();
  return ONEBOX_STATUS_SUCCESS;
}

/** This function gives the usage of the onebox utility
 * @param  void
 * @return void 
 */
void usage()
{
  ONEBOX_PRINT("Usage: ./bt_util bt_e2e_stats < filename >\n");
  ONEBOX_PRINT("Usage: ./bt_util bt_e2e_periodic_stats < filename > < duration >\n");
  ONEBOX_PRINT("Usage: ./bt_util bt_stats < filename > <no_of_packets>\n");
  ONEBOX_PRINT("Usage: ./bt_util bb_write  <addr> < value> \n");
  ONEBOX_PRINT("Usage: ./bt_util bb_read  <addr>\n");
  return;
}
