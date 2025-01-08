// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/types.h>
#include <linux/if.h>
#include <stdlib.h>
#include <linux/wireless.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include "per_util.h"

#define NO_OF_ARGS 2

#define MAX_STRUCTS     4
#define MAX_STRUCT_SIZE 100
#define MAX_NUM_REGION  5
#define MAX_FILE_SIZE   (40 * 1000)

#define FILENAME "wlan_gain_table.txt"
#define INTFNAME argv[1]

//#define DEBUG
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

unsigned char tmp[100];

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

int update_wlan_gain_table(int argc, char *argv[], char *ifName, int sfd)
{
  update_wlan_gain_table_t table_info;
  uint16_t max_struct_length;
  FILE *fp;
  unsigned int string_matched, i, str_indx;
  unsigned int ret = ONEBOX_STATUS_SUCCESS, count, no_of_structs = 0, comment_decoded = 0;
  unsigned char str[MAX_STRUCT_SIZE], *str_tmp, ch;
  struct structure_info {
    unsigned char name[MAX_STRUCT_SIZE];
    uint16_t max_struct_length;
    uint8_t band;
    uint8_t bandwidth;
    uint16_t struct_size;
    uint32_t reserved;
  } struct_names[MAX_STRUCTS] = {
    { "_RS9116_HP_MODE_REGION_BASED_RATE_VS_MAXPOWER_SINGLE_BAND_NONHPM[]",
      MAX_WLAN_GAIN_TABLE_SIZE,
      _2GHZ_BAND,
      BAND_WIDTH_20,
      0 },
    { "_RS9116_HP_MODE_REGION_BASED_RATE_VS_MAXPOWER_SINGLE_BAND_NONHPM_40MHZ[]", 256, _2GHZ_BAND, BAND_WIDTH_40, 0 },
    { "_RS9113_RS8111_5G_HP_MODE_REGION_BASED_RATE_VS_MAXPOWER_NONHPM[]", 256, _5GHZ_BAND, BAND_WIDTH_20, 0 },
    { "_RS9113_RS8111_5G_HP_MODE_REGION_BASED_RATE_VS_MAXPOWER_NONHPM_40MHZ[]", 256, _5GHZ_BAND, BAND_WIDTH_40, 0 },
  };
  struct region_info {
    unsigned char extracted_str[40];
    unsigned char updated_str[3];
  } region[MAX_NUM_REGION] = {
    { "FCC", "0," }, { "ETSI", "1," }, { "TELEC", "2," }, { "WORLDWIDE", "3," }, { "KCC", "4," },
  };

  //! Open file.
  fp = fopen(FILENAME, "r");

  if (fp == NULL) {
    printf("Unable to Open %s file to read\n", FILENAME);
    return ONEBOX_STATUS_FAILURE;
  } else {
    printf("Opening file %s \n", FILENAME);

    fseek(fp, 0, SEEK_END);
    unsigned int len = ftell(fp);
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
      i = 0;
      READ_STRING(ONEBOX_STATUS_SUCCESS);
      while (i < MAX_STRUCTS) {
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
    table_info.band      = struct_names[i].band;
    table_info.bandwidth = struct_names[i].bandwidth;
    table_info.reserved  = struct_names[i].reserved;
    max_struct_length    = struct_names[i].max_struct_length;
#if 0
    do {
      READ_STRING(ONEBOX_STATUS_FAILURE);
      if(strcmp(str, "=") == 0) {
        break;
      }
    } while(1);
#ifdef DEBUG
    printf("= \n");
#endif
#endif
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

    printf("Sending Gain table %d with %d bytes to Driver\n\n", no_of_structs, str_indx);
    table_info.struct_size = str_indx;
    if (table_info.struct_size <= max_struct_length) {
      gain_table_sckt_creation(table_info, sfd);
    } else {
      printf("struct size excedded max size %d \n", max_struct_length);
    }
  } while (1);

file_failure:
  ret = ONEBOX_STATUS_FAILURE;
end_of_file:
  fclose(fp);
#ifdef DEBUG
  printf("return %d\n", ret);
#endif
  return ret;
}
