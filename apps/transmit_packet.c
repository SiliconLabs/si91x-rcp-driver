// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include "per_util.h"
//#include "../wlan/supplicant/linux/src/utils/common.h"

int cal_rate(char *);
int char_to_hwaddr(const char *txt, unsigned char *addr, unsigned int length);
static int hex2num(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

/**
 * hwaddr_compact_aton - Convert ASCII string to MAC address (no colon delimitors format)
 * @txt: MAC address as a string (e.g., "001122334455")
 * @addr: Buffer for the MAC address (ETH_ALEN = 6 bytes)
 * Returns: 0 on success, -1 on failure (e.g., string not a MAC address)
 */
int char_to_hwaddr(const char *txt, uint8_t *addr, unsigned int length)
{
  int i;

  for (i = 0; i < (length * 2); i++) {
    int a, b;

    a = hex2num(*txt++);
    if (a < 0)
      return -1;
    b = hex2num(*txt++);
    if (b < 0)
      return -1;
    *addr++ = (a << 4) | b;
  }

  return 0;
}

//main
int main(int argc, char *argv[])
{
  struct iwreq iwr;
  int sockfd, i;
  int status, cmd;
  int file_size;
  per_packet_t per_packet;
  unsigned short length;
  unsigned char pkt[1536 * 2] = { 0 };
  FILE *fp, *fp1 = NULL;
  /*Creating a Socket*/
  sockfd = socket_creation();
  if (sockfd < 0) {
    printf("Unable to create a socket\n");
    return sockfd;
  }
  cmd = (unsigned short)PER_PACKET;
  memset(&per_packet, 0, sizeof(per_packet_t));
  if (argc == 4 || argc == 3) {
    per_packet.enable = atoi(argv[1]);
    if (!(per_packet.enable == 0 || per_packet.enable == 1)) {
      printf("Invalid Enable field,Enter either 0 or 1 \n");
      exit(0);
    }
    per_packet.length = atoi(argv[2]);
    if (argc == 4)
      per_packet.insert_seq = atoi(argv[3]);
    else
      per_packet.insert_seq = 0;
    if (!(per_packet.insert_seq == 0 || per_packet.insert_seq == 1)) {
      printf("Invalid sequence number field,Enter either 0 or 1 \n");
      exit(0);
    }
    if (per_packet.length > 1536) {
      printf("Invalid length,Give the length <= 1536 \n");
      exit(0);
    }
    fp = fopen("per_packet.txt", "r");
    if (fp == NULL) {
      printf("Unable to open file per_packet.txt\n");
      exit(0);
    }
    fp1 = fopen("per_packet.txt", "r");
    if (fp1 == NULL) {
      fclose(fp);
      fp = NULL;
      printf("Unable to open file per_packet.txt\n");
      exit(0);
    }
    fseek(fp1, 0, SEEK_END);
    file_size = ftell(fp1);
    if (per_packet.length > ((file_size - 1) / 2)) {
      printf("Please enter length less than or equal to content length in per_packet.txt file \n");
      exit(0);
    }
    for (i = 0; i < (per_packet.length * 2); i++) {
      pkt[i] = getc(fp); /* returns unsigned char cast to int */
    }
    status = char_to_hwaddr(&pkt[0], &per_packet.packet[0], per_packet.length);
    printf("Transmit Packet Application\n");
    if (per_transmit_packet_wrapper(per_packet, cmd, sockfd)) {
      printf("Application Failed to send the data\n");
      exit(0);
    } else {
      printf("Tx Packet Configuration is DONE\n");
    }
    fclose(fp);
    fclose(fp1);
  }

  else if (argc == 2) {
    i = atoi(argv[1]);
    if (i == 0) {
      if (per_transmit_packet_wrapper(per_packet, cmd, sockfd)) {
        exit(0);
      } else {
        printf("Default configuration is set\n");
      }
    } else {
      printf("Please enter either 0 or 1 as an argument, instead of %d to stop..\n", i);
    }
  }

  else {
    printf("\nUSAGE   : ./transmit_packet Enable Length \n");
    printf("\nEnable  : 1-enable\n");
    printf("        : 0-Disable \n");
    printf("\nLength  : Length in number of bytes\n");
    printf("\nSEQ_NO  : 1-Takes sequence number from per_packet.txt file\n");
    printf("        : 0-Ignore sequence number from per_packet.txt file\n");
    return 0;
  }
  return 0;
}
