// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */
#ifndef CMFG_H
#define CMFG_H

#include "cmfg.h"

#endif

int netlink_socket_creation()
{
  int nl_sfd;
  struct sockaddr_nl src_addr;
  nl_sfd = socket(PF_NETLINK, SOCK_RAW, SOCKET_PROTOCOL);

  if (nl_sfd < 0) {
    printf("User space netlink socket could not be created\n");
    return -1;
  }

  memset(&src_addr, 0, sizeof(src_addr));
  src_addr.nl_family = AF_NETLINK;
  src_addr.nl_pid    = getpid();

  if (bind(nl_sfd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) {
    printf("Unable to bind the socket\n");
    return -1;
  }
  return nl_sfd;
}

int send_tx_packet(mfg_rw_t *mfg_rw, int sfd, int size)
{
  printf("send_tx_packet entry\n");
  struct msghdr msg    = { 0 };
  struct nlmsghdr *nlh = NULL;
  struct iovec iov     = { 0 };
  struct sockaddr_nl dest_addr;
  struct rsi_nl_desc *nlh_desc = NULL;

  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0;
  dest_addr.nl_groups = 0;
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(size + 16));
  memset(nlh, 0, NLMSG_SPACE(size + 16));

  nlh->nlmsg_len         = NLMSG_SPACE(size + 16);
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = MANUFACTURING;
  nlh->nlmsg_pid         = getpid();

  iov.iov_base    = (void *)nlh;
  iov.iov_len     = nlh->nlmsg_len;
  msg.msg_name    = (void *)&dest_addr;
  msg.msg_namelen = sizeof(dest_addr);
  msg.msg_iov     = &iov;
  msg.msg_iovlen  = 1;

  memcpy(NLMSG_DATA(nlh) + 16, mfg_rw, size);

  if (sendmsg(sfd, &msg, 0) < 0) {
    printf("Unable to send commander tx packet to driver\n");
    close(sfd);
    free(nlh);
    return -1;
  }
  free(nlh);
  return 0;
}

int receive_rx_packet(mfg_rw_t *mfg_rw, int sfd, int size)
{
  struct msghdr msg    = { 0 };
  struct nlmsghdr *nlh = NULL;
  struct iovec iov     = { 0 };
  struct sockaddr_nl dest_addr;
  struct rsi_nl_desc *nlh_desc = NULL;

  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0;
  dest_addr.nl_groups = 0;
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_RX_BUFF_SUPPORTED));
  memset(nlh, 0, NLMSG_SPACE(MAX_RX_BUFF_SUPPORTED));
  nlh->nlmsg_len         = NLMSG_SPACE(MAX_RX_BUFF_SUPPORTED);
  nlh->nlmsg_type        = (unsigned short)WLAN_PACKET;
  nlh_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nlh_desc->desc_word[0] = MANUFACTURING;
  nlh->nlmsg_pid         = getpid();
  nlh->nlmsg_flags       = 0;
  iov.iov_base           = (void *)nlh;
  iov.iov_len            = nlh->nlmsg_len;
  msg.msg_name           = (void *)&dest_addr;
  msg.msg_namelen        = sizeof(dest_addr);
  msg.msg_iov            = &iov;
  msg.msg_iovlen         = 1;

  if (recvmsg(sfd, &msg, 0) < 0) {
    printf("Unable to receive rx packet from driver\n");
    close(sfd);
    free(nlh);
    return -1;
  }

  memcpy(mfg_rw->data, NLMSG_DATA(nlh), MAX_RX_BUFF_SUPPORTED);
  free(nlh);
  return 0;
}
