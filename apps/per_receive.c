// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include "per_util.h"
#include <signal.h>
#include <stdlib.h>

#define NO_OF_STATS         8
#define NUM_PACKETS_ARG_NUM 7
#define PER_MODE
//#define PER_BASIC_STATS
#ifdef RSSI_AVG
int kk;
#endif
per_stats *gsta_info;
int glen;
unsigned char grate_flags;
unsigned int gfreq;
int gsock_fd;
FILE *fd;
struct nlmsghdr *gnlh;

#define STOP_BIT 1
int crc_pass_cumulative = 0, crc_fail_cumulative = 0, no_of_packets = 0;
void receive_stats_exit_handler(int sig)
{
  /* This is for calculating  PER %.
   * This is computed as follows:
   * 1) If you are given a non-zero value for the expected number of packets (N), then just look at the packets received correctly (C) and compute the % PER as (N-C)*100/N
   * 2) If you are given '0' as the expected number of packets, then look at the packets received correctly (C) and the packets with CRC error (E) and compute the % as (E*100)/(C+E).
   */
  float rcv_percentage;
  signal(sig, SIG_IGN);
  if (no_of_packets) {
    rcv_percentage = ((no_of_packets - crc_pass_cumulative) * (100.0 / no_of_packets));
    printf("\n\nExpected packet percentage: %f%%\n", rcv_percentage);
  } else {
    rcv_percentage = (crc_fail_cumulative * 100.0) / (crc_pass_cumulative + crc_fail_cumulative);
    printf("\n\nReceived CRC Error percentage: %f%%\n", rcv_percentage);
  }
  if (per_recv_send_wrapper(gsta_info, PER_RCV_STOP, glen, STOP_BIT, grate_flags, gfreq, gsock_fd) < 0)
    printf("%s: failed to send per stop\n", __func__);

  free(gnlh);
  free(gsta_info);
  close(gsock_fd);
  fclose(fd);
  exit(0);
}
//__9117_CODE_START
void grouped_stats(per_stats *sta_info, FILE *pFile, int count, unsigned int user_mask)
{
  struct iwreq iwr;
  int bit_value;
  int i;
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
  for (i = 0; i < 5; i++) {
    bit_value = user_mask & (1 << i);
    switch (bit_value) {
      case GROUP_0:
        if ((count % 20) == 0) {
          printf("%s %s %s %s %s %s %s ",
                 "non_wifi_pkt",
                 "false_ed",
                 "pkt_abort",
                 "fls_rx_start",
                 "false_cca",
                 "cal_rssi",
                 "ack_sent");
          fprintf(pFile,
                  "%s %s %s %s %s %s %s ",
                  "non_wifi_pkt",
                  "false_ed",
                  "pkt_abort",
                  "fls_rx_start",
                  "false_cca",
                  "cal_rssi",
                  "ack_sent");
        } else {
          printf("%12d %10d %8d %10d %12d %8d %8d",
                 sta_info->cca_stk,
                 sta_info->cca_not_stk,
                 sta_info->pkt_abort,
                 sta_info->fls_rx_start,
                 sta_info->cca_idle,
                 sta_info->cal_rssi,
                 sta_info->ack_sent);
          fprintf(pFile,
                  "%12d %10d %8d %10d %12d %8d %8d",
                  sta_info->cca_stk,
                  sta_info->cca_not_stk,
                  sta_info->pkt_abort,
                  sta_info->fls_rx_start,
                  sta_info->cca_idle,
                  sta_info->cal_rssi,
                  sta_info->ack_sent);
        }
        break;
      case GROUP_1:
        if ((count % 20) == 0) {
          printf("%s %s %s %s %s %s ",
                 "false_under_sat",
                 "bss_bcast_pkts",
                 "bss_mcast_pkts",
                 "bss_da_matched_mcast_pkts",
                 "eof_pkt_drops",
                 "mask_pkt_drop");
          fprintf(pFile,
                  "%s %s %s %s %s %s ",
                  "false_under_sat",
                  "bss_bcast_pkts",
                  "bss_mcast_pkts",
                  "bss_da_matched_mcast_pkts",
                  "eof_pkt_drops",
                  "mask_pkt_drop");
        } else {
          printf("%12d %15d %15d %17d %19d %15d",
                 sta_info->false_under_sat,
                 sta_info->bss_broadcast_pkts,
                 sta_info->bss_multicast_pkts,
                 sta_info->bss_da_matched_multicast_pkts,
                 sta_info->eof_pkt_drop_count,
                 sta_info->mask_pkt_drop_count);
          fprintf(pFile,
                  "%12d %15d %15d %17d %19d %15d",
                  sta_info->false_under_sat,
                  sta_info->bss_broadcast_pkts,
                  sta_info->bss_multicast_pkts,
                  sta_info->bss_da_matched_multicast_pkts,
                  sta_info->eof_pkt_drop_count,
                  sta_info->mask_pkt_drop_count);
        }
        break;
      case GROUP_2:
        if ((count % 20) == 0) {
          printf("%s %s %s %s %s %s %s %s %s %s %s ",
                 "11G_pkts",
                 "11B_pkts",
                 "11N/mcs_rates",
                 "11AC_pkts",
                 "11AX_pkts",
                 "HE_SU_PPDU_pkts",
                 "HE_ER_SU_PPDU_pkts",
                 "HE_TRIG_PPDU_pkts",
                 "HE_MU_PPDU_pkts",
                 "HE_MU_MIMO_pkts",
                 "HE_MU_OFDMA_pkts");
          fprintf(pFile,
                  "%s %s %s %s %s %s %s %s %s %s %s ",
                  "11G_pkts",
                  "11B_pkts",
                  "11N/mcs_rates",
                  "11AC_pkts",
                  "11AX_pkts",
                  "HE_SU_PPDU_pkts",
                  "HE_ER_SU_PPDU_pkts",
                  "HE_TRIG_PPDU_pkts",
                  "HE_MU_PPDU_pkts",
                  "HE_MU_MIMO_pkts",
                  "HE_MU_OFDMA_pkts");
        } else {
          printf("%9d %8d %12d %10d %9d %9d %18d %18d %18d %18d %19d",
                 total_11g,
                 total_11b,
                 total_11n,
                 sta_info->pkt_count_11AC,
                 total_11ax,
                 sta_info->pkt_count_HE_SU_PPDU,
                 sta_info->pkt_count_HE_ER_SU_PPDU,
                 sta_info->pkt_count_HE_TRIG_PPDU,
                 sta_info->pkt_count_HE_MU_PPDU,
                 sta_info->he_mu_mimo_pkt_cnt,
                 sta_info->he_mu_ofdma_pkt_cnt);
          fprintf(pFile,
                  "%9d %8d %12d %10d %9d %9d %18d %18d %18d %18d %19d",
                  total_11g,
                  total_11b,
                  total_11n,
                  sta_info->pkt_count_11AC,
                  total_11ax,
                  sta_info->pkt_count_HE_SU_PPDU,
                  sta_info->pkt_count_HE_ER_SU_PPDU,
                  sta_info->pkt_count_HE_TRIG_PPDU,
                  sta_info->pkt_count_HE_MU_PPDU,
                  sta_info->he_mu_mimo_pkt_cnt,
                  sta_info->he_mu_ofdma_pkt_cnt);
        }
        break;
      case GROUP_3:
        if ((count % 20) == 0) {
          printf("%s ", "tx_done");
          fprintf(pFile, "%s ", "tx_done");
        } else {
          printf("%10d", sta_info->tx_done);
          fprintf(pFile, "%10d", sta_info->tx_done);
        }
        break;
      case GROUP_4:
        if ((count % 20) == 0) {
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
          fprintf(pFile,
                  "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s ",
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
        } else {
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
          fprintf(
            pFile,
            "%8d %8d %8d %8d %8d %8d %8d %15d %12d %10d %12d %10d %12d %15d %10d %12d %12d %10d %12d %9d %8d %15d",
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
        }
        break;
    }
  }
  printf("\n");
  fprintf(pFile, "\n");
}
//__9117_CODE_END

int main(int argc, char **argv)
{
  int sock_fd;
  struct nlmsghdr *nlh;
  unsigned int freq, user_mask = 0;
  int first_time = 1;
  int count = 0, stop_bit = 0;
  int i, pid;
  int cmd, len, length;
  unsigned short ch_width = 0;
  FILE *pFile;
  struct iwreq iwr;
#ifdef RSSI_AVG
  int rssi_buff[10];
  int jj         = 0, avg_loop;
  int rssi_total = 0, rssi_avg = 0;
#endif
  unsigned int valid_channels_5_Ghz[]         = { 36,  40,  44,  48,  52,  56,  60,  64,  100, 104, 108, 112,
                                          116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165 };
  unsigned int valid_channels_5_Ghz_40Mhz[]   = { 38,  42,  46,  50,  54,  58,  62,  102, 106, 110, 114,
                                                118, 122, 126, 130, 134, 138, 151, 155, 159, 163 };
  unsigned int valid_channels_4_9_Ghz_20Mhz[] = { 184, 188, 192, 196, 8, 12, 16, 0xff, 0x00 };
  unsigned int valid_channels_4_9_Ghz_10Mhz[] = { 183, 185, 187, 189, 7, 9, 11, 0xff, 0x00 };
  unsigned char enable_40                     = 0;
  unsigned char enable_11j                    = 0;
  unsigned char rate_flags                    = 0;

  if ((argc != NO_OF_STATS) && (argc != (NO_OF_STATS - 1)) && (argc != (NO_OF_STATS - 2))) {
    printf("Onebox dump stats application\n");
    printf("Usage: %s <rpine_interface> <file_name> <channel num> <start-stop> <Channel BW> [stat selection] [no of "
           "packets]\n",
           argv[0]);
    printf("\tFile_name   		- File to dump status\n");
    printf("\tChannel num 		- Channel to operate\n");
    printf("\tStart-Stop value	- 0: Start & 1: Stop \n");
    printf("\tChannel BW  		- 0: 20MHz, 2: Upper 40MHz, 4: Lower 40MHz 6: Full 40MHz & 8: 20MHz for 11J Channel\n");
    printf("\t[Optional]: stat selection  - value to display the required stats\n");
    printf("\t[Optional]: no_of_packets   - 0: gives pass %%, non-zero: gives expected packets %%\n");

    return 1;
  }

  gfreq = freq = atoi(argv[3]);
  stop_bit     = atoi(argv[4]);
  grate_flags = rate_flags = atoi(argv[5]);
  //__9117_CODE_START
  if ((argc == NO_OF_STATS) || (argc == (NO_OF_STATS - 1))) {
    user_mask = atoi(argv[6]);
  }
  //__9117_CODE_END
  ch_width = rate_flags & (0x07);
  if (!stop_bit) {
    signal(SIGINT, receive_stats_exit_handler);
    if (argc == NO_OF_STATS)
      no_of_packets = atoi(argv[NUM_PACKETS_ARG_NUM]);
  }

  enable_11j = (rate_flags & BIT(3));

  if (ch_width == BW_U40 || ch_width == BW_L40 || ch_width == BW_F40) {
    enable_40 = 1;
  }

  if (!enable_11j) {
    if (freq == 0xFF) {
      /* Pass 0xFF so as to skip channel programming */
    } else if ((freq >= 36 && freq <= 165 && ch_width == BW_20)) {
      for (i = 0; i < 24; i++) {
        if (freq == valid_channels_5_Ghz[i]) {
          break;
        }
      }
      if (i == 24) {
        printf("Invalid Channel issued by user\n");
        exit(0);
      }
    } else if ((freq >= 36 && freq <= 165 && enable_40)) {
      for (i = 0; i < 21; i++) {
        if (freq == valid_channels_5_Ghz_40Mhz[i]) {
          break;
        }
      }
      if (i == 21) {
        printf("Invalid Channel issued by user\n");
        exit(0);
      }
    } else if (!(freq <= 14)) {
      printf("Invalid Channel issued by user\n");
      exit(0);
    }
  } else {
    if (ch_width == BW_20) {
      for (i = 0; i < sizeof(valid_channels_4_9_Ghz_20Mhz) / sizeof(valid_channels_4_9_Ghz_20Mhz[0]); i++) {
        if (freq == valid_channels_4_9_Ghz_20Mhz[i]) {
          break;
        }
      }
      if (i == sizeof(valid_channels_4_9_Ghz_20Mhz) / sizeof(valid_channels_4_9_Ghz_20Mhz[0])) {
        printf("Invalid Channel issued by user\n");
        exit(0);
      }
    } else if (ch_width == BW_10) {
      for (i = 0; i < sizeof(valid_channels_4_9_Ghz_10Mhz) / sizeof(valid_channels_4_9_Ghz_10Mhz[0]); i++) {
        if (freq == valid_channels_4_9_Ghz_10Mhz[i]) {
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

  pFile = fopen(argv[2], "w");
  if (pFile == NULL) {
    printf("Unable to create a file\n");
    return -1;
  }

  fd                  = pFile;
  per_stats *sta_info = malloc(sizeof(per_stats));
  gsta_info           = sta_info;
  length              = sizeof(per_stats);
  sock_fd             = socket_creation();
  if (sock_fd < 0) {
    return -1;
  }
  gsock_fd = sock_fd;
  if (!stop_bit)
    cmd = (unsigned short)PER_RECEIVE;
  else
    cmd = (unsigned short)PER_RCV_STOP;
  glen = len = sizeof(sta_info);
  printf("receive application\n");
  if (per_recv_send_wrapper(sta_info, cmd, len, stop_bit, rate_flags, freq, sock_fd) < 0)
    return -1;

  while (1) {
#if 0
		if(sleep(1)!=0)
		{
			printf("Unable to sleep\n");            
			free(sta_info);        
			break;
		}
#endif

    if (stop_bit) {
      printf("RECEIVE STOPPED\n");
      break;
    }
    nlh = common_recv_mesg_wrapper(sock_fd, length);
    if (nlh == NULL) {
      break;
    } else {

      gnlh = nlh;
#ifdef RSSI_AVG
      if (jj < 10) {
        rssi_buff[jj] = sta_info->cal_rssi;
        jj            = jj + 1;
      } else {
        rssi_total    = 0;
        rssi_buff[kk] = sta_info->cal_rssi;
        for (avg_loop = 0; avg_loop < 10; avg_loop++) {
          rssi_total = rssi_total + rssi_buff[avg_loop];
        }
        rssi_avg = rssi_total / 10;
        kk++;
        sta_info->cal_rssi = rssi_avg;
        if (kk == 10) {
          kk = 0;
        }
      }
#endif

      freq = 0;
      memcpy(sta_info, NLMSG_DATA(nlh), length);

      if (first_time) {
        first_time = 0;
        continue;
      }
      crc_pass_cumulative += sta_info->crc_pass;
      crc_fail_cumulative += sta_info->crc_fail;

      //__9117_CODE_START
      if (user_mask != 0) {
        if (user_mask < 0 || user_mask > 31) {
          printf("Please give the user mask from 0 to 31\n");
          exit(0);
        }
        if ((count % 20) == 0) {
          printf("%s %s ", "crc_pass", "crc_fail");
          fprintf(pFile, "%s %s ", "crc_pass", "crc_fail");
        } else {
          printf("%4d %8d", sta_info->crc_pass, sta_info->crc_fail);
          fprintf(pFile, "%4d %8d", sta_info->crc_pass, sta_info->crc_fail);
        }
        grouped_stats(sta_info, pFile, count, user_mask);
      } else {
        //__9117_CODE_END

        if ((count % 20) == 0) {
#ifdef PER_MODE
#ifdef PER_BASIC_STATS
          printf(" %8s %8s %8s %8s\n", "crc_pass", "crc_fail", "false_cca", "cal_rssi");
          fprintf(pFile, "%8s %8s %8s %8s\n", "crc_pass", "crc_fail", "false_cca", "cal_rssi");
#else
          printf("%12s %12s\n", "crc_pass", "crc_fail");
          fprintf(pFile, "\n%12s %12s\n", "crc_pass", "crc_fail");
#endif
        }
#ifdef PER_BASIC_STATS
        printf("%7d %7d %7d %7d\n", sta_info->crc_pass, sta_info->crc_fail, sta_info->cca_idle, sta_info->cal_rssi);
        fprintf(pFile,
                "%7d %7d %7d %7d\n",
                sta_info->crc_pass,
                sta_info->crc_fail,
                sta_info->cca_idle,
                sta_info->cal_rssi);
#else
        printf("%12d %12d\n", sta_info->crc_pass, sta_info->crc_fail);
        fprintf(pFile, "%12d %12d\n", sta_info->crc_pass, sta_info->crc_fail);
#endif
#else
          printf(" %20s %8s %10s %10s %10s \n", "tx_pkts", "retries", "pkts_drop", "rssi", "cons_drops");
          fprintf(pFile, "%20s %10s %10s %10s %10s \n", "tx_pkts", "retries", "pkts_drop", "rssi", "cons_drops");
        }
        printf("%20d %9d %9d %9d %9d \n",
               sta_info->tx_pkts,
               sta_info->tx_retries,
               sta_info->xretries,
               sta_info->bea_avg_rssi,
               sta_info->max_cons_pkts_dropped);
        fprintf(pFile,
                "%20d %9d %9d %9d %9d\n",
                sta_info->tx_pkts,
                sta_info->tx_retries,
                sta_info->xretries,
                sta_info->bea_avg_rssi,
                sta_info->max_cons_pkts_dropped);
#endif

        //__9117_CODE_START
      }
      //__9117_CODE_END
      ++count;
      free(nlh);
    }
    fflush(pFile);
  }
  free(sta_info);
  close(sock_fd);
  fclose(pFile);

  return 0;
}
