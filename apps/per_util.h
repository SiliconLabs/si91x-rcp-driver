/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#ifndef _ONEBOX_UTILS_H_
#define _ONEBOX_UTILS_H_

#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define BW_20  0
#define BW_1   1
#define BW_U40 2
#define BW_2   3
#define BW_L40 4
#define BW_5   5
#define BW_F40 6
#define BW_10  7

//__9117_CODE_START
/* Receive stats groups */
#define GROUP_0 BIT(0)
#define GROUP_1 BIT(1)
#define GROUP_2 BIT(2)
#define GROUP_3 BIT(3)
#define GROUP_4 BIT(4)
//__9117_CODE_END
typedef struct {
  //! no. of tx pkts
  unsigned short tx_pkts;
  //! no. of rx pkts
  unsigned short rx_pkts;
  //! no. of tx retries
  unsigned short tx_retries;
  //! no. of pkts that pass crc
  unsigned short crc_pass;
  //! no. of pkts failing crc chk
  unsigned short crc_fail;
  //! no. of times cca got stuck
  unsigned short cca_stk;
  //! no of times cca didn't get stuck
  unsigned short cca_not_stk;
  //! no. of pkt aborts
  unsigned short pkt_abort;
  //! no. of false rx starts
  unsigned short fls_rx_start;
  //! cca idle time
  unsigned short cca_idle;
  //! no. of greenfield pkts
  unsigned short gf_pkts;
  //! no. of high throughput pkts
  unsigned short ht_pkts;
  //! no. of legacy pkts
  unsigned short leg_pkt;
  //! add description
  unsigned short leg_prty_fails;
  //! no. of ht pkts failing crc chk
  unsigned short ht_crc_fails;
  //! add description
  unsigned short sp_rejected;
  //! add description
  unsigned short lp_rejected;
  //! Channel 1 signal power
  unsigned short ch1_sig_pow;
  //! channel 1 noise power
  unsigned short ch1_noise_pow;
  //! channel 2 signal power
  unsigned short ch2_sig_pow;
  //! channel 2 noise power
  unsigned short ch2_noise_pow;
  //! channel 3 signal power
  unsigned short ch3_sig_pow;
  //! channel 3 noise power
  unsigned short ch3_noise_pow;
  //! no. of rx retries
  unsigned short rx_retries;
  //! rssi value
  unsigned short bea_avg_rssi;
  //! cal_rssi
  unsigned short cal_rssi;
  //! lna_gain bb_gain
  unsigned short lna_bb_gain;
  //! avg_val
  unsigned short avg_val;
  //! xretries pkts dropped
  unsigned short xretries;
  //! consecutive pkts dropped
  unsigned short max_cons_pkts_dropped;
  //!
  unsigned short false_under_sat;
  //!BSS MATCHED BROADCAST PKT STATS
  unsigned short bss_broadcast_pkts;
  //!BSS MATCHED MULTICAST PKT STATS
  unsigned short bss_multicast_pkts;
  //!BSS and DA MATCHED MULTICAST PKT STATS
  unsigned short bss_da_matched_multicast_pkts;
  unsigned int eof_pkt_drop_count;
  unsigned int mask_pkt_drop_count;
  unsigned int ack_sent;
  //!No.of pkts rcvd with 48M
  unsigned short pkt_rcvd_with_48M;
  //!No.of pkts rcvd with 24M
  unsigned short pkt_rcvd_with_24M;
  //!No.of pkts rcvd with 12M
  unsigned short pkt_rcvd_with_12M;
  //!No.of pkts rcvd with 6M
  unsigned short pkt_rcvd_with_6M;
  //!No.of pkts rcvd with 54M
  unsigned short pkt_rcvd_with_54M;
  //!No.of pkts rcvd with 36M
  unsigned short pkt_rcvd_with_36M;
  //!No.of pkts rcvd with 18M
  unsigned short pkt_rcvd_with_18M;
  //!No.of pkts rcvd with 9M
  unsigned short pkt_rcvd_with_9M;
  //!No.of pkts rcvd with 11M
  unsigned short pkt_rcvd_with_11M;
  //!No.of pkts rcvd with 5.5M
  unsigned short pkt_rcvd_with_5M;
  //!No.of pkts rcvd with 2M
  unsigned short pkt_rcvd_with_2M;
  //!No.of pkts rcvd with 1M
  unsigned short pkt_rcvd_with_1M;
  //!No.of pkts rcvd with mcs0
  unsigned short pkt_rcvd_with_mcs0;
  //!No.of pkts rcvd with mcs1
  unsigned short pkt_rcvd_with_mcs1;
  //!No.of pkts rcvd with mcs2
  unsigned short pkt_rcvd_with_mcs2;
  //!No.of pkts rcvd with mcs3
  unsigned short pkt_rcvd_with_mcs3;
  //!No.of pkts rcvd with mcs4
  unsigned short pkt_rcvd_with_mcs4;
  //!No.of pkts rcvd with mcs5
  unsigned short pkt_rcvd_with_mcs5;
  //!No.of pkts rcvd with mcs6
  unsigned short pkt_rcvd_with_mcs6;
  //!No.of pkts rcvd with mcs7
  unsigned short pkt_rcvd_with_mcs7;
  //__9117_CODE_START
  //!No.of pkts rcvd with 11ax SU PPDU Type
  unsigned short pkt_count_HE_SU_PPDU;
  //!No.of pkts rcvd with ER SU PPDU Type
  unsigned short pkt_count_HE_ER_SU_PPDU;
  //!No.of pkts rcvd with TB PPDU Type
  unsigned short pkt_count_HE_TRIG_PPDU;
  //!No.of pkts rcvd with MU PPDU Type
  unsigned short pkt_count_HE_MU_PPDU;
  //! Sum of all the pkts rcvd with SU, TB, MU PPDUs
  unsigned short pkt_count_11AC;
  //!sum of all the pkts transmitted
  unsigned short tx_done;
  //!No.of HE_MU_MIMO pkts rcvd
  unsigned short he_mu_mimo_pkt_cnt;
  //!No.of HE_MU_OFDMA pkts rcvd
  unsigned short he_mu_ofdma_pkt_cnt;
  //__9117_CODE_END
  /* Channel Utilization related stats */
  unsigned int utilization;
  unsigned int rssi_utilization;
  unsigned int tot_bytes;
  unsigned int rssi_bytes;
  unsigned int interval_duration;
  unsigned int false_cca_avg_rssi;
  unsigned int max_cca_avg_rssi;
  unsigned int cca_duration;
  unsigned int ed_duration;
  unsigned short int noise_rssi;
  int stop_per;
} per_stats;

//__9117_CODE_START
struct he_conf {
  unsigned char coding_type;
  unsigned char nominal_pe;
  unsigned char UL_DL;
  unsigned char he_ppdu_type;
  unsigned char beam_change;
  unsigned char BW;
  unsigned char STBC;
  unsigned char Tx_BF;
  unsigned char GI_LTF;
  unsigned char DCM;
  unsigned char NSTS_MIDAMBLE;
  unsigned char spatial_reuse;
  unsigned char BSS_color;
  unsigned short HE_SIGA2_RESERVED;
  unsigned char RU_ALLOCATION;
  unsigned char N_HELTF_TOT;
  unsigned char SIGB_DCM;
  unsigned char SIGB_MCS;
  unsigned short USER_STA_ID;
  unsigned char USER_IDX;
  unsigned char SIGB_COMPRESSION_FIELD;
};
//__9117_CODE_END

typedef struct per_params_s {
  unsigned short enable;
  signed short power;
  unsigned int rate;
  unsigned short pkt_length;
  unsigned short mode;
  unsigned short channel;
  unsigned short rate_flags;
  unsigned short per_ch_bw;
  unsigned short aggr_enable;
  unsigned short aggr_count;
  unsigned short no_of_pkts;
  unsigned int delay;
  unsigned char ctry_region;
  unsigned char enable_11j;
  //__9117_CODE_START
  uint8_t enable_11ax;
  struct he_conf heconf;
  //__9117_CODE_END
} per_params_t;

typedef struct per_packet_s {
  unsigned char enable;
  unsigned int length;
  unsigned char insert_seq;
  unsigned char packet[1536];
} per_packet_t;

#define PER_RECEIVE            2
#define PER_RCV_STOP           6
#define PER_TRANSMIT           1
#define PER_PACKET             8
#define PER_AGGR_LIMIT_PER_PKT 1792
#define WLAN_PACKET            1

#define NL_DATA_DESC_SZ 16
#define NETLINK_USER    31

#define RSI_RATE_00      0x00
#define RSI_RATE_1       0x0
#define RSI_RATE_2       0x2
#define RSI_RATE_5_5     0x4
#define RSI_RATE_11      0x6
#define RSI_RATE_6       0x8b
#define RSI_RATE_9       0x8f
#define RSI_RATE_12      0x8a
#define RSI_RATE_18      0x8e
#define RSI_RATE_24      0x89
#define RSI_RATE_36      0x8d
#define RSI_RATE_48      0x88
#define RSI_RATE_54      0x8c
#define RSI_RATE_MCS0    0x100
#define RSI_RATE_MCS1    0x101
#define RSI_RATE_MCS2    0x102
#define RSI_RATE_MCS3    0x103
#define RSI_RATE_MCS4    0x104
#define RSI_RATE_MCS5    0x105
#define RSI_RATE_MCS6    0x106
#define RSI_RATE_MCS7    0x107
#define RSI_RATE_MCS7_SG 0x307

#define RSI_RATE_1M    1
#define RSI_RATE_2M    2
#define RSI_RATE_5_5M  5.5
#define RSI_RATE_11M   11
#define RSI_RATE_6M    6
#define RSI_RATE_9M    9
#define RSI_RATE_12M   12
#define RSI_RATE_18M   18
#define RSI_RATE_24M   24
#define RSI_RATE_36M   36
#define RSI_RATE_48M   48
#define RSI_RATE_54M   54
#define RSI_RATE_6_5M  mcs0
#define RSI_RATE_13M   mcs1
#define RSI_RATE_19_5M mcs2
#define RSI_RATE_26M   mcs3
#define RSI_RATE_39M   mcs4
#define RSI_RATE_52M   mcs5
#define RSI_RATE_58_5M mcs6
#define RSI_RATE_65M   mcs7

struct rsi_nl_desc {
  uint16_t desc_word[8];
};

int socket_creation(void);

#define BIT(n) (1 << (n))
typedef unsigned char uint_8;
typedef char int_8;
typedef unsigned short uint_16;
typedef short int_16;
typedef unsigned int uint_32;
typedef int int_32;

/* EFUSE_MAP related define */
#define EFUSE_MAP 80
struct efuse_content_t {
  u_int16_t efuse_map_version;
  u_int16_t module_version;
  u_int16_t module_part_no;
  u_int16_t mfg_sw_version;
  u_int16_t module_type;
  u_int16_t chip_version;
  u_int16_t m4sb_cert_en;
  u_int16_t mfg_sw_subversion;
  u_int32_t chip_id_no;
  u_int16_t sdb_mode;
};

struct master_params_s {
  unsigned int address;
  unsigned short no_of_bytes;
  unsigned int data[32];
  unsigned int read_write;
};

typedef struct wlan_9116_features_s {
  u_int8_t pll_mode;
  u_int8_t rf_type;
  u_int8_t wireless_mode;
  u_int8_t afe_type;
  u_int8_t enable_ppe;
  u_int8_t dpd;
  u_int8_t SIFSTransmitenable; //sifs_tx_en
  uint32_t pwrsave_options;
} w_9116_features_t;

/* BB READ WRITE related define */
#define RSI_SET_BB_READ      0x01
#define RSI_SET_BB_WRITE     0x02
#define RSI_MULTIPLE_BB_READ 0x03
#define RSI_SET_BB_RF        5
#define MASTER_OPS           0xA1
#define SOC_REG_WRITE        18
#define SOC_REG_READ         19

struct bb_rf_param_t {
  unsigned short Data[1024];
  unsigned short no_of_fields;
  unsigned short no_of_values;
  unsigned char value;
  unsigned char soft_reset;
  unsigned char protocol_id;
};

struct bb_rf_param_bt_t {
  unsigned char value; //type is present here
  unsigned char no_of_fields;
  unsigned char no_of_values;
  unsigned char soft_reset;
  unsigned short Data[128];
};

/* Wlan Gain Table related define*/
#define UPDATE_WLAN_GAIN_TABLE   78
#define MAX_WLAN_GAIN_TABLE_SIZE (512 - 128 - 16 - 20 - 8)
typedef struct update_wlan_gain_table_s {
#define _2GHZ_BAND 1
#define _5GHZ_BAND 2
  uint8_t band;
#define BAND_WIDTH_40 1
#define BAND_WIDTH_20 0
  uint8_t bandwidth;
  uint16_t struct_size;
  uint32_t reserved;
  uint8_t structure[MAX_WLAN_GAIN_TABLE_SIZE];
} update_wlan_gain_table_t;

#define RSI_GET_RSSI 81
/*Filter broadcast frame */
#define RSI_FILTER_BCAST 79
struct fltr_bcast {
  unsigned char reserved;
  unsigned char filter_bcast_in_tim;
  unsigned short int beacon_drop_threshold;
  unsigned char filter_bcast_tim_till_next_cmd;
};

//__9117_CODE_START
#define SET_HW_BMISS_THRESHOLD 14
#define SET_KEEP_ALIVE_PERIOD  15
/* Real time stats defines*/
#define REAL_TIME_STATS 10
#define TX_STATS        1
#define RX_STATS        2
struct tx_status_info {
  uint16_t status_valid : 1;
  uint16_t success_valid : 1;
  uint16_t type_of_failure : 2;
  uint16_t response_type : 5;
  uint16_t last_pkt : 1;
  uint16_t rts_selfcts_status : 1;
  uint16_t underun : 1;
  uint16_t tx_qdma_idle : 1;
  uint16_t cca_not_sensed : 1;
  uint16_t rx_disabled : 1;
  uint16_t desc_prog_error : 1;
  uint16_t tack_received : 1;
  uint16_t ndp_11ah_received : 1;
  uint16_t tp_ppdu_txed : 1;
  uint16_t num_data_pkts_txed : 7;
  uint16_t all_ack_count : 1;
  uint16_t reserved : 5;
} __attribute__((__packed__));

typedef struct {
  unsigned short tx_stat_type : 4;
  unsigned short tx_stat_len : 12;
  unsigned char tx_que_no;
  unsigned char reserved1;
  unsigned int tx_real_timestamp;
  struct tx_status_info status_info;
  unsigned int tx_timestamp;
  unsigned long long ba_bitmap;
  unsigned short ba_seq_no;
  unsigned short pkt_start_seq_no;
  unsigned long long pkt_seq_no_bitmap;
  unsigned short tx_pkt_type;
  unsigned char tx_pe_applied;
  unsigned char tx_retry_count;
  unsigned short tx_power;
} __attribute__((__packed__)) real_time_tx_stats;

struct rx_status_info {
  uint16_t mpdu_hdr_len : 6;
  uint16_t pkt_type_is_data : 1;
  uint16_t reserved1 : 7;
  uint16_t ps_poll_received : 1;
  uint16_t mic_computed : 1;
  uint16_t ppe_stats_valid : 1;
  uint16_t reserved2 : 2;
  uint16_t TA_Match : 1;
  uint16_t first_MPDU : 1;
  uint16_t firt_msdu : 1;
  uint16_t last_msdu : 1;
  uint16_t A_MPDU : 1;
  uint16_t A_MSDU : 1;
  uint16_t cipher_suite_mismatch : 1;
  uint16_t icv_error : 1;
  uint16_t mic_error : 1;
  uint16_t crc_error : 1;
  uint16_t aborted : 1;
  uint16_t pkt_copmleted : 1;
} __attribute__((__packed__));

struct rx_pkt_type {
  uint16_t pkt_rate : 7;
  uint16_t pkt_mode : 2;
  uint16_t stbc_sgi : 3;
  uint16_t bandwidth : 2;
  uint16_t mode_11ax : 1;
  uint16_t mode_11ac : 1;
  uint16_t pkt_length : 16;
} __attribute__((__packed__));

typedef struct {
  unsigned short rx_stat_type : 4;
  unsigned short rx_stat_len : 12;
  unsigned char rx_pe_applied;
  unsigned char reserved2;
  unsigned int rx_real_timestamp;
  unsigned int rx_timestamp;
  struct rx_status_info status_info;
  struct rx_pkt_type pkt_type;
  unsigned int rssi;
} __attribute__((__packed__)) real_time_rx_stats;

int set_hw_bmiss_threshold_value(unsigned short int bmiss_threshold_value, int sock_fd);
int set_keep_alive_period(unsigned short int keep_alive_period, int sock_fd);
int real_time_stats_wrapper(int rltime_enable, unsigned long int rltime_timer, int sock_fd, int len);
int process_real_time_stats(real_time_tx_stats *tx_stats_info, real_time_rx_stats *rx_stats_info, int stat_type);
void print_mcs_rate(unsigned short pkt_rate);
void print_11g_11b_rate(unsigned short pkt_rate);
unsigned int validate_11ax_lsig_len(per_params_t *per_params);

#ifdef CONFIG_TWT_SUPPORT
/* TWT parameters */
#define TWT_CONFIG               11
#define TWT_STATUS               13
#define RESCHEDULE_TWT           17
#define TWT_AUTO_CONFIG          23
#define MAX_TWT_SUSPEND_DURATION 0x5265c00

//TWT STATUS CODES
#define TWT_DEVICE_NOT_IN_CONNECTED_STATE   0
#define TWT_SETUP_ERR_SESSION_ACTIVE        1
#define TWT_TEARDOWN_ERR_FLOWID_NOT_MATCHED 2
#define TWT_TEARDOWN_ERR_NOACTIVE_SESS      3
#define TWT_SETUP_SESSION_IN_PROGRESS       4
#define TWT_SESSION_SUCC                    5
#define TWT_UNSOL_SESSION_SUCC              6
#define TWT_SETUP_AP_REJECTED               7
#define TWT_SETUP_RSP_OUTOF_TOL             8
#define TWT_SETUP_RSP_NOT_MATCHED           9
#define TWT_SETUP_UNSUPPORTED_RSP           10
#define TWT_TEARDOWN_SUCC                   11
#define TWT_AP_TEARDOWN_SUCC                12
#define TWT_SETUP_FAIL_MAX_RETRIES_REACHED  13
#define TWT_INACTIVE_DUETO_ROAMING          14
#define TWT_INACTIVE_DUETO_DISCONNECT       15
#define TWT_INACTIVE_NO_AP_SUPPORT          16
#define TWT_RESCHEDULE_SUCC                 17
#define TWT_RESCHEDULE_FAIL                 18
#define TWT_AUTO_CONFIG_FAIL                19

// TWT request structure to configure a session
typedef struct rsi_twt_user_params_s {
  // Nominal minimum wake duration. Range : 0 - 255
  uint8_t wake_duration;
  // Wake duration tolerance. Range : 0 - 255
  uint8_t wake_duration_tol;
  // Wake interval exponent tolerance. Range : 0 - 31
  uint8_t wake_int_exp;
  // Wake interval exponent tolerance. Range : 0 - 31
  uint8_t wake_int_exp_tol;
  // Wake interval mantissa. Range : 0 - 65535
  uint16_t wake_int_mantissa;
  // Wake interval mantissa tolerance.  Range : 0 - 65535
  uint16_t wake_int_mantissa_tol;
  // Implicit TWT : 0 or 1
  uint8_t implicit_twt;
  // Un-announced TWT : 0 or 1
  uint8_t un_announced_twt;
  // Triggered TWT : 0 or 1
  uint8_t triggered_twt;
  // Negotiation Type : 0 - Individual TWT; 1 - Broadcast TWT
  uint8_t negotiation_type;
  // TWT Channel. Range : 0 - 7
  uint8_t twt_channel;
  // TWT protection : 0 or 1
  uint8_t twt_protection;
  // TWT Session Flow ID. 0 - 7 valid. 0xFF to disable all active TWT sessions.
  uint8_t twt_flow_id;
  // Restrict TX outside TSP : 0 or 1
  uint8_t restrict_tx_outside_tsp;
  // TWT Retry limit. Range : 0 - 15
  uint8_t twt_retry_limit;
  // TWT retry interval. Range : 5 - 255
  uint8_t twt_retry_interval;
  // TWT Request type. 0 - Request TWT; 1 - Suggest TWT; 2 - Demand TWT
  uint8_t req_type;
  // TWT Enable. 0 - TWT session teardown; 1 - TWT session setup
  uint8_t twt_enable;
  // Wake duration unit. 0 - 256 TU; 1 - 1024 TU
  uint8_t wake_duration_unit;
} rsi_twt_user_params;

typedef struct twt_status_resp {
  uint8_t twt_flow_id;
  uint8_t wake_duration;
  uint8_t wake_duration_unit;
  uint8_t implicit_twt;
  uint8_t triggered_twt;
  uint8_t un_announced_twt;
  uint8_t wake_int_exp;
  uint16_t wake_int_mantissa;
  uint8_t req_type;
} rsi_twt_status_resp;

// Enumeration defining actions related to Target Wake Time (TWT).
typedef enum {
  SUSPEND_INDEFINITELY, ///< Indicates the suspension of TWT for an indefinite period, effectively
                        //disabling TWT functionality until explicitly resumed.
  SUSPEND_FOR_DURATION, ///< Specifies that TWT should be suspended for a specified duration of time,
                        ///after which it can automatically resume.
  RESUME_IMMEDIATELY,   ///< Signifies an immediate resumption of TWT, allowing devices to continue
                        //adhering to TWT schedules.
} wifi_reschedule_twt_action_t;

typedef struct {
  uint8_t flow_id;
  wifi_reschedule_twt_action_t twt_action;
  uint16_t reserved1;
  uint8_t reserved2;
  uint64_t suspend_duration;
} wifi_reschedule_twt_config_t;

// Use case based twt selection
typedef struct twt_selection_s {
  uint8_t twt_enable;
  uint16_t avg_tx_throughput;
  uint32_t tx_latency;
  uint32_t rx_latency;
  uint16_t device_avg_throughput;
  uint8_t estimated_extra_wake_duration_percent;
  uint8_t twt_tolerable_deviation;
  uint32_t default_wake_interval_ms;
  uint32_t default_minimum_wake_duration_ms;
  uint8_t beacon_wake_up_count_after_sp;
} twt_selection_t;

#define DEVICE_AVG_THROUGHPUT                20000
#define ESTIMATE_EXTRA_WAKE_DURATION_PERCENT 0
#define TWT_TOLERABLE_DEVIATION              10
#define TWT_DEFAULT_WAKE_INTERVAL_MS         1024     // in milli seconds
#define TWT_DEFAULT_WAKE_DURATION_MS         8        // in milli seconds
#define MAX_TX_AND_RX_LATENCY_LIMIT          22118400 // 6hrs in milli seconds
#define MAX_BEACON_WAKE_UP_AFTER_SP \
  2 // The number of beacons after the service period completion for which the module wakes up and listens for any pending RX.
#define MIN_OF_2(X, Y)    (X > Y) ? Y : X
#define MIN_OF_3(A, B, C) (A > B) ? ((B > C) ? C : B) : ((A > C) ? C : A)

int check_twt_status(rsi_twt_status_resp *twt_resp, int sfd, struct nlmsghdr *nlh);
int twt_status_wrapper(int sock_fd, int len);
int twt_config_wrapper(rsi_twt_user_params twt_params, int sock_fd);
#endif

#define DIAG_TOOL               16
#define DIAGNOSTIC_REPORT       85
#define DIAG_TOOL_UPDATE        86
#define DIAG_READ               1
#define DIAG_WRITE              2
#define DPD_EN_GLOBAL           1
#define DPD_TEMP_CTRL_EN_GLOBAL 2
#define DIAG_TOOL_REPORT_DIR    "./reports/"
int diag_generate_reg_report(int sfd, char *ref_file, char *mode);
int send_query_register_to_drv(int sock_fd, unsigned int reg_addr, int no_of_bytes);
int diag_impu_flash_dump(int sfd, char *ifName);
int diag_rf_flash_dump(int sfd, char *ifName);
int diag_tool_param_update(int sock_fd, int diag_cmd, int diag_param_name, int diag_param_val);

//__9117_CODE_END
int gain_table_sckt_creation(update_wlan_gain_table_t table_info, int sfd);
int update_wlan_gain_table(int argc, char *argv[], char *ifName, int sfd);
int update_wlan_region_based_struct(int argc, char *argv[], char *ifName, int sfd);
struct nlmsghdr *common_recv_mesg_wrapper(int sock_fd, int len);
void usage();
int getcmdnumber(char *command, char *ifName);
int rsi_print_efuse_map(struct efuse_content_t *efuse_content, int sock_fd);
int ble_per_transmit_wrapper(struct bb_rf_param_bt_t bb_rf_params, int cmd, int sock_fd);
int per_recv_send_wrapper(per_stats *sta_info,
                          int cmd,
                          int len,
                          int stop_bit,
                          unsigned char rate_flags,
                          int freq,
                          int sock_fd);
int bb_read_write_wrapper(struct bb_rf_param_t bb_rf_params, int sock_fd);
int bb_rf_read_write_wrapper(struct bb_rf_param_t bb_rf_params, int sock_fd, unsigned short flags);
int master_read_write_wrapper(struct master_params_s master, int sock_fd, unsigned short read_write);
int prepare_and_send_nl_cmd(int sock_fd,
                            unsigned short cmd_type,
                            unsigned char rd_wr,
                            unsigned char msg_type,
                            void *ptr,
                            int len);
int_32 master_ops(int sfd, uint_8, uint_32, uint_8 *, uint_8);
int per_transmit_packet_wrapper(per_packet_t per_packet, int cmd, int sock_fd);
int per_transmit_wrapper(per_params_t per_params, int cmd, int sock_fd);
int send_get_rssi_frame_to_drv(int sock_fd);
int send_filter_broadcast_frame_to_drv(struct fltr_bcast bcast, int sock_fd);
int send_bb_read_write_frame_to_drv(struct bb_rf_param_bt_t bb_rf_params, int sfd);
#define ONEBOX_STATUS_FAILURE      -1
#define ONEBOX_STATUS_SUCCESS      0
#define ONEBOX_STATUS              int_32
#define ONEBOX_PRINT(fmt, args...) fprintf(stdout, fmt, ##args)

#endif

/*** MATLAB utils ***/
/*** SRC Destn Port And IP Defines ***/
#define SOURCE_PORT_NUM 6666
#define DESTN_PORT_NUM  6666
#define DESTN_IP        "192.168.70.222"

#define ONEBOX_MAX_PKT_LEN 6348 /* Bytes */

/* Read values for MATALB frame types should always be an EVEN NUMBER */

#define BB_READ           0x0
#define BB_WRITE          0x1
#define BUFFER_READ       0x6
#define BUFFER_WRITE      0x7
#define M_RX_STATS        0x8
#define M_TX_STATS        0xD
#define BB_READ_MULTIPLE  0x18
#define AFE_SPI_R_W       64
#define AHB_R_W           65
#define SOC_PLL_R_W       66
#define WLAN_9116_FEATURE 68
#define MASTER_READ       21
#define MASTER_WRITE      22
#define READ              0
#define WRITE             1
