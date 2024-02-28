// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "per_util.h"
#include "bt_util.h"
#include <unistd.h>
#include <linux/wireless.h>

int socket_creation()
{
  int sock_fd;
  struct sockaddr_nl src_addr;
  sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
  if (sock_fd < 0)
    return -1;
  memset(&src_addr, 0, sizeof(src_addr));
  src_addr.nl_family = AF_NETLINK;
  src_addr.nl_pid    = getpid(); /* self pid */

  bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
  return sock_fd;
}

int common_send_mesg_wrapper(int sock_fd, struct sockaddr_nl dest_addr, struct nlmsghdr *nlh)
{
  struct iovec iov  = { 0 };
  struct msghdr msg = { 0 };
  iov.iov_base      = (void *)nlh;
  iov.iov_len       = nlh->nlmsg_len;
  msg.msg_name      = (void *)&dest_addr;
  msg.msg_namelen   = sizeof(dest_addr);
  msg.msg_iov       = &iov;
  msg.msg_iovlen    = 1;
  if (sendmsg(sock_fd, &msg, 0) < 0) {
    printf("Sending Failed\n");
    return -1;
  }
  return 0;
}

struct nlmsghdr *common_recv_mesg_wrapper(int sock_fd, int len)
{
  /* Read message from kernel */
  struct msghdr msg = { 0 };
  struct iovec iov  = { 0 };
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh;
  nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(len));
  memset(nlh, 0, NLMSG_SPACE(len));
  nlh->nlmsg_len  = NLMSG_SPACE(len);
  iov.iov_base    = (void *)nlh;
  iov.iov_len     = nlh->nlmsg_len;
  msg.msg_name    = (void *)&dest_addr;
  msg.msg_namelen = sizeof(dest_addr);
  msg.msg_iov     = &iov;
  msg.msg_iovlen  = 1;

  if (recvmsg(sock_fd, &msg, 0) < 0) {
    printf("RECEVING FAILED\n");
    close(sock_fd);
    free(nlh);
    return NULL;
  }

  return nlh;
}

int gain_table_sckt_creation(update_wlan_gain_table_t table_info, int sfd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh        = NULL;
  struct rsi_nl_desc *nl_desc = NULL;
  int ret;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux Kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(table_info.struct_size + 8 + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(table_info.struct_size + 8 + NL_DATA_DESC_SZ));
  nlh->nlmsg_len        = NLMSG_SPACE(table_info.struct_size + 8 + NL_DATA_DESC_SZ);
  nl_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nl_desc->desc_word[0] = UPDATE_WLAN_GAIN_TABLE;
  nl_desc->desc_word[1] = table_info.struct_size + 8;
  nlh->nlmsg_type       = (unsigned short)WLAN_PACKET;
  nlh->nlmsg_pid        = getpid();
  nlh->nlmsg_flags      = 0;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, &table_info, table_info.struct_size + 8);
  ret = common_send_mesg_wrapper(sfd, dest_addr, nlh);
  if (ret < 0)
    close(sfd);
  free(nlh);
  return ret;
}

int ble_per_transmit_wrapper(struct bb_rf_param_bt_t bb_rf_params, int cmd, int sock_fd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret, len;
  len = sizeof(bb_rf_params);
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(len + NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = cmd;
  nlh_desc->desc_word[1] = len;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)BLE_PACKET;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, &bb_rf_params, len);
  ret = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);
  free(nlh);
  return ret;
}

int per_recv_send_wrapper(per_stats *sta_info,
                          int cmd,
                          int len,
                          int stop_bit,
                          unsigned char rate_flags,
                          int freq,
                          int sock_fd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux Kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(len + NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = cmd;
  nlh_desc->desc_word[1] = rate_flags;
  nlh_desc->desc_word[2] = freq;
  nlh_desc->desc_word[3] = stop_bit;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  ret                    = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if ((ret < 0) || stop_bit)
    close(sock_fd);
  free(nlh);
  return ret;
}

int per_transmit_wrapper(per_params_t per_params, int cmd, int sock_fd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret, len;
  len = sizeof(per_params_t);
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(len + NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = cmd;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, &per_params, len);
  ret = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);
  free(nlh);
  return ret;
}

int per_transmit_packet_wrapper(per_packet_t per_packet, int cmd, int sock_fd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret, len;
  len = sizeof(per_packet_t);
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(len + NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = cmd;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, &per_packet, len);
  ret = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);
  free(nlh);
  return ret;
}

int bb_read_write_wrapper(struct bb_rf_param_t bb_rf_params, int sock_fd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret, len;
  len = sizeof(bb_rf_params);
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(len + NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = RSI_SET_BB_RF;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, &bb_rf_params, len);
  ret = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);
  free(nlh);
  return ret;
}

int bb_rf_read_write_wrapper(struct bb_rf_param_t bb_rf_params, int sock_fd, unsigned short flags)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret, len;

  len = sizeof(bb_rf_params);
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(len + NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = RSI_SET_BB_RF;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = flags;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, &bb_rf_params, len);

  ret = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);

  free(nlh);
  return ret;
}

int master_read_write_wrapper(struct master_params_s master, int sock_fd, unsigned short read_write)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret, len;
  len = sizeof(master);

  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(len + NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = MASTER_OPS;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = read_write;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, &master, len);

  ret = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);

  free(nlh);
  return ret;
}

int_32 master_ops(int sfd, uint_8 type, uint_32 addr, uint_8 *data, uint_8 len)
{
  struct nlmsghdr *nlh = NULL;
  struct master_params_s master;
  uint_8 i, ii = 0;
  uint_16 read_write;

  memset(&master, 0, sizeof(struct master_params_s));
  master.address     = addr;
  master.no_of_bytes = len;
  len                = sizeof(master);
  if (type == 0) { //READ
    read_write = MASTER_READ;
    if (master_read_write_wrapper(master, sfd, read_write) < 0) {
      printf("Error Reading in master read \n");
    } else {
      nlh = common_recv_mesg_wrapper(sfd, len);
      if (nlh == NULL) {
        printf("Error receving from bb\n");
      } else {
        memcpy(&master, NLMSG_DATA(nlh), len);
        free(nlh);
        for (i = 0; i < master.no_of_bytes; i++) {
          if (!(i % 16))
            printf("0x%x ", master.data[i]);
          data[i] = master.data[i];
        }
        printf("\n");
      }
    }
  } else if (type == 1) { //WRITE
    if ((master.no_of_bytes != 1) && (master.no_of_bytes != 2) && (master.no_of_bytes != 4)) {
      printf("Enter valid no of bytes to write either 1, 2 or 4 )\n");
      return -1;
    }
    for (i = 0; i < master.no_of_bytes; i++) {
      master.data[i] = data[i];
    }
    for (i = 0; i < master.no_of_bytes; i++) {
      if (!(i % 16))
        printf("Data\n");
      printf("%x", master.data[i]);
    }
    printf("\n");
    read_write = MASTER_WRITE;
    if (master_read_write_wrapper(master, sfd, read_write) < 0) {
      printf("Error Reading in master write \n");
    }
  } else {
    printf("Invalid Master Operation \n");
    return -1;
  }
  return 0;
}

int prepare_and_send_nl_cmd(int sock_fd,
                            unsigned short cmd_type,
                            unsigned char rd_wr,
                            unsigned char msg_type,
                            void *ptr,
                            int len)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret;

  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  if (!nlh) {
    printf("malloc failed %s\n", __func__);
    return 0;
  }
  memset(nlh, 0, NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(len + NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = cmd_type;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = rd_wr;
  nlh->nlmsg_type        = (unsigned short)msg_type;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, ptr, len);
  ret = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);

  free(nlh);
  return ret;
}

int rsi_print_efuse_map(struct efuse_content_t *efuse_content, int sock_fd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret, len;
  len = sizeof(struct efuse_content_t);
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(len + NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = EFUSE_MAP;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, efuse_content, len);
  ret = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);
  free(nlh);
  return ret;
}

int send_filter_broadcast_frame_to_drv(struct fltr_bcast bcast, int sock_fd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret, len;
  len = sizeof(struct fltr_bcast);
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(len + NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = RSI_FILTER_BCAST;
  nlh_desc->desc_word[1] = len;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, &bcast, len);
  ret = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);
  free(nlh);
  return ret;
}

//__9117_CODE_START
int send_query_register_to_drv(int sock_fd, unsigned int reg_addr, int no_of_bytes)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux Kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = DIAGNOSTIC_REPORT;
  nlh_desc->desc_word[1] = reg_addr >> 16;
  nlh_desc->desc_word[2] = reg_addr;
  nlh_desc->desc_word[3] = no_of_bytes;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  ret                    = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);
  free(nlh);
  return ret;
}
//__9117_CODE_END

int send_get_rssi_frame_to_drv(int sock_fd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret, len;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = RSI_GET_RSSI;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  ret                    = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);
  free(nlh);
  return ret;
}

//__9117_CODE_START
int real_time_stats_wrapper(int rltime_enable, unsigned long int rltime_timer, int sock_fd, int len)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux Kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(len + NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = REAL_TIME_STATS;
  nlh_desc->desc_word[1] = rltime_enable;
  nlh_desc->desc_word[2] = rltime_timer & 0xffff;
  nlh_desc->desc_word[3] = ((rltime_timer >> 16) & 0xffff);
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  ret                    = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);
  free(nlh);
  return ret;
}

int set_hw_bmiss_threshold_value(unsigned short int bmiss_threshold_value, int sock_fd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux Kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = SET_HW_BMISS_THRESHOLD;
  nlh_desc->desc_word[1] = bmiss_threshold_value;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  ret                    = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);
  free(nlh);
  return ret;
}

int set_keep_alive_period(unsigned short int keep_alive_period, int sock_fd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux Kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(NL_DATA_DESC_SZ));
  if (nlh == NULL)
    return ONEBOX_STATUS_FAILURE;
  memset(nlh, 0, NLMSG_SPACE(NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = SET_KEEP_ALIVE_PERIOD;
  nlh_desc->desc_word[1] = keep_alive_period;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  ret                    = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);
  free(nlh);
  return ret;
}

#ifdef CONFIG_TWT_SUPPORT
int twt_config_wrapper(rsi_twt_user_params twt_params, int sock_fd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret, len;
  len = sizeof(rsi_twt_user_params);
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux Kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(len + NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = TWT_CONFIG;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, &twt_params, len);
  ret = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);
  free(nlh);
  return ret;
}

int twt_status_wrapper(int sock_fd, int len)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux Kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(len + NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(len + NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = TWT_STATUS;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  ret                    = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);
  free(nlh);
  return ret;
}
#endif

int diag_tool_param_update(int sock_fd, int diag_cmd, int diag_param_name, int diag_param_val)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh         = NULL;
  struct rsi_nl_desc *nlh_desc = NULL;
  int ret;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux Kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(NL_DATA_DESC_SZ));
  if (nlh == NULL)
    return ONEBOX_STATUS_FAILURE;
  memset(nlh, 0, NLMSG_SPACE(NL_DATA_DESC_SZ));
  nlh->nlmsg_len         = NLMSG_SPACE(NL_DATA_DESC_SZ);
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = DIAG_TOOL_UPDATE;
  nlh_desc->desc_word[1] = diag_cmd;
  nlh_desc->desc_word[2] = diag_param_name;
  nlh_desc->desc_word[3] = diag_param_val;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  ret                    = common_send_mesg_wrapper(sock_fd, dest_addr, nlh);
  if (ret < 0)
    close(sock_fd);
  free(nlh);
  return ret;
}

//__9117_CODE_END
