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

#define FCC        0x00
#define ETSI       0x01
#define TELEC      0x02
#define WORLD      0x03
#define WORLD_SAFE 0x05
#define SRRC       0x06

int is11brate(unsigned int rate)
{
  switch (rate) {
    case RSI_RATE_1:
    case RSI_RATE_2:
    case RSI_RATE_5_5:
    case RSI_RATE_11:
      return 1;
    default:
      return 0;
  }
}
int islegacyrate(unsigned int rate)
{
  switch (rate) {
    case RSI_RATE_6:
    case RSI_RATE_9:
    case RSI_RATE_12:
    case RSI_RATE_18:
    case RSI_RATE_24:
    case RSI_RATE_36:
    case RSI_RATE_48:
    case RSI_RATE_54:
      return 1;
    default:
      return 0;
  }
}
//main
int main(int argc, char *argv[])
{
  struct iwreq iwr;
  int tx_pwr, tx_pktlen, tx_mode, chan_number;
  int sockfd, i, cmd;
  char *tmp_rate;
  per_params_t per_params;
  unsigned char rate_flags                    = 0;
  unsigned int valid_channels_5_Ghz[]         = { 36,  40,  44,  48,  52,  56,  60,  64,  100, 104, 108, 112,
                                          116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165 };
  unsigned int valid_channels_5_Ghz_40Mhz[]   = { 38,  42,  46,  50,  54,  58,  62,  102, 106, 110, 114,
                                                118, 122, 126, 130, 134, 138, 151, 155, 159, 163 };
  unsigned int valid_channels_4_9_Ghz_20Mhz[] = { 184, 188, 192, 196, 8, 12, 16, 0xff, 0x0 };
  unsigned int valid_channels_4_9_Ghz_10Mhz[] = { 183, 185, 187, 189, 7, 9, 11, 0xff, 0x0 };
  unsigned char enable_40                     = 0;
  unsigned char enable_11j                    = 0;

  //__9117_CODE_START
  char string[50] = { 0 };
  int total;
  //__9117_CODE_END
  /*Creating a Socket*/
  sockfd = socket_creation();
  if (sockfd < 0) {
    printf("Unable to create a socket\n");
    return sockfd;
  }

  memset(&per_params, 0, sizeof(per_params_t));
  if ((argc == 13)
      //__9117_CODE_START
      || (argc == 34)
      //__9117_CODE_END
  ) {

    per_params.enable     = 1;
    per_params.enable_11j = 0;

    tx_pwr = atoi(argv[2]);
    printf("TX PWR is %d\n", tx_pwr);
#if 0
		if (tx_pwr > 63)
		{
			tx_pwr = 63; 
		}
		if (tx_pwr > 31 && tx_pwr < 63)
		{
			tx_pwr += 32;
		}
#endif
    if (tx_pwr >= -15 && tx_pwr <= 127) {
      per_params.power = tx_pwr;
    } else {
      printf("Invalid tx_pwr is given by user . Please enter tx_pwr between -15 to 127 \n");
      exit(0);
    }

    //Rate
    tmp_rate = argv[3];

    if (!strcmp(tmp_rate, "1"))
      per_params.rate = RSI_RATE_1;
    else if (!strcmp(tmp_rate, "2"))
      per_params.rate = RSI_RATE_2;
    else if (!strcmp(tmp_rate, "5.5"))
      per_params.rate = RSI_RATE_5_5;
    else if (!strcmp(tmp_rate, "11"))
      per_params.rate = RSI_RATE_11;
    else if (!strcmp(tmp_rate, "6"))
      per_params.rate = RSI_RATE_6;
    else if (!strcmp(tmp_rate, "9"))
      per_params.rate = RSI_RATE_9;
    else if (!strcmp(tmp_rate, "12"))
      per_params.rate = RSI_RATE_12;
    else if (!strcmp(tmp_rate, "18"))
      per_params.rate = RSI_RATE_18;
    else if (!strcmp(tmp_rate, "24"))
      per_params.rate = RSI_RATE_24;
    else if (!strcmp(tmp_rate, "36"))
      per_params.rate = RSI_RATE_36;
    else if (!strcmp(tmp_rate, "48"))
      per_params.rate = RSI_RATE_48;
    else if (!strcmp(tmp_rate, "54"))
      per_params.rate = RSI_RATE_54;
    else if (!strcasecmp(tmp_rate, "mcs0"))
      per_params.rate = RSI_RATE_MCS0;
    else if (!strcasecmp(tmp_rate, "mcs1"))
      per_params.rate = RSI_RATE_MCS1;
    else if (!strcasecmp(tmp_rate, "mcs2"))
      per_params.rate = RSI_RATE_MCS2;
    else if (!strcasecmp(tmp_rate, "mcs3"))
      per_params.rate = RSI_RATE_MCS3;
    else if (!strcasecmp(tmp_rate, "mcs4"))
      per_params.rate = RSI_RATE_MCS4;
    else if (!strcasecmp(tmp_rate, "mcs5"))
      per_params.rate = RSI_RATE_MCS5;
    else if (!strcasecmp(tmp_rate, "mcs6"))
      per_params.rate = RSI_RATE_MCS6;
    else if (!strcasecmp(tmp_rate, "mcs7"))
      per_params.rate = RSI_RATE_MCS7;
    else {
      printf("Invalid tx_rate is given by user .  Please enter Valid tx_rate\n");
      exit(0);
    }

    //pkt length
    tx_pktlen = atoi(argv[4]);

    per_params.pkt_length = tx_pktlen;

    //mode
    tx_mode = atoi(argv[5]);
    if (tx_mode == 1 || tx_mode == 0) {
      per_params.mode = tx_mode;
    } else {
      per_params.mode = 0;
    }

    chan_number = atoi(argv[6]);

    if ((chan_number >= 36) && (chan_number <= 165)) {
      if (is11brate(per_params.rate)) {
        printf("Invalid Rate in 5G\n");
        return -1;
      }
    }
    rate_flags             = atoi(argv[8]);
    per_params.rate_flags  = rate_flags;
    per_params.per_ch_bw   = (rate_flags >> 2) & (0x07); /* BW Configuration is BIT(2)-BIT(4) 3 bits in rate flags */
    enable_11j             = (rate_flags & BIT(5));
    per_params.enable_11j  = enable_11j;
    per_params.aggr_enable = atoi(argv[9]);
    per_params.aggr_count  = (per_params.pkt_length / PER_AGGR_LIMIT_PER_PKT);
    if ((per_params.pkt_length - (per_params.aggr_count * PER_AGGR_LIMIT_PER_PKT)) > 0) {
      per_params.aggr_count++;
    }
    per_params.no_of_pkts  = atoi(argv[10]);
    per_params.delay       = atoi(argv[11]);
    per_params.ctry_region = atoi(argv[12]);

    if (per_params.ctry_region == 255) {
      /*** Remove Me When Updated in Doc and More regions are added*/
      per_params.ctry_region = 3; /* changing ctry_region to 3  from 127 to make sure same value in PER and End-to-End*/
    } else if (((per_params.ctry_region < 0) || (per_params.ctry_region > 2)) && (per_params.ctry_region != 5)
               && (per_params.ctry_region != 6)) {
      printf("Invalid Country region \n");
      printf("Valid country regions are : 0- FCC(US), 1- ETSI(Europe), 2-JP (japan), 5-WORLD_SAFE, 6-SRRC (china), "
             "255-World\n");
      return -1;
    }

    if (per_params.ctry_region == FCC) {
      if ((chan_number < 1) || (chan_number > 11)) {
        printf("Invalid Channel (%d) selected in  FCC Region\n", chan_number);
        return -1;
      }
    } else if (per_params.ctry_region == ETSI) {
      if ((chan_number < 1) || (chan_number > 13)) {
        printf("Invalid Channel (%d) selected in  ETSI Region\n", chan_number);
        return -1;
      }
    } else if (per_params.ctry_region == TELEC) {
      if ((chan_number < 1) || (chan_number > 14)) {
        printf("Invalid Channel (%d) selected in  JP Region\n", chan_number);
        return -1;
      }
      if (chan_number == 14) {
        if (strcmp(tmp_rate, "1") && strcmp(tmp_rate, "2")) {
          printf("Invalid data rate selected for JP Region in Channel %d\n", chan_number);
          return -1;
        }
      }
    } else if (per_params.ctry_region == WORLD_SAFE) {
      if ((chan_number < 1) || (chan_number > 14)) {
        printf("Invalid Channel (%d) selected in  WORLD_SAFE Region\n", chan_number);
        return -1;
      }
    } else if (per_params.ctry_region == SRRC) {
      if ((chan_number < 1) || (chan_number > 13)) {
        printf("Invalid Channel (%d) selected in  SRRC Region\n", chan_number);
        return -1;
      }
    } else if (per_params.ctry_region == WORLD) {
      if ((chan_number < 1) || (chan_number > 13)) {
        printf("Invalid Channel (%d) selected in  WORLD Region\n", chan_number);
        return -1;
      }
    }

#if 1
    if (tx_pktlen > 1536 && per_params.aggr_enable == 0) {
      printf("Invalid length,Give the length <= 1536 \n");
      exit(0);
    }
    if ((tx_pktlen > 30000) && (per_params.aggr_enable)) {
      printf("Cant aggregate,Give the length <= 30000 \n");
      exit(0);
    }
    if ((per_params.aggr_enable) && !(per_params.rate >= RSI_RATE_MCS0 && per_params.rate <= RSI_RATE_MCS7)) {
      printf("Cant aggregate,Give 11n rate \n");
      exit(0);
    }
#endif

    if (per_params.per_ch_bw == BW_U40 || per_params.per_ch_bw == BW_L40 || per_params.per_ch_bw == BW_F40) {
      enable_40 = 1;
    }
    if (enable_11j == 0) {
      if (chan_number == 0xFF) {
        per_params.channel = chan_number;
        /* Pass 0xFF so as to skip channel programming */
      } else if (chan_number <= 14 && ((per_params.per_ch_bw == BW_20) || enable_40)) {
        per_params.channel = chan_number;
      } else if ((chan_number >= 36 && chan_number <= 165) && (per_params.per_ch_bw == BW_20)) /* For 20Mhz BW */
      {
        for (i = 0; i < 24; i++) {
          if (chan_number == valid_channels_5_Ghz[i]) {
            per_params.channel = chan_number;
            break;
          }
        }
        if (!(per_params.channel == chan_number)) {
          printf("Invalid Channel issued by user for 20Mhz BW\n");
          exit(0);
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
          exit(0);
        }
      } else {
        printf("Invalid parameters for transmit. Please check channel and rate flags\n");
        exit(0);
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
          exit(0);
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
          exit(0);
        }
      } else if (per_params.per_ch_bw == BW_5) {
        printf("5MHz BW is not supported\n");
        exit(0);
      } else {
        printf("Invalid BW Configuration\n");
        exit(0);
      }
    }

    //__9117_CODE_START
    per_params.enable_11ax = atoi(argv[7]);

    if (per_params.enable_11ax) {
      if (!strcasecmp(tmp_rate, "mcs0"))
        per_params.rate = 0;
      else if (!strcasecmp(tmp_rate, "mcs1"))
        per_params.rate = 1;
      else if (!strcasecmp(tmp_rate, "mcs2"))
        per_params.rate = 2;
      else if (!strcasecmp(tmp_rate, "mcs3"))
        per_params.rate = 3;
      else if (!strcasecmp(tmp_rate, "mcs4"))
        per_params.rate = 4;
      else if (!strcasecmp(tmp_rate, "mcs5"))
        per_params.rate = 5;
      else if (!strcasecmp(tmp_rate, "mcs6"))
        per_params.rate = 6;
      else if (!strcasecmp(tmp_rate, "mcs7"))
        per_params.rate = 7;
      else {
        printf("Please enter 11ax supported rate \n");
        exit(0);
      }
      per_params.rate |= BIT(10); //11ax en indication to rate info
    }
    //__9117_CODE_END
    printf("\n--Tx TEST CONFIGURATION--\n\n");
    printf("Tx POWER      : %d\n", atoi(argv[2]));
    printf("Tx RATE       : %s\n", argv[3]);
    printf("PACKET LENGTH : %d\n", per_params.pkt_length);
    if (tx_mode == 1) {
      printf("Tx MODE       : CONTINUOUS\n");
      per_params.pkt_length = 28;
    } else if (tx_mode == 0) {
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
    printf("CTRY_REGION : %d\n", per_params.ctry_region);

    //__9117_CODE_START
    memset(&per_params.heconf, 0, sizeof(per_params.heconf));

    if (per_params.enable_11ax) {
      if (argc == 13) {
        printf("Enter 11ax params \n");
        printf("coding_type nominal_pe UL_DL he_ppdu_type beam_change BW STBC Tx_BF GI_LTF DCM NSTS_MIDAMBLE "
               "spatial_reuse BSS_color HE_SIGA2_RESERVED RU_ALLOCATION N_HELTF_TOT SIGB_DCM SIGB_MCS USER_STA_ID "
               "SIGB_COMPRESSION_FIELD USER_IDX\n");
        fgets(string, sizeof(string), stdin);
        total = sscanf(
          string,
          "%hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hd %hhd %hhd %hhd %hhd %hd %hhd %hhd",
          &per_params.heconf.coding_type,
          &per_params.heconf.nominal_pe,
          &per_params.heconf.UL_DL,
          &per_params.heconf.he_ppdu_type,
          &per_params.heconf.beam_change,
          &per_params.heconf.BW,
          &per_params.heconf.STBC,
          &per_params.heconf.Tx_BF,
          &per_params.heconf.GI_LTF,
          &per_params.heconf.DCM,
          &per_params.heconf.NSTS_MIDAMBLE,
          &per_params.heconf.spatial_reuse,
          &per_params.heconf.BSS_color,
          &per_params.heconf.HE_SIGA2_RESERVED,
          &per_params.heconf.RU_ALLOCATION,
          &per_params.heconf.N_HELTF_TOT,
          &per_params.heconf.SIGB_DCM,
          &per_params.heconf.SIGB_MCS,
          &per_params.heconf.USER_STA_ID,
          &per_params.heconf.SIGB_COMPRESSION_FIELD,
          &per_params.heconf.USER_IDX);
        if (total != 21) {
          printf("Invalid No of parameters \n");
          exit(0);
        }
      } else if (argc == 34) {
        per_params.heconf.coding_type            = atoi(argv[13]);
        per_params.heconf.nominal_pe             = atoi(argv[14]);
        per_params.heconf.UL_DL                  = atoi(argv[15]);
        per_params.heconf.he_ppdu_type           = atoi(argv[16]);
        per_params.heconf.beam_change            = atoi(argv[17]);
        per_params.heconf.BW                     = atoi(argv[18]);
        per_params.heconf.STBC                   = atoi(argv[19]);
        per_params.heconf.Tx_BF                  = atoi(argv[20]);
        per_params.heconf.GI_LTF                 = atoi(argv[21]);
        per_params.heconf.DCM                    = atoi(argv[22]);
        per_params.heconf.NSTS_MIDAMBLE          = atoi(argv[23]);
        per_params.heconf.spatial_reuse          = atoi(argv[24]);
        per_params.heconf.BSS_color              = atoi(argv[25]);
        per_params.heconf.HE_SIGA2_RESERVED      = atoi(argv[26]);
        per_params.heconf.RU_ALLOCATION          = atoi(argv[27]);
        per_params.heconf.N_HELTF_TOT            = atoi(argv[28]);
        per_params.heconf.SIGB_DCM               = atoi(argv[29]);
        per_params.heconf.SIGB_MCS               = atoi(argv[30]);
        per_params.heconf.USER_STA_ID            = atoi(argv[31]);
        per_params.heconf.SIGB_COMPRESSION_FIELD = atoi(argv[32]);
        per_params.heconf.USER_IDX               = atoi(argv[33]);
      }
      if (per_params.heconf.coding_type != 0 && per_params.heconf.coding_type != 1) {
        printf("Invalid option for coding_type\n 0-BCC \n 1-LDPC \n");
        exit(0);
      }
      if (per_params.heconf.nominal_pe < 0 || per_params.heconf.nominal_pe > 2) {
        printf("Invalid option for nominal_pe\n 0-0us \n 1-8us \n 2-16us \n");
        exit(0);
      }
      if (per_params.heconf.BW != 0 && per_params.heconf.BW != 1) {
        printf("Invalid option for BW\n For ER PPDU 0 - 242-tone RU \n 1 - upper 106-tone RU \n");
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

    cmd = (unsigned short)PER_TRANSMIT;
    //__9117_CODE_START
    /* check L-sig length is proper for 11ax parameters*/
    if (per_params.enable_11ax) {
      if (validate_11ax_lsig_len(&per_params)) {
        printf("Length is beyond supported length for these 11ax configuration.\n");
        exit(0);
      }
    }
    //__9117_CODE_END

    if (per_transmit_wrapper(per_params, cmd, sockfd) < 0) {
      printf("Sending Failed\n");
      printf("Transmit already running or Driver not installed properly\n");
    } else
      printf("Tx Started\n");
  }

  else if (argc == 3) {
    if (!(strcmp(argv[2], "1") && strcmp(argv[2], "0"))) {
      per_params.enable = 0;
      cmd               = (unsigned short)PER_TRANSMIT;

      if (per_transmit_wrapper(per_params, cmd, sockfd) < 0) {
        perror(argv[0]);
        printf("&&Please ensure Burst or Continuous Mode is running\n");
      } else {
        printf("Tx Stopped\n");
      }
    } else {
      printf("Please enter either 0 or 1 as an argument, instead of %s to stop..\n", argv[2]);
    }
  }

  else {
    //__9117_CODE_START
    printf("\nUSAGE to start transmit: %s rpine_interface tx_power rate length tx_mode channel 11ax-Enable Rate_flags "
           "Aggr_enable no_of_packets delay ctry_region\n",
           argv[0]);
    printf("\nUSAGE to start transmit in 11ax mode: %s rpine_interface tx_power rate length tx_mode channel "
           "11ax-Enable Rate_flags Aggr_enable no_of_packets delay ctry_region coding_type nominal_pe UL_DL "
           "he_ppdu_type beam_change BW STBC Tx_BF GI_LTF DCM NSTS_MIDAMBLE spatial_reuse BSS_color HE_SIGA2_RESERVED "
           "RU_ALLOCATION N_HELTF_TOT SIGB_DCM SIGB_MCS USER_STA_ID SIGB_COMPRESSION_FIELD USER_IDX\n",
           argv[0]);
    //__9117_CODE_END
    printf("\nUSAGE to start transmit: %s rpine_interface tx_power rate length tx_mode channel ExtPA-Enable Rate_flags "
           "Aggr_enable no_of_packets delay ctry_region\n",
           argv[0]);
    printf("\nUSAGE to stop transmit: %s rpine_interface tx_mode\n\t****** FIELDS *******", argv[0]);
    printf("\ntx_mode : 0 - Burst , 1 - Continuous mode\n");
    printf("\nTX-Power 127 to use max Power in Flash\n");
    printf("\nRate_flags Bits: \n");
    printf("Bit 0		: (Short_GI for HT mode)/(Short_preamble in 11b)\n");
    printf("Bit 1		: (GreenField for HT mode)/(preamble enable for 11b)\n");
    printf("Bit 4-2		: CH_BW flags\n");
    printf("Bit 5		: This bit has to be set when the user selects 11J channel\n\n");
    printf("Bit 15-6	: Reserved\n\n");
    printf("CTRY_REGION : 0- FCC(US), 1- ETSI(Europe), 2-JP (japan), 255-World\n");

#ifdef MODE_11AH
    printf("\n\t11AH\nRate Flags = 8 for 2MHz \n Rate Flags = 12 for 4MHz  	 \n\n");

#endif
    //__9117_CODE_START
    printf("If 11ax enabled\n");
    printf("\nCODING_TYPE: ");
    printf("\nIndicates whether BCC or LDPC to be used");
    printf("\n0 - BCC");
    printf("\n1 - LDPC");
    printf("\nNominal PE: ");
    printf("\nIndicates Nominal T-PE value");
    printf("\n0 - 0us");
    printf("\n1 - 8us");
    printf("\n2 - 16us");
    printf("\nSpatial Reuse Enable  1 Uses spatial reuse information that's given peer notify");
    printf("\nHE_PPDU_TYPE: ");
    printf("\n0 - HE SU PPDU");
    printf("\n1 - HE ER SU PPDU");
    printf("\n2 - HE TB PPDU");
    printf("\n3 - HE MU PPDU");
    printf("\nBeam Change: ");
    printf("\nIndicates the spatial mapping of pre-HE and HE fields");
    printf("\n0 - pre-HE and HE fields are spatially mapped in the same way");
    printf("\n1 - pre-HE and HE fields are spatially mapped differently");
    printf("\nUL_DL: ");
    printf("\nIndicates whether the PPDU is UL/DL");
    printf("\nSet to 1,if PPDU is Sent by Station to AP");
    printf("\nSet to 0,if PPDU is sent by AP");
    printf("\nSTBC: ");
    printf("\nIndicates whether STBC is used for PPDU transmission");
    printf("\n0 - no STBC");
    printf("\n1 - STBC only if DCM field is set to 0");
    printf("\nTxBF: ");
    printf("\nIndicates whether beamforming matrix is applied to the transmission");
    printf("\n0 - No beamforming matrix");
    printf("\n1 - beamformaing matrix.");
    printf("\nSet to 0 by default.");
    printf("\nGI_LTF: ");
    printf("\nIndicates the GI and LTF size");
    printf("\nSU/ER_SU:");
    printf("\n0 - 1x HE-LTF and 0.8us GI");
    printf("\n1 - 2x HE-LTF and 0.8us GI");
    printf("\n2 - 2x HE-LTF and 1.6us GI");
    printf("\n3 - 4x HE-LTF and 0.8us GI if both DCM and STBC fields are 1, else 4x HE-LTF and 3.2us GI");
    printf("\nMU:");
    printf("\n0 - 4x HE-LTF and 0.8us GI");
    printf("\n1 - 2x HE-LTF and 0.8us GI");
    printf("\n2 - 2x HE-LTF and 1.6us GI");
    printf("\n3 - 4x HE-LTF and 3.2us GI");
    printf("\nTB:");
    printf("\n0 - 1x HE-LTF and 1.6us GI");
    printf("\n1 - 2x HE-LTF and 1.6us GI");
    printf("\n2 - 4x HE-LTF and 3.2us GI");
    printf("\n3 - Reserved");
    printf("\nDCM: ");
    printf("\nIndicates whether DCM is applied to Data Symbols");
    printf("\n0 - no DCM");
    printf("\n1 - DCM");
    printf("\nNSTS_MIDAMBLE: ");
    printf("\nIndicates the Nsts and Midamble Periodicity");
    printf("\nIf Doppler is set to 0");
    printf("\n      BIT[15:13] = Number of space-time streams-1");
    printf("\nIf Doppler is set to 1");
    printf("\n     BIT[14:13] = = Number of space-time streams-1");
    printf("\n     BIT[15] = 0 for MIDAMBLE Periodicity of 10");
    printf("\n             = 1 for MIDAMBLE Periodicity of 20");
    printf("\nDefault Doppler value is 0.");
    printf("\nSpatial Reuse 4 Indicates whether spatial reuse is allowed during the transmission of the PPDU");
    printf("\nBSS color: ");
    printf("\n");
    printf("HE_SIGA2_RESERVED: The value should be between 0 to 511\n");
    printf("RU_ALLOCATION: The value should be between 0 to 255\n");
    printf("N_HELTF_TOT: The vale should be between 0 to 7\n");
    printf("SIGB_DCM: The value should be 0 to Enable and 1 to Disable\n");
    printf("SIGB_MCS: The value should be between 0 to 5\n");
    printf("USER_STA_ID: The value should be between 0 to 2047\n");
    printf("SIGB_COMPRESSION_FIELD: The value should be 0 or 1\n");
    printf("USER_IDX: The value should be between 0 to 8\n");
    printf("USAGE to start transmit: %s rpine_interface tx_power rate length tx_mode channel 11ax-Enable Rate_flags "
           "Aggr_enable no_of_packets delay ctry_region\n\n",
           argv[0]);
    printf("USAGE to start transmit in 11ax mode: %s rpine_interface tx_power rate length tx_mode channel 11ax-Enable "
           "Rate_flags Aggr_enable \nno_of_packets delay ctry_region coding_type nominal_pe UL_DL he_ppdu_type "
           "beam_change BW STBC Tx_BF GI_LTF DCM NSTS_MIDAMBLE \nspatial_reuse BSS_color HE_SIGA2_RESERVED "
           "RU_ALLOCATION N_HELTF_TOT SIGB_DCM SIGB_MCS USER_STA_ID SIGB_COMPRESSION_FIELD USER_IDX\n\n",
           argv[0]);
    //__9117_CODE_END
    return 0;
  }

  return 0;
}

//__9117_CODE_START
#define T_RL_SIG_10X          (40)
#define T_HE_SIG_A_10X        (80)
#define T_HE_SIG_A_R_10X      (160)
#define T_HE_STF_T_10X        (80)
#define T_HE_STF_NT_10X       (40)
#define T_DFT_HE_10X          (128)
#define T_LEGACY_PREAMBLE_10X (200)
#define T_HE_SIG_B_10X        (40)

#define T_RL_SIG          4
#define T_HE_SIG_A        8
#define T_HE_SIG_A_R      16
#define T_HE_STF_T        8
#define T_HE_STF_NT       4
#define T_DFT_HE          12.8 //! DFT duration in Data symbol.
#define T_LEGACY_PREAMBLE (20)

#define N_HE_LTF  1
#define M_STBC    1
#define N_TAIL    6
#define N_SERVICE 16

//! Coding type
#define BCC_CODING  0
#define LDPC_CODING 1

//! PPDU typpe
#define HE_SU_PPDU    0
#define HE_ER_SU_PPDU 1
#define HE_TB_PPDU    2
#define HE_MU_PPDU    3
#define NOMINAL_PE    (((rate & 0x60) >> 5) << 3)
#define LDPC_ENABLE   ((rate & 0x200) >> 9)
#define GUARD_INTVL   ((rate & 0x1800) >> 11)
#define DCM_ENABLE    ((rate & 0x2000) >> 13)
//! HE TB and HE MU parameters frame format
typedef struct he_ppdu_per_params {
  union tb_ppdu_params {
    struct tb_ppdu_fields {
      unsigned int spatial_reuse_2 : 4;
      unsigned int spatial_reuse_3 : 4;
      unsigned int spatial_reuse_4 : 4;
      unsigned int n_heltf_tot : 3;
      unsigned int ru_allocation : 8;
      unsigned int he_siga2_reserved : 9;
    } fields;
    unsigned int params;
  } tb_ppdu;
  union mu_ppdu_params {
    struct mu_ppdu_fields {
      unsigned int sigb_mcs : 3;
      unsigned int sigb_dcm : 1;
      unsigned int he_ltf_sym : 3;
      unsigned int sigb_comp_field : 1;
      unsigned int reserved : 24;
      unsigned int ru_allocation;
      unsigned int reserved1 : 1;
      unsigned int user_sta_id : 11;
      unsigned int user_nsts : 3;
      unsigned int user_txbf : 1;
      unsigned int user_mcs : 4;
      unsigned int user_dcm : 1;
      unsigned int user_coding_type : 1;
      unsigned int no_of_user_fields : 4;
      unsigned int user_idx : 6;
    } fields;
    struct mu_params {
      unsigned int params[3];
    } mu_params;
  } mu_ppdu;
} he_ppdu_per_params_t;

const unsigned char mcs_mf[8]                             = { 1, 2, 3, 4, 6, 8, 9, 10 };
const unsigned short int ru_mf_ndbps[4]                   = { 12, 24, 51, 117 };
const unsigned short int ru_mf_ndbps_short[4]             = { 3, 6, 12, 30 };
const unsigned short int Ndbps_short_dcm_mf_fix[2][5]     = { { 1, 3, 6, 15, 0 }, { 1, 2, 0, 4, 6 } };
const unsigned short int he_ltf_symbol_gi_time_10x[12][2] = { { 40, 8 },  { 72, 8 },  { 80, 16 },  { 160, 32 },
                                                              { 48, 16 }, { 80, 16 }, { 160, 32 }, { 0, 0 },
                                                              { 136, 8 }, { 72, 8 },  { 80, 16 },  { 160, 32 } };
const unsigned char mcs_dcm_rfactor[2][5]                 = { { 1, 1, 0, 1, 3 }, { 2, 2, 0, 2, 4 } };
const unsigned char mcs_rfactor[2][8]                     = { { 1, 1, 3, 1, 3, 2, 3, 5 }, { 2, 2, 4, 2, 4, 3, 4, 6 } };
const unsigned int ru_allocation_to_ru_width[29][2]       = {
  { 0x00000000, 9 }, { 0x10000000, 8 }, { 0x00100000, 8 }, { 0x01100000, 7 },

  { 0x00000100, 8 }, { 0x01000100, 7 }, { 0x00010100, 7 }, { 0x00110100, 6 },

  { 0x00000001, 8 }, { 0x01000001, 7 }, { 0x00010001, 7 }, { 0x00110001, 6 },

  { 0x00000011, 7 }, { 0x00100011, 6 }, { 0x00001011, 6 }, { 0x00011011, 5 },

  { 0x211, 3 },      { 0x112, 3 },      { 0x200000, 6 },   { 0x20100, 5 },    { 0x20001, 5 }, { 0x2011, 4 }, { 0x2, 6 },
  { 0x10002, 5 },    { 0x102, 5 },      { 0x1102, 4 },     { 0x22, 2 },       { 0x22, 2 },    { 0x1111, 4 },
};
const unsigned short int bits_per_symbol[8][2] = {
  //! 20MHz 40MHz
  { 26, 54 },   /*  0: BPSK */
  { 52, 108 },  /*  1: QPSK 1/2 */
  { 78, 162 },  /*  2: QPSK 3/4 */
  { 104, 216 }, /*  3: 16-QAM 1/2 */
  { 156, 324 }, /*  4: 16-QAM 3/4 */
  { 208, 432 }, /*  5: 64-QAM 2/3 */
  { 234, 486 }, /*  6: 64-QAM 3/4 */
  { 260, 540 }, /*  7: 64-QAM 5/6 */
};

unsigned int get_nsym_he_sig_b(unsigned int sigb_comp_field, unsigned int noof_users, unsigned int dcm_mcs_indx)
{
  unsigned int noof_bits_common, noof_bits_user, total_bits, n_dbps, n_sym;
  unsigned int mcs_inx    = (dcm_mcs_indx & 0x7);
  unsigned int dcm_enable = ((dcm_mcs_indx & 0x8) >> 3);

  //! Get total number of bits in SIGB
  if (sigb_comp_field) {
    noof_bits_common = 0;
  } else {
    noof_bits_common = 18;
  }

  noof_bits_user = 21 * noof_users + 10 * ((noof_users >> 1) + (noof_users & BIT(0)));

  total_bits = noof_bits_common + noof_bits_user;

  //! Determine number of symbols in SIGB
  n_dbps = (bits_per_symbol[mcs_inx][0]) >> dcm_enable;

  n_sym = total_bits / n_dbps;

  if (total_bits % n_dbps) {
    n_sym++;
  }

  return n_sym;
}
unsigned int ldpc_extra_sym_check(unsigned int mcs_indx,
                                  unsigned int n_dbps_short,
                                  unsigned int n_dbps,
                                  unsigned int a,
                                  unsigned int n_sym,
                                  unsigned int dcm_enable)
{
#define RD_LCM 12
  unsigned int rn, rd, n_dbps_last_init, Npld, n_cbps, n_cbps_short, n_cbps_last_init;
  unsigned int Navbits, Ncw, Lldpc, Lldpc_Nshrt = 0, Nshrt, Npunct, ldpc_extra_sym;

  //! Ndbps_last_init = aint * Ndbps_short
  n_dbps_last_init = a * n_dbps_short;

  //! Npld = (Nsym_init-m_stbc)*Ndbps + m_stbc*Ndbps_last_init;
  Npld = (n_sym - M_STBC) * n_dbps + (M_STBC * n_dbps_last_init);

  //! Ncbps = Ndbps/Rfacotr of mcs
  //! Ncbps_short = Ndbps_short/Rfacotr of mcs
  if (dcm_enable) {
    rn           = mcs_dcm_rfactor[0][mcs_indx];
    rd           = mcs_dcm_rfactor[1][mcs_indx];
    n_cbps       = n_dbps * rd / rn;
    n_cbps_short = n_dbps_short * rd / rn;
  } else {
    rn           = mcs_rfactor[0][mcs_indx];
    rd           = mcs_rfactor[1][mcs_indx];
    n_cbps       = n_dbps * rd / rn;
    n_cbps_short = n_dbps_short * rd / rn;
  }

  //! Ncbps_last_init = if (a =4) ncbps, else Ainit*Ncbps_short
  if (a == 4) {
    n_cbps_last_init = n_cbps;
  } else {
    n_cbps_last_init = a * n_cbps_short;
  }

  //! Navbits  = (Nsym_init-m_stbc)*Ncbps + m_stbc*Ncbps_last_init;
  Navbits = (n_sym - M_STBC) * n_cbps + M_STBC * n_cbps_last_init;

  if (Navbits <= 648) {
    Ncw = 1;
    if ((RD_LCM * Navbits * rd) >= ((RD_LCM * Npld * rd) + (912 * RD_LCM * (rd - rn)))) {
      Lldpc = 1296;
    } else {
      Lldpc = 648;
    }
  } else if ((648 < Navbits) & (Navbits <= 1296)) {
    Ncw = 1;
    if ((RD_LCM * Navbits * rd) >= ((RD_LCM * Npld * rd) + (1464 * RD_LCM * (rd - rn)))) {
      Lldpc = 1944;
    } else {
      Lldpc = 1296;
    }
  } else if ((1296 < Navbits) & (Navbits <= 1944)) {
    Ncw   = 1;
    Lldpc = 1944;
  } else if ((1944 < Navbits) & (Navbits <= 2592)) {
    Ncw = 2;
    if ((RD_LCM * Navbits * rd) >= ((RD_LCM * Npld * rd) + (2916 * RD_LCM * (rd - rn)))) {
      Lldpc = 1944;
    } else {
      Lldpc = 1296;
    }
  } else {
    Ncw = Npld / (1944 * rn / rd);
    if (Npld % (1944 * rn / rd)) {
      Ncw++;
    }
    Lldpc = 1944;
  }

  switch (mcs_indx) {
    case 0:
    case 1:
    case 3:
      Lldpc_Nshrt = Lldpc / 2;
      break;
    case 5:
      Lldpc_Nshrt = (Lldpc / 3) * 2;
      break;
    case 2:
    case 4:
    case 6:
    case 8:
    case 10:
      Lldpc_Nshrt = (Lldpc / 4) * 3;
      break;
    case 7:
    case 9:
    case 11:
      Lldpc_Nshrt = (Lldpc / 6) * 5;
      break;
  }

  if ((Ncw * Lldpc_Nshrt) <= Npld) {
    Nshrt = 0;
  } else {
    Nshrt = (Ncw * Lldpc_Nshrt) - Npld;
  }

  if ((Ncw * Lldpc) <= (Navbits + Nshrt)) {
    Npunct = 0;
  } else {
    Npunct = (Ncw * Lldpc) - Navbits - Nshrt;
  }

  if ((((Npunct * 10 * rd) > (Ncw * Lldpc * (rd - rn))) & ((Nshrt * 10 * (rd - rn)) < (12 * Npunct * rn)))
      | ((Npunct * 10 * rd) > (3 * Ncw * Lldpc * (rd - rn)))) {
    ldpc_extra_sym = 1;
  } else {
    ldpc_extra_sym = 0;
  }

  return ldpc_extra_sym;
}
/* extra_symbol_afactor:
 *    a factor in         [1:0]
 *    PE disambiguity/LDPC Extra symbol in  BIT(2)
 *    SIGB_SYM            [6:3]
 *    no_of_users_field   [10:7]
 *    t_pe						    [13:11]
 */
unsigned int get_11ax_pkt_duration(unsigned short int rate,
                                   unsigned short int length,
                                   unsigned int bbp_info,
                                   unsigned int *extra_symbol_afactor,
                                   he_ppdu_per_params_t *he_ppdu_per_params_p)
{
  unsigned int a, n_dbps, n_dbps_short, n_sym = 0, n_excess, t_pe, t_he_ltf_sym_10x;
  unsigned int tx_time_10x, t_gi_10x = 0, t_hepreamble_10x = 0, t_sym_10x, rem;
  unsigned int fec_coding      = LDPC_ENABLE;
  unsigned int nominal_pkt_pad = NOMINAL_PE;
  unsigned int ppdu_type       = ((bbp_info & 0x30) >> 4);
  unsigned int stbc_enable     = ((bbp_info & 0x100) >> 8);
  unsigned int dcm_enable      = DCM_ENABLE;
  unsigned int gi_ltf          = GUARD_INTVL;
  unsigned int ru_width        = 0, ru_alloc, n_he_ltf;
  unsigned int mcs_indx        = (rate & 0xF);
  unsigned int n_he_sig_b = 0, ldpc_extra_sym = 0;
  unsigned int sigb_comp_field, user_indx, no_of_users, indx = 0;

  switch (ppdu_type) {
    case HE_SU_PPDU:
    case HE_ER_SU_PPDU:
      //! Get the RU Width.
      if ((ppdu_type == HE_ER_SU_PPDU) && (bbp_info & BIT(7))) {
        ru_width = 2; //! 106 tone RU
      } else {
        ru_width = 3; //! 242 tone RU
      }

      //! Get HE-LTF GI and LTF duration
      if (dcm_enable && stbc_enable) {
        t_he_ltf_sym_10x = 136;
        t_gi_10x         = 8;
      } else {
        t_he_ltf_sym_10x = he_ltf_symbol_gi_time_10x[gi_ltf][0];
        t_gi_10x         = he_ltf_symbol_gi_time_10x[gi_ltf][1];
      }

      /* Thepreamble varies based on PPDU type, symbol duration and guard interval.
       *    For SU,    Trl_sig + The_sig_a   + The_stf_nt + Nhe_ltf * THe_ltf_sym 
       *    For HE-SU, Trl_sig + The_sig_a_r + The_stf_nt + Nhe_ltf * THe_ltf_sym
       */
      if (ppdu_type == HE_SU_PPDU) {
        t_hepreamble_10x = T_RL_SIG_10X + T_HE_SIG_A_10X + T_HE_STF_NT_10X + N_HE_LTF * t_he_ltf_sym_10x;
      } else {
        t_hepreamble_10x = T_RL_SIG_10X + T_HE_SIG_A_R_10X + T_HE_STF_NT_10X + N_HE_LTF * t_he_ltf_sym_10x;
      }
      break;
    case HE_TB_PPDU:
      //! Get the RU Width.
      ru_alloc = ((he_ppdu_per_params_p->tb_ppdu.params & 0x7F8000) >> 15); //! Bits [22:15]
      if (ru_alloc <= 36) {
        ru_width = 0; //! 26 tone RU
      } else if (ru_alloc <= 52) {
        ru_width = 1; //! 52 tone RU
      } else if (ru_alloc <= 60) {
        ru_width = 2; //! 106 tone RU
      } else if (ru_alloc <= 64) {
        ru_width = 3; //! 242 tone RU
      }

      //! Get HE-LTF GI and LTF duration
      t_he_ltf_sym_10x = he_ltf_symbol_gi_time_10x[gi_ltf + 4][0];
      t_gi_10x         = he_ltf_symbol_gi_time_10x[gi_ltf + 4][1];

      /* Thepreamble varies based on PPDU type, symbol duration and guard interval.
       *    For TB,    Trl_sig + The_sig_a   + The_stf_t  + Nhe_ltf * THe_ltf_sym
       */
      n_he_ltf         = ((he_ppdu_per_params_p->tb_ppdu.params & 0x7000) >> 12); //! N_HE_LTF
      t_hepreamble_10x = T_RL_SIG_10X + T_HE_SIG_A_10X + T_HE_STF_T_10X + (n_he_ltf + 1) * t_he_ltf_sym_10x;
      break;
    case HE_MU_PPDU:
      //! Get the RU Width.
      ru_alloc  = (he_ppdu_per_params_p->mu_ppdu.mu_params.params[1] & 0xFF); //! RU ALLOCATION
      user_indx = (he_ppdu_per_params_p->mu_ppdu.mu_params.params[2] >> 24);  //! user_indx
      if (ru_alloc == 0) {
        ru_width    = 0;
        indx        = 0;
        no_of_users = ru_allocation_to_ru_width[indx][1];
      } else if (ru_alloc < 16) {
        indx        = ru_alloc;
        ru_width    = (((ru_allocation_to_ru_width[indx][0]) >> (user_indx << 2)) & 0xF);
        no_of_users = ru_allocation_to_ru_width[indx][1];
      } else if (ru_alloc <= 112) {
        indx        = ((ru_alloc >> 3) + 14);
        ru_width    = (((ru_allocation_to_ru_width[indx][0]) >> (user_indx << 2)) & 0xF);
        no_of_users = ru_allocation_to_ru_width[indx][1];
      } else if ((ru_alloc & 0xC0) == 0x80) {
        if (user_indx == 1) {
          ru_width = 0;
        } else {
          ru_width = 2;
        }
        no_of_users = 3;
      } else if ((ru_alloc & 0xC0) == 0xC0) {
        ru_width    = 3;
        no_of_users = 1;
      } else {
        while (1)
          ;
      }

      //! Get HE-LTF GI and LTF duration
      t_he_ltf_sym_10x = he_ltf_symbol_gi_time_10x[gi_ltf + 8][0];
      t_gi_10x         = he_ltf_symbol_gi_time_10x[gi_ltf + 8][1];

      //! Calculate N_HE_SIG_B
      sigb_comp_field = (he_ppdu_per_params_p->mu_ppdu.mu_params.params[0] & 0x80) >> 7;
      n_he_sig_b =
        get_nsym_he_sig_b(sigb_comp_field, no_of_users, ((he_ppdu_per_params_p->mu_ppdu.mu_params.params[0]) & 0xF));

      //! Fill in SIGB_SYM
      if (sigb_comp_field == 0) {
        if (n_he_sig_b > 15) {
          *extra_symbol_afactor |= (15 << 3);
        } else {
          *extra_symbol_afactor |= ((n_he_sig_b - 1) << 3);
        }
      } else {
        *extra_symbol_afactor |= ((no_of_users - 1) << 3);
      }

      //! Fill in no_of_users_field
      *extra_symbol_afactor |= ((no_of_users - 1) << 7);

      /* Thepreamble varies based on PPDU type, symbol duration and guard interval.
       *    For MU,    Trl_sig + The_sig_a   + The_stf_nt + Nhe_ltf * THe_ltf_sym + Nhe_sig_b* The_sig_b
       */
      t_hepreamble_10x =
        T_RL_SIG_10X + T_HE_SIG_A_10X + T_HE_STF_NT_10X + N_HE_LTF * t_he_ltf_sym_10x + n_he_sig_b * T_HE_SIG_B_10X;
      break;
  }

  //! Tsym is the symbol time.
  t_sym_10x = t_gi_10x + T_DFT_HE_10X;

  /* Ndbps calculation is simpilfied and valid for MCS0-7 20MHz band width.
   *    Ndbps = (RUmf_ndbps *MCSmf) >>DCM_enable
   */
  n_dbps = (ru_mf_ndbps[ru_width] * mcs_mf[mcs_indx]) >> dcm_enable;

  //! Nexcess = mod(8*APEP_LENGTH+Ntail+Nservice, mstbc*Ndbps)
  n_excess = ((length << 3) + N_TAIL + N_SERVICE) % (M_STBC * n_dbps);

  /* For HE SU & HE ER SU for BCC coding,
   *    Nsym = Nsym,init = mstbc * celling((8*APEP_LENGTH + Ttail +Tservice)/mstbc *Ndbps)
   */
  n_sym = ((length << 3) + N_TAIL + N_SERVICE) / (M_STBC * n_dbps);
  if (n_excess) {
    n_sym++;
  }
  n_sym = M_STBC * n_sym;

  /* Ndbps,short = if DCM enabled, Ndbps_short_dcm_mf[0][i] * Ndbps_short_dcm_mf[1][i]
   *               else, MCSmf*RUmf_ndps_short
   */
  if (dcm_enable) {
    n_dbps_short = Ndbps_short_dcm_mf_fix[0][ru_width] * Ndbps_short_dcm_mf_fix[1][mcs_indx];
  } else {
    n_dbps_short = mcs_mf[mcs_indx] * ru_mf_ndbps_short[ru_width];
  }

  /* a factor, PRE-FEC padding factor, 
   *    For SU and ER SU, 
   *      a = ainit = 4 if Nexcess = 0 else
   *                  min((Nexcess/(mstbc*Ndbps,short)), 4)
   */
  if (n_excess == 0) {
    a = 4;
  } else {
    a = n_excess / (M_STBC * n_dbps_short);
    if (n_excess % (M_STBC * n_dbps_short)) {
      a++;
    }
    if (a > 4) {
      a = 4;
    }
  }

  if (fec_coding == LDPC_CODING) {
    ldpc_extra_sym = ldpc_extra_sym_check(mcs_indx, n_dbps_short, n_dbps, a, n_sym, dcm_enable);
    if (ldpc_extra_sym) {
      *extra_symbol_afactor |= BIT(2);
      if (a == 4) {
        n_sym += M_STBC;
        a = 1;
      } else {
        a += 1;
      }
    }
  }
  if (a != 4) {
    *extra_symbol_afactor |= a;
  }

  /* Tpe is dependent on "a" factor and  NOMINAL_PACKET_PADDING
   *    = NOMINAL_PACKET_PADDING - (4*(4-a)). If value is -ve, we limit this value to 0.
   */
#if 0
  if ((ru_width <= 1) || ((ru_width == 2) && (!(dcm_enable)))) {
    t_pe = 0;
  } else
#endif
  {
    t_pe = nominal_pkt_pad - ((4 - a) << 2);
    if (t_pe & BIT(31)) {
      t_pe = 0;
    }
  }

  /* Total on air time of the packet, 
   * TXTIME = 20 + Thepreamble + Nsym*Tsym + Nma*Nhe_ltf*The_ltf +Tpe + SignalExtention.
   *
   * Ttail = 6, Tservice =16, mstbc =1 if STBC is disabled, else 2. APEP_LENGTH is length in bytes.
   *
   * Nma, no.of midambles, is 0 as miable is not supported currently.
   */
  tx_time_10x = (T_LEGACY_PREAMBLE_10X + t_hepreamble_10x + (t_sym_10x * n_sym) + (t_pe * 10));

  //! calculate pe_disambiguity
  rem = (tx_time_10x - T_LEGACY_PREAMBLE_10X) % 40;
  if (rem == 0) { //! changing value to 40 so that calculation and checks will be less when calculating PE disambiguity.
    rem = 40;
  }

  if (((t_pe * 10) + 40 - rem) >= t_sym_10x) {
    *extra_symbol_afactor |= BIT(2);
  }
  *extra_symbol_afactor |= ((t_pe >> 2) << 11);
  return tx_time_10x;
}
unsigned int heppdu_spoof_length_calculation(unsigned short int rate,
                                             unsigned short int length,
                                             unsigned int bbp_info,
                                             unsigned int *extra_symbol_afactor,
                                             he_ppdu_per_params_t *he_ppdu_per_params_p)
{
  unsigned int x, m, tx_time_10x;
  unsigned short int l_sig_length;
  unsigned int ppdu_type = ((bbp_info & 0x30) >> 4);
  tx_time_10x            = get_11ax_pkt_duration(rate, length, bbp_info, extra_symbol_afactor, he_ppdu_per_params_p);

  //!m
  if (ppdu_type & BIT(0)) {
    m = 1;
  } else {
    m = 2;
  }

  /* L-SIG Length = celling((TXTIME-SignalExtention -20)/4)*3-3-m
   * SignalExtention = 0, 
   * m = 1 for SU, TB and 2 for ER SU, MU
   */
  x = (tx_time_10x - T_LEGACY_PREAMBLE_10X) / 40;
  if ((tx_time_10x - T_LEGACY_PREAMBLE_10X) % 40) {
    x++;
  }

  l_sig_length = (x << 1) + x - 3 - m;
  return l_sig_length;
}
unsigned int validate_11ax_lsig_len(per_params_t *per_params)
{
  unsigned int spoof_len = 0xffff, extra_symbol_afactor = 0, bbp_info;
  unsigned short int rate, length;
  he_ppdu_per_params_t he_ppdu_per_params;

  //! initialize parameters from per_params structure.
  memset(&he_ppdu_per_params, 0, sizeof(he_ppdu_per_params_t));
  rate = per_params->rate | (per_params->heconf.nominal_pe << 5) | (per_params->heconf.coding_type << 9)
         | (per_params->heconf.GI_LTF << 11) | (per_params->heconf.DCM << 13);
#define CRC_LENGTH    4
#define AGGR_DELIM_SZ 4
  length = per_params->pkt_length + per_params->aggr_count * (CRC_LENGTH + AGGR_DELIM_SZ);

  bbp_info = (per_params->heconf.he_ppdu_type << 4) | (per_params->heconf.UL_DL << 3)
             | (per_params->heconf.beam_change << 6) | (per_params->heconf.BW << 7) | (per_params->heconf.STBC << 8)
             | (per_params->heconf.Tx_BF << 9) | (per_params->heconf.NSTS_MIDAMBLE << 13);

  if (per_params->heconf.he_ppdu_type == HE_TB_PPDU) {
    he_ppdu_per_params.tb_ppdu.fields.he_siga2_reserved = per_params->heconf.HE_SIGA2_RESERVED;
    he_ppdu_per_params.tb_ppdu.fields.ru_allocation     = per_params->heconf.RU_ALLOCATION;
    he_ppdu_per_params.tb_ppdu.fields.n_heltf_tot       = per_params->heconf.N_HELTF_TOT;
    he_ppdu_per_params.tb_ppdu.fields.spatial_reuse_4   = per_params->heconf.spatial_reuse;
    he_ppdu_per_params.tb_ppdu.fields.spatial_reuse_3   = per_params->heconf.spatial_reuse;
    he_ppdu_per_params.tb_ppdu.fields.spatial_reuse_2   = per_params->heconf.spatial_reuse;
  } else if (per_params->heconf.he_ppdu_type == HE_MU_PPDU) {
    he_ppdu_per_params.mu_ppdu.fields.he_ltf_sym       = 0;
    he_ppdu_per_params.mu_ppdu.fields.sigb_comp_field  = per_params->heconf.SIGB_COMPRESSION_FIELD;
    he_ppdu_per_params.mu_ppdu.fields.sigb_dcm         = per_params->heconf.SIGB_DCM;
    he_ppdu_per_params.mu_ppdu.fields.sigb_mcs         = per_params->heconf.SIGB_MCS;
    he_ppdu_per_params.mu_ppdu.fields.ru_allocation    = per_params->heconf.RU_ALLOCATION;
    he_ppdu_per_params.mu_ppdu.fields.user_coding_type = per_params->heconf.coding_type;
    he_ppdu_per_params.mu_ppdu.fields.user_dcm         = per_params->heconf.DCM;
#define MCS_AX_RATE_MASK 0x7
    he_ppdu_per_params.mu_ppdu.fields.user_mcs    = per_params->rate & MCS_AX_RATE_MASK;
    he_ppdu_per_params.mu_ppdu.fields.user_txbf   = per_params->heconf.Tx_BF;
    he_ppdu_per_params.mu_ppdu.fields.user_nsts   = 0;
    he_ppdu_per_params.mu_ppdu.fields.user_sta_id = per_params->heconf.USER_STA_ID;
    he_ppdu_per_params.mu_ppdu.fields.user_idx    = per_params->heconf.USER_IDX;
  }

  /*Return Non-zero if invalid Else send 0. */
  spoof_len = heppdu_spoof_length_calculation(rate, length, bbp_info, &extra_symbol_afactor, &he_ppdu_per_params);
  printf("Spoof length calculated is %d\n", spoof_len);
  return (spoof_len & ~0xFFF);
}
//__9117_CODE_END
