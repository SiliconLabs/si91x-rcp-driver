// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */
#ifndef CMFG_H
#define CMFG_H

#include "cmfg.h"

#endif

int netlink_sfd;

// Function to provide user the access to terminate the application
void sig_handler(int signal)
{
  if (signal == SIGINT) {
    printf("\nCtrl + C sent by the user\n");
    printf("Closing the CMFG Application\n");
    exit(0);
  }
}

// Function to check for signal sent by the user
void interrupt_checker(void)
{
  struct sigaction action;
  bzero(&action, sizeof(action));
  action.sa_handler = &sig_handler;
  sigaction(SIGINT, &action, NULL);
}

//Calculate crc of the packet
uint16_t crc16_computation(uint8_t *data_p, uint16_t length)
{
  uint16_t x;
  uint16_t crc = 0xFFFF;
  while (length--) {
    x = crc >> 8 ^ *data_p++;
    x ^= x >> 4;
    crc = (crc << 8) ^ ((x << 12)) ^ ((x << 5)) ^ (x);
  }
  return crc;
}

int main(int argc, char **argv)
{
  int server_status = 0;

  netlink_sfd = netlink_socket_creation();
  if (netlink_sfd < 0)
    printf("Unable to start netlink\n");

  interrupt_checker();

  while (1) {
    sleep(0.5);
    if (server_status == 0) {
      server_status = tcp_server();
      if (server_status < 0)
        printf("Unable to start server\n");
    }
  }
  return 0;
}
