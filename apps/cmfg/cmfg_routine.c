// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */
#ifndef CMFG_H
#define CMFG_H

#include "cmfg.h"

#endif

// This function resets the device
void reset_routine(rx_packet_command_desc_t *rx_packet, int sfd)
{
  close(sfd);
  system("rmmod rsi_sdio rsi_91x");
  system("insmod rsi_91x.ko dev_oper_mode=1 rsi_zone_enabled=0x601 skip_fw_load=1");
  system("insmod rsi_sdio.ko");
  sleep(1);
  netlink_sfd = netlink_socket_creation();

  rx_packet->content[0] = SOFT_RESET_TARGET;
  rx_packet->content[1] = 0; //reserved
  rx_packet->content[2] = 0; //reserved
  rx_packet->content[3] = 0; //reserved
}

// This function reads data from the address and size specified by the commander
void read_routine(rx_packet_command_desc_t *rx_packet, tx_packet_command_desc_t *tx_packet, int sfd)
{
  mfg_rw_t mfg_rw, *ptr;
  int size                  = 0;
  uint16_t remaining_length = 0;
  uint16_t aligned_length   = 0;

  // Check if length is aligned or unaligned
  remaining_length = (tx_packet->length & 3);
  // Get the aligned length
  aligned_length = tx_packet->length - remaining_length;

  if (remaining_length == 0) {
    mfg_rw.address    = tx_packet->address;
    mfg_rw.length     = tx_packet->length;
    mfg_rw.read_write = MFG_READ;
    size              = sizeof(mfg_rw);
#ifdef DEBUG
    printf("Read routine rem_len = 0 mfg_rw packet size = %d\n", size);
#endif
    ptr = &mfg_rw;
    send_tx_packet(ptr, sfd, size);
    rx_packet->status = receive_rx_packet(ptr, sfd, size);
    memcpy(rx_packet->content, ptr->data, tx_packet->length);
  } else {

    mfg_rw.address    = tx_packet->address;
    mfg_rw.length     = aligned_length;
    mfg_rw.read_write = MFG_READ;
    size              = sizeof(mfg_rw);
#ifdef DEBUG
    printf("1. Read routine rem_len!= 0 mfg_rw packet size = %d\n", size);
#endif
    ptr = &mfg_rw;
    send_tx_packet(ptr, sfd, size);
    rx_packet->status = receive_rx_packet(ptr, sfd, size);
    memcpy(rx_packet->content, ptr->data, tx_packet->length);

    mfg_rw.address    = tx_packet->address + aligned_length;
    mfg_rw.length     = remaining_length;
    mfg_rw.read_write = MFG_READ;
    size              = sizeof(mfg_rw);
#ifdef DEBUG
    printf("2. Read routine rem_len!= 0 mfg_rw packet size = %d\n", size);
#endif
    ptr = &mfg_rw;
    send_tx_packet(ptr, sfd, size);
    rx_packet->status = receive_rx_packet(ptr, sfd, size);
    memcpy(rx_packet->content + aligned_length, ptr->data, tx_packet->length);
  }
  rx_packet->packet_length += tx_packet->packet_length;
}

// This function reads data from the address and size specified by the commander
void write_routine(rx_packet_command_desc_t *rx_packet, tx_packet_command_desc_t *tx_packet, int sfd)
{
  mfg_rw_t mfg_rw, *ptr;
  int size                  = 0;
  uint16_t remaining_length = 0;
  uint16_t aligned_length   = 0;

  // Check if length is aligned or unaligned
  remaining_length = (tx_packet->length & 3);
  // Get the aligned length
  aligned_length = tx_packet->length - remaining_length;

  if (remaining_length == 0) {
    mfg_rw.address    = tx_packet->address;
    mfg_rw.length     = aligned_length;
    mfg_rw.read_write = MFG_WRITE;
    memcpy(&mfg_rw.data, tx_packet->content, aligned_length);
    size = sizeof(mfg_rw);
#ifdef DEBUG
    printf("Write routine rem_len=0 mfg_rw packet size = %d\n", size);
#endif
    ptr               = &mfg_rw;
    rx_packet->status = send_tx_packet(ptr, sfd, size);
  } else {
    mfg_rw.address    = tx_packet->address;
    mfg_rw.length     = aligned_length;
    mfg_rw.read_write = MFG_WRITE;
    memcpy(&mfg_rw.data, tx_packet->content, aligned_length);
    size = sizeof(mfg_rw);
#ifdef DEBUG
    printf("1. Write routine rem_len!= 0 mfg_rw packet size = %d\n", size);
#endif
    ptr               = &mfg_rw;
    rx_packet->status = send_tx_packet(ptr, sfd, size);

    mfg_rw.address    = tx_packet->address + aligned_length;
    mfg_rw.length     = remaining_length;
    mfg_rw.read_write = MFG_WRITE;
    memcpy(&mfg_rw.data + aligned_length, tx_packet->content + aligned_length, remaining_length);
    size = sizeof(mfg_rw);
#ifdef DEBUG
    printf("2. Write routine rem_len!= 0 mfg_rw packet size = %d\n", size);
#endif
    ptr               = &mfg_rw;
    rx_packet->status = send_tx_packet(ptr, sfd, size);
  }
}
