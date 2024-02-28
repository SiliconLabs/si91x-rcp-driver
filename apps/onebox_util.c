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
//__9117_CODE_START
#include <sys/stat.h>
//__9117_CODE_END
#include "per_util.h"
#include <linux/netlink.h>

/******************************************************************************
 * main()
 *****************************************************************************/
int main(int argc, char *argv[])
{
  int sfd;
  char ifName[32];
  int ret          = 0;
  int cmdNo        = -1;
  unsigned char jj = 0, ii = 0;
  unsigned char *pkt1;
  unsigned int pkt, i;
  unsigned int bb_addr = 0, bb_val = 0, len = 0;
  struct bb_rf_param_t bb_rf_params;
  struct bb_rf_param_t bb_rf_read;
  w_9116_features_t w_9116_features;
  struct master_params_s master;
  struct efuse_content_t *efuse_content;
  struct fltr_bcast bcast;
  int length;
  int multiple_bb_read = 0;
  //__9117_CODE_START
  int rltime_enable;
  unsigned long int rltime_timer;
  int tx_length, rx_length;
#ifdef CONFIG_TWT_SUPPORT
  int twt_status;
  rsi_twt_user_params twt_params;
  rsi_twt_status_resp *twt_resp = malloc(sizeof(rsi_twt_status_resp));
  wifi_reschedule_twt_config_t reschedule_twt_config;
#endif
  unsigned short int bmiss_threshold_value, prev_bmiss_threshold;
  unsigned short int keep_alive_period;
  //__9117_CODE_END
  short int received_rssi;
  //__9117_CODE_START
  int diag_cmd          = 0;
  int diag_cmd_rsp_data = 0;
  int diag_param_name   = 0;
  int diag_param_val    = 0;
  //__9117_CODE_END
  struct nlmsghdr *nlh;

  if (argc < 3) {
    usage();
    return ONEBOX_STATUS_FAILURE;
  } else if (argc <= 50) {
    /* Get interface name */
    if (strnlen(argv[1], sizeof(ifName)) < sizeof(ifName)) {
      ifName[sizeof(ifName) - 1] = '\0';
      strncpy(ifName, argv[1], sizeof(ifName));
      if (ifName[sizeof(ifName) - 1] != '\0') {
        printf("ifName Buffer overflow\n");
        return ONEBOX_STATUS_FAILURE;
      }
    } else {
      ONEBOX_PRINT("length of given interface name is more than the buffer size\n");
      return ONEBOX_STATUS_FAILURE;
    }

    while (ifName[jj] != '\0') {
      if ((ifName[jj] == ';') || (ifName[jj] == '#')) {
        ONEBOX_PRINT("Error: Wrong interface name given, aborting.\n");
        return ONEBOX_STATUS_FAILURE;
      }
      jj++;
    }
    cmdNo = getcmdnumber(argv[2], ifName);
    if (cmdNo == ONEBOX_STATUS_FAILURE)
      return ONEBOX_STATUS_FAILURE;
    //printf("cmd is %d \n",cmdNo);
  }

  /* Open socket */
  sfd = socket_creation();
  if (sfd < 0) {
    ONEBOX_PRINT("Socket Creation Failed\n");
    return ONEBOX_STATUS_FAILURE;
  }

  switch (cmdNo) {
    case UPDATE_WLAN_GAIN_TABLE: {
      ret = update_wlan_gain_table(argc, argv, ifName, sfd);
    } break;
    case RSI_SET_BB_WRITE:
      if (argc > 3) {
        bb_addr = strtol(argv[3], NULL, 16);
        bb_val  = strtol(argv[4], NULL, 16);
        ONEBOX_PRINT("BB addr: 0x%x value 0x%x\n", bb_addr, bb_val);
        bb_rf_params.value        = 1;
        bb_rf_params.no_of_values = 2;
        bb_rf_params.soft_reset   = 3;
        bb_rf_params.Data[1]      = bb_addr;
        bb_rf_params.Data[2]      = bb_val;
      }
      if (argc > 5) {
        bb_rf_params.no_of_values = (argc - 3);
        for (ii = 3; ii < bb_rf_params.no_of_values - 2 + 3; ii += 2) {
          bb_addr                   = strtol(argv[ii + 2], NULL, 16);
          bb_val                    = strtol(argv[ii + 3], NULL, 16);
          bb_rf_params.Data[ii]     = bb_addr;
          bb_rf_params.Data[ii + 1] = bb_val;
          ONEBOX_PRINT("BB addr: 0x%x value 0x%x\n", bb_addr, bb_val);
        }
      }
      ret = bb_read_write_wrapper(bb_rf_params, sfd);
      if (ret < 0) {
        printf("ERROR writing to BB\n");
      } else {
        printf("SUCCESS writing to BB\n");
      }
      break;
    case EFUSE_MAP: {
      if (argc > 3) {
        printf("Usage \n");
        printf("./onebox_util rpine0  print_efuse_map\n");
        break;
      }
      efuse_content = malloc(sizeof(struct efuse_content_t));
      length        = sizeof(struct efuse_content_t);
      if (rsi_print_efuse_map(efuse_content, sfd) < 0) {
        printf("Error while reading EFUSE MAP from driver\n");
        ret = ONEBOX_STATUS_FAILURE;
        free(efuse_content);
        break;
      } else {
        nlh = common_recv_mesg_wrapper(sfd, length);
        memcpy(efuse_content, NLMSG_DATA(nlh), length);
        printf(" ***********************EFUSE MAP******************************\n");
        printf(" efuse_map_version %18x\n", efuse_content->efuse_map_version);
        printf(" module_version	%21x\n", efuse_content->module_version);
        printf(" module_part_no	%21x\n", efuse_content->module_part_no);
        printf(" mfg_sw_version	%21x\n", efuse_content->mfg_sw_version);
        printf(" module_type %24x\n", efuse_content->module_type);
        printf(" chip_version %23x\n", efuse_content->chip_version);
        printf(" hw_configuration %19s\n",
               efuse_content->m4sb_cert_en ? "1.4 silicon acting as 1.3" : "1.4 silicon only");
        printf(" mfg_sw_subversion %18x\n", efuse_content->mfg_sw_subversion);
        printf(" chip_id_no	%21x\n", efuse_content->chip_id_no);
        printf(" sdb_mode	%21x\n", efuse_content->sdb_mode);
        printf(" ***********************EFUSE MAP******************************\n");
      }
      free(efuse_content);
    } break;
    case RSI_MULTIPLE_BB_READ:
      if (argc > 9) {
        printf("Maximum 6 address allowed\n");
        break;
      }
      multiple_bb_read = 1;
    case RSI_SET_BB_READ:
      len = sizeof(bb_rf_params);
      if (argc > 3) {
        bb_rf_params.value        = 0; //BB_READ_TYPE
        bb_rf_params.no_of_values = argc - 3;
        bb_rf_params.soft_reset   = 0;
        bb_rf_params.Data[2]      = 1;

        for (ii = 3, jj = 1; ii < argc; ii++, jj++) {
          bb_addr                 = strtol(argv[ii], NULL, 16);
          bb_rf_params.Data[jj]   = bb_addr;
          bb_rf_read.Data[jj - 1] = bb_addr;
          if (!multiple_bb_read)
            break;
        }
        ret = bb_read_write_wrapper(bb_rf_params, sfd);
        if (ret < 0) {
          printf("Error Sending to bb\n");
        }
        nlh = common_recv_mesg_wrapper(sfd, len);
        if (nlh == NULL) {
          printf("Error receving from bb\n");
          break;
        } else {
          memcpy(&bb_rf_params, NLMSG_DATA(nlh), len);
          for (ii = 0; ii < bb_rf_params.no_of_values; ii++) {
            printf("BB addr : 0x%x BB_read value is 0x%x\n", bb_rf_read.Data[ii], bb_rf_params.Data[ii]);
            if (!multiple_bb_read)
              break;
          }
        }
        free(nlh);
      }
      break;
      //__9117_CODE_START
    case REAL_TIME_STATS:
      if (argc > 5 || argc < 5) {
        usage();
        break;
      }
      real_time_tx_stats *tx_stats_info = malloc(sizeof(real_time_tx_stats));
      if (tx_stats_info == NULL) {
        printf("Malloc of tx_stats_info failed\n");
        break;
      }
      real_time_rx_stats *rx_stats_info = malloc(sizeof(real_time_rx_stats));
      if (rx_stats_info == NULL) {
        printf("Malloc of rx_stats_info failed\n");
        free(tx_stats_info);
        break;
      }
      len           = sizeof(tx_stats_info);
      tx_length     = sizeof(real_time_tx_stats);
      rx_length     = sizeof(real_time_rx_stats);
      rltime_enable = atoi(argv[3]);
      rltime_timer  = atoll(argv[4]);
      if (rltime_timer < 10) {
        printf("Timer value should be greater than or equal to 10");
        free(tx_stats_info);
        free(rx_stats_info);
        break;
      }
      real_time_stats_wrapper(rltime_enable, rltime_timer, sfd, len);
      while (1) {
        if (!rltime_enable) {
          printf("Received STOPPED\n");
          break;
        }
        nlh = common_recv_mesg_wrapper(sfd, tx_length);
        if (nlh == NULL) {
          printf("Error receving\n");
          break;
        } else if (nlh->nlmsg_flags == TX_STATS) {
          memcpy(tx_stats_info, NLMSG_DATA(nlh), tx_length);
        } else if (nlh->nlmsg_flags == RX_STATS) {
          memcpy(rx_stats_info, NLMSG_DATA(nlh), rx_length);
        }
        process_real_time_stats(tx_stats_info, rx_stats_info, nlh->nlmsg_flags);
      }
      free(tx_stats_info);
      free(rx_stats_info);
      break;
      //__9117_CODE_END
    case RSI_GET_RSSI:
      if (argc != 3) {
        ONEBOX_PRINT("Usage: onebox_util rpine0 get_rssi\n");
        return ONEBOX_STATUS_FAILURE;
      }
      if (send_get_rssi_frame_to_drv(sfd) < 0) {
        ONEBOX_PRINT("Error while issuing get_rssi ioctl\n");
        ret = ONEBOX_STATUS_FAILURE;
        break;
      } else {
        nlh = common_recv_mesg_wrapper(sfd, 2);
        memcpy(&received_rssi, (short int *)NLMSG_DATA(nlh), 2);
        printf(" ***** Received RSSI is =-%d *****\n ", received_rssi);
      }
      break;
    case RSI_FILTER_BCAST: {
      if (argc != 6) {
        ONEBOX_PRINT("Usage: onebox_util rpine0 filter_bcast beacon_drop_threshold(ms) filter_bcast_in_tim "
                     "filter_bcast_tim_till_next_cmd\n");
        return ONEBOX_STATUS_FAILURE;
      }
      bcast.beacon_drop_threshold = atoi(argv[3]);
      bcast.filter_bcast_in_tim   = atoi(argv[4]);
      if (bcast.filter_bcast_in_tim != 0 && bcast.filter_bcast_in_tim != 1) {
        printf("Valid value for filter_bcast_in_tim argument is either 0 or 1\n");
        return ONEBOX_STATUS_FAILURE;
      }
      bcast.filter_bcast_tim_till_next_cmd = atoi(argv[5]);
      if (bcast.filter_bcast_tim_till_next_cmd != 0 && bcast.filter_bcast_tim_till_next_cmd != 1) {
        printf("Valid value for filter_bcast_tim_till_next_cmd argument is either 0 or 1\n");
        return ONEBOX_STATUS_FAILURE;
      }
      ret = send_filter_broadcast_frame_to_drv(bcast, sfd);
      if (ret < 0) {
        ONEBOX_PRINT("Error while issuing filter_bcast ioctl\n");
        ret = ONEBOX_STATUS_FAILURE;
      }
      break;
    }
    //__9117_CODE_START
#ifdef CONFIG_TWT_SUPPORT
    case TWT_CONFIG: {
      if (argc != 22) {
        usage();
        return ONEBOX_STATUS_FAILURE;
      }
      memset(&twt_params, 0, sizeof(rsi_twt_user_params));
      twt_status            = check_twt_status(twt_resp, sfd, nlh);
      twt_params.twt_enable = atoi(argv[20]);
      if (twt_params.twt_enable == 1) {
        if ((twt_status == TWT_SETUP_ERR_SESSION_ACTIVE) || (twt_status == TWT_SESSION_SUCC)
            || (twt_status == TWT_UNSOL_SESSION_SUCC) || (twt_status == TWT_TEARDOWN_ERR_FLOWID_NOT_MATCHED)) {
          printf("ERROR: TWT SESSION ALREADY ACTIVE\n");
          return ONEBOX_STATUS_FAILURE;
        } else if (twt_status == TWT_SETUP_SESSION_IN_PROGRESS) {
          printf("ERROR: TWT SESSION IN PROGRESS\n");
          return ONEBOX_STATUS_FAILURE;
        }
      } else if (twt_params.twt_enable == 0) {
        if ((twt_status == TWT_TEARDOWN_ERR_NOACTIVE_SESS) || (twt_status == TWT_TEARDOWN_SUCC)
            || (twt_status == TWT_DEVICE_NOT_IN_CONNECTED_STATE) || (twt_status == TWT_SETUP_AP_REJECTED)
            || (twt_status == TWT_SETUP_RSP_OUTOF_TOL) || (twt_status == TWT_SETUP_RSP_NOT_MATCHED)
            || (twt_status == TWT_SETUP_UNSUPPORTED_RSP) || (twt_status == TWT_AP_TEARDOWN_SUCC)
            || (twt_status == TWT_SETUP_FAIL_MAX_RETRIES_REACHED)) {
          printf("ERROR: NO ACTIVE SESSION\n");
          return ONEBOX_STATUS_FAILURE;
        }
      }
      twt_params.wake_duration = atoi(argv[3]);
      if ((atoi(argv[3]) < 0) || (atoi(argv[3]) > 255)) {
        printf("Invalid: Wake Duration Range Is 0-255\n");
        return ONEBOX_STATUS_FAILURE;
      } else
        twt_params.wake_duration = atoi(argv[3]);
      if ((atoi(argv[4]) < 0) || (atoi(argv[4]) > 255)) {
        printf("Invalid: Wake Duration Tolerance Range Is 0-255\n");
        return ONEBOX_STATUS_FAILURE;
      } else
        twt_params.wake_duration_tol = atoi(argv[4]);
      twt_params.wake_int_exp = atoi(argv[5]);
      if (twt_params.wake_int_exp < 0 || twt_params.wake_int_exp > 31) {
        printf("Invalid: Wake Interval Exponent Range Is 0-31\n");
        return ONEBOX_STATUS_FAILURE;
      }
      twt_params.wake_int_exp_tol = atoi(argv[6]);
      if (twt_params.wake_int_exp_tol < 0 || twt_params.wake_int_exp_tol > 31) {
        printf("Invalid: Wake Interval Exponent Tolerance Range Is 0-31\n");
        return ONEBOX_STATUS_FAILURE;
      }
      if ((atoi(argv[7])) < 0 || (atoi(argv[7])) > 65535) {
        printf("Invalid: Wake Interval Mantissa Range Is 0-65535\n");
        return ONEBOX_STATUS_FAILURE;
      } else
        twt_params.wake_int_mantissa = atoi(argv[7]);
      twt_params.wake_int_mantissa_tol = atoi(argv[8]);
      if ((atoi(argv[8]) < 0) || (atoi(argv[8]) > 65535)) {
        printf("Invalid: Wake Interval Mantissa Tolerance Range Is 0-65535\n");
        return ONEBOX_STATUS_FAILURE;
      }
      twt_params.implicit_twt = atoi(argv[9]);
      if (twt_params.implicit_twt != 0 && twt_params.implicit_twt != 1) {
        printf("Invalid: 0-Explicit twt 1-Implicit TWT\n");
        return ONEBOX_STATUS_FAILURE;
      }
      twt_params.un_announced_twt = atoi(argv[10]);
      if (twt_params.un_announced_twt != 0 && twt_params.un_announced_twt != 1) {
        printf("Invalid: 0-Announced 1-Unannounced TWT\n");
        return ONEBOX_STATUS_FAILURE;
      }
      twt_params.triggered_twt = atoi(argv[11]);
      if (twt_params.triggered_twt != 0 && twt_params.triggered_twt != 1) {
        printf("Invalid Triggered/Non Triggered TWT\n");
        return ONEBOX_STATUS_FAILURE;
      }
      twt_params.negotiation_type = atoi(argv[12]);
      if (twt_params.negotiation_type < 0 || twt_params.negotiation_type > 3) {
        printf("Invalid: 0-Individual twt 1-Broadcast TWT\n");
        return ONEBOX_STATUS_FAILURE;
      }
      twt_params.twt_channel = atoi(argv[13]);
      /*if (twt_params.twt_channel == 0) {
                printf("Invalid Value For Channel\n");
                return ONEBOX_STATUS_FAILURE;
        }*/
      twt_params.twt_protection = atoi(argv[14]);
      if (twt_params.twt_protection != 0 && twt_params.twt_protection != 1) {
        printf("Invalid: 0-Disable 1-Enable TWT Protection\n");
        return ONEBOX_STATUS_FAILURE;
      }
      if (twt_params.twt_enable != 0 && twt_params.twt_enable != 1) {
        printf("Invalid: 0-Disable 1-Enable TWT\n");
        return ONEBOX_STATUS_FAILURE;
      }
      twt_params.twt_flow_id = atoi(argv[15]);
      if (twt_params.twt_enable) {
        if (twt_params.twt_flow_id < 0 || twt_params.twt_flow_id > 7) {
          printf("Invalid: Flow ID Range Is 0-7\n");
          return ONEBOX_STATUS_FAILURE;
        }
      } else if (!twt_params.twt_enable) {
        if (!(strcasecmp(argv[15], "0xff"))) {
          twt_params.twt_flow_id = 255;
          printf("DISABLING ALL TWT SESSIONS\n");
        } else if ((twt_params.twt_flow_id < 0 || twt_params.twt_flow_id > 7)) {
          printf("Invalid: Flow ID Range Is 0-7 And To Disable All TWT Sessions Value Is 0xff\n");
          return ONEBOX_STATUS_FAILURE;
        }
      }
      twt_params.restrict_tx_outside_tsp = atoi(argv[16]);
      if (twt_params.restrict_tx_outside_tsp != 0 && twt_params.restrict_tx_outside_tsp != 1) {
        printf("Invalid: 0-Disable 1-Enable Restrict TX Outside TSP\n");
        return ONEBOX_STATUS_FAILURE;
      }
      twt_params.twt_retry_limit = atoi(argv[17]);
      if (twt_params.twt_retry_limit < 0 || twt_params.twt_retry_limit > 15) {
        printf("Invalid: TWT Retry Limit Range Is 0-15\n");
        return ONEBOX_STATUS_FAILURE;
      }
      if (atoi(argv[18]) < 5 || atoi(argv[18]) > 255) {
        printf("Invalid: TWT Retry Interval Range Is 5-255\n");
        return ONEBOX_STATUS_FAILURE;
      } else
        twt_params.twt_retry_interval = atoi(argv[18]);
      twt_params.req_type = atoi(argv[19]);
      if (twt_params.req_type < 0 || twt_params.req_type > 2) {
        printf("Invalid: 0-Request TWT 1-Suggest TWT 2-Demand TWT\n");
        return ONEBOX_STATUS_FAILURE;
      }
      twt_params.wake_duration_unit = atoi(argv[21]);
      if (twt_params.wake_duration_unit != 0 && twt_params.wake_duration_unit != 1) {
        printf("Invalid: 0 - 256us  1 - 1024us\n");
        return ONEBOX_STATUS_FAILURE;
      }
      if ((twt_params.req_type != 0)
          && ((twt_params.wake_duration == 0) || (twt_params.wake_int_mantissa == 0)
              || (((unsigned long)twt_params.wake_duration
                   * (unsigned long)(twt_params.wake_duration_unit ? 1024 : 256))
                  > ((unsigned long)twt_params.wake_int_mantissa * ((unsigned long)1 << twt_params.wake_int_exp))))) {
        return ONEBOX_STATUS_FAILURE;
      }
      if ((twt_config_wrapper(twt_params, sfd)) < 0) {
        return ONEBOX_STATUS_FAILURE;
      }
      printf("====================TWT PARAMS=====================\n");
      printf("wake duration = %d\n", twt_params.wake_duration);
      printf("wake duration tolerance = %d\n", twt_params.wake_duration_tol);
      printf("wake int exp = %d\n", twt_params.wake_int_exp);
      printf("wake int exp tolerance = %d\n", twt_params.wake_int_exp_tol);
      printf("wake int mantissa = %d\n", twt_params.wake_int_mantissa);
      printf("wake int mantissa tolerance = %d\n", twt_params.wake_int_mantissa_tol);
      printf("implicit twt = %d\n", twt_params.implicit_twt);
      printf("unannounced twt = %d\n", twt_params.un_announced_twt);
      printf("triggered twt = %d\n", twt_params.triggered_twt);
      printf("negotiation type = %d\n", twt_params.negotiation_type);
      printf("twt channel = %d\n", twt_params.twt_channel);
      printf("twt protection = %d\n", twt_params.twt_protection);
      printf("twt flow id = %d\n", twt_params.twt_flow_id);
      printf("restrict tx outside tsp = %d\n", twt_params.restrict_tx_outside_tsp);
      printf("twt retry limit = %d\n", twt_params.twt_retry_limit);
      printf("twt retry interval = %d\n", twt_params.twt_retry_interval);
      printf("twt req type = %d\n", twt_params.req_type);
      printf("twt enable = %d\n", twt_params.twt_enable);
      printf("wake_duration_unit = %d\n", twt_params.wake_duration_unit);
      free(twt_resp);
      break;
    }
    case TWT_STATUS: {
      if (argc > 3) {
        usage();
        return ONEBOX_STATUS_FAILURE;
      }
      twt_status = check_twt_status(twt_resp, sfd, nlh);
      switch (twt_status) {
        case TWT_DEVICE_NOT_IN_CONNECTED_STATE:
          printf("Device Not in Connected State\n");
          break;
        case TWT_SESSION_SUCC:
          printf("TWT Setup success\n");
          printf("Flow ID        = %d\n", twt_resp->twt_flow_id);
          printf("Wake Duration  = %d\n", twt_resp->wake_duration);
          printf("Wake Duration Unit  = %d\n", twt_resp->wake_duration_unit);
          printf("Wake Interval Exponent  = %d\n", twt_resp->wake_int_exp);
          printf("Wake Interval Mantissa  = %d\n", twt_resp->wake_int_mantissa);
          printf("Implicit TWT   = %d\n", twt_resp->implicit_twt);
          printf("Triggered TWT  = %d\n", twt_resp->triggered_twt);
          printf("Un_Announced_TWT = %d\n", twt_resp->un_announced_twt);
          printf("Request Type   = %d\n", twt_resp->req_type);
          break;
        case TWT_UNSOL_SESSION_SUCC:
          printf("Unsolicited TWT Setup success\n");
          printf("Flow ID        = %d\n", twt_resp->twt_flow_id);
          printf("Wake Duration  = %d\n", twt_resp->wake_duration);
          printf("Wake Duration Unit  = %d\n", twt_resp->wake_duration_unit);
          printf("Wake Interval Exponent  = %d\n", twt_resp->wake_int_exp);
          printf("Wake Interval Mantissa  = %d\n", twt_resp->wake_int_mantissa);
          printf("Implicit TWT   = %d\n", twt_resp->implicit_twt);
          printf("Triggered TWT  = %d\n", twt_resp->triggered_twt);
          printf("Un_Announced_TWT = %d\n", twt_resp->un_announced_twt);
          printf("Request Type   = %d\n", twt_resp->req_type);
          break;
        case TWT_SETUP_ERR_SESSION_ACTIVE:
          printf("TWT Setup Failed due to Already session is Active\n");
          break;
        case TWT_TEARDOWN_ERR_FLOWID_NOT_MATCHED:
          printf("TWT Teardown Failed. FlowId not matched\n");
          break;
        case TWT_TEARDOWN_ERR_NOACTIVE_SESS:
          printf("No Active TWT Session\n");
          break;
        case TWT_SETUP_AP_REJECTED:
          printf("TWT Setup Failed. TWT Setup rejected by AP\n");
          break;
        case TWT_SETUP_RSP_OUTOF_TOL:
          printf("TWT Setup Failed. TWT response out of tolerence limits\n");
          break;
        case TWT_SETUP_RSP_NOT_MATCHED:
          printf("TWT Setup Failed. TWT Response not matched with the request parameters\n");
          break;
        case TWT_SETUP_UNSUPPORTED_RSP:
          printf("TWT Setup Failed. TWT Response Unsupported\n");
          break;
        case TWT_SETUP_FAIL_MAX_RETRIES_REACHED:
          printf("TWT Setup Failed. Max retries reached\n");
          break;
        case TWT_INACTIVE_DUETO_ROAMING:
          printf("TWT session inactive due to roaming\n");
          break;
        case TWT_INACTIVE_DUETO_DISCONNECT:
          printf("TWT session inactive due to wlan disconnection\n");
          break;
        case TWT_TEARDOWN_SUCC:
          printf("TWT session teardown success\n");
          break;
        case TWT_AP_TEARDOWN_SUCC:
          printf("TWT session teardown from AP\n");
          break;
        case TWT_INACTIVE_NO_AP_SUPPORT:
          printf("Connected AP Does not support TWT\n");
          break;
        case TWT_RESCHEDULE_SUCC:
          printf("RESCHEDULE TWT Success\n");
          break;
        case TWT_RESCHEDULE_FAIL:
          printf("RESCHEDULE TWT Fail\n");
          break;
        default:
          printf("Unsupported TWT status");
          break;
      }
      free(twt_resp);
      break;
    }
    case RESCHEDULE_TWT:
      if (argc != 6) {
        usage();
        return ONEBOX_STATUS_FAILURE;
      }
      memset(&reschedule_twt_config, 0, sizeof(wifi_reschedule_twt_config_t));
      reschedule_twt_config.flow_id = atoi(argv[3]);
      if (reschedule_twt_config.flow_id < 0 || reschedule_twt_config.flow_id > 7) {
        printf("Invalid: Flow ID Range Is 0-7\n");
        return ONEBOX_STATUS_FAILURE;
      }
      reschedule_twt_config.twt_action = atoi(argv[4]);
      if (reschedule_twt_config.twt_action < 0 || reschedule_twt_config.twt_action > 2) {
        printf("Invalid: 0:Suspend Indefinitely, 1: Suspend for duration, 2: Resume Immediately\n");
        return ONEBOX_STATUS_FAILURE;
      }
      reschedule_twt_config.suspend_duration = atoi(argv[5]);
      if (reschedule_twt_config.suspend_duration < 0
          || reschedule_twt_config.suspend_duration > MAX_TWT_SUSPEND_DURATION) {
        printf("Invalid: Suspend Duration Range Is 0 - 86,400,000\n");
        return ONEBOX_STATUS_FAILURE;
      }
      if ((reschedule_twt_config.twt_action == SUSPEND_INDEFINITELY
           || reschedule_twt_config.twt_action == RESUME_IMMEDIATELY)
          && reschedule_twt_config.suspend_duration > 0) {
        printf("Invalid: In case of SUSPEND_INDEFINITELY or RESUME_IMMEDIATELY suspend_duration should be zero\n");
        return ONEBOX_STATUS_FAILURE;
      }
      if (reschedule_twt_config.twt_action == SUSPEND_FOR_DURATION
          && (reschedule_twt_config.suspend_duration < 1
              || reschedule_twt_config.suspend_duration > MAX_TWT_SUSPEND_DURATION)) {
        printf("Invalid: In case of SUSPEND_FOR_DURATION, suspend_duration should be in range of  0 - 86,400,000\n");
        return ONEBOX_STATUS_FAILURE;
      }
      prepare_and_send_nl_cmd(sfd,
                              RESCHEDULE_TWT,
                              0,
                              WLAN_PACKET,
                              (void *)&reschedule_twt_config,
                              sizeof(wifi_reschedule_twt_config_t));

      break;
#endif
    case SET_HW_BMISS_THRESHOLD:
      if (argc != 4) {
        usage();
        return ONEBOX_STATUS_FAILURE;
      }
      bmiss_threshold_value = atoi(argv[3]);
      if (set_hw_bmiss_threshold_value(bmiss_threshold_value, sfd) < 0) {
        ONEBOX_PRINT("Error while issuing hw_bmiss_threshold ioctl\n");
        ret = ONEBOX_STATUS_FAILURE;
      } else {
        nlh = common_recv_mesg_wrapper(sfd, 2);
        memcpy(&prev_bmiss_threshold, (short int *)NLMSG_DATA(nlh), 2);
        printf(" ***** Previous HW_BMISS Threshold value is = %d *****\n ", prev_bmiss_threshold);
      }
      break;
    case SET_KEEP_ALIVE_PERIOD:
      if (argc != 4) {
        usage();
        return ONEBOX_STATUS_FAILURE;
      }
      keep_alive_period = atoi(argv[3]);
      if (set_keep_alive_period(keep_alive_period, sfd) < 0) {
        ONEBOX_PRINT("Error while issuing set_keep_alive_period ioctl\n");
        ret = ONEBOX_STATUS_FAILURE;
      }
      break;
    case DIAG_TOOL:
      if (argc < 4) {
        usage();
        return ONEBOX_STATUS_FAILURE;
      }
      if (!strcmp(argv[3], "register_report")) {
        if (argc != 6) {
          usage();
          return ONEBOX_STATUS_FAILURE;
        }
        mkdir(DIAG_TOOL_REPORT_DIR, S_IRWXU | S_IRWXG);
        ret = diag_generate_reg_report(sfd, argv[4], argv[5]);
      } else if (!strcmp(argv[3], "flash_report")) {
        mkdir(DIAG_TOOL_REPORT_DIR, S_IRWXU | S_IRWXG);
        if (diag_impu_flash_dump(sfd, ifName) < 0) {
          ONEBOX_PRINT("%s: Failed to generate IPMU flash report\n", __func__);
          ret = ONEBOX_STATUS_FAILURE;
        }
        if (diag_rf_flash_dump(sfd, ifName) < 0) {
          ONEBOX_PRINT("%s: Failed to generate RF flash report\n", __func__);
          ret = ONEBOX_STATUS_FAILURE;
        }
      } else if (!strcmp(argv[3], "dpd_enable") || !strcmp(argv[3], "dpd_temp_ctrl_en")) {
        diag_param_name = (!strcmp(argv[3], "dpd_enable")) ? DPD_EN_GLOBAL : DPD_TEMP_CTRL_EN_GLOBAL;
        if (argc == 4) {
          diag_cmd = DIAG_READ;
        } else {
          diag_cmd       = DIAG_WRITE;
          diag_param_val = atoi(argv[4]);
          if (diag_param_val != 0 && diag_param_val != 1) {
            ONEBOX_PRINT("Valid value for dpd_enable/dpd_temp_ctrl is either 0 or 1\n");
            return ONEBOX_STATUS_FAILURE;
          }
        }
        if (diag_tool_param_update(sfd, diag_cmd, diag_param_name, diag_param_val) < 0) {
          ONEBOX_PRINT("Error while issuing %s command\n", argv[3]);
          ret = ONEBOX_STATUS_FAILURE;
          break;
        }
        if (diag_cmd == DIAG_READ) {
          nlh = common_recv_mesg_wrapper(sfd, 2);
          memcpy(&diag_cmd_rsp_data, (int *)NLMSG_DATA(nlh), 2);
          printf(" ***** Received %s value is = %d *****\n ", argv[3], diag_cmd_rsp_data);
        }
      } else {
        usage();
        return ONEBOX_STATUS_FAILURE;
      }
      break;
      //__9117_CODE_END
    case WLAN_9116_FEATURE: {
      if (argc < 11) {
        printf("Usage \n");
        printf("./onebox_util rpine0 w_9116_features pll_mode rf_type wireless_mode enable_ppe afe_type dpd "
               "SIFSTransmitenable pwrsave_options\n");
        printf("\x1B[31m"
               "PLL_MODE:"
               "\x1B[0m"
               "0-PLLMODE0, 1-PLLMODE1, 2-PLLMODE2 \n");
        printf("RF_TYPE[ONLY FOR 2GHz]: 0-External_RF_8111, 1-Internal_RF_9116, 2-AVIACOM_RF \n");
        printf("WIRELESS_MODE:12 – for LP chain enable, else LP chain disable \n");
        printf("ENABLE_PPE: 0-Disable_per_packet_TX_programming, 1-Enable_per_packet_TX_programming_mode_1, "
               "2-Enable_per_packet_TX_programming_mode_2 \n");
        printf("AFE: ? Default=0\n");
        printf("DPD: ? Default=0\n");
        printf("SIFSTransmitenable\n");
        printf("pwrsave_options[0-3]: 0 -Disable [duty cycling & end of frame], 1-[Duty cycling Enabled], "
               "2-[End_of_Frame] 3- Enable [Duty_cycling & End_of_frame] \n");
        break;
      }
      memset(&w_9116_features, 0, sizeof(w_9116_features));
      w_9116_features.pll_mode           = atoi(argv[3]);
      w_9116_features.rf_type            = atoi(argv[4]);
      w_9116_features.wireless_mode      = atoi(argv[5]);
      w_9116_features.enable_ppe         = atoi(argv[6]);
      w_9116_features.afe_type           = atoi(argv[7]);
      w_9116_features.dpd                = atoi(argv[8]);
      w_9116_features.SIFSTransmitenable = atoi(argv[9]);
      w_9116_features.pwrsave_options    = atoi(argv[10]);
      if (w_9116_features.pll_mode > 2 || w_9116_features.pll_mode < 0) {
        printf("Invalid pll_mode Configuration\n");
        printf("\x1B[31m"
               "PLL_MODE:"
               "\x1B[0m"
               "0-PLLMODE0, 1-PLLMODE1, 2-PLLMODE2 \n");
        break;
      } else if (w_9116_features.rf_type > 2 || w_9116_features.rf_type < 0) {
        printf("Invalid rf_type Configuration\n");
        printf("RF_TYPE[ONLY FOR 2GHz]: 0-External_RF_8111, 1-Internal_RF_9116, 2-AVIACOM_RF \n");
        break;
      } else if (w_9116_features.enable_ppe > 2 || w_9116_features.enable_ppe < 0) {
        printf("Invalid enable_ppe Configuration\n");
        printf("ENABLE_PPE: 0-Disable_per_packet_TX_programming, 1-Enable_per_packet_TX_programming_mode_1, "
               "2-Enable_per_packet_TX_programming_mode_2 \n");
        break;
      } else if (w_9116_features.dpd > 1 || w_9116_features.dpd < 0) {
        printf("Invalid dpd Configuration\n");
        printf("DPD: ? \n");
        break;
      } else if (w_9116_features.SIFSTransmitenable > 1 || w_9116_features.SIFSTransmitenable < 0) {
        printf("Invalid SIFSTransmitenable Configuration\n");
        printf("SIFSTransmitenable: ? \n");
        break;
      } else if (w_9116_features.pwrsave_options < 0) {
        printf("Invalid pwrsave_options Configuration\n");
        printf("pwrsave_options[0-3]: 0 -Disable [duty cycling & end of frame], 1-[Duty cycling Enabled], "
               "2-[End_of_Frame] 3- Enable [Duty_cycling & End_of_frame] \n");
        break;
      }
      prepare_and_send_nl_cmd(sfd,
                              WLAN_9116_FEATURE,
                              0,
                              WLAN_PACKET,
                              (void *)&w_9116_features,
                              sizeof(w_9116_features));
    } break;
    case MASTER_READ: {
      memset(&master, 0, sizeof(master));
      if (argc != 5) {
        ONEBOX_PRINT("Usage: onebox_util base_interface master_read $32bit_address $no_bytes_to_read )\n");
        close(sfd);
        return ONEBOX_STATUS_FAILURE;
      }
      master.address     = strtol(argv[3], NULL, 16);
      master.no_of_bytes = atoi(argv[4]);
      if (master.no_of_bytes >= 4) {
        if (master.no_of_bytes % 4) {
          ONEBOX_PRINT("Enter valid no of bytes to read either 1, 2 or multiples of 4 )\n");
          close(sfd);
          return ONEBOX_STATUS_FAILURE;
        }

      } else if ((master.no_of_bytes != 1) && (master.no_of_bytes != 2)) {
        ONEBOX_PRINT("Enter valid no of bytes to read either 1, 2 or multiples of 4 )\n");
        close(sfd);
        return ONEBOX_STATUS_FAILURE;
      }
      if (master_ops(sfd, READ, master.address, (uint_8 *)&master.data, master.no_of_bytes)) {
        printf("%s: Master read failed\n", __func__);
      }

      for (i = 0; i < master.no_of_bytes; i++) {
        if (!(i % 16))
          printf("\n");
        printf("0x%x ", master.data[i]);
      }
      printf("\n");
    } break;
    case MASTER_WRITE: {
      memset(&master, 0, sizeof(master));
      if (argc != 6) {
        ONEBOX_PRINT("Usage: onebox_util base_interface master_write $32bit_address $no_bytes_to_write $data)\n");
        close(sfd);
        return ONEBOX_STATUS_FAILURE;
      }
      master.address     = strtol(argv[3], NULL, 16);
      master.no_of_bytes = atoi(argv[4]);
      if ((master.no_of_bytes != 1) && (master.no_of_bytes != 2) && (master.no_of_bytes != 4)) {
        ONEBOX_PRINT("Enter valid no of bytes to write either 1, 2 or 4 )\n");
        close(sfd);
        return ONEBOX_STATUS_FAILURE;
      }
      pkt  = strtol(argv[5], NULL, 16);
      pkt1 = (unsigned char *)&pkt;
      for (i = 0; i < master.no_of_bytes; i++) {
        if (!(i % 16))
          printf("\n");
        master.data[i] = pkt1[i];
        printf("0x%x ", master.data[i]);
      }
      if (master_ops(sfd, WRITE, master.address, (uint_8 *)&master.data, master.no_of_bytes)) {
        printf("%s: Master write failed\n", __func__);
      }
    } break;

    default:
      return ONEBOX_STATUS_FAILURE;
  }
  close(sfd);
  return ret;
}

int getcmdnumber(char *command, char *ifName)
{
  if (!strcmp(command, "update_wlan_gain_table") && !strncmp(ifName, "rpine", 5)) {
    return UPDATE_WLAN_GAIN_TABLE;
  } else if (!strcmp(command, "print_efuse_map") && !strncmp(ifName, "rpine", 5)) {
    return EFUSE_MAP;
  } else if (!strcmp(command, "bb_write") && !strncmp(ifName, "rpine", 5)) {
    return RSI_SET_BB_WRITE;
  } else if (!strcmp(command, "bb_read_multiple") && !strncmp(ifName, "rpine", 5)) {
    return RSI_MULTIPLE_BB_READ;
  } else if (!strcmp(command, "bb_read") && !strncmp(ifName, "rpine", 5)) {
    return RSI_SET_BB_READ;
    //__9117_CODE_START
  } else if (!strcmp(command, "real_time_stats") && !strncmp(ifName, "rpine", 5)) {
    return REAL_TIME_STATS;
    //__9117_CODE_END
  } else if (!strcmp(command, "filter_bcast") && !strncmp(ifName, "rpine", 5)) {
    return RSI_FILTER_BCAST;
  } else if (!strcmp(command, "get_rssi") && !strncmp(ifName, "rpine", 5)) {
    return RSI_GET_RSSI;
  }
//__9117_CODE_START
#ifdef CONFIG_TWT_SUPPORT
  else if (!strcmp(command, "twt_config") && !strncmp(ifName, "rpine0", 5)) {
    return TWT_CONFIG;
  } else if (!strcmp(command, "twt_status") && !strncmp(ifName, "rpine0", 5)) {
    return TWT_STATUS;
  } else if (!strcmp(command, "reschedule_twt") && !strncmp(ifName, "rpine0", 5)) {
    return RESCHEDULE_TWT;
  }
#endif
  else if (!strcmp(command, "hw_bmiss_threshold") && !strncmp(ifName, "rpine0", 5)) {
    return SET_HW_BMISS_THRESHOLD;
  } else if (!strcmp(command, "keep_alive") && !strncmp(ifName, "rpine0", 5)) {
    return SET_KEEP_ALIVE_PERIOD;
  } else if (!strcmp(command, "diag_tool") && !strncmp(ifName, "rpine0", 5)) {
    return DIAG_TOOL;
  }
  //__9117_CODE_END
  else if (!strcmp(command, "w_9116_features") && !strncmp(ifName, "rpine0", 5)) {
    return WLAN_9116_FEATURE;
  } else if (!strcmp(command, "master_read") && !strncmp(ifName, "rpine", 5)) {
    return MASTER_READ;
  } else if (!strcmp(command, "master_write") && !strncmp(ifName, "rpine", 5)) {
    return MASTER_WRITE;
  } else {
    ONEBOX_PRINT("Error: Wrong command , Please follow usage...\n");
    usage();
    return ONEBOX_STATUS_FAILURE;
  }
}

void usage()
{
  ONEBOX_PRINT("Usage:./onebox_util rpine0 update_wlan_gain_table\n");
  ONEBOX_PRINT("Usage:./onebox_util rpine0 addr data\n");
  ONEBOX_PRINT("Usage:./onebox_util rpine0 addr\n");
  ONEBOX_PRINT("Usage:./onebox_util rpine0 print_efuse_map\n");
  //__9117_CODE_START
  ONEBOX_PRINT("Usage:./onebox_util rpine0 real_time_stats enable timer value\n");
#ifdef CONFIG_TWT_SUPPORT
  ONEBOX_PRINT("Usage:./onebox_util rpine0 twt_config wake_duration wake_duration_tolerance wake_int_exp "
               "wake_int_exp_tolerance wake_int_mantissa wake_int_mantissa_tolerance implicit_twt unannounced_twt "
               "triggered_twt negotiation_type twt_channel twt_protection twt_flow_id restrict_twt_outside_tsp "
               "twt_retry_limit twt_retry_interval twt_req_type twt_enable wake_duration_unit\n");
  ONEBOX_PRINT("Usage:./onebox_util rpine0 twt_status\n");
  ONEBOX_PRINT("Usage:./onebox_util rpine0 reschedule_twt twt_flow_id twt_action duration\n");
#endif
  ONEBOX_PRINT("Usage:./onebox_util rpine0 hw_bmiss_threshold value\n");
  ONEBOX_PRINT("Usage:./onebox_util rpine0 keep_alive value\n");
  ONEBOX_PRINT("Usage:./onebox_util rpine0 diag_tool register_report <rf_input_reg_ref.csv | "
               "ipmu_input_reg_ref.csv> <mode>\n");
  ONEBOX_PRINT("Usage:./onebox_util rpine0 diag_tool flash_report\n");
  ONEBOX_PRINT("Usage:./onebox_util rpine0 diag_tool dpd_enable\n");
  ONEBOX_PRINT("Usage:./onebox_util rpine0 diag_tool dpd_enable <0|1>\n");
  ONEBOX_PRINT("Usage:./onebox_util rpine0 diag_tool dpd_temp_ctrl_en\n");
  ONEBOX_PRINT("Usage:./onebox_util rpine0 diag_tool dpd_temp_ctrl_en <0|1>\n");
  ONEBOX_PRINT("Usage: onebox_util base_interface w_9116_features pll_mode rf_type wireless_mode enable_ppe afe_type "
               "dpd SIFSTransmitenable pwrsave_options\n");
  ONEBOX_PRINT("Usage: onebox_util base_interface master_read $32bit_address $no_bytes_to_read )\n");
  ONEBOX_PRINT("Usage: onebox_util base_interface master_write $32bit_address $no_bytes_to_write $data)\n");
  //__9117_CODE_END
  return;
}

//__9117_CODE_START
#ifdef CONFIG_TWT_SUPPORT
int check_twt_status(rsi_twt_status_resp *twt_resp, int sfd, struct nlmsghdr *nlh)
{
  int len, twt_status;
  len = sizeof(rsi_twt_status_resp);
  if ((twt_status_wrapper(sfd, len)) < 0) {
    return ONEBOX_STATUS_FAILURE;
  }
  nlh = common_recv_mesg_wrapper(sfd, len);
  if (nlh == NULL) {
    printf("RECEVING FAILED\n");
    return ONEBOX_STATUS_FAILURE;
  } else {
    memcpy(twt_resp, NLMSG_DATA(nlh), len);
    twt_status = nlh->nlmsg_flags;
  }
  return twt_status;
}
#endif

int process_real_time_stats(real_time_tx_stats *tx_stats_info, real_time_rx_stats *rx_stats_info, int stat_type)
{
  unsigned short tx_pkt_rate, tx_pkt_mode, _11ax_mode;
  unsigned char rx_pkt_rate;

  if (stat_type == TX_STATS) {
    printf("==================TX PKT INFORMATION===========================\n");
    printf("QUEUE No = %d\n"
           "tx_real_timestamp = %u\n"
           "tx_timestamp = %u\n"
           "ba_bitmap = %llx\n"
           "ba_seq_no = %d\n"
           "pkt_start_seq_no = %d\n"
           "pkt_seq_no_bitmap = %llx\n"
           "tx_retry_count = %d  "
           "tx_power = %d\n",
           tx_stats_info->tx_que_no,
           tx_stats_info->tx_real_timestamp,
           tx_stats_info->tx_timestamp,
           tx_stats_info->ba_bitmap,
           tx_stats_info->ba_seq_no,
           tx_stats_info->pkt_start_seq_no,
           tx_stats_info->pkt_seq_no_bitmap,
           tx_stats_info->tx_retry_count,
           tx_stats_info->tx_power);
    printf("TX status info : "
           "succes indication = %d  "
           "All ack count =%d\n",
           tx_stats_info->status_info.success_valid,
           tx_stats_info->status_info.all_ack_count);
    printf("\t\ttype of failure = %s\n",
           ((tx_stats_info->status_info.type_of_failure == 0)
              ? "Unable to get the channel within maximum channel access time"
              : (tx_stats_info->status_info.type_of_failure == 1
                   ? "Response is not received (rx_start not asserted) within Ack-Timeout value"
                   : (tx_stats_info->status_info.type_of_failure == 2
                        ? "rx_start detected within Ack-Timeout but frame is received with CRC error"
                        : (tx_stats_info->status_info.type_of_failure == 3 ? "Unexpected frame is received" : 0)))));
    printf("\t\tResponse type = %s\n",
           ((tx_stats_info->status_info.response_type == 4)
              ? "Ack"
              : (tx_stats_info->status_info.response_type == 5
                   ? "CTS"
                   : (tx_stats_info->status_info.response_type == 6
                        ? "BA"
                        : (tx_stats_info->status_info.response_type == 7
                             ? "PV1MTBA"
                             : (tx_stats_info->status_info.response_type == 8 ? "MPDU" : 0))))));
    tx_pkt_mode = (tx_stats_info->tx_pkt_type >> 7) & 0x0003;
    _11ax_mode  = (tx_stats_info->tx_pkt_type >> 10) & 0x0001;
    if (_11ax_mode == 1) {
      printf("tx pkt type = 11AX mode ");
      tx_pkt_rate = tx_stats_info->tx_pkt_type & 0x000f;
      print_mcs_rate(tx_pkt_rate);
    } else if (tx_pkt_mode == 3) {
      tx_pkt_rate = tx_stats_info->tx_pkt_type & 0x000f;
      printf("tx pkt type = 11AC mode ");
      print_mcs_rate(tx_pkt_rate);
    } else if (tx_pkt_mode == 2) {
      tx_pkt_rate = tx_stats_info->tx_pkt_type & 0x003f;
      printf("tx pkt type = 11N mode ");
      print_mcs_rate(tx_pkt_rate);
    } else if (tx_pkt_mode == 1) {
      tx_pkt_rate = tx_stats_info->tx_pkt_type & 0x003f;
      printf("tx pkt type = 11G mode ");
      print_11g_11b_rate(tx_pkt_rate);
    } else if (tx_pkt_mode == 0) {
      tx_pkt_rate = tx_stats_info->tx_pkt_type & 0x003f;
      printf("tx pkt type = 11B mode ");
      print_11g_11b_rate(tx_pkt_rate);
    }

  } else if (stat_type == RX_STATS) {
    printf("==================RX PKT INFORMATION===========================\n");
    printf("rx_timestamp = %d\n"
           "rx_real_timestamp = %d\n"
           "rx_status_info : \n"
           "\tAborted = %d  "
           "crc_error = %d  "
           "mic_error = %d  "
           "icv_error = %d  "
           "cipher suite mismatch = %d  "
           "Pkt type data = %d\n"
           "rx rssi = %d\n",
           rx_stats_info->rx_timestamp,
           rx_stats_info->rx_real_timestamp,
           rx_stats_info->status_info.aborted,
           rx_stats_info->status_info.crc_error,
           rx_stats_info->status_info.mic_error,
           rx_stats_info->status_info.icv_error,
           rx_stats_info->status_info.cipher_suite_mismatch,
           rx_stats_info->status_info.pkt_type_is_data,
           rx_stats_info->rssi);

    printf("pkt length = %d\n", rx_stats_info->pkt_type.pkt_length);
    if (rx_stats_info->pkt_type.mode_11ax == 1) {
      printf("rx pkt type = 11AX mode ");
      rx_pkt_rate = rx_stats_info->pkt_type.pkt_rate & 0x0f;
      print_mcs_rate(rx_pkt_rate);
    } else if (rx_stats_info->pkt_type.mode_11ac == 1) {
      printf("rx pkt type = 11AC mode ");
      rx_pkt_rate = rx_stats_info->pkt_type.pkt_rate & 0x0f;
      print_mcs_rate(rx_pkt_rate);
    } else if (rx_stats_info->pkt_type.pkt_mode == 3) {
      printf("rx pkt type = 11N gf mode ");
      print_mcs_rate(rx_stats_info->pkt_type.pkt_rate);
    } else if (rx_stats_info->pkt_type.pkt_mode == 2) {
      printf("rx pkt type = 11N mf mode ");
      print_mcs_rate(rx_stats_info->pkt_type.pkt_rate);
    } else if (rx_stats_info->pkt_type.pkt_mode == 1) {
      printf("rx pkt type = 11G mode ");
      print_11g_11b_rate(rx_stats_info->pkt_type.pkt_rate);
    } else if (rx_stats_info->pkt_type.pkt_mode == 0) {
      printf("rx pkt type = 11B mode ");
      print_11g_11b_rate(rx_stats_info->pkt_type.pkt_rate);
    }
    if ((rx_stats_info->pkt_type.mode_11ac == 0) && (rx_stats_info->pkt_type.mode_11ax == 1)) {
      printf("Pkt Mode = %s\n",
             ((rx_stats_info->pkt_type.pkt_mode == 0)
                ? "HE SU PPDU"
                : (rx_stats_info->pkt_type.pkt_mode == 1
                     ? "HE ER SU PPDU"
                     : (rx_stats_info->pkt_type.pkt_mode == 2
                          ? "HE TRIG PPDU"
                          : (rx_stats_info->pkt_type.pkt_mode == 3 ? "HE MU PPDU" : "0")))));
    }
    printf("Bandwidth = %s\n",
           ((rx_stats_info->pkt_type.bandwidth == 0)
              ? "20MHZ"
              : (rx_stats_info->pkt_type.bandwidth == 1 ? "40 MHZ"
                                                        : (rx_stats_info->pkt_type.bandwidth == 2 ? "80 MHZ" : "0"))));
  }
  return 0;
}

void print_mcs_rate(unsigned short pkt_rate)
{
  switch (pkt_rate) {
    case 0:
      printf("MCS0\n");
      break;
    case 1:
      printf("MCS1\n");
      break;
    case 2:
      printf("MCS2\n");
      break;
    case 3:
      printf("MCS3\n");
      break;
    case 4:
      printf("MCS4\n");
      break;
    case 5:
      printf("MSC5\n");
      break;
    case 6:
      printf("MCS6\n");
      break;
    case 7:
      printf("MCS7\n");
      break;
    default:
      printf("Invalid rate\n");
      break;
  }
}

void print_11g_11b_rate(unsigned short pkt_rate)
{
  switch (pkt_rate) {
    case 0:
      printf("1 Mbps\n");
      break;
    case 2:
      printf("2 Mbps\n");
      break;
    case 4:
      printf("5.5 Mbps\n");
      break;
    case 6:
      printf("11 Mbps\n");
      break;
    case 11:
      printf("6 Mbps\n");
      break;
    case 15:
      printf("9 Mbps\n");
      break;
    case 10:
      printf("12 Mbps\n");
      break;
    case 14:
      printf("18 Mbps\n");
      break;
    case 9:
      printf("24 Mbps\n");
      break;
    case 13:
      printf("36 Mbps\n");
      break;
    case 8:
      printf("48 Mbps\n");
      break;
    case 12:
      printf("54 Mbps\n");
      break;
    default:
      printf("Invalid rate\n");
      break;
  }
}
//__9117_CODE_END
