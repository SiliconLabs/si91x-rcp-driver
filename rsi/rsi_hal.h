/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#ifndef __RSI_HAL_H__
#define __RSI_HAL_H__

/* Device Operating modes */
#define DEV_OPMODE_WIFI_ALONE     1
#define DEV_OPMODE_BT_ALONE       4
#define DEV_OPMODE_BT_LE_ALONE    8
#define DEV_OPMODE_BT_DUAL        12
#define DEV_OPMODE_STA_BT         5
#define DEV_OPMODE_STA_BT_LE      9
#define DEV_OPMODE_STA_BT_DUAL    13
#define DEV_OPMODE_AP_BT          6
#define DEV_OPMODE_AP_BT_LE       10
#define DEV_OPMODE_AP_BT_DUAL     14
#define DEV_OPMODE_ZB_ALONE       16
#define DEV_OPMODE_STA_ZB         17
#define DEV_OPMODE_ZB_COORDINATOR 32
#define DEV_OPMODE_ZB_ROUTER      48

/* ZigBee Operating Modes */
#define ZIGBEE_END_DEVICE  1
#define ZIGBEE_COORDINATOR 2
#define ZIGBEE_ROUTER      3

#define ZIGBEE_OPERMODE_MASK 0x3

#define TA_LOAD_ADDRESS            0x00
#define FIRMWARE_RSI9113           "rsi_91x.fw"
#define FLASH_WRITE_CHUNK_SIZE     (4 * 1024)
#define USB_FLASH_READ_CHUNK_SIZE  ((2 * 1024) - 4)
#define SDIO_FLASH_READ_CHUNK_SIZE (2 * 1024)
#define FLASH_SECTOR_SIZE          (4 * 1024)
#define STARTING_BLOCK_INDEX       0
#define FLASH_BLOCK_SIZE           (32 * 1024)

#define RS9116_RPS_HEADER_SIZE   0x40
#define PING_BUFFER_ADDRESS_9116 (0x18000 + 0x4)

#define FLASH_SIZE_ADDR 0x04000016

#ifdef CONFIG_RS9116_FLASH_MODE
#define PING_BUFFER_ADDRESS 0x18000
#define PONG_BUFFER_ADDRESS 0x19000
#else
#define PONG_BUFFER_ADDRESS_9116 0x19400
#define PING_BUFFER_ADDRESS      0x19000
#define PONG_BUFFER_ADDRESS      0x1a000
#endif

#define SWBL_REGIN          0x41050034
#define SWBL_REGOUT         0x4105003c
#define PING_WRITE          0x1
#define PONG_WRITE          0x2
#define FW_WDT_DISABLE_REQ  0x69
#define FW_WDT_DISABLE_DONE 0x96

#define BL_CMD_TIMEOUT  2000
#define BL_BURN_TIMEOUT (50 * 1000)

#define MASTER_READ_MODE 1
#define EEPROM_READ_MODE 2

#ifdef CONFIG_RS9116_FLASH_MODE
#define RSI_FLASH_READ_COEX_IMAGE  (0x04000000 + 0x80000 + 0x40)
#define RSI_FLASH_READ_WLAN_IMAGE  (0x04000000 + 0x20000 + 0x40)
#define RSI_LMAC_VER_OFFSET_RS9116 (0x22c0 + 0x40)
#define RSI_9116_FLASH_SIZE        (4 * 1024 * 1024)
#endif

#define REGIN_VALID    0xA
#define REGIN_INPUT    0xA0
#define REGOUT_VALID   0xAB
#define REGOUT_INVALID (~0xAB)
#define CMD_PASS       0xAA
#define CMD_FAIL       0xCC
#define INVALID_ADDR   0x4C
#define CHIP_REV_1P5   0x15

#define BURN_BL               0x23
#define LOAD_HOSTED_FW        'A'
#define BURN_HOSTED_FW        'B'
#define PING_VALID            'I'
#define PONG_VALID            'O'
#define PING_AVAIL            'I'
#define PONG_AVAIL            'O'
#define EOF_REACHED           'E'
#define CHECK_CRC             'K'
#define POLLING_MODE          'P'
#define CONFIG_AUTO_READ_MODE 'R'
#define JUMP_TO_ZERO_PC       'J'
#define FW_LOADING_SUCCESSFUL 'S'
#define LOADING_INITIATED     '1'
#define IMAGE_STORED_IN_DUMP  0xF7
//__9117_CODE_START
#define RSI_FW_BLOCK_SIZE      4096
#define RSI_FW_UPGRADE         'n'
#define LMAC_VER_OFFSET_RS9117 (0x22c0 + 0x80)
#define LMAC_VER_OFFSET_RS917  (0x40)
//__9117_CODE_END

/* Total RAM access commands from TA */
#define MEM_ACCESS_CTRL_FROM_HOST 0x41300000
#define RAM_384K_ACCESS_FROM_TA \
  (BIT(2) | BIT(3) | BIT(4) | BIT(5) | BIT(20) | BIT(21) | BIT(22) | BIT(23) | BIT(24) | BIT(25))

/* Boot loader commands */
#define HOST_INTF_REG_OUT                    0x4105003C
#define HOST_INTF_REG_IN                     0x41050034
#define BOARD_READY                          0xABCD
#define REG_READ                             0xD1
#define REG_WRITE                            0xD2
#define SEND_RPS_FILE                        '2'
#define BOOTUP_OPTIONS_LAST_CONFIG_NOT_SAVED 0xF1
#define BOOTUP_OPTIONS_CHECKSUM_FAIL         0xF2
#define INVALID_OPTION                       0xF3
#define CHECKSUM_SUCCESS                     0xAA
#define CHECKSUM_FAILURE                     0xCC
#define CHECKSUM_INVALID_ADDRESS             0x4C
#define REG_CHIP_REV                         0x04000437

#define EEPROM_VERSION_OFFSET   77
#define CALIB_CRC_OFFSET        4092
#define MAGIC_WORD              0x5A
#define MAGIC_WORD_OFFSET_1     40
#define MAGIC_WORD_OFFSET_2     424
#define FW_IMAGE_MIN_ADDRESS    (68 * 1024)
#define FLASH_MAX_ADDRESS       (4 * 1024 * 1024) //4MB
#define MAX_FLASH_FILE_SIZE     (400 * 1024)      //400K
#define FLASHING_START_ADDRESS  16
#define CALIB_VALUES_START_ADDR 16
#define SOC_FLASH_ADDR          0x04000000
#define EEPROM_DATA_SIZE        4096
#define CALIB_DATA_SIZE         (EEPROM_DATA_SIZE - CALIB_VALUES_START_ADDR)
#define BL_HEADER               32

#define BT_CARD_READY_IND         0x89
#define WLAN_CARD_READY_IND       0x0
#define COMMON_HAL_CARD_READY_IND 0x0
#define ZIGB_CARD_READY_IND       0xff

#define COMMAN_HAL_WAIT_FOR_CARD_READY       1
#define COMMON_HAL_SEND_CONFIG_PARAMS        2
#define COMMON_HAL_TX_ACCESS                 3
#define COMMON_HAL_WAIT_FOR_PROTO_CARD_READY 4
#define HEX_FILE                             1
#define BIN_FILE                             0
#define UNIX_FILE_TYPE                       8
#define DOS_FILE_TYPE                        9
#define LMAC_INSTRUCTIONS_SIZE               (16 * 1024) /* 16Kbytes */

#define ULP_RESET_REG           0x161
#define WATCH_DOG_TIMER_1       0x16c
#define WATCH_DOG_TIMER_2       0x16d
#define WATCH_DOG_DELAY_TIMER_1 0x16e
#define WATCH_DOG_DELAY_TIMER_2 0x16f
#define WATCH_DOG_TIMER_ENABLE  0x170

#define RESTART_WDT       BIT(11)
#define BYPASS_ULP_ON_WDT BIT(1)

#define RF_SPI_PROG_REG_BASE_ADDR 0x40080000

#define GSPI_CTRL_REG0 (RF_SPI_PROG_REG_BASE_ADDR)
#define GSPI_CTRL_REG1 (RF_SPI_PROG_REG_BASE_ADDR + 0x2)
#define GSPI_DATA_REG0 (RF_SPI_PROG_REG_BASE_ADDR + 0x4)
#define GSPI_DATA_REG1 (RF_SPI_PROG_REG_BASE_ADDR + 0x6)
#define GSPI_DATA_REG2 (RF_SPI_PROG_REG_BASE_ADDR + 0x8)

#define GSPI_DMA_MODE BIT(13)

#define GSPI_2_ULP         BIT(12)
#define GSPI_TRIG          BIT(7)
#define GSPI_READ          BIT(6)
#define GSPI_RF_SPI_ACTIVE BIT(8)

#define FW_FLASH_OFFSET        0x820
#define LMAC_VER_OFFSET        FW_FLASH_OFFSET + 0x200
#define LMAC_VER_OFFSET_RS9116 0x22c0

/* Buffer status register related info */
#define PKT_BUFF_SEMI_FULL 0
#define PKT_BUFF_FULL      1
#define PKT_MGMT_BUFF_FULL 2
#define MSDU_PKT_PENDING   3

#define NWP_AHB_BASE_ADDR          0x41300000
#define NWP_WWD_INTERRUPT_TIMER    (NWP_AHB_BASE_ADDR + 0x300)
#define NWP_WWD_SYSTEM_RESET_TIMER (NWP_AHB_BASE_ADDR + 0x304)
#define NWP_WWD_WINDOW_TIMER       (NWP_AHB_BASE_ADDR + 0x308)
#define NWP_WWD_TIMER_SETTINGS     (NWP_AHB_BASE_ADDR + 0x30C)
#define NWP_WWD_MODE_AND_RSTART    (NWP_AHB_BASE_ADDR + 0x310)
#define NWP_WWD_RESET_BYPASS       (NWP_AHB_BASE_ADDR + 0x314)
#define NWP_FSM_INTR_MASK_REG      (NWP_AHB_BASE_ADDR + 0x104)

#define LP_POWER_SAVE  1
#define ULP_POWER_SAVE 2

struct bl_header {
  __le32 flags;
  __le32 image_no;
  __le32 check_sum;
  __le32 flash_start_address;
  __le32 flash_len;
} __packed;

struct ta_metadata {
  char *name;
  unsigned int address;
};

#ifdef CONFIG_RSI_MULTI_MODE
extern atomic_t drv_instances[5];
#define DRV_INSTANCE(__x)          (atomic_read(&drv_instances[(__x)-1]))
#define DRV_INSTANCE_SET(__x, __y) (atomic_set(&drv_instances[((__x) ? (__x)-1 : 0)], (__y)))
#define MAX_INSTANCES              5
#endif

#define RSI_BL_CTRL_LEN_MASK          0xFFFFFF
#define RSI_BL_CTRL_SPI_32BIT_MODE    BIT(27)
#define RSI_BL_CTRL_REL_TA_SOFTRESET  BIT(28)
#define RSI_BL_CTRL_START_FROM_ROM_PC BIT(29)
#define RSI_BL_CTRL_SPI_8BIT_MODE     BIT(30)
#define RSI_BL_CTRL_LAST_ENTRY        BIT(31)

//__9117_CODE_START
/* 11ax parameters */
#define OFFSET_CODING_TYPE          0
#define OFFSET_NOMINAL_PE           1
#define OFFSET_SPATIAL_REUSE_ENABLE 3
#define OFFSET_HE_PPDU_TYPE         4
#define OFFSET_BEAM_CHANGE          6
#define OFFSET_UL_DL                7
#define OFFSET_STBC                 8
#define OFFSET_TX_BF                9
#define OFFSET_GI_LTF               10
#define OFFSET_DCM                  12
#define OFFSET_NSTS_MIDAMBLE        13
//__9117_CODE_END
struct bootload_entry {
  __le32 control;
  __le32 dst_addr; /* Destination address */
} __packed;

struct bootload_ds {
  __le16 fixed_pattern;
  __le16 offset;
  __le32 reserved;
  struct bootload_entry bl_entry[7];
} __packed;

int rsi_prepare_mgmt_desc(struct rsi_common *common, struct sk_buff *skb);
int rsi_prepare_data_desc(struct rsi_common *common, struct sk_buff *skb);
int rsi_hal_device_init(struct rsi_hw *adapter);
int rsi_send_data_pkt(struct rsi_common *common, struct sk_buff *skb);
int rsi_send_bt_pkt(struct rsi_common *common, struct sk_buff *skb);
int rsi_send_zb_pkt(struct rsi_common *common, struct sk_buff *skb);
#ifndef CONFIG_STA_PLUS_AP
int rsi_prepare_beacon(struct rsi_common *common, struct sk_buff *skb);
#else
int rsi_prepare_beacon(struct rsi_common *common, struct sk_buff *skb, struct ieee80211_vif *vif);
#endif
int rsi_deregister_bt(struct rsi_common *common);
int rsi_validate_oper_mode(u16 oper_mode);
#ifdef CONFIG_RSI_MULTI_MODE
int rsi_opermode_instances(struct rsi_hw *adapter);
#endif
#endif
