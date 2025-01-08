/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define SOCKET_PROTOCOL 31

// Minimun TX packet length
#define TX_CMD_DESC 14
// Minimun RX packet length
#define RX_CMD_DESC 10
// Maximum TX packet buffer
#define MAX_TX_BUFF_SUPPORTED (4096 + TX_CMD_DESC)
// Maximum RX packet buffer
#define MAX_RX_BUFF_SUPPORTED (4096 + RX_CMD_DESC)

// MCU commands
#define RESET_INTERFACE_COMMAND 0x0000
#define INIT_INTERFACE_COMMAND  0x0001
#define RESET_TARGET_COMMAND    0x0002
#define POWER_TARGET_COMMAND    0x0003
#define READ_MEMORY_COMMAND     0x0004
#define WRITE_MEMORY_COMMAND    0x0005

// Error Marcos
#define MEMORY_ALLOCATION_FAILURE 0x0
#define TX_PACKET_CRC_CHECK_ERROR 0xE0
#define RX_PACKET_CRC_CHECK_ERROR 0xE1
#define INVALID_COMMAND           0xE2
#define UNSUPPORTED_COMMAND       0xE3
#define SOFT_RESET_TARGET         0x4

#define CMFG_MAX_COMMAND_LENGTH MAX_TX_BUFF_SUPPORTED
#define CMFG_MIN_COMMAND_LENGTH TX_CMD_DESC

// Transfer packet Structure
typedef struct tx_packet_command_desc_s {
  uint8_t start_tag;
  uint8_t sequence_number;
  uint16_t packet_length;
  uint16_t MCU_command;
  uint16_t length;
  uint32_t address;
  uint8_t content[0];
} tx_packet_command_desc_t;

// Receive Packet Structure
typedef struct rx_packet_command_desc_s {
  uint8_t start_tag;
  uint8_t sequence_number;
  uint16_t packet_length;
  uint32_t status;
  uint8_t content[0];
} rx_packet_command_desc_t;

// TCP socket related
#define SOURCE_PORT 51917
int tcp_server(void);
int server_response(rx_packet_command_desc_t *, uint16_t, int);
int process_request(tx_packet_command_desc_t *, uint16_t, int);
int allocate_tx_buffer(tx_packet_command_desc_t **);
void allocate_rx_buffer(rx_packet_command_desc_t **);
void sig_handler(int);
void interrupt_checker(void);

// Netlink socket related
#define WLAN_PACKET   1
#define MANUFACTURING 16
#define MFG_READ      'R'
#define MFG_WRITE     'W'
#define MFG_RESET     'X'
// Master structure for read and write
typedef struct mfg_rw_s {
  uint32_t address;
  uint16_t length;
  uint8_t data[MAX_TX_BUFF_SUPPORTED];
  uint32_t reset_value;
  char read_write;
} mfg_rw_t;
//Command Type
struct rsi_nl_desc {
  uint16_t desc_word[0];
};

extern int netlink_sfd;
int netlink_socket_creation(void);
int send_tx_packet(mfg_rw_t *, int, int);
int receive_rx_packet(mfg_rw_t *, int, int);

// Device R/W Routines
void reset_routine(rx_packet_command_desc_t *, int);
void read_routine(rx_packet_command_desc_t *, tx_packet_command_desc_t *, int);
void write_routine(rx_packet_command_desc_t *, tx_packet_command_desc_t *, int);

// CRC related
uint16_t crc16_computation(uint8_t *, uint16_t);
void rx_crc_check(rx_packet_command_desc_t *, uint32_t content_len);
