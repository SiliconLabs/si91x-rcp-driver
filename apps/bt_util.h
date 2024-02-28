/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#ifndef _BT_UTIL_H_
#define _BT_UTIL_H_

#include <arpa/inet.h>
#include "bt_util.h"
#include <sys/socket.h>
#include <linux/genetlink.h>

typedef unsigned char uint_8, uint8;
typedef char int_8, int8;
typedef unsigned short uint_16, uint16;
typedef short int_16, int16;
typedef unsigned int uint_32, uint32;
typedef int int_32, int32;
#define NL_DATA_DESC_SZ 16
#define NETLINK_USER    31
#define SLEEP_TIME      998752

int socket_creation(void);
int common_send_mesg_wrapper(int sock_fd, struct sockaddr_nl dest_addr, struct nlmsghdr *nlh);

/*Link Type*/
#define SCO_LINK  0
#define ACL_LINK  1
#define ESCO_LINK 2

/*Data Rate */
#define ONE_MBPS   1
#define TWO_MBPS   2
#define THREE_MBPS 3

/* Hopping Mode */
#define FREQ_NO_HOP    0
#define FREQ_FIXED_HOP 1
#define FREQ_RAND_HOP  2

/* LE Phy Rate */
#define BLE_1MBPS      1
#define BLE_2MBPS      2
#define BLR_500KBPS    4
#define BLR_125KBPS    8
#define BLR_BLE_SIM_RX 20

/* BT Operating Modes */
#define BT_CLASSIC_MODE 1
#define BT_LE_MODE      2
#define BT_DUAL_MODE    3

/* BT LE rf channels */
#define BT_LE_START_CHANNEL 0
#define BT_LE_END_CHANNEL   39

/* BT Classic Packet Types */
#define BT_DM1_PKT_TYPE  0x3
#define BT_DH1_PKT_TYPE  0x4
#define BT_DM3_PKT_TYPE  0xA
#define BT_DH3_PKT_TYPE  0xB
#define BT_DM5_PKT_TYPE  0xE
#define BT_DH5_PKT_TYPE  0xF
#define BT_2DH1_PKT_TYPE 0x4
#define BT_2DH3_PKT_TYPE 0xA
#define BT_2DH5_PKT_TYPE 0xE
#define BT_3DH1_PKT_TYPE 0x8
#define BT_3DH3_PKT_TYPE 0xB
#define BT_3DH5_PKT_TYPE 0xF
#define BT_HV1_PKT_TYPE  0x5
#define BT_HV2_PKT_TYPE  0x6
#define BT_HV3_PKT_TYPE  0x7
#define BT_DV_PKT_TYPE   0x8
#define BT_EV3_PKT_TYPE  0x7
#define BT_2EV3_PKT_TYPE 0x6
#define BT_3EV3_PKT_TYPE 0x7
#define BT_EV4_PKT_TYPE  0xC
#define BT_2EV5_PKT_TYPE 0xC
#define BT_EV5_PKT_TYPE  0xD
#define BT_3EV5_PKT_TYPE 0xD

/* BT Classic Max Packet Lengths */
#define BT_DM1_PAYLOAD_MAX_LEN    17
#define BT_DM3_PAYLOAD_MAX_LEN    121
#define BT_DM5_PAYLOAD_MAX_LEN    224
#define BT_DH1_PAYLOAD_MAX_LEN    27
#define BT_DH3_PAYLOAD_MAX_LEN    183
#define BT_DH5_PAYLOAD_MAX_LEN    339
#define BT_2DH1_PAYLOAD_MAX_LEN   54
#define BT_2DH3_PAYLOAD_MAX_LEN   367
#define BT_2DH5_PAYLOAD_MAX_LEN   679
#define BT_3DH1_PAYLOAD_MAX_LEN   83
#define BT_3DH3_PAYLOAD_MAX_LEN   552
#define BT_3DH5_PAYLOAD_MAX_LEN   1021
#define BT_HV1_VOICE_PAYLOAD_LEN  10
#define BT_HV2_VOICE_PAYLOAD_LEN  20
#define BT_HV3_VOICE_PAYLOAD_LEN  30
#define BT_EV3_VOICE_PAYLOAD_LEN  30
#define BT_2EV3_VOICE_PAYLOAD_LEN 60
#define BT_3EV3_VOICE_PAYLOAD_LEN 90
#define BT_EV4_VOICE_PAYLOAD_LEN  120
#define BT_EV5_VOICE_PAYLOAD_LEN  180
#define BT_2EV5_VOICE_PAYLOAD_LEN 360
#define BT_3EV5_VOICE_PAYLOAD_LEN 540

#define DISABLE_SKIP_PROG 0
#define ENABLE_SKIP_PROG  1

#define DISABLE_LOOP_BACK 0
#define ENABLE_LOOP_BACK  1

#define PLL_MODE0 0
#define PLL_MODE1 1
#define PLL_MODE2 2

#define EXTERNAL_RF 0
#define INTERNAL_RF 1

#define WLAN_HP_CHAIN                        0
#define WLAN_LP_CHAIN                        1
#define BT_HP_CHAIN                          2
#define BT_LP_CHAIN                          3
#define BT_LP_CHAIN_THRESHOLD                63
#define BT_LP_CHAIN_MIN_INVALID_TX_POWER_IDX 0
#define BT_LP_CHAIN_MAX_INVALID_TX_POWER_IDX 32

/* Duty Cycle Defines */
#define DISABLE_DUTY_CYCLE 0
#define ENABLE_DUTY_CYCLE  1

#define BT_BER_HDR_LEN 10

struct get_info {
  u_int8_t param_name[16];
  u_int8_t param_length;
  u_int8_t *data;
};

typedef struct driver_data {
  unsigned short val;
} dummy_data_t;

typedef struct bt_per_stats_s {
  unsigned short crc_fail;
  unsigned short crc_pass;
  unsigned short tx_aborts;
  unsigned short cca_stk;
  unsigned short cca_not_stk;
  unsigned short fls_rx_start;
  unsigned short cca_idle;
  unsigned short tx_dones;
  signed short int rssi;
  unsigned short id_pkts_rcvd;
} bt_per_stats_t;

typedef struct bt_ber_stats_s {
  unsigned short length;
  unsigned char data[1032];
  unsigned short num_pkts;
} __attribute__((packed)) bt_ber_stats_t;

typedef struct bt_ber_pkt_cnt_s {
  unsigned short num_pkts;
} __attribute__((packed)) bt_ber_pkt_cnt_t;

typedef struct __attribute__((__packed__)) bt_per_params_s {
  unsigned char enable;
  unsigned char bt_addr[6];
  unsigned char pkt_type;
  unsigned short int pkt_length;
  unsigned char link_type;
  unsigned char edr_indication;
  unsigned char bt_rx_rf_chnl;
  unsigned char bt_tx_rf_chnl;
  unsigned char scrambler_seed;
  unsigned int num_pkts;
  unsigned char payload_data_type;
  unsigned char mode;
  unsigned char le_chnl;
  unsigned char tx_pwr_indx;
  unsigned char transmit_mode;
  unsigned char enable_hopping;
  unsigned char ant_select;
} bt_per_params_t;

typedef struct __attribute__((__packed__)) bt_afh_params_s {
  unsigned char protocol_mode;
  unsigned char afh_map[10];
} bt_afh_params_t;

typedef struct __attribute__((__packed__)) le_afh_params_s {
  unsigned char protocol_mode;
  unsigned char afh_map[5];
} le_afh_params_t;

typedef struct __attribute__((__packed__)) bt_br_edr_per_params_s {
  unsigned char enable;
  unsigned char bt_addr[6];
  unsigned short int pkt_length;
  unsigned char pkt_type;
  unsigned char br_edr_indication;
  unsigned char bt_rx_rf_chnl;
  unsigned char bt_tx_rf_chnl;
  unsigned char link_type;
  unsigned char scrambler_seed;
  unsigned char enable_hopping;
  unsigned char ant_select;
  unsigned char pll_mode;
  unsigned char rf_type;
  unsigned char rf_chain;
  unsigned char payload_data_type;
  unsigned char tx_pwr_indx;
  unsigned char transmit_mode;
  unsigned char inter_pkt_gap;
  unsigned char reserved;
  unsigned int num_pkts;
} bt_br_edr_per_params_t;

typedef struct __attribute__((__packed__)) ble_per_params_s {
  unsigned char enable;
  unsigned char access_addr[4];
  unsigned char ble_rate;
  unsigned char bt_rx_rf_chnl;
  unsigned char bt_tx_rf_chnl;
  unsigned char scrambler_seed;
  unsigned char le_chnl;
  unsigned char hopping_type;
  unsigned char ant_select;
  unsigned char pll_mode;
  unsigned char rf_type;
  unsigned char rf_chain;
  unsigned short int pkt_length;
  unsigned char payload_data_type;
  unsigned char tx_pwr_indx;
  unsigned char transmit_mode;
  unsigned char inter_pkt_gap;
  unsigned int num_pkts;
} ble_per_params_t;

typedef struct __attribute__((__packed__)) bt_per_recv_params_s {
  unsigned char enable;
  unsigned char bt_addr[6];
  unsigned char link_type;
  unsigned char pkt_type;
  unsigned short int pkt_length;
  unsigned char scrambler_seed;
  unsigned char edr_indication;
  unsigned char bt_rx_rf_chnl;
  unsigned char bt_tx_rf_chnl;
  unsigned char le_mode;
  unsigned char le_chnl;
  unsigned char enable_hopping;
  unsigned char ant_sel;
} bt_per_recv_params_t;

typedef struct __attribute__((__packed__)) bt_br_edr_per_recv_params_s {
  unsigned char enable;
  unsigned char bt_addr[6];
  unsigned short int pkt_length;
  unsigned char pkt_type;
  unsigned char br_edr_indication;
  unsigned char bt_rx_rf_chnl;
  unsigned char bt_tx_rf_chnl;
  unsigned char link_type;
  unsigned char scrambler_seed;
  unsigned char enable_hopping;
  unsigned char ant_sel;
  unsigned char pll_mode;
  unsigned char rf_type;
  unsigned char rf_chain;
  unsigned char loop_back_mode;
} bt_br_edr_per_recv_params_t;

typedef struct __attribute__((__packed__)) ble_per_recv_params_s {
  unsigned char enable;
  unsigned char access_addr[4];
  unsigned char ble_rate;
  unsigned char bt_rx_rf_chnl;
  unsigned char bt_tx_rf_chnl;
  unsigned char scrambler_seed;
  unsigned char le_chnl_type;
  unsigned char freq_hop_en;
  unsigned char ant_sel;
  unsigned char pll_mode;
  unsigned char rf_type;
  unsigned char rf_chain;
  unsigned char data_length_indication;
  unsigned char loop_back_mode;
  unsigned char pwrsave_options;
} ble_per_recv_params_t;

typedef struct bt_per_packet_s {
  unsigned char enable;
  unsigned int length;
  unsigned char packet[1024];
} bt_per_packet_t;

typedef struct __attribute__((__packed__)) ble_aoa_aod_transmit_per_params_s {
  unsigned char enable;
  unsigned char pkt_length;
  unsigned char phy_rate;
  unsigned char bt_tx_rf_chnl;
  unsigned char scrambler_seed;
  unsigned char payload_data_type;
  unsigned char supp_length;
  unsigned char supp_slot_type;
  unsigned char num_antenna;
  unsigned char ant_switch_pattern;
  unsigned char le_chnl;
  unsigned char tx_pwr_indx;
  unsigned char enable_hopping;
  unsigned char ant_select;
} ble_aoa_aod_transmit_per_params_t;

typedef struct __attribute__((__packed__)) ble_aoa_aod_receive_per_params_s {
  unsigned char enable;
  unsigned char phy_rate;
  unsigned char bt_rx_rf_chnl;
  unsigned char scrambler_seed;
  unsigned char supp_length;
  unsigned char supp_slot_type;
  unsigned char num_antenna;
  unsigned char ant_switch_pattern;
  unsigned char le_chnl;
  unsigned char enable_hopping;
  unsigned char ant_select;
} ble_aoa_aod_receive_per_params_t;

typedef struct __attribute__((__packed__)) ant_transmit_params_s {
  unsigned char enable;
  unsigned char dtm_mode;
  unsigned int mode_flags;
  unsigned char rf_channel;
  unsigned char tx_power;
  unsigned char sync_word[4];
  unsigned short int pkt_length;
} ant_transmit_params_t;

typedef struct ant_stats_s {
  unsigned short crc_fail;
  unsigned short crc_pass;
  unsigned short tx_aborts;
  unsigned short cca_stk;
  unsigned short cca_not_stk;
  unsigned short fls_rx_start;
  unsigned short cca_idle;
  unsigned short tx_dones;
  signed short int rssi;
  unsigned short id_pkts_rcvd;
} ant_stats_t;

typedef struct __attribute__((__packed__)) atm_data_s {
  unsigned char data_length;
  unsigned char data[254];
} atm_data_t;

typedef struct __attribute__((__packed__)) ant_encryption_s {
  unsigned char encryption_offset;
  unsigned char encryption_key[16];
  unsigned char encryption_ctr[16];
} ant_encryption_t;

typedef struct bt_stats_s {
  unsigned short crc_pass;
  unsigned short crc_fail;
  unsigned short tx_aborts;
  unsigned short cca_stk;
  unsigned short cca_idle;
  unsigned short fls_rx_start;
  unsigned short rx_phy_abort;
  unsigned short tx_dones;
  unsigned short fifo_occupied;
  signed short int rssi;
  unsigned short id_pkts_rcvd;
} bt_stats_t;

#define ONEBOX_PRINT(fmt, args...) fprintf(stdout, fmt, ##args)
#define ONEBOX_PRINT_INFO(a, fmt) \
  if (a)                          \
    printf(fmt);

/* Function prototypes */
void usage();
int get_bt_cmdnumber(char *command);
int rsi_netlink_init(void);

void get_driver_state(char *state);
int bt_ber(void);
int bt_stats(unsigned char *, int sfd, int no_of_packets);

#define ONEBOX_STATUS_FAILURE -1
#define ONEBOX_STATUS_SUCCESS 0
#define ONEBOX_STATUS         int_32

#define ADDR0 0x315
#define ADDR1 0x316
#define ADDR2 0x317

#define BIT(n) (1 << (n))

/* Common Frame nos for matlab_utils.c and bt_util.c */
#define BT_E2E_STAT  0x13
#define BT_E2E_STATS 0x26
#define BT_PACKET    2

#define BB_READ           0x0
#define BB_WRITE          0x1
#define RF_READ           0x2
#define RF_WRITE          0x3
#define BT_PER_TRANSMIT   0x4
#define BT_RECEIVE        0x5
#define BUFFER_READ       0x6
#define BUFFER_WRITE      0x7
#define BT_PER_STATS      0x8
#define ANT_SEL           0x9
#define BT_BER_PKT_CNT    0xA
#define BT_BER_RECEIVE    0xB
#define BT_BER_MODE       0xC
#define BT_CW_MODE        0xD
#define TX_STATUS         0xE
#define GET_DRV_COEX_MODE 0xF
#define BT_PER_AFH_MAP    0x10
#define BT_STATS          0x11

#define BT_PER  0x10
#define BT_BER  0x11
#define BT_CW   0x12
#define ANT_PER 0x20

#define BT_BLE_USER_GAIN_TABLE 0x06

#define PER_BLE_TRANSMIT    0x13
#define PER_BLE_RECEIVE     0x14
#define PER_BR_EDR_TRANSMIT 0x15
#define PER_BR_EDR_RECEIVE  0x16

#define BLE_AOA_AOD_TRANSMIT 0x17
#define BLE_AOA_AOD_RECEIVE  0x18
#define ANT_TX               0x20
#define ATM_DATA             0x23
#define ANT_ENCRYPTION       0x24
//Remove if not needed in application layer
#define TX_IS_IN_PROG     -1
#define TX_IS_NOT_IN_PROG 1

#define RSI_SET_BB_READ         0x01
#define RSI_SET_BB_WRITE        0x02
#define RSI_RF_WRITE            0x03
#define RSI_RF_READ             0x04
#define RSI_ANT_SEL             0x05
#define RSI_SET_CW_MODE         0x06
#define RSI_GET_BT_STATS        0x07
#define RSI_BT_BER              0x08
#define RSI_SET_AFH_MAP         0x09
#define RSI_GET_BT_PERIOD_STATS 0x0A

#define RSI_GET_BT_E2E_STATS        0x0B
#define RSI_GET_BT_E2E_PERIOD_STATS 0x0C

/* Common Frame nos for matlab_utils.c and bt_util.c */
#define BT_PACKET  2
#define BLE_PACKET 3

int_32 bt_e2e_stats(uint_8 *file_name, int sfd);
int_32 bt_e2e_period_stats(uint_8 *file_name, int period, int sfd);
#endif
