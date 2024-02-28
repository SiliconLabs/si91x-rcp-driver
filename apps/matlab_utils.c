// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include <linux/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <inttypes.h>
#include <netinet/in.h>
#include "per_util.h"

#define DIRECT_ADDR   1
#define INDIRECT_ADDR 2
int sock_fd;
int conn_socket;
int client_sock;
struct nlmsghdr *nlh;

int_32 bb_read(uint_32, uint_32, uint_32);
int_32 bb_read_multiple(uint_32, uint_32, uint_32 *);
int_32 bb_write(uint_32, uint_32, uint_32 *);
int_32 buffer_read(uint_32 *, uint_32, uint_32, uint_8);
int_32 buffer_write(uint_32 *, uint_32, uint_32, uint_8);
int_32 master_ops(int sfd, uint_8, uint_32, uint_8 *, uint_8);
int_32 read_stats(uint_32 *, uint_32);
int_32 tx_packet(uint_32 *, uint_32);

uint_32 BUFFER[5 * 1024];

ONEBOX_STATUS rsi_process_packet(uint_8 *, uint_32, struct sockaddr_in *, int);
struct bb_rf_param_t bb_rf_params;

ONEBOX_STATUS rsi_send_packet(uint_16 *packet, uint_16 length)
{
  uint_8 *server_name = DESTN_IP;
  uint_16 send_socket;
  uint_16 swap, i;
  struct sockaddr_in server;
  ssize_t ret_val;

  server.sin_addr.s_addr = inet_addr(server_name);
  server.sin_family      = AF_INET;
  server.sin_port        = htons(DESTN_PORT_NUM);
  printf(" packet length = %x\n", length);
#ifdef UDP_CONN
  send_socket = socket(AF_INET, SOCK_DGRAM, 0);
#else
  send_socket = socket(AF_INET, SOCK_STREAM, 0);
#endif

#ifdef UDP_CONN
  ret_val = sendto(send_socket, packet, length, 0, (struct sockaddr *)&server, sizeof(server));
#else
  ret_val     = send(client_sock, packet, length, 0);
#endif

  if (ret_val == -1)
    printf("Unable to send\n");
  close(send_socket);

  return 0;
}

ONEBOX_STATUS rsi_process_packet(uint_8 *packet, uint_32 packet_len, struct sockaddr_in *client_addr, int client_len)
{

  uint_16 type, ii, jj, i, j, flags;
  uint_32 bb_addr, bb_len, soft_reset = 0, vals_per_reg, num_of_regs, no_of_addr;
  uint_32 tx_count;
  uint_32 bb_val;
  uint_32 RCV_BUFFER[5];
  uint_8 arg, read_type = 0, write_type = 0;
  uint_32 Data       = 0;
  uint_8 read_write  = 0;
  uint_32 address    = 0;
  uint_8 no_of_bytes = 0;
  uint_32 buf        = 0;

  /* Check if packet is invalid */
  if (packet_len <= 0) {
    printf("Invalid length\n");
    goto fail;
  }

  /* Get the command type */
  type = ntohl(*(uint_32 *)&packet[0]);

  /* For Register read/write commands, take the address and
 *            length from the packet */

  /* Process individiual packets */
  switch (type) {
    case BB_READ:
      printf("===> BB Read <====\n");
      soft_reset = ntohl(*(uint_32 *)&packet[4]);
      bb_addr    = ntohl(*(uint_32 *)&packet[8]);
      bb_len     = ntohl(*(uint_32 *)&packet[12]);
      printf(" soft_reset is %d :\n", soft_reset);
      printf(" BB_addr is %x :\n", bb_addr);
      printf(" BB_len is %d :\n", bb_len);
      bb_read(bb_addr, bb_len, soft_reset);
      break;
    case BB_READ_MULTIPLE:
      printf("===> BB Read <====\n");
      soft_reset = ntohl(*(uint_32 *)&packet[4]);
      bb_len     = ntohl(*(uint_32 *)&packet[8]);
      bb_addr    = ntohl(*(uint_32 *)&packet[12]);
      printf(" soft_reset is %d :\n", soft_reset);
      printf(" BB_addr is %x :\n", bb_addr);
      printf(" BB_len is %d :\n", bb_len);
      for (ii = 8, jj = 0; jj < bb_len; ii = ii + 4, jj++) {
        BUFFER[jj] = ntohl(*(uint_32 *)&packet[ii + 4]);
        printf(" BBffer[%d] is %8x :\n", jj, BUFFER[jj]);
      }
      bb_read_multiple(bb_len, soft_reset, BUFFER);
      break;
    case BB_WRITE:
      printf("===> BB Write <===\n");
      soft_reset = ntohl(*(uint_32 *)&packet[4]);
      bb_len     = ntohl(*(uint_32 *)&packet[8]);
      bb_addr    = ntohl(*(uint_32 *)&packet[12]);
      bb_val     = ntohl(*(uint_32 *)&packet[16]);
      printf(" soft_reset is %x :\n", soft_reset);
      printf(" BB_len is %x :\n", bb_len);
      printf(" BB_addr is %x :\n", bb_addr);
      printf(" BB_val is %x :\n", bb_val);
      for (ii = 8, jj = 0; jj < bb_len * 2; ii = ii + 4, jj++) {
        BUFFER[jj] = ntohl(*(uint_32 *)&packet[ii + 4]);
        printf(" BBffer[%d] is %8x :\n", jj, BUFFER[jj]);
      }
      bb_write(bb_len, soft_reset, BUFFER);
      break;
    case BUFFER_READ:
      printf("===> Buffer Read <===\n");
      read_type  = ntohl(*(uint_32 *)&packet[4]);
      soft_reset = ntohl(*(uint_32 *)&packet[8]);
      if (read_type == DIRECT_ADDR) {
        no_of_addr = ntohl(*(uint_32 *)&packet[12]);
        printf("Direct Addressing\n");
      } else if (read_type == INDIRECT_ADDR) {
        printf("In-Direct Addressing\n");
        no_of_addr = ntohl(*(uint_32 *)&packet[24]);
        printf(" soft_reset is= %d, num_of_sets= %d \n", soft_reset, no_of_addr);
        BUFFER[0] = ntohl(*(uint_32 *)&packet[12]);
        BUFFER[1] = ntohl(*(uint_32 *)&packet[16]);
        BUFFER[2] = ntohl(*(uint_32 *)&packet[20]);
        printf(" X_reg_addr= %08x, Y_reg_addr= %08x, Z_reg_addr= %08x \n", BUFFER[0], BUFFER[1], BUFFER[2]);
        for (ii = 24, jj = 3; ii <= no_of_addr * 12 + 24; ii = ii + 4, jj++) {
          BUFFER[jj] = ntohl(*(uint_32 *)&packet[ii + 4]);
        }

        for (jj = 3; jj < no_of_addr * 3 + 3; jj += 3)
          printf(" XData2[%d] = %08x ,YData= %08x ,ZData= %08x \n",
                 (jj - 3),
                 BUFFER[jj],
                 BUFFER[jj + 1],
                 BUFFER[jj + 2]);
      } else {
        printf("Invalid read type %d, select direct(1) or indirect(2) addressing\n", read_type);
        return ONEBOX_STATUS_FAILURE;
      }
      buffer_read(BUFFER, no_of_addr, soft_reset, read_type);
      break;
    case BUFFER_WRITE:
      printf("===> Buffer Write <===\n");
      write_type = ntohl(*(uint_32 *)&packet[4]);
      soft_reset = ntohl(*(uint_32 *)&packet[8]);
      if (write_type == INDIRECT_ADDR) {
        no_of_addr = ntohl(*(uint_32 *)&packet[24]);
        printf(" soft_reset is= %d, num_of_sets= %d \n", soft_reset, no_of_addr);
        BUFFER[0] = ntohl(*(uint_32 *)&packet[12]);
        BUFFER[1] = ntohl(*(uint_32 *)&packet[16]);
        BUFFER[2] = ntohl(*(uint_32 *)&packet[20]);
        printf(" X_reg_addr= %08x, Y_reg_addr= %08x, Z_reg_addr= %08x \n", BUFFER[0], BUFFER[1], BUFFER[2]);
        for (ii = 24, jj = 3; ii <= no_of_addr * 12 + 24; ii = ii + 4, jj++) {
          BUFFER[jj] = ntohl(*(uint_32 *)&packet[ii + 4]);
        }

        for (jj = 3; jj < no_of_addr * 3 + 3; jj += 3)
          printf(" XData2[%d] = %08x ,YData= %08x ,ZData= %08x ,Delay = %d\n",
                 (jj - 3),
                 BUFFER[jj],
                 BUFFER[jj + 1],
                 BUFFER[jj + 2],
                 BUFFER[jj + 3]);
      } else if (write_type == DIRECT_ADDR) {
        no_of_addr = ntohl(*(uint_32 *)&packet[12]);
        printf(" soft_reset is= %d, length= %d \n", soft_reset, no_of_addr);
        for (ii = 12, jj = 0; ii < 4 * 2 * no_of_addr + 12; ii = ii + 4, jj++) {
          BUFFER[jj] = ntohl(*(uint_32 *)&packet[ii + 4]);
        }
      }

      buffer_write(BUFFER, no_of_addr, soft_reset, write_type);

      break;
    case AFE_SPI_R_W: {
      printf("\n=============== AFE_SPI [R/W] ===============\n");
#define MODEM_PLL_CONFIG_IO_BASE_ADDR 0x41138000
      Data        = 0;
      read_write  = ntohl(*(uint_32 *)&packet[4]);
      address     = ntohl(*(uint_32 *)&packet[8]);
      no_of_bytes = 2;
      if (read_write) {
        Data = ntohl(*(uint_32 *)&packet[12]);
        printf("Data(W) = %x \n", Data);
      }
      address = (MODEM_PLL_CONFIG_IO_BASE_ADDR + (address * 4));
      if (master_ops(sock_fd, read_write, address, (uint_8 *)&Data, 2)) {
        printf("Unable to Access AFR_SPI_REG 's\n");
      }
      if (read_write == 0) {
        printf("Data(R) = %x \n", *(uint_16 *)&Data);
        printf("Sending AFE_SPI_R_W packet to Matlab\n");
        rsi_send_packet((uint_16 *)&Data, (2));
      }
    } break;
    case SOC_PLL_R_W: {
      printf("\n=============== SOC_PLL [R/W] ===============\n ");
#define NWP_BASE_ADDR                 0x41300000
#define M4_CLK_ADDR                   (NWP_BASE_ADDR + 0x110)
#define REG_SPI_BASE_ADDR_PLL         0x46180000
#define SPI_MEM_MAP_PLL(REG_ADR)      (REG_SPI_BASE_ADDR_PLL + 0x00008000 + (REG_ADR << 2))
#define MODEM_PLL_CONFIG_IO_BASE_ADDR 0x41138000
#define UNGATE_CLOCK                  0x00830033
#define GATE_CLOCK                    0x00830000
      Data        = 0;
      read_write  = ntohl(*(uint_32 *)&packet[4]); //ntohl(packet[4]);
      address     = ntohl(*(uint_32 *)&packet[8]);
      no_of_bytes = 2;
      if (read_write) {
        Data = ntohl(*(uint_32 *)&packet[12]);
        printf("Data(W) = %x \t", Data);
      }
      buf = UNGATE_CLOCK;
      master_ops(sock_fd, WRITE, M4_CLK_ADDR, (uint_8 *)&buf, 4); //Ungate clk
      address = SPI_MEM_MAP_PLL(address);
      printf(" Address = 0x%x \n", address);
      master_ops(sock_fd, read_write, address, (uint_8 *)&Data, 2); //Execute read or write operation
      buf = GATE_CLOCK;
      master_ops(sock_fd, WRITE, M4_CLK_ADDR, (uint_8 *)&buf, 4); //Gate clk
      if (read_write == 0) {
        rsi_send_packet((uint_16 *)&Data, (2));
        printf("Sending SOC_PLL_REG:[%04x] packet to Matlab \n", *(uint_16 *)&Data);
      }
    } break;

    case AHB_R_W: {
      printf("\n=============== AHB [R/W] =============== \n");
      Data        = 0;
      read_write  = ntohl(*(uint_32 *)&packet[4]); // ntohl(packet[4]);
      address     = ntohl(*(uint_32 *)&packet[8]);
      no_of_bytes = ntohl(*(uint_32 *)&packet[12]);
      printf(" AHB R/W = %d address =%x no_of_bytes =%x \n", read_write, address, no_of_bytes);
      if (read_write) {
        Data = ntohl(*(uint_32 *)&packet[16]);
        printf("Data[W] =0x%04x \n", Data);
      }
      if (master_ops(sock_fd, read_write, address, (uint_8 *)&Data, no_of_bytes)) {
        printf("AHB operation FAILED \n");
        break;
      }
      if (read_write == 0) {
        printf("Data[R] =0x%04x \n", *(uint_16 *)&Data);
        rsi_send_packet((uint_16 *)&Data, no_of_bytes);
      }
      printf("Sending AHB READ DATA to Matlab\n");
    } break;

    case M_RX_STATS: {
      printf("===> RX_STATS <===\n");
      arg = ntohl(*(uint_32 *)&packet[4]);
      memset(RCV_BUFFER, 0, arg);
      for (i = 0; i < arg; i++)
        RCV_BUFFER[i] = ntohl(*(uint_32 *)&packet[(i * 4) + 8]);
      read_stats(RCV_BUFFER, arg);
    } break;
    case M_TX_STATS: {
      printf("===> TX_STATS <===\n");
      tx_count = ntohl(*(uint_32 *)&packet[4]); //no of arguments
      memset(BUFFER, 0, tx_count);
      for (i = 0, j = 0; i < tx_count; i++) {
        BUFFER[i] = ntohl(*(uint_32 *)&packet[j + 8]);
        j         = j + 4;
      }
      tx_packet(BUFFER, tx_count);
    } break;
    default:
      printf("### Unknow command %d ###\n", type);
      packet[0] = type;
      rsi_send_packet((uint_16 *)packet, 2);
  }
  return ONEBOX_STATUS_SUCCESS;
fail:
  return ONEBOX_STATUS_FAILURE;
}

int_32 bb_read(uint_32 bb_addr, uint_32 bb_len, uint_32 soft_rst)
{
  int_32 count, cc = 1, m;
  uint_32 addr, ref, swap, len;
  struct bb_rf_param_t bb_rf_params, bb_print_params;
  count = bb_len;
  addr  = bb_addr;
  printf(" Final read format for base band is 0x%x length is 0x%x :\n", addr, count);
  while (count) {
    ref                          = addr;
    bb_print_params.Data[cc - 1] = addr;
    printf(" address value is %x :\n", addr);
    addr = addr + 1;

    count--;
    bb_rf_params.Data[cc] = (uint_16)ref;
    cc++;
  }
  printf("Num of addrs are 0x%x, bb-rfparms.Data = 0x%x\n", bb_len, bb_rf_params.Data[0]);
  bb_rf_params.no_of_values = (uint_8)bb_len;
  bb_rf_params.value        = 0x0000;
  bb_rf_params.soft_reset   = (uint_8)soft_rst;

  len = sizeof(bb_rf_params);
  if (bb_read_write_wrapper(bb_rf_params, sock_fd) < 0) {
    printf("Error Reading in BB_READ \n");
  } else {
    nlh = common_recv_mesg_wrapper(sock_fd, len);
    if (nlh == NULL) {
      printf("Error receving from bb\n");
    } else {
      memcpy(&bb_rf_params, NLMSG_DATA(nlh), len);
      free(nlh);
      printf("\n BB_READ success\n");
      printf("Address           Data\n");
      for (m = 0; m < bb_len; m++) {
        printf(" 0x%x            0x%x\n", bb_print_params.Data[m], bb_rf_params.Data[m]);
        swap                 = bb_rf_params.Data[m];
        bb_rf_params.Data[m] = ((swap & 0x00FF) << 8) | ((swap & 0xFF00) >> 8);
        printf("After swap 0x%x            0x%x\n", bb_print_params.Data[m], bb_rf_params.Data[m]);
      }
    }
  }
  rsi_send_packet((uint_16 *)bb_rf_params.Data, (bb_len * 2));
  return 0;
}

int_32 bb_read_multiple(uint_32 bb_len, uint_32 soft_rst, uint_32 *BUFFER)
{
  int_32 count, cc = 1, m;
  uint_32 addr, i, len;
  struct bb_rf_param_t bb_rf_params, bb_print_params;
  count = bb_len;

  if (bb_len > 6) {
    printf("Maximum 6 Values allowed");
    return 0;
  }
  while (count) {
    bb_print_params.Data[cc - 1] = *(uint_16 *)&BUFFER[cc - 1];
    printf(" address value is %x :\n", addr);
    count--;
    bb_rf_params.Data[cc] = *(uint_16 *)&BUFFER[cc - 1];
    cc++;
  }
  printf("Num of addrs are 0x%x, bb-rfparms.Data = 0x%x\n", bb_len, bb_rf_params.Data[0]);
  bb_rf_params.no_of_values = (uint_8)bb_len;
  bb_rf_params.value        = 0x0000;
  bb_rf_params.soft_reset   = (uint_8)soft_rst;
  len                       = sizeof(bb_rf_params);
  if (bb_read_write_wrapper(bb_rf_params, sock_fd) < 0) {
    printf("Error Reading in BB_READ \n");
  } else {
    nlh = common_recv_mesg_wrapper(sock_fd, len);
    if (nlh == NULL) {
      printf("Error receving from bb\n");
    } else {
      memcpy(&bb_rf_params, NLMSG_DATA(nlh), len);
      free(nlh);
      printf("\n BB_READ success\n");
      printf("Address           Data\n");
      for (m = 0; m < bb_len; m++) {
        printf(" 0x%x            0x%x\n", bb_print_params.Data[m], bb_rf_params.Data[m]);
      }
    }
  }
  rsi_send_packet((uint_16 *)bb_rf_params.Data, (bb_len * 2));
  return 0;
}

int_32 bb_write(uint_32 bb_len, uint_32 soft_rst, uint_32 *BUFFER)
{
  uint_16 val, i, j = 0, index              = 0, ii, k;
  uint_8 blocks, count, count1, write_block = 50;
  val = (uint_16)bb_len;
  struct bb_rf_param_t bb_rf_params;

  memset(&bb_rf_params, 0, sizeof(bb_rf_params));
  blocks = (bb_len / (write_block));
  count  = (bb_len % (write_block));
  for (i = 0; i <= blocks; i++) {
    if (i == blocks) {
      count1 = count;
    } else {
      count1 = write_block;
    }
    index = j;
    printf("index = %d :\n", index);
    for (ii = 1, k = 1; ii < count1 * 2; j += 2, ii += 2, k++) {
      bb_rf_params.Data[ii]     = *(uint_16 *)&BUFFER[j];
      bb_rf_params.Data[ii + 1] = *(uint_16 *)&BUFFER[j + 1];

      printf(" addr[%d]= %08x, Data= %08x\n", k, bb_rf_params.Data[j], bb_rf_params.Data[j + 1]);
    }

    val = (count1 * 2);
    printf("writing BB len is:%d\n", val);
    bb_rf_params.value        = 1;
    bb_rf_params.no_of_values = val;
    bb_rf_params.soft_reset   = (uint_8)soft_rst;
    printf("writing BB value is:%d\n", bb_rf_params.value);
    printf("writing BB no_of_vals is:%d\n", bb_rf_params.no_of_values);
    printf("writing BB reset is:%d\n", bb_rf_params.soft_reset);
    if (bb_read_write_wrapper(bb_rf_params, sock_fd) < 0)
      printf("Error writing to BB\n");
    else
      printf("SUCCESS Writing to BB\n");
  }
  return 0;
}

int_32 buffer_read(uint_32 *BUFFER, uint_32 no_of_addr, uint_32 soft_rst, uint_8 read_type)
{
  uint_16 val, i, ii, jj, j = 3, index = 0, k = 0, buf_len;
  uint_8 blocks, count, count1, write_block = 27;
  struct bb_rf_param_t bb_rf_params, bb_rf_print;
  uint_32 swap, len;
  val = (uint_16)no_of_addr;
  printf("Total no regs are = %d :\n", val);
  if (read_type == INDIRECT_ADDR) {
    blocks = (no_of_addr / (write_block));
    count  = (no_of_addr % (write_block));
    for (i = 0; i <= blocks; i++) {
      if (i == blocks) {
        count1 = count;
      } else {
        count1 = write_block;
      }
      index = j;
      printf("index = %d :\n", index);
      bb_rf_params.Data[1] = *(uint_16 *)&BUFFER[0];
      bb_rf_params.Data[2] = *(uint_16 *)&BUFFER[1];
      bb_rf_params.Data[3] = *(uint_16 *)&BUFFER[2];
      for (ii = 4, k = 1; ii < (count1 * 3 + 3); ii += 3, k++, j += 3) {
        bb_rf_params.Data[ii]     = *(uint_16 *)&BUFFER[j];
        bb_rf_params.Data[ii + 1] = *(uint_16 *)&BUFFER[j + 1];
        bb_rf_params.Data[ii + 2] = ((*(uint_16 *)&BUFFER[j + 2]) | 0x80);

        printf("X Data[%d] = %08x ,YData = %08x ,ZData = %08x \n",
               k,
               bb_rf_params.Data[ii],
               bb_rf_params.Data[ii + 1],
               bb_rf_params.Data[ii + 2]);
      }
      buf_len = count1;
      printf("Total block = %d :\n", buf_len);
      bb_rf_params.value        = 6;
      bb_rf_params.no_of_values = buf_len;
      bb_rf_params.soft_reset   = (uint_8)soft_rst;
      bb_rf_params.soft_reset |= (read_type << 4);
      len = sizeof(bb_rf_params);
      if (bb_rf_read_write_wrapper(bb_rf_params, sock_fd, 0) < 0) {
        printf("Error reading from buffer\n");
      } else {
        nlh = common_recv_mesg_wrapper(sock_fd, len);
        if (nlh == NULL) {
          printf("Error receving from bb\n");
        } else {
          printf("Buffer reading Successful:\n");
          memcpy(&bb_rf_params, NLMSG_DATA(nlh), len);
          free(nlh);
          buf_len = (count1 * 3);
          for (ii = 0, jj = 0; ii < buf_len; ii += 3, jj += 2) {
            bb_rf_print.Data[jj]     = bb_rf_params.Data[ii + 1];
            swap                     = bb_rf_print.Data[jj];
            bb_rf_print.Data[jj]     = ((swap & 0x00FF) << 8) | ((swap & 0xFF00) >> 8);
            bb_rf_print.Data[jj + 1] = bb_rf_params.Data[ii + 2];
            swap                     = bb_rf_print.Data[jj + 1];
            bb_rf_print.Data[jj + 1] = ((swap & 0x00FF) << 8) | ((swap & 0xFF00) >> 8);
          }
          buf_len = ((buf_len / 3) * 2) * 2;
          printf("Buffer length = %d :\n", buf_len);
          for (ii = 0, jj = 0; ii < buf_len / 2; ii++, jj++) {
            printf(" Data 0x%x\n", bb_rf_print.Data[ii]);
          }
          rsi_send_packet((uint_16 *)bb_rf_print.Data, buf_len);
        }
      }
    }
  } else if (read_type == DIRECT_ADDR) {
    uint_8 addr = 0;
    write_block = 62;
    blocks      = (no_of_addr / (write_block));
    count       = (no_of_addr % (write_block));
    jj          = 0;
    for (i = 0; i <= blocks; i++) {
      if (i == blocks) {
        count1 = count;
      } else {
        count1 = write_block;
      }
      bb_rf_params.value        = 6;
      bb_rf_params.no_of_values = count1;
      bb_rf_params.no_of_fields = addr;
      addr += count1;
      printf("no of values = %d\n", bb_rf_params.no_of_values);
      bb_rf_params.soft_reset = (uint_8)soft_rst;
      bb_rf_params.soft_reset |= (read_type << 4);
      len = sizeof(bb_rf_params);
      if (bb_rf_read_write_wrapper(bb_rf_params, sock_fd, 0) < 0) {
        printf("Error reading from buffer\n");
      } else {
        nlh = common_recv_mesg_wrapper(sock_fd, len);
        if (nlh == NULL) {
          printf("Error receving from bb\n");
        } else {
          printf("Buffer reading Successful:\n");
          memcpy(&bb_rf_params, NLMSG_DATA(nlh), len);
          free(nlh);
          buf_len = no_of_addr;
          for (ii = 0; ii < count1 * 2; ii++, jj++) {
            bb_rf_print.Data[jj] = bb_rf_params.Data[ii];
            swap                 = bb_rf_print.Data[jj];
            bb_rf_print.Data[jj] = ((swap & 0x00FF) << 8) | ((swap & 0xFF00) >> 8);
            printf("Data :0x%0x \n ", bb_rf_print.Data[jj]);
          }
        }
      }
    }
    printf("total num vals:%d\n", jj);
    rsi_send_packet((uint_16 *)bb_rf_print.Data, jj * 2);
  }
  printf("Buffer Read Done\n");
  return 0;
}
int_32 buffer_write(uint_32 *BUFFER, uint_32 no_of_addr, uint_32 soft_rst, uint_8 write_type)
{
  uint_16 val, i, ii, j = 3, index = 0, k = 0, buf_len;
  uint_8 blocks, count, count1, write_block = 27;
  struct bb_rf_param_t bb_rf_params;

  val = (uint_16)no_of_addr;
  if (write_type == INDIRECT_ADDR) {
    printf("Total no regs are = %d :\n", val);
    blocks = (no_of_addr / (write_block));
    count  = (no_of_addr % (write_block));
    for (i = 0; i <= blocks; i++) {
      if (i == blocks) {
        count1 = count;
      } else {
        count1 = write_block;
      }
      index = j;
      printf("index = %d :\n", index);
      bb_rf_params.Data[1] = *(uint_16 *)&BUFFER[0];
      bb_rf_params.Data[2] = *(uint_16 *)&BUFFER[1];
      bb_rf_params.Data[3] = *(uint_16 *)&BUFFER[2];
      for (ii = 4, k = 1; ii < (count1 * 3 + 3); ii += 3, k++, j += 3) {
        bb_rf_params.Data[ii]     = *(uint_16 *)&BUFFER[j];
        bb_rf_params.Data[ii + 1] = *(uint_16 *)&BUFFER[j + 1];
        bb_rf_params.Data[ii + 2] = ((*(uint_16 *)&BUFFER[j + 2]) | 0x40);

        printf("X Data[%d] = %08x ,YData = %08x ,ZData = %08x \n",
               k,
               bb_rf_params.Data[ii],
               bb_rf_params.Data[ii + 1],
               bb_rf_params.Data[ii + 2]);
      }
      buf_len = count1;
      printf("Total block = %d :\n", buf_len);
      bb_rf_params.value        = 7;
      bb_rf_params.no_of_values = buf_len;
      bb_rf_params.soft_reset   = (uint_8)soft_rst;
      bb_rf_params.soft_reset |= (write_type << 4);
      if (bb_rf_read_write_wrapper(bb_rf_params, sock_fd, 0) < 0) {
        printf("Error writing from RF\n");
      } else {
        printf("Buffer Writing Successful:\n");
      }
    }
  } else if (write_type == DIRECT_ADDR) {
    uint_8 addr = 0;
    j           = 0;
    i           = 0;
    ii          = 0;
    write_block = 62, printf("Total no regs are = %d :\n", val);
    blocks      = (no_of_addr / (write_block)); //8
    count       = (no_of_addr % (write_block)); //16
    addr        = 0;
    for (i = 0; i <= blocks; i++) {
      if (i == (blocks)) {
        count1 = count;
      } else {
        count1 = write_block;
      }
      index = j;
      printf("index = %d :\n", index);
      for (ii = 1; ii <= count1 * 2; ii++, j++) {
        bb_rf_params.Data[ii] = *(uint_16 *)&BUFFER[j];
        printf("BUFFER[%d]:%08x\n", ii, bb_rf_params.Data[ii]);
      }
      buf_len = count1 * 2;
      printf("Total block = %d :\n", buf_len);
      bb_rf_params.value        = 7;
      bb_rf_params.no_of_values = buf_len;
      bb_rf_params.soft_reset   = (uint_8)soft_rst;
      bb_rf_params.soft_reset |= (write_type << 4);
      bb_rf_params.no_of_fields = addr; /* Make sure the no_of_fields is uint16*/
      addr += (count1);
      printf("no of vals:%d\n", bb_rf_params.no_of_values);
      printf("soft reset:%d\n", bb_rf_params.soft_reset);
      if (bb_rf_read_write_wrapper(bb_rf_params, sock_fd, 0) < 0)
        printf("Error writing to BB\n");
      else
        printf("Buffer Writing Successful:\n");
    }
  }
  return 0;
}

int_32 tx_packet(uint_32 *buf, uint_32 tx_cnt)
{
  unsigned int valid_channels_5_Ghz[]         = { 36,  40,  44,  48,  52,  56,  60,  64,  100, 104, 108, 112,
                                          116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165 };
  unsigned int valid_channels_5_Ghz_40Mhz[]   = { 38,  42,  46,  50,  54,  58,  62,  102, 106, 110, 114,
                                                118, 122, 126, 130, 134, 138, 151, 155, 159, 163 };
  unsigned int valid_channels_4_9_Ghz_20Mhz[] = { 184, 188, 192, 196, 8, 12, 16, 0xff, 0x00 };
  unsigned int valid_channels_4_9_Ghz_10Mhz[] = { 183, 184, 185, 187, 188, 189, 7, 8, 9, 11, 0xff, 0x00 };
  unsigned char enable_40                     = 0;
  unsigned char enable_11j                    = 0;
  int chan_number, cmd;
  int ii = 0, i;
  unsigned short tmp_rate;
  per_params_t per_params;
  unsigned char rate_flags = 0;

  if (tx_cnt == 1) {
    printf("TX STOP\n");

    if ((buf[0] == 0) || (buf[1] == 1)) {
      per_params.mode   = buf[0];
      per_params.enable = 0;
      cmd               = (unsigned short)PER_TRANSMIT;

      if (per_transmit_wrapper(per_params, cmd, sock_fd) < 0) {
        printf("&&Please ensure Burst or Continuous Mode is running\n");
      } else {
        printf("Tx Stopped\n");
      }
    } else {
      printf("Please enter either 0 or 1 as an argument, instead of %d to stop..\n", buf[0]);
    }
  } else if ((tx_cnt == 11)
             //__9117_CODE_START
             || (tx_cnt == 32)
             //__9117_CODE_END
  ) {
    printf("TX START MODE\n");

    per_params.enable = 1;
    per_params.power  = buf[0];
    tmp_rate          = buf[1];

    if (tmp_rate == 1)
      per_params.rate = RSI_RATE_1;
    else if (tmp_rate == 2)
      per_params.rate = RSI_RATE_2;
    else if (tmp_rate == 5)
      per_params.rate = RSI_RATE_5_5;
    else if (tmp_rate == 11)
      per_params.rate = RSI_RATE_11;
    else if (tmp_rate == 6)
      per_params.rate = RSI_RATE_6;
    else if (tmp_rate == 9)
      per_params.rate = RSI_RATE_9;
    else if (tmp_rate == 12)
      per_params.rate = RSI_RATE_12;
    else if (tmp_rate == 18)
      per_params.rate = RSI_RATE_18;
    else if (tmp_rate == 24)
      per_params.rate = RSI_RATE_24;
    else if (tmp_rate == 36)
      per_params.rate = RSI_RATE_36;
    else if (tmp_rate == 48)
      per_params.rate = RSI_RATE_48;
    else if (tmp_rate == 54)
      per_params.rate = RSI_RATE_54;
    else if (tmp_rate == RSI_RATE_MCS0)
      per_params.rate = RSI_RATE_MCS0;
    else if (tmp_rate == RSI_RATE_MCS1)
      per_params.rate = RSI_RATE_MCS1;
    else if (tmp_rate == RSI_RATE_MCS2)
      per_params.rate = RSI_RATE_MCS2;
    else if (tmp_rate == RSI_RATE_MCS3)
      per_params.rate = RSI_RATE_MCS3;
    else if (tmp_rate == RSI_RATE_MCS4)
      per_params.rate = RSI_RATE_MCS4;
    else if (tmp_rate == RSI_RATE_MCS5)
      per_params.rate = RSI_RATE_MCS5;
    else if (tmp_rate == RSI_RATE_MCS6)
      per_params.rate = RSI_RATE_MCS6;
    else if (tmp_rate == RSI_RATE_MCS7)
      per_params.rate = RSI_RATE_MCS7;
    else
      per_params.rate = RSI_RATE_1;

    per_params.pkt_length = buf[2];

    if (buf[3] == 1 || buf[3] == 0) {
      per_params.mode = buf[3];
    } else {
      per_params.mode = 0;
    }

    chan_number = buf[4];

    rate_flags             = buf[6];
    per_params.rate_flags  = rate_flags;
    per_params.per_ch_bw   = (rate_flags >> 2) & 0x07;
    enable_11j             = (rate_flags & BIT(5));
    per_params.enable_11j  = enable_11j;
    per_params.aggr_enable = buf[7];

    per_params.aggr_count = (per_params.pkt_length / PER_AGGR_LIMIT_PER_PKT);
    if ((per_params.pkt_length - (per_params.aggr_count * PER_AGGR_LIMIT_PER_PKT)) > 0) {
      per_params.aggr_count++;
    }

    per_params.no_of_pkts = buf[8];

    per_params.delay       = buf[9];
    per_params.ctry_region = buf[10];
    if (per_params.ctry_region == 255) {
      /*** Remove Me When Updated in Doc and More regions are added*/
      per_params.ctry_region = 127;
    } else if ((per_params.ctry_region < 0) || (per_params.ctry_region > 2)) {
      printf("Invalid Country region \n");
      printf("Valid country regions are : 0- FCC(US), 1- ETSI(Europe), 2-JP (japan), 255-World\n");
      return -1;
    }

    if (per_params.pkt_length > 1536 && per_params.aggr_enable == 0) {
      printf("Invalid length,Give the length <= 1536 \n");
      exit(0);
    }
    if ((per_params.pkt_length > 30000) && (per_params.aggr_enable)) {
      printf("Cant aggregate,Give the length <= 30000 \n");
      exit(0);
    }
    if ((per_params.aggr_enable) && !(per_params.rate >= RSI_RATE_MCS0 && per_params.rate <= RSI_RATE_MCS7)) {
      printf("Cant aggregate,Give 11n rate \n");
      exit(0);
    }
    if (per_params.per_ch_bw == BW_U40 || per_params.per_ch_bw == BW_L40 || per_params.per_ch_bw == BW_F40) {
      enable_40 = 1;
    }
    if (enable_11j == 0) {
      if (chan_number == 0xFF) {
        per_params.channel = chan_number;
        /* Pass 0xFF so as to skip channel programming */
      } else if (chan_number <= 14) {
        per_params.channel = chan_number;
      } else if ((chan_number >= 36 && chan_number <= 165) && per_params.per_ch_bw == BW_20) /* For 20Mhz BW */
      {
        for (i = 0; i < 24; i++) {
          if (chan_number == valid_channels_5_Ghz[i]) {
            per_params.channel = chan_number;
            break;
          }
        }
        if (!(per_params.channel == chan_number)) {
          printf("Invalid Channel issued by user for 20Mhz BW\n");
          return -1;
        }
      } else if ((chan_number >= 36 && chan_number <= 165) && enable_40) /* For 20Mhz BW */
      {
        for (i = 0; i < 21; i++) {
          if (chan_number == valid_channels_5_Ghz_40Mhz[i]) {
            per_params.channel = chan_number;
            break;
          }
        }
        if (!(per_params.channel == chan_number)) {
          printf("Invalid Channel issued by user for 40Mhz BW\n");
          return -1;
        }
      } else {
        printf("Invalid Channel issued by user\n");
        return -1;
      }
    } else {
      if (per_params.per_ch_bw == BW_20) {
        for (i = 0; i < sizeof(valid_channels_4_9_Ghz_20Mhz) / sizeof(valid_channels_4_9_Ghz_20Mhz[0]); i++) {
          if (chan_number == valid_channels_4_9_Ghz_20Mhz[i]) {
            per_params.channel = chan_number;
            break;
          }
        }
        if (!(per_params.channel == chan_number)) {
          printf("Invalid Channel issued by user for 20Mhz BW\n");
          return -1;
        }
      } else if (per_params.per_ch_bw == BW_10) {
        for (i = 0; i < sizeof(valid_channels_4_9_Ghz_10Mhz) / sizeof(valid_channels_4_9_Ghz_10Mhz[0]); i++) {
          if (chan_number == valid_channels_4_9_Ghz_10Mhz[i]) {
            per_params.channel = chan_number;
            break;
          }
        }
        if (!(per_params.channel == chan_number)) {
          printf("Invalid Channel issued by user for 20Mhz BW\n");
          return -1;
        }
      } else if (per_params.per_ch_bw == BW_5) {
        printf("5MHz BW is not supported\n");
        return -1;
      } else {
        printf("Invalid BW Configuration\n");
        return -1;
      }
    }
    //__9117_CODE_START
    per_params.enable_11ax = buf[5];
    memset(&per_params.heconf, 0, sizeof(per_params.heconf));
    if (per_params.enable_11ax) {
      if (tx_cnt != 32) {
        printf("Invalid no of Params for 11AX \n");
        exit(0);
      }
      if (tmp_rate == RSI_RATE_MCS0)
        per_params.rate = 0;
      else if (tmp_rate == RSI_RATE_MCS1)
        per_params.rate = 1;
      else if (tmp_rate == RSI_RATE_MCS2)
        per_params.rate = 2;
      else if (tmp_rate == RSI_RATE_MCS3)
        per_params.rate = 3;
      else if (tmp_rate == RSI_RATE_MCS4)
        per_params.rate = 4;
      else if (tmp_rate == RSI_RATE_MCS5)
        per_params.rate = 5;
      else if (tmp_rate == RSI_RATE_MCS6)
        per_params.rate = 6;
      else if (tmp_rate == RSI_RATE_MCS7)
        per_params.rate = 7;
      else {
        printf("Please enter 11ax supported rate \n");
        exit(0);
      }
      per_params.rate |= BIT(10); //11ax en indication to rate info

      per_params.heconf.coding_type            = buf[11];
      per_params.heconf.nominal_pe             = buf[12];
      per_params.heconf.UL_DL                  = buf[13];
      per_params.heconf.he_ppdu_type           = buf[14];
      per_params.heconf.beam_change            = buf[15];
      per_params.heconf.BW                     = buf[16];
      per_params.heconf.STBC                   = buf[17];
      per_params.heconf.Tx_BF                  = buf[18];
      per_params.heconf.GI_LTF                 = buf[19];
      per_params.heconf.DCM                    = buf[20];
      per_params.heconf.NSTS_MIDAMBLE          = buf[21];
      per_params.heconf.spatial_reuse          = buf[22];
      per_params.heconf.BSS_color              = buf[23];
      per_params.heconf.HE_SIGA2_RESERVED      = buf[24];
      per_params.heconf.RU_ALLOCATION          = buf[25];
      per_params.heconf.N_HELTF_TOT            = buf[26];
      per_params.heconf.SIGB_DCM               = buf[27];
      per_params.heconf.SIGB_MCS               = buf[28];
      per_params.heconf.USER_STA_ID            = buf[29];
      per_params.heconf.SIGB_COMPRESSION_FIELD = buf[30];
      per_params.heconf.USER_IDX               = buf[31];

      if (per_params.heconf.coding_type != 0 && per_params.heconf.coding_type != 1) {
        printf("Invalid option for coding_type\n 0-BCC \n 1-LDPC \n");
        exit(0);
      }
      if (per_params.heconf.nominal_pe < 0 || per_params.heconf.nominal_pe > 2) {
        printf("Invalid option for nominal_pe\n 0-0us \n 1-8us \n 2-16us \n");
        exit(0);
      }
      if (per_params.heconf.he_ppdu_type < 0 || per_params.heconf.he_ppdu_type > 3) {
        printf("Invalid option for he_ppdu_type\n 0-HE SU PPDU \n 1-HE ER SU PPDU \n 2-HE TB PPDU \n 3-HE MU PPDU \n");
        exit(0);
      }
      if (per_params.heconf.beam_change != 0 && per_params.heconf.beam_change != 1) {
        printf("Invalid option for beam change\n 0- same way \n 1- differently\n");
        exit(0);
      }
      if (per_params.heconf.UL_DL != 0 && per_params.heconf.UL_DL != 1) {
        printf("Invalid option for UL_DL\n 0-PPDU is sent by AP \n 1-PPDU is sent by station to AP \n");
        exit(0);
      }
      if (per_params.heconf.STBC != 0 && per_params.heconf.STBC != 1) {
        printf("Invalid option for STBC \n 0-no STBC \n 1-STBC only if DCM field is set to 0 \n");
        exit(0);
      }
      if (per_params.heconf.Tx_BF != 0 && per_params.heconf.Tx_BF != 1) {
        printf("Invalid option for Tx_BF \n 0-No beamforming matrix \n 1-beamforming matrix \n");
        exit(0);
      }
      if (per_params.heconf.GI_LTF < 0 || per_params.heconf.GI_LTF > 3) {
        printf("Invalid option for GI_LTF\n please select between 0 to 3  \n");
        exit(0);
      }
      if (per_params.heconf.DCM != 0 && per_params.heconf.DCM != 1) {
        printf("Invalid option for DCM \n 0-no DCM \n 1-DCM \n");
        exit(0);
      }
      if (per_params.heconf.NSTS_MIDAMBLE > 7 || per_params.heconf.NSTS_MIDAMBLE < 0) {
        printf("Invalid option for NSTS_MIDAMBLE Please select b/w 0 to 7\n");
        exit(0);
      }
      if (per_params.heconf.spatial_reuse > 15 || per_params.heconf.spatial_reuse < 0) {
        printf("Invalid option for spatial_reuse Please select b/w 0 to 15\n");
        exit(0);
      }
      if (per_params.heconf.BSS_color > 63 || per_params.heconf.BSS_color < 0) {
        printf("Invalid option for BSS_color Please select b/w 0 to 63\n");
        exit(0);
      }
      if (per_params.heconf.HE_SIGA2_RESERVED > 511 || per_params.heconf.HE_SIGA2_RESERVED < 0) {
        printf("Invalid option for HE_SIGA2_RESERVED Please select b/w 0 to 511\n");
        exit(0);
      }
      if (per_params.heconf.RU_ALLOCATION > 255 || per_params.heconf.RU_ALLOCATION < 0) {
        printf("Invalid option for RU_ALLOCATION Please select b/w 0 to 255\n");
        exit(0);
      }
      if (per_params.heconf.N_HELTF_TOT > 7 || per_params.heconf.N_HELTF_TOT < 0) {
        printf("Invalid option for N_HELTF_TOT Please select b/w 0 to 7\n");
        exit(0);
      }
      if (per_params.heconf.SIGB_DCM != 0 && per_params.heconf.SIGB_DCM != 1) {
        printf("Invalid option for SIGB_DCM Please select 0 to Disable 1 to Enable DCM\n");
        exit(0);
      }
      if (per_params.heconf.SIGB_MCS > 5 || per_params.heconf.SIGB_MCS < 0) {
        printf("Invalid option for SIGB_MCS Please select b/w 0 to 5\n");
        exit(0);
      }
      if (per_params.heconf.USER_STA_ID > 2047 || per_params.heconf.USER_STA_ID < 0) {
        printf("Invalid option for USER_STA_ID Please select b/w 0 to 2047\n");
        exit(0);
      }
      if (per_params.heconf.SIGB_COMPRESSION_FIELD > 1) {
        printf("Invalid option for SIGB_COMPRESSION_FIELD Please select 0 or 1\n");
        exit(0);
      }
      if (per_params.heconf.USER_IDX > 8) {
        printf("Invalid option for USER_IDX Please select b/w 0 to 8\n");
        exit(0);
      }
      printf("CODING_TYPE           : %d\n", per_params.heconf.coding_type);
      printf("NOMINAL_PE            : %d\n", per_params.heconf.nominal_pe);
      printf("UL_DL                 : %d\n", per_params.heconf.UL_DL);
      printf("HE_PPDU_TYPE          : %d\n", per_params.heconf.he_ppdu_type);
      printf("BEAM_CHANGE           : %d\n", per_params.heconf.beam_change);
      printf("BW                    : %d\n", per_params.heconf.BW);
      printf("STBC                  : %d\n", per_params.heconf.STBC);
      printf("TX_BF                 : %d\n", per_params.heconf.Tx_BF);
      printf("GI_LTF                : %d\n", per_params.heconf.GI_LTF);
      printf("DCM                   : %d\n", per_params.heconf.DCM);
      printf("NSTS_MIDAMBLE         : %d\n", per_params.heconf.NSTS_MIDAMBLE);
      printf("SPATIAL_REUSE         : %d\n", per_params.heconf.spatial_reuse);
      printf("BSS_COLOR             : %d\n", per_params.heconf.BSS_color);
      printf("HE_SIGA2_RESERVED     : %d\n", per_params.heconf.N_HELTF_TOT);
      printf("RU_ALLOCATION         : %d\n", per_params.heconf.RU_ALLOCATION);
      printf("N_HELTF_TOT           : %d\n", per_params.heconf.HE_SIGA2_RESERVED);
      printf("SIGB_DCM              : %d\n", per_params.heconf.SIGB_DCM);
      printf("SIGB_MCS              : %d\n", per_params.heconf.SIGB_MCS);
      printf("USER_STA_ID           : %d\n", per_params.heconf.USER_STA_ID);
      printf("SIGB_COMPRESSION_FIELD: %d\n", per_params.heconf.SIGB_COMPRESSION_FIELD);
      printf("USER_IDX              : %d\n", per_params.heconf.USER_IDX);

    } else {
      printf("No 11ax params \n");
    }
    //__9117_CODE_END

    printf("\n--Tx TEST CONFIGURATION--\n\n");
    printf("Tx POWER      : %d\n", buf[0]);
    printf("Tx RATE       : %d\n", buf[1]);
    printf("PACKET LENGTH : %d\n", per_params.pkt_length);
    if (per_params.mode == 1) {
      printf("Tx MODE       : CONTINUOUS\n");
      per_params.pkt_length = 28;
    } else if (per_params.mode == 0) {
      printf("Tx MODE       : BURST\n");
    } else {
      printf("Tx MODE       : CONTINUOUS\n");
    }
    printf("CHANNEL NUM   : %d\n", chan_number);
    printf("RATE_FLAGS    : %d\n", per_params.rate_flags);
    printf("CHAN_WIDTH    : %d\n", per_params.per_ch_bw);
    printf("AGGR_ENABLE   : %d\n", per_params.aggr_enable);
    printf("NO OF PACKETS : %d\n", per_params.no_of_pkts);
    printf("DELAY         : %d\n", per_params.delay);
    printf("CTRY_REGION   : %d\n", per_params.ctry_region);

    cmd = (unsigned short)PER_TRANSMIT;

    if (per_transmit_wrapper(per_params, cmd, sock_fd) < 0) {
      printf("Please ensure Driver is running with valid arguments \tor stop existing transmit utility\n");
    } else {
      printf("Tx Started\n");
    }
  } else {
    printf("\nUSAGE to start transmit through matlab: type no_of_arguments tx_power rate length tx_mode channel "
           "ExtPA-Enable Rate_flags Aggr_enable no_of_packets delay \n");
    //__9117_CODE_START
    printf("\nUSAGE to start transmit in 11ax mode: type no_of_arguments tx_power rate length tx_mode channel "
           "11ax-Enable Rate_flags Aggr_enable no_of_packets delay ctry_region coding_type nominal_pe UL_DL "
           "he_ppdu_type beam_change BW STBC Tx_BF GI_LTF DCM NSTS_MIDAMBLE spatial_reuse BSS_color HE_SIGA2_RESERVED "
           "RU_ALLOCATION N_HELTF_TOT SIGB_DCM SIGB_MCS USER_STA_ID SIGB_COMPRESSION_FIELD USER_IDX\n");
    //__9117_CODE_END
    printf("\nUSAGE to stop transmit: type no_of_arguments 0/1\n\t****** FIELDS *******");
    printf("\ntx_mode : 0 - Burst , 1 - Continuous mode\n");
    printf("\nRate_flags Bits: Reserved: 6-15   CH_BW: 5-2   GreenField: 1    Short_GI: 0 \n\n");
    return 0;
  }
  return 0;
}

//__9117_CODE_START
void grouped_stats(per_stats *sta_info, uint_16 *local_buf, unsigned int user_mask, uint_32 inx)
{
  int bit_value;
  int i, local_inx = 0;
  int total_11g, total_11b, total_11n, total_11ax;
  total_11g = sta_info->pkt_rcvd_with_48M + sta_info->pkt_rcvd_with_24M + sta_info->pkt_rcvd_with_12M
              + sta_info->pkt_rcvd_with_6M + sta_info->pkt_rcvd_with_54M + sta_info->pkt_rcvd_with_36M
              + sta_info->pkt_rcvd_with_18M + sta_info->pkt_rcvd_with_9M;
  total_11b =
    sta_info->pkt_rcvd_with_11M + sta_info->pkt_rcvd_with_5M + sta_info->pkt_rcvd_with_2M + sta_info->pkt_rcvd_with_1M;
  total_11n = sta_info->pkt_rcvd_with_mcs0 + sta_info->pkt_rcvd_with_mcs1 + sta_info->pkt_rcvd_with_mcs2
              + sta_info->pkt_rcvd_with_mcs3 + sta_info->pkt_rcvd_with_mcs4 + sta_info->pkt_rcvd_with_mcs5
              + sta_info->pkt_rcvd_with_mcs6 + sta_info->pkt_rcvd_with_mcs7;
  total_11ax = sta_info->pkt_count_HE_SU_PPDU + sta_info->pkt_count_HE_ER_SU_PPDU + sta_info->pkt_count_HE_TRIG_PPDU
               + sta_info->pkt_count_HE_MU_PPDU;

  if (inx == 0) {
    printf("  %s %s %s ", "crc_pass", "crc_fail", "cal_rssi");
    for (i = 0; i < 5; i++) {
      bit_value = user_mask & (1 << i);
      switch (bit_value) {
        case GROUP_0:
          printf("%s %s %s %s %s %s %s ",
                 "non_wifi_pkt",
                 "false_ed",
                 "pkt_abort",
                 "fls_rx_start",
                 "false_cca",
                 "cal_rssi",
                 "ack_sent");
          break;
        case GROUP_1:
          printf("%s %s %s %s %s %s ",
                 "false_under_sat",
                 "bss_bcast_pkts",
                 "bss_cast_pkts",
                 "bss_da_matched_mcast_pkts",
                 "eof_pkt_drops",
                 "mask_pkt_drop");
          break;
        case GROUP_2:
          printf("%s %s %s %s %s %s %s %s %s ",
                 "11G_pkts",
                 "11B_pkts",
                 "11N/mcs_rates",
                 "11AC_pkts",
                 "11AX_pkts",
                 "HE_SU_PPDU_pkts",
                 "HE_ER_SU_PPDU_pkts",
                 "HE_TRIG_PPDU_pkts",
                 "HE_MU_PPDU_pkts");
          break;
        case GROUP_3:
          printf("%s ", "tx_done");
          break;
        case GROUP_4:
          printf("%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s ",
                 "tx_pkts",
                 "rx_pkts",
                 "tx_retries",
                 "gf_pkts",
                 "ht_pkts",
                 "leg_pkt",
                 "leg_prty_fails",
                 "ht_crc_fails",
                 "sp_rejected",
                 "lp_rejected",
                 "ch1_sig_pow",
                 "ch1_noise_pow",
                 "ch2_sig_pow",
                 "ch2_noise_pow",
                 "ch3_sig_pow",
                 "ch3_noise_pow",
                 "rx_retries",
                 "bea_avg_rssi",
                 "lna_bb_gain",
                 "avg_val",
                 "xretries",
                 "max_cons_pkts_dropped");
          break;
      }
    }
    printf("\n");
  }
  printf("%2d  %6d %6d %6x ", (inx + 1), sta_info->crc_pass, sta_info->crc_fail, sta_info->cal_rssi);
  local_buf[local_inx++] = htons(inx + 1); //(*(unsigned short *)&sta_info[2]);
  local_buf[local_inx++] = htons(sta_info->crc_pass);
  local_buf[local_inx++] = htons(sta_info->crc_fail);
  local_buf[local_inx++] = htons(sta_info->cal_rssi);
  for (i = 0; i < 5; i++) {
    bit_value = user_mask & (1 << i);
    switch (bit_value) {
      case GROUP_0:
        printf("%12d %10d %8d %10d %12d %8d %8d",
               sta_info->cca_stk,
               sta_info->cca_not_stk,
               sta_info->pkt_abort,
               sta_info->fls_rx_start,
               sta_info->cca_idle,
               sta_info->cal_rssi,
               sta_info->ack_sent);
        local_buf[local_inx++]            = htons(sta_info->cca_stk);
        local_buf[local_inx++]            = htons(sta_info->cca_not_stk);
        local_buf[local_inx++]            = htons(sta_info->pkt_abort);
        local_buf[local_inx++]            = htons(sta_info->fls_rx_start);
        local_buf[local_inx++]            = htons(sta_info->cca_idle);
        local_buf[local_inx++]            = htons(sta_info->cal_rssi);
        *(uint_32 *)&local_buf[local_inx] = htons(sta_info->ack_sent);
        local_inx += 2;
        break;
      case GROUP_1:
        printf("%12d %15d %15d %17d %19d %15d",
               sta_info->false_under_sat,
               sta_info->bss_broadcast_pkts,
               sta_info->bss_multicast_pkts,
               sta_info->bss_da_matched_multicast_pkts,
               sta_info->eof_pkt_drop_count,
               sta_info->mask_pkt_drop_count);
        local_buf[local_inx++]            = htons(sta_info->false_under_sat);
        local_buf[local_inx++]            = htons(sta_info->bss_broadcast_pkts);
        local_buf[local_inx++]            = htons(sta_info->bss_multicast_pkts);
        local_buf[local_inx++]            = htons(sta_info->bss_da_matched_multicast_pkts);
        *(uint_32 *)&local_buf[local_inx] = htons(sta_info->eof_pkt_drop_count);
        local_inx += 2;
        *(uint_32 *)&local_buf[local_inx] = htons(sta_info->mask_pkt_drop_count);
        local_inx += 2;
        break;
      case GROUP_2:
        printf("%9d %8d %12d %10d %9d %9d %18d %18d %19d",
               total_11g,
               total_11b,
               total_11n,
               sta_info->pkt_count_11AC,
               total_11ax,
               sta_info->pkt_count_HE_SU_PPDU,
               sta_info->pkt_count_HE_ER_SU_PPDU,
               sta_info->pkt_count_HE_TRIG_PPDU,
               sta_info->pkt_count_HE_MU_PPDU);
        local_buf[local_inx++] = htons(total_11g);
        local_buf[local_inx++] = htons(total_11b);
        local_buf[local_inx++] = htons(total_11n);
        local_buf[local_inx++] = htons(sta_info->pkt_count_11AC);
        local_buf[local_inx++] = htons(total_11ax);
        local_buf[local_inx++] = htons(sta_info->pkt_count_HE_SU_PPDU);
        local_buf[local_inx++] = htons(sta_info->pkt_count_HE_ER_SU_PPDU);
        local_buf[local_inx++] = htons(sta_info->pkt_count_HE_TRIG_PPDU);
        local_buf[local_inx++] = htons(sta_info->pkt_count_HE_MU_PPDU);
        break;
      case GROUP_3:
        printf("%10d", sta_info->tx_done);
        local_buf[local_inx++] = htons(sta_info->tx_done);
        break;
      case GROUP_4:
        printf("%8d %8d %8d %8d %8d %8d %8d %15d %12d %10d %12d %10d %12d %15d %10d %12d %12d %10d %12d %9d %8d %15d",
               sta_info->tx_pkts,
               sta_info->rx_pkts,
               sta_info->tx_retries,
               sta_info->gf_pkts,
               sta_info->ht_pkts,
               sta_info->leg_pkt,
               sta_info->leg_prty_fails,
               sta_info->ht_crc_fails,
               sta_info->sp_rejected,
               sta_info->lp_rejected,
               sta_info->ch1_sig_pow,
               sta_info->ch1_noise_pow,
               sta_info->ch2_sig_pow,
               sta_info->ch2_noise_pow,
               sta_info->ch3_sig_pow,
               sta_info->ch3_noise_pow,
               sta_info->rx_retries,
               sta_info->bea_avg_rssi,
               sta_info->lna_bb_gain,
               sta_info->avg_val,
               sta_info->xretries,
               sta_info->max_cons_pkts_dropped);
        local_buf[local_inx++] = htons(sta_info->tx_pkts);
        local_buf[local_inx++] = htons(sta_info->rx_pkts);
        local_buf[local_inx++] = htons(sta_info->tx_retries);
        local_buf[local_inx++] = htons(sta_info->gf_pkts);
        local_buf[local_inx++] = htons(sta_info->ht_pkts);
        local_buf[local_inx++] = htons(sta_info->leg_pkt);
        local_buf[local_inx++] = htons(sta_info->leg_prty_fails);
        local_buf[local_inx++] = htons(sta_info->ht_crc_fails);
        local_buf[local_inx++] = htons(sta_info->sp_rejected);
        local_buf[local_inx++] = htons(sta_info->lp_rejected);
        local_buf[local_inx++] = htons(sta_info->ch1_sig_pow);
        local_buf[local_inx++] = htons(sta_info->ch1_noise_pow);
        local_buf[local_inx++] = htons(sta_info->ch2_sig_pow);
        local_buf[local_inx++] = htons(sta_info->ch2_noise_pow);
        local_buf[local_inx++] = htons(sta_info->ch3_sig_pow);
        local_buf[local_inx++] = htons(sta_info->ch3_noise_pow);
        local_buf[local_inx++] = htons(sta_info->rx_retries);
        local_buf[local_inx++] = htons(sta_info->bea_avg_rssi);
        local_buf[local_inx++] = htons(sta_info->lna_bb_gain);
        local_buf[local_inx++] = htons(sta_info->avg_val);
        local_buf[local_inx++] = htons(sta_info->xretries);
        local_buf[local_inx++] = htons(sta_info->max_cons_pkts_dropped);
        break;
    }
  }
  printf("\n");
}
//__9117_CODE_END
#define NO_OF_PARAMS  4
#define SIZE_OF_PARAM 2
//__9117_CODE_START
#if defined(_9117_MACRO_DEF)
#define ADDITIONAL_STATS NO_OF_PARAMS + 80
#else
//__9117_CODE_END
#define ADDITIONAL_STATS NO_OF_PARAMS
//__9117_CODE_START
#endif
//__9117_CODE_END

int_32 read_stats(uint_32 *packet, uint_32 arg)
{

  unsigned int valid_channels_5_Ghz[]         = { 36,  40,  44,  48,  52,  56,  60,  64,  100, 104, 108, 112,
                                          116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165 };
  unsigned int valid_channels_5_Ghz_40Mhz[]   = { 38,  42,  46,  50,  54,  58,  62,  102, 106, 110, 114,
                                                118, 122, 126, 130, 134, 138, 151, 155, 159, 163 };
  unsigned int valid_channels_4_9_Ghz_20Mhz[] = { 184, 188, 192, 196, 8, 12, 16, 0xff, 0x00 };
  unsigned int valid_channels_4_9_Ghz_10Mhz[] = { 183, 184, 185, 187, 188, 189, 7, 8, 9, 11, 0xff, 0x00 };
  unsigned char enable_40                     = 0;
  unsigned char enable_11j                    = 0;
  unsigned char rate_flags                    = 0;
  unsigned char i                             = 0;
  uint_16 offset                              = 0;
  int len, length, cmd, stop_bit;
  uint_16 local_buf[ADDITIONAL_STATS];
  uint_16 no_of_params;
  uint_8 chan_number;
  uint_8 send_stats_disable;
  uint_32 stats_cnt;
  uint_32 loop_var        = 0;
  uint_32 loop_cnt        = 0;
  uint_32 loop_chunk      = 10;
  uint_32 write_block     = 0;
  uint_32 user_mask       = 0;
  int ii                  = 0;
  unsigned short ch_width = 0;

  if ((arg == 4)
      //__9117_CODE_START
      || (arg == 5)
      //__9117_CODE_END
  ) {
    chan_number        = packet[0];
    stats_cnt          = packet[1];
    send_stats_disable = packet[2]; //do not receive stats
    rate_flags         = packet[3];
    ch_width           = rate_flags & 0x07; //channel width
    enable_11j         = (rate_flags & BIT(3));
    no_of_params       = NO_OF_PARAMS;
    //__9117_CODE_START
    if (arg == 5) {
      user_mask    = packet[4];
      no_of_params = 4;
      if (user_mask) {
        while (ii < 5) {
          switch (user_mask & BIT(ii)) {
            case GROUP_0:
              no_of_params += 8;
              break;
            case GROUP_1:
              no_of_params += 8;
              break;
            case GROUP_2:
              no_of_params += 9;
              break;
            case GROUP_3:
              no_of_params += 1;
              break;
            case GROUP_4:
              no_of_params += 22;
              break;
          }
          ii++;
        }
      }
    }
    //__9117_CODE_END
    printf("ch_width: %d \n", ch_width);
    if (ch_width == BW_U40 || ch_width == BW_L40 || ch_width == BW_F40) {
      enable_40 = 1;
    }
    uint_16 *send_rx_packet = malloc(loop_chunk * (no_of_params * SIZE_OF_PARAM));
    if (send_rx_packet == NULL) {
      printf("Unable to Allocate Memory\n");
      return ONEBOX_STATUS_FAILURE;
    }
    per_stats *sta_info = malloc(sizeof(per_stats));
    if (sta_info == NULL) {
      printf("Unable to Allocate Memory\n");
      free(send_rx_packet);
      return ONEBOX_STATUS_FAILURE;
    }

    memset(local_buf, 0, ADDITIONAL_STATS * SIZE_OF_PARAM);

    if (!enable_11j) {
      if (chan_number == 0xFF) {
        /* Pass 0xFF so as to skip channel programming */
      } else if ((chan_number >= 36 && chan_number <= 165 && ch_width == BW_20)) {
        /* 20Mhz bandwidth, verifying whether the channel is correct */
        for (i = 0; i < 24; i++) {
          if (chan_number == valid_channels_5_Ghz[i]) {
            break;
          }
        }
        if (i == 24) {
          printf("Invalid Channel issued by user\n");
          free(send_rx_packet);
          return -1;
        }
      } else if ((chan_number >= 38 && chan_number <= 163 && enable_40)) {
        /* 40Mhz bandwidth, verifying whether the channel is correct */
        for (i = 0; i < 21; i++) {
          if (chan_number == valid_channels_5_Ghz_40Mhz[i]) {
            break;
          }
        }
        if (i == 21) {
          printf("Invalid Channel issued by user\n");
          free(send_rx_packet);
          return -1;
        }
      } else if (!(chan_number <= 14)) {
        printf("Invalid Channel issued by user\n");
        free(sta_info);
        free(send_rx_packet);
        return -1;
      }
    } else {
      if (ch_width == BW_20) {
        for (i = 0; i < sizeof(valid_channels_4_9_Ghz_20Mhz) / sizeof(valid_channels_4_9_Ghz_20Mhz[0]); i++) {
          if (chan_number == valid_channels_4_9_Ghz_20Mhz[i]) {
            break;
          }
        }
        if (i == sizeof(valid_channels_4_9_Ghz_20Mhz) / sizeof(valid_channels_4_9_Ghz_20Mhz[0])) {
          printf("Invalid Channel issued by user\n");
          exit(0);
        }
      } else if (ch_width == BW_10) {
        for (i = 0; i < sizeof(valid_channels_4_9_Ghz_10Mhz) / sizeof(valid_channels_4_9_Ghz_10Mhz[0]); i++) {
          if (chan_number == valid_channels_4_9_Ghz_10Mhz[i]) {
            break;
          }
        }
        if (i == sizeof(valid_channels_4_9_Ghz_10Mhz) / sizeof(valid_channels_4_9_Ghz_10Mhz[0])) {
          printf("Invalid Channel issued by user\n");
          exit(0);
        }
      } else if (ch_width == BW_5) {
        printf("5MHz BW is not supported\n");
        exit(0);
      } else {
        printf("Invalid BW Configuration\n");
        exit(0);
      }
    }

    length = sizeof(per_stats);
    len    = sizeof(sta_info);
    if (stats_cnt) {
      cmd      = (unsigned short)PER_RECEIVE;
      stop_bit = 0;
    } else {
      cmd      = (unsigned short)PER_RCV_STOP;
      stop_bit = 1;
    }
    if (per_recv_send_wrapper(sta_info, cmd, len, stop_bit, rate_flags, chan_number, sock_fd) < 0) {
      free(send_rx_packet);
      return -1;
    }

    while (stats_cnt) {
      loop_cnt = (stats_cnt / loop_chunk);
      if (!loop_cnt) {
        write_block = stats_cnt % loop_chunk;
        loop_cnt    = 1;
      } else {
        write_block = loop_chunk;
      }

      for (loop_var = 0; loop_var < loop_cnt; loop_var++) {
        offset = 0;
        memset(send_rx_packet, 0, loop_chunk * no_of_params * SIZE_OF_PARAM);

        for (ii = 0; ii < write_block; ii++) {
          memset(sta_info, 0, sizeof(per_stats));
          if (sleep(1) != 0) {
            printf("Unable to sleep\n");
            free(sta_info);
            free(send_rx_packet);
            return ONEBOX_STATUS_FAILURE;
          }

          nlh = common_recv_mesg_wrapper(sock_fd, length);

          if (nlh == NULL) {
            free(sta_info);
            free(send_rx_packet);
            return ONEBOX_STATUS_FAILURE;
          } else {
            chan_number = 0;
            memcpy(sta_info, NLMSG_DATA(nlh), length);
            //__9117_CODE_START
            if (user_mask) {
              grouped_stats(sta_info, local_buf, user_mask, ii);
            } else {
              //__9117_CODE_END
              printf("%2d  %12d %12d %12x \n", (ii + 1), sta_info->crc_pass, sta_info->crc_fail, sta_info->cal_rssi);
              //__9117_CODE_START
            }
            //__9117_CODE_END
          }

          //__9117_CODE_START
          if (user_mask == 0) {
            //__9117_CODE_END
            local_buf[0] = htons(ii + 1); //(*(unsigned short *)&sta_info[2]);
            local_buf[1] = htons(sta_info->crc_pass);
            local_buf[2] = htons(sta_info->crc_fail);
            local_buf[3] = htons(sta_info->cal_rssi);

            //__9117_CODE_START
          }
          //__9117_CODE_END

          memcpy(&send_rx_packet[offset], local_buf, no_of_params * SIZE_OF_PARAM);
          offset = offset + no_of_params;
        }
        if (!send_stats_disable)
          rsi_send_packet(send_rx_packet, write_block * (no_of_params * SIZE_OF_PARAM));
        stats_cnt = stats_cnt - write_block;
        printf("no of stats remaining is - %d\n", stats_cnt);
      }
    }

    free(send_rx_packet);
    free(sta_info);
    free(nlh);

  } else {

    printf("matlab recv stats application\n");
    printf("Usage: <type> <no_of_arrguments>  <channel num> <no of stats>\n");
    printf("\tno of arguments should always be 3\n");
    printf("\tChannel num               - Channel to operate\n");
    printf("\tenter the no of stats\n");
    printf("\tenter 1 to disable packets write on to socket and vice-versa\n");

    return 1;
  }

  return ONEBOX_STATUS_SUCCESS;
}

int udp_server(void)
{
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  uint_8 packet[ONEBOX_MAX_PKT_LEN];
  uint_32 read_len, client_len;
  ONEBOX_STATUS status;

  server_addr.sin_family      = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port        = htons(SOURCE_PORT_NUM);

  conn_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (bind(conn_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    printf("udp_server: Binding Failed\n");
  }

  /* Run the UDP server */
  printf("**********************************************\n");
  printf("***** Server Is Listening On UDP Port %d *****\n", SOURCE_PORT_NUM);
  printf("***********************************************\n");

  while (1) {
    client_len = sizeof(client_addr);
    read_len   = recvfrom(conn_socket, packet, ONEBOX_MAX_PKT_LEN, 0, (struct sockaddr *)&client_addr, &client_len);

    printf("Received PKT Length from MATLAB : %d(0x%x)\n", read_len, read_len);
    printf("Received Packet in UDP :");

    /* Process the packet */
    status = rsi_process_packet(packet, read_len, &client_addr, client_len);
    if (status == ONEBOX_STATUS_FAILURE) {
      printf("Closing the Server\n");
      break;
    }
  }
  close(conn_socket);

  return 0;
}

int tcp_server(void)
{
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  uint_8 packet[ONEBOX_MAX_PKT_LEN];
  uint_8 packet1[ONEBOX_MAX_PKT_LEN];
  uint_32 read_len, client_len, i = 0, burst = 0, read_type;
  ONEBOX_STATUS status;
  uint_8 flag = 0;

  server_addr.sin_family      = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port        = htons(SOURCE_PORT_NUM);

  conn_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (bind(conn_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    printf("tcp_server: Binding Failed\n");
  }

  if (listen(conn_socket, 5) == -1) {
    perror("Listen");
    exit(1);
  }

  /* Run the TCP server */
  printf("**************************************************\n");
  printf("***** Server Is Listening On TCP Port %d *****\n", SOURCE_PORT_NUM);
  printf("**************************************************\n");

  client_sock = accept(conn_socket, (struct sockaddr *)&client_addr, &client_len);
  while (1) {
    client_len = sizeof(client_addr);

    read_len = recv(client_sock, packet, 5000, 0);
    printf("Received PKT Length from MATLAB : %d(0x%x)\n", read_len, read_len);

    i         = read_len;
    burst     = ntohl(*(&packet[0]));
    burst     = ntohl(*(uint_32 *)&packet[0]);
    read_type = ntohl(*(uint_32 *)&packet[4]);
    printf("Received burst :%d 0x%8x, %d", burst, packet[0], read_type);

    flag = 0;
    if (read_type == 1) {
      if (packet[0] == 7 || burst == 7)
        flag = 1;
    } else if (read_type == 2) {
      if (packet1[0] == 6 || packet1[0] == 7 || burst == 6 || burst == 7)
        flag = 1;
    }
    if (flag == 1) {
      if (read_len < 3000) {
        printf("read_len1 : %d  i : (%d)\n", read_len, i);
        usleep(400000);
        read_len = recv(client_sock, &packet[i++], 5000, 0);
        i += read_len;
        read_len = i;
        printf("read_len_final : %d  i : (%d)\n", read_len, i);
        usleep(400000);
      }
    }
    /* Process the received Packet */
    status = rsi_process_packet(packet, read_len, &client_addr, client_len);
    if (status == ONEBOX_STATUS_FAILURE) {
      printf("Closing the Server\n");
      break;
    }
  }

  close(client_sock);
  close(conn_socket);
  return 0;
}

int main(int argc, char **argv)
{
  printf("Creating interface to rpine0\n");

  /* Open socket */
  sock_fd = socket_creation();
  if (sock_fd < 0) {
    printf("Socket Creation Failed\n");
    return -1;
  }

#ifdef UDP_CONN
  /*** Starting UDP Server ***/
  udp_server();
#else
  /*** Starting TCP Server ***/
  tcp_server();
#endif
  close(sock_fd);
  printf("Device Closed\n");
  return 0;
}
