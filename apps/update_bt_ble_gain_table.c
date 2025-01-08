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
#include <linux/errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include "per_util.h"
#include "bt_util.h"

#define BT_BLE_GAIN_TABLE_UPDATE 0x06

#define NO_OF_ARGS 2

#define MAX_STRUCTS     4 //! Max Gain table structures, deecided based on opermode
#define MAX_STRUCT_SIZE 100
#define MAX_NUM_REGION  5
#define MAX_FILE_SIZE   (40 * 1000)

#define PROTOCOL_ID argv[1]

//__9117_CODE_START
#ifdef CONFIG_917
#define FILENAME "ble_gain_table.txt"
#else
//__9117_CODE_END
#define FILENAME "bt_ble_gain_table.txt"
//__9117_CODE_START
#endif
//__9117_CODE_END

//#define DEBUG  //! Enable for applicaiton debug logs
#ifdef DEBUG
#define PRINT_CHARACTER   printf("character read: %c\n", ch);
#define PRINT_LINE_END    printf("Found Line end\n");
#define PRINT_READ_STRING printf("Read string: %s %d\n", str, strlen(str));
#define PRINT_REGION_CODE \
  printf("String %s matched with Region %s and updated to %s\n", str, region[i].extracted_str, region[i].updated_str);
#define PRINT_COMMENT_FOUND printf("comment\n");
#else
#define PRINT_CHARACTER
#define PRINT_LINE_END
#define PRINT_READ_STRING
#define PRINT_REGION_CODE
#define PRINT_COMMENT_FOUND

#endif

#define EOF_FOUND(x)               \
  printf("End of File reached\n"); \
  ret = x;                         \
  goto end_of_file;

#define CHK_NEW_LINE_CHAR                \
  str_tmp = str;                         \
  do {                                   \
    if (strstr(str_tmp, "\n") == NULL) { \
      break;                             \
    }                                    \
    PRINT_LINE_END;                      \
  } while (1);

#define VALID_STRING(x)                                                         \
  count = 0;                                                                    \
  DECODE_COMMENTS(ONEBOX_STATUS_FAILURE);                                       \
  if (!comment_decoded) {                                                       \
    str_tmp = str;                                                              \
    while ((str_tmp = strstr(str_tmp, ","))) {                                  \
      str_tmp++;                                                                \
      count++;                                                                  \
    }                                                                           \
    if (count > 1) {                                                            \
      printf("ERROR in structure %d: string invalid %s\n", no_of_structs, str); \
      goto file_failure;                                                        \
    }                                                                           \
  }

#define DECODE_COMMENTS(x)                                                                             \
  if (strlen(str) > 1 && (str[0] == '/' && str[1] == '/')) {                                           \
    comment_decoded = 1;                                                                               \
    PRINT_COMMENT_FOUND;                                                                               \
    GO_TO_LINE_END(x);                                                                                 \
  } else {                                                                                             \
    str_tmp = str;                                                                                     \
    if (strstr(str_tmp, "//") != NULL) {                                                               \
      printf("ERROR in structure %d: Need space between comment and string %s\n", no_of_structs, str); \
      goto file_failure;                                                                               \
    }                                                                                                  \
  }

#define GO_TO_LINE_END(x) \
  do {                    \
    ch = fgetc(fp);       \
    if (ch == EOF) {      \
      PRINT_CHARACTER;    \
      EOF_FOUND(x);       \
    }                     \
    if (ch == '\n') {     \
      PRINT_LINE_END;     \
      break;              \
    }                     \
  } while (1);

#define READ_STRING(x)                  \
  do {                                  \
    comment_decoded = 0;                \
    if (fscanf(fp, "%s", str) != EOF) { \
      PRINT_READ_STRING;                \
      VALID_STRING(x);                  \
      CHK_NEW_LINE_CHAR;                \
    } else {                            \
      EOF_FOUND(x);                     \
    }                                   \
  } while (comment_decoded);

#define DECODE_REGION                                   \
  while (i < MAX_NUM_REGION) {                          \
    if (strstr(str, region[i].extracted_str) != NULL) { \
      strcpy(str, region[i].updated_str);               \
      PRINT_REGION_CODE;                                \
      break;                                            \
    }                                                   \
    i++;                                                \
  }

#define MAX_BT_GAIN_TABLE_MAXPOWER_SIZE  10
#define MAX_BLE_GAIN_TABLE_MAXPOWER_SIZE 10
#define MAX_BT_GAIN_TABLE_OFFSET_SIZE    128
#define MAX_BLE_GAIN_TABLE_OFFSET_SIZE   128

typedef struct update_bt_ble_gain_table_s {
#define BT_NODE_ID  1
#define BLE_NODE_ID 0
  uint_8 node_id;
#define BT_BLE_GAIN_TABLE_MAX_POWER 0
#define BT_BLE_GAIN_TABLE_OFFSET    1
  uint_8 request_type;
  uint_8 struct_size;                              //! Gain table length
  uint_8 structure[MAX_BT_GAIN_TABLE_OFFSET_SIZE]; //! Gain table data
} update_bt_ble_gain_table_t;

unsigned char tmp[100];

int send_bt_ble_gain_table_update_frame_to_drv(update_bt_ble_gain_table_t *params, int sfd);
int recv_bt_ble_gain_table_update_from_drv(uint_8 *data, uint_8 len, int sfd);

/* reverse:  reverse string s in place */
void reverse(char s[])
{
  int i, j;
  char c;

  for (i = 0, j = strnlen(s, sizeof(tmp)) - 1; i < j; i++, j--) {
    c    = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

/* itoa:  convert n to characters in s */
void itoa(int n, char s[])
{
  int i, sign;

  if ((sign = n) < 0) /* record sign */
    n = -n;           /* make n positive */
  i = 0;
  do {                     /* generate digits in reverse order */
    s[i++] = n % 10 + '0'; /* get next digit */
  } while ((n /= 10) > 0); /* delete it */
  if (sign < 0)
    s[i++] = '-';
  s[i] = '\0';
  reverse(s);
}

char *itoa_w(unsigned int value)
{
  itoa(value, tmp);
  return tmp;
}

int is_struct_data_valid(char *str)
{
  int i = 0;
  if (strnlen(str, MAX_STRUCT_SIZE) != 0 && strnlen(str, MAX_STRUCT_SIZE) <= 3) {
    for (i = 0; str[i] != '\0'; i++) {
      if (str[i] < '0' || str[i] > '9')
        return 0;
    }
    if (atoi(str) >= 256)
      return 0;
    return 1;
  } else {
    return 0;
  }
}

int main(int argc, char *argv[])
{
  int_32 sfd;
  update_bt_ble_gain_table_t table_info;
  uint_16 max_struct_length;
  int_16 status;

  FILE *fp;
  uint_32 string_matched, i = 0, max_check, str_indx;
  uint_32 ret = ONEBOX_STATUS_SUCCESS, count, comment_decoded = 0;
  int_32 no_of_structs = 0;
  uint_8 str[MAX_STRUCT_SIZE], *str_tmp, ch;
  struct structure_info {
    uint_8 name[MAX_STRUCT_SIZE];
    uint_16 max_struct_length;
    uint_8 node_id;
    uint_8 request_type;
    uint_16 struct_size;
  } struct_names[MAX_STRUCTS] = {
    { "_RS9116_BT_REGION_BASED_MAXPOWER_XX[]",
      MAX_BT_GAIN_TABLE_MAXPOWER_SIZE,
      BT_NODE_ID,
      BT_BLE_GAIN_TABLE_MAX_POWER,
      0 },
    { "_RS9116_BT_REGION_BASED_MAXPOWER_VS_OFFSET_XX[]",
      MAX_BT_GAIN_TABLE_OFFSET_SIZE,
      BT_NODE_ID,
      BT_BLE_GAIN_TABLE_OFFSET,
      0 },
    { "_RS9116_BLE_REGION_BASED_MAXPOWER_XX[]",
      MAX_BLE_GAIN_TABLE_MAXPOWER_SIZE,
      BLE_NODE_ID,
      BT_BLE_GAIN_TABLE_MAX_POWER,
      0 },
    { "_RS9116_BLE_REGION_BASED_MAXPOWER_VS_OFFSET_XX[]",
      MAX_BT_GAIN_TABLE_OFFSET_SIZE,
      BLE_NODE_ID,
      BT_BLE_GAIN_TABLE_OFFSET,
      0 },
  };

  struct region_info {
    uint_8 extracted_str[40];
    uint_8 updated_str[3];
  } region[MAX_NUM_REGION] = {
    { "FCC", "0," }, { "ETSI", "1," }, { "TELEC", "2," }, { "WORLDWIDE", "3," }, { "KCC", "4," },
  };

  if (argc < 2) {
    usage();
    return ONEBOX_STATUS_FAILURE;
  }

  /* Open socket */
  sfd = socket_creation();
  if (sfd < 0) {
    ONEBOX_PRINT("Socket Creation Failed\n");
    return ONEBOX_STATUS_FAILURE;
  }

  //! Open file.
  fp = fopen(FILENAME, "r");

  if (fp == NULL) {
    printf("Unable to Open %s file to read\n", FILENAME);
    return ONEBOX_STATUS_FAILURE;
  } else {
    printf("Opening file %s \n", FILENAME);

    fseek(fp, 0, SEEK_END);
    uint_32 len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (len > MAX_FILE_SIZE) {
      printf("File %s size %d is to big\n", FILENAME, len);
      goto file_failure;
    }
  }

  printf("Parsing File\n\n");
  do {
    no_of_structs++;
#ifdef DEBUG
    printf("Finding gain table %d\n", no_of_structs);
#endif
    do {
      if (atoi(PROTOCOL_ID) == BT_NODE_ID) //! Check for BT Classic Gain tables
      {
        i = 0;

      } else if (atoi(PROTOCOL_ID) == BLE_NODE_ID) //! Check for BLE Gain tables
      {
        i = 2;
      }
      max_check = i + 2;
      READ_STRING(ONEBOX_STATUS_SUCCESS);
      while (i < (max_check)) {
        if (strcmp(struct_names[i].name, str) == 0) {
          goto string_matched;
        }
        i++;
      }
    } while (1);

string_matched:
#ifdef DEBUG
    printf("String : %s, indx:%d\n", str, i);
#else
    printf("Gain table %d name: %s\n", no_of_structs, str);
#endif

#ifdef DEBUG
    printf("Finding Gain table start\n");
#endif
    table_info.node_id      = struct_names[i].node_id;
    table_info.request_type = struct_names[i].request_type;
    max_struct_length       = struct_names[i].max_struct_length;
    memset(table_info.structure, 0, sizeof(table_info.structure));
    do {
      READ_STRING(ONEBOX_STATUS_FAILURE);
      if (strstr(str, "{") != NULL) {
        break;
      }
    } while (1);
#ifdef DEBUG
    printf("{ \n");
#endif

    printf("Parsing Gain table\n");
    str_indx = 0;
    do {
      READ_STRING(ONEBOX_STATUS_FAILURE);

      if ((strstr(str, "}") != NULL)) {
        break;
      }
      if ((strstr(str, "{") != NULL)) {
        goto file_failure;
      }
      i = 0;
      DECODE_REGION;

      if (str[strnlen(str, MAX_STRUCT_SIZE) - 1] == ',') {
        str[strnlen(str, MAX_STRUCT_SIZE) - 1] = '\0';
#ifdef DEBUG
        printf("string trimmed:%s, decimal:%d str_indx: %d\n", str, atoi(str), str_indx);
#endif
        if (str_indx < max_struct_length) {
          //! check string validity before copying.
          if (is_struct_data_valid(str)) {
            table_info.structure[str_indx++] = atoi(str);
          } else {
            printf("ERROR in Gain table %d: Invalid Data %s found\n", no_of_structs, str);
            goto file_failure;
          }
        } else {
          printf("ERROR: struct size excedded max size %d \n", max_struct_length);
          goto file_failure;
        }
      } else {
        printf("ERROR: At string %s in Gain table %d \n", str, no_of_structs);
        goto file_failure;
      }
    } while (1);

    printf("Sending Gain table %d with %d bytes to Driver\n", no_of_structs, str_indx);
    table_info.struct_size = str_indx;

    if (send_bt_ble_gain_table_update_frame_to_drv(&table_info, sfd) < 0) {
      printf("Unable to perform Gain table update\n");
      goto file_failure;
    } else {
      if (recv_bt_ble_gain_table_update_from_drv((uint_8 *)&status, sizeof(status), sfd) < 0) {
        goto file_failure;
      }
      if (status == 0) {
        printf("Gain table updated successfully \n\n");
      } else {
        if (status == -EINVAL) {
          printf("ERROR : Update BLE Gain table not supported for ACx modules \n\n", status);
        } else {
          printf("Gain table update failed with status : %x \n\n", status);
        }
        goto file_failure;
      }
    }
  } while (1);

file_failure:
  ret = ONEBOX_STATUS_FAILURE;
end_of_file:
  fclose(fp);
  if (ret == ONEBOX_STATUS_SUCCESS) {
    printf("\n**************Successfully completed programming %d gain tables ************* \n\n", (no_of_structs - 1));
  }

#ifdef DEBUG
  printf("return %d\n", ret);
#endif
  return ret;
}

/** This function gives the usage of the onebox utility
 * @param  void
 * @return void
 */
void usage()
{
  ONEBOX_PRINT("Usage: ./bt_ble_gain_table_update protocol_node \n");
  return;
}

int send_bt_ble_gain_table_update_frame_to_drv(update_bt_ble_gain_table_t *params, int sfd)
{
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh        = NULL;
  struct rsi_nl_desc *nl_desc = NULL;
  int ret;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0; /* For Linux Kernel */
  dest_addr.nl_groups = 0; /* unicast */
  nlh                 = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(update_bt_ble_gain_table_t) + NL_DATA_DESC_SZ));
  memset(nlh, 0, NLMSG_SPACE(sizeof(update_bt_ble_gain_table_t) + NL_DATA_DESC_SZ));
  nlh->nlmsg_len        = NLMSG_SPACE(sizeof(update_bt_ble_gain_table_t) + NL_DATA_DESC_SZ);
  nl_desc               = (struct rsi_nl_desc *)NLMSG_DATA(nlh);
  nl_desc->desc_word[0] = BT_BLE_GAIN_TABLE_UPDATE;
  nl_desc->desc_word[1] = sizeof(update_bt_ble_gain_table_t);
  nlh->nlmsg_type       = BT_PACKET;
  nlh->nlmsg_pid        = getpid();
  nlh->nlmsg_flags      = 0;
  memcpy(NLMSG_DATA(nlh) + NL_DATA_DESC_SZ, params, sizeof(update_bt_ble_gain_table_t));
  ret = common_send_mesg_wrapper(sfd, dest_addr, nlh);
  if (ret < 0)
    close(sfd);
  free(nlh);
  return ret;
}

int recv_bt_ble_gain_table_update_from_drv(uint_8 *data, uint_8 len, int sfd)
{
  struct msghdr msg    = { 0 };
  struct nlmsghdr *nlh = NULL;
  struct iovec iov     = { 0 };
  struct sockaddr_nl dest_addr;
  nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(len));
  memset(nlh, 0, NLMSG_SPACE(len));
  nlh->nlmsg_len  = NLMSG_SPACE(len);
  iov.iov_base    = (void *)nlh;
  iov.iov_len     = nlh->nlmsg_len;
  msg.msg_name    = (void *)&dest_addr;
  msg.msg_namelen = sizeof(dest_addr);
  msg.msg_iov     = &iov;
  msg.msg_iovlen  = 1;

  if (recvmsg(sfd, &msg, 0) < 0) {
    close(sfd);
    return -1;
  }
  memcpy(data, NLMSG_DATA(nlh), len);
  free(nlh);
  return 0;
}
