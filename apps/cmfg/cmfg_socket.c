// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */
#ifndef CMFG_H
#define CMFG_H

#include "cmfg.h"

#endif

int server_fd;
int client_fd;

int tcp_server(void)
{

  int pktlen = 0;
#ifdef DEBUG
  int count = 0;
#endif
  unsigned int client_len = 0;
  uint8_t packet[CMFG_MAX_COMMAND_LENGTH];
  tx_packet_command_desc_t *tx_pkt;
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;

  server_addr.sin_family      = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port        = htons(SOURCE_PORT);

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    printf("TCP Socket creation failed\n");
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &(int){ 1 }, sizeof(int)) < 0) {
    printf("Enabling port reuse using SO_REUSEPORT via setsockopt failed\n");
    return -1;
  }

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    printf("TCP server bind of CMFG Application failed\n");
    return -1;
  }

  if (listen(server_fd, 1) == -1) {
    printf("Error : Server is not listening on specified port\n");
    return -1;
  }

  printf("********** Server is up and listening on TCP PORT NUMBER : %d **********\n", SOURCE_PORT);
  printf("********** Please press Ctrl + C to terminate the application **********\n");
  client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
  if (client_fd < 0) {
    printf("Server could not establish connection with commander\n");
    return -1;
  }
  printf("Connection with commander established successfully!\n");
  printf("Waiting for input from USER\n");
  while (1) {
    allocate_tx_buffer(&tx_pkt);
    memset(packet, 0, CMFG_MAX_COMMAND_LENGTH);
    pktlen = recv(client_fd, packet, CMFG_MAX_COMMAND_LENGTH, 0);
    if (pktlen == 0)
      break;

    if ((allocate_tx_buffer(&tx_pkt) < 0)) {
      printf("Tx buffer allocation failed\n");
      break;
    }
    memcpy(tx_pkt, packet, pktlen);
    if (pktlen > 8) {
#ifdef DEBUG
      printf("\n/*****TX PACKET START*****/\n");
      printf("Packet of size %d received\n", pktlen);
      printf("Start tag -> %x\n", tx_pkt->start_tag);
      printf("sequence number -> %d\n", tx_pkt->sequence_number);
      printf("Packet_length (from start tag via commander)-> %d\n", tx_pkt->packet_length);
      printf("MCU_command -> %x\n", tx_pkt->MCU_command);
      printf("data size -> %d\n", tx_pkt->length);
      printf("address -> %02x\n", tx_pkt->address);
      printf("Data section of TX packet\n");
      for (int i = 0; i < tx_pkt->packet_length; i++) {
        printf("%02x ", tx_pkt->content[i]);
        count++;
        if (count == 16) {
          printf("\n");
          count = 0;
        }
      }
      printf("\nEnd of data section\n");
      printf("/*****TX PACKET ENDING*****/\n");
      printf("\n");
#endif
      if (process_request(tx_pkt, pktlen, netlink_sfd) < 0) {
        printf("Unable to process request\n");
        free(tx_pkt);
        break;
      }
      free(tx_pkt);
    }
  }

  printf("Closing socket connection\n");
  close(server_fd);
  close(client_fd);

  return 0;
}

int process_request(tx_packet_command_desc_t *tx_packet, uint16_t length, int netlink_sfd)
{
  int status                       = 0;
  uint16_t tx_packet_crc           = 0;
  uint16_t tx_packet_crc_calculate = 0;

  rx_packet_command_desc_t *rx_packet;
  allocate_rx_buffer(&rx_packet);
  memset(rx_packet, 0, MAX_RX_BUFF_SUPPORTED);
  rx_packet->start_tag       = 0xaa;
  rx_packet->sequence_number = tx_packet->sequence_number;
  rx_packet->packet_length   = 10;

  // TX Packet CRC Check
  tx_packet_crc           = *(uint16_t *)((uint8_t *)tx_packet + tx_packet->packet_length - 2);
  tx_packet_crc_calculate = crc16_computation((uint8_t *)tx_packet, tx_packet->packet_length - 2);
  if (tx_packet_crc != tx_packet_crc_calculate) {
    printf("TX Packet CRC Error\n");
    rx_packet->status        = TX_PACKET_CRC_CHECK_ERROR;
    rx_packet->packet_length = 0;
    if (server_response(rx_packet, rx_packet->packet_length, client_fd)) {
      free(rx_packet);
      return -1;
    }
  }

  switch (tx_packet->MCU_command) {
    case RESET_INTERFACE_COMMAND:
      printf("Reset Interface Case Entry\n");
      rx_packet->status = UNSUPPORTED_COMMAND;
      rx_crc_check(rx_packet, 0);
      if (server_response(rx_packet, rx_packet->packet_length, client_fd))
        status = -1;
      free(rx_packet);
      break;

    case INIT_INTERFACE_COMMAND:
      printf("Init Interface Case Entry\n");
      rx_packet->status                   = 0;
      rx_packet->content[0]               = 1;
      rx_packet->content[1]               = 0; //reserved
      *(uint16_t *)&rx_packet->content[2] = MAX_TX_BUFF_SUPPORTED;
#ifdef DEBUG
      printf("1.Content[0]-> %d\n", rx_packet->content[0]);
      printf("1.Content[1]-> %d\n", rx_packet->content[1]);
      printf("1.Content[2]-> %d\n", rx_packet->content[2]);
#endif
      rx_crc_check(rx_packet, 4);
      if (server_response(rx_packet, rx_packet->packet_length, client_fd))
        status = -1;
      free(rx_packet);
      break;

    case RESET_TARGET_COMMAND:
      printf("Reset Target Case Entry\n");
      reset_routine(rx_packet, netlink_sfd);
      rx_crc_check(rx_packet, 4);
      if (server_response(rx_packet, rx_packet->packet_length, client_fd))
        status = -1;
      free(rx_packet);
      break;

    case POWER_TARGET_COMMAND:
      printf("Power Target Case Entry\n");
      rx_packet->status = UNSUPPORTED_COMMAND;
      rx_crc_check(rx_packet, 0);
      printf("Unsupported command as of now\n");
      if (server_response(rx_packet, rx_packet->packet_length, client_fd))
        status = -1;
      free(rx_packet);
      break;

    case READ_MEMORY_COMMAND:
      printf("Read Case Entry\n");
      read_routine(rx_packet, tx_packet, netlink_sfd);
      rx_crc_check(rx_packet, tx_packet->length);
      if (server_response(rx_packet, rx_packet->packet_length, client_fd))
        status = -1;
      free(rx_packet);
      printf("Read Case exit\n\n");
      break;

    case WRITE_MEMORY_COMMAND:
      printf("Write Case Entry\n");
      write_routine(rx_packet, tx_packet, netlink_sfd);
      rx_crc_check(rx_packet, 0);
      if (server_response(rx_packet, rx_packet->packet_length, client_fd))
        status = -1;
      free(rx_packet);
      printf("Write Case Exit\n\n");
      break;

    default:
      rx_packet->status = INVALID_COMMAND;
      rx_crc_check(rx_packet, 0);
      if (server_response(rx_packet, rx_packet->packet_length, client_fd))
        status = -1;
      free(rx_packet);
      break;
  }
  return status;
}

int server_response(rx_packet_command_desc_t *resp_packet, uint16_t length, int client_fd)
{
  int resp_pktlen = 0;
#ifdef DEBUG
  int count = 0;
  printf("\n/*****RX PACKET START*****/\n");
  printf("Start tag -> %x\n", resp_packet->start_tag);
  printf("sequence number -> %d\n", resp_packet->sequence_number);
  printf("Packet_length (from start tag via commander)-> %d\n", resp_packet->packet_length);
  printf("Status-> %x\n", resp_packet->status);
  printf("Data section of RX packet\n");
  for (int i = 0; i < resp_packet->packet_length; i++) {
    printf("%02x ", resp_packet->content[i]);
    count++;
    if (count == 16) {
      printf("\n");
      count = 0;
    }
  }
  printf("\nEnd of data section\n");
  printf("/*****RX PACKET ENDING*****/\n");
#endif
  resp_pktlen = write(client_fd, resp_packet, length);
  if (resp_pktlen < 0) {
    printf("Failed to send response back to commander\n");
    return 1;
  } else
    printf("Response of size %d sent to commander\n", resp_pktlen);
  return 0;
}

void rx_crc_check(rx_packet_command_desc_t *resp_packet, uint32_t content_len)
{
  uint16_t rx_packet_crc           = 0;
  uint16_t rx_packet_crc_calculate = 0;
#ifdef DEBUG
  printf("RX CRC Check function entry\n");
#endif
  resp_packet->packet_length = 10 + content_len;
  rx_packet_crc              = crc16_computation((uint8_t *)resp_packet, resp_packet->packet_length - 2);
  *(uint16_t *)((uint8_t *)resp_packet + resp_packet->packet_length - 2) = rx_packet_crc;
  rx_packet_crc_calculate = *(uint16_t *)((uint8_t *)resp_packet + resp_packet->packet_length - 2);
  if (rx_packet_crc_calculate != rx_packet_crc) {
    printf("RX Packet CRC Error\n");
    resp_packet->status        = RX_PACKET_CRC_CHECK_ERROR;
    resp_packet->packet_length = 0;
  }
}

int allocate_tx_buffer(tx_packet_command_desc_t **tx_packet_command)
{
  *tx_packet_command = malloc(MAX_TX_BUFF_SUPPORTED);
  if (*tx_packet_command == NULL) {
    printf("Tx Packet malloc failed\n");
    return -1;
  }
  return 0;
}

void allocate_rx_buffer(rx_packet_command_desc_t **rx_packet_command)
{
  *rx_packet_command = malloc(MAX_RX_BUFF_SUPPORTED);
  if (*rx_packet_command == NULL) {
    printf("Rx Packet malloc failed\n");
  }
}
