# SPDX-License-Identifier: GPL-2.0-only

SUPP_DIR=$(PWD)/supplicant
KERNELRELEASE=$(shell uname -r)
KERNELDIR=/lib/modules/$(KERNELRELEASE)/build

# uncomment below line for Caracalla board
#CONFIG_CARACALLA_BOARD=y

#uncomment below line for bluetooth Android support
#CONFIG_RSI_BT_ANDROID=y

# Uncomment below line for sdio inetrrupt polling
#CONFIG_SDIO_INTR_POLL=y

# Uncomment below line for BT alone (Classic/LE/Dual) mode
#CONFIG_RSI_BT_ALONE=y

# Uncomment below line for Wi-Fi BT coex mode
#CONFIG_RSI_COEX_MODE=y

# Uncomment below line for WLAN + Zigbee coex mode
#CONFIG_RSI_ZIGB=y

# Uncomment below line for using WoWLAN
#CONFIG_RSI_WOW=y

# Uncomment below line for using P2P support
#CONFIG_RSI_P2P=y

# Uncomment below line for using Concurrent mode
#CONFIG_STA_PLUS_AP=y

# Uncomment below line for platforms with no SDIO multiblock support
#CONFIG_RSI_NO_SDIO_MULTIBLOCK=y

# Uncomment below line for enabling RRM feature
#CONFIG_RSI_11K=y

# Uncomment below line for debugging RRM through debugs
#RSI_DEBUG_RRM=y

# Uncomment below line for Multi dev oper mode support
#CONFIG_RSI_MULTI_MODE=y

# Uncomment below line for 9116 flash based firmware loading
#CONFIG_RS9116_FLASH_MODE=y

#Uncomment below line for purism requirement \
also need to enabled CONFIG_RS9116_FLASH_MODE flag
#CONFIG_RS9116_PURISM=y

#Uncomment bolow line to load firmware from Flash \
also need to enabled CONFIG_RS9116_FLASH_MODE flag
#CONFIG_RSI_LOAD_FW_FROM_FLASH_ONLY=y
#__9117_CODE_START
#CONFIG_TWT_SUPPORT=y
#CONFIG_UORA_SUPPORT=y

#Uncomment below line for secure f/w load
#CONFIG_SECURE_FW_LOAD=y
#__9117_CODE_END

#Uncomment below line to ignore -84 sdio write failure error
COMMAND_84_ERROR_WAR=y

#Uncomment below line to offload scanning to driver as kernel verion above 4.20.17 \
mac80211 will do scan
OFFLOAD_SCAN_TO_DEVICE=y

EXTRA_CFLAGS += -DLINUX -Wimplicit -Wstrict-prototypes -Wall -Wno-implicit-fallthrough
EXTRA_CFLAGS += -I$(PWD)/include
EXTRA_CFLAGS += -DCONFIG_RSI_DEBUGFS

COMMON_SDIO_OBJS += rsi/rsi_91x_sdio_ops.o rsi/rsi_91x_sdio.o

RSI_91X_OBJS := rsi/rsi_91x_hal.o \
		  rsi/rsi_91x_main.o \
		  rsi/rsi_91x_mac80211.o \
		  rsi/rsi_91x_mgmt.o \
		  rsi/rsi_91x_core.o \
		  rsi/rsi_91x_ps.o \
		  rsi/rsi_91x_debugfs.o\
		  rsi/rsi_91x_nlsock.o\
		  rsi/rsi_91x_per.o
#__9117_CODE_START
RSI_91X_OBJS += rsi/rsi_91x_he.o
#__9117_CODE_END
ifeq ($(CONFIG_ARCH_HAVE_CUSTOM_GPIO_H), y)
RSI_91X_OBJS += rsi/rsi_91x_gpio.o
endif

ifeq ($(CONFIG_CARACALLA_BOARD), y)
EXTRA_CFLAGS += -DCONFIG_CARACALLA_BOARD
#EXTRA_CFLAGS += -DCONFIG_SDIO_INTR_POLL
endif

ifeq ($(CONFIG_RSI_MULTI_MODE), y)
EXTRA_CFLAGS += -DCONFIG_RSI_MULTI_MODE
endif

ifeq ($(CONFIG_SDIO_INTR_POLL), y)
EXTRA_CFLAGS += -DCONFIG_SDIO_INTR_POLL
endif

ifeq ($(CONFIG_RS9116_FLASH_MODE), y)
EXTRA_CFLAGS += -DCONFIG_RS9116_FLASH_MODE
endif

ifeq ($(CONFIG_RSI_BT_ALONE), y)
EXTRA_CFLAGS += -DCONFIG_RSI_BT_ALONE
RSI_91X_OBJS += rsi/rsi_91x_hci.o
endif

ifeq ($(CONFIG_RSI_COEX_MODE), y)
EXTRA_CFLAGS += -DCONFIG_RSI_COEX_MODE
RSI_91X_OBJS += rsi/rsi_91x_hci.o
RSI_91X_OBJS += rsi/rsi_91x_coex.o
endif

ifeq ($(CONFIG_RSI_ZIGB), y)
EXTRA_CFLAGS += -DCONFIG_RSI_ZIGB
RSI_91X_OBJS += rsi/rsi_91x_zigb.o
endif

ifeq ($(CONFIG_RSI_WOW), y)
EXTRA_CFLAGS += -DCONFIG_RSI_WOW
endif

ifeq ($(CONFIG_RSI_P2P), y)
EXTRA_CFLAGS += -DCONFIG_RSI_P2P
endif

ifeq ($(CONFIG_STA_PLUS_AP), y)
EXTRA_CFLAGS += -DCONFIG_STA_PLUS_AP
endif

ifeq ($(CONFIG_RSI_11K), y)
EXTRA_CFLAGS += -DCONFIG_RSI_11K
RSI_91X_OBJS += rsi/rsi_91x_rrm.o
endif

ifeq ($(RSI_DEBUG_RRM), y)
ifeq ($(CONFIG_RSI_11K), y)
EXTRA_CFLAGS += -DRSI_DEBUG_RRM
endif
endif

ifeq ($(CONFIG_RSI_NO_SDIO_MULTIBLOCK), y)
EXTRA_CFLAGS += -DCONFIG_RSI_NO_SDIO_MULTIBLOCK
endif

ifeq ($(CONFIG_RS9116_PURISM), y)
EXTRA_CFLAGS += -DCONFIG_RS9116_PURISM
endif

ifeq ($(CONFIG_RSI_LOAD_FW_FROM_FLASH_ONLY), y)
EXTRA_CFLAGS += -DCONFIG_RSI_LOAD_FW_FROM_FLASH_ONLY
endif

ifeq ($(CONFIG_RSI_BT_ANDROID), y)
EXTRA_CFLAGS += -DCONFIG_RSI_BT_ANDROID
RSI_91X_OBJS += rsi/rsi_91x_bluedroid.o
endif
#__9117_CODE_START
ifeq ($(CONFIG_UORA_SUPPORT), y)
EXTRA_CFLAGS += -DCONFIG_UORA_SUPPORT
endif

ifeq ($(CONFIG_TWT_SUPPORT), y)
EXTRA_CFLAGS += -DCONFIG_TWT_SUPPORT
endif

ifeq ($(CONFIG_SECURE_FW_LOAD), y)
EXTRA_CFLAGS += -DCONFIG_SECURE_FW_LOAD
endif
#__9117_CODE_END

ifeq ($(COMMAND_84_ERROR_WAR), y)
EXTRA_CFLAGS += -DCOMMAND_84_ERROR_WAR
endif

ifeq ($(OFFLOAD_SCAN_TO_DEVICE), y)
EXTRA_CFLAGS += -DOFFLOAD_SCAN_TO_DEVICE
endif

obj-m := rsi_sdio.o rsi_91x.o
rsi_sdio-objs := $(COMMON_SDIO_OBJS)
rsi_91x-objs := $(RSI_91X_OBJS)

all:
	@echo -e "\033[32mCompiling RSI drivers...\033[0m"
	make -C$(KERNELDIR)/ M=$(PWD) modules
#__9117_CODE_START
	mv *.ko release/
#__9117_CODE_END
	make CC="$(CC)" ROOT_DIR=$(ROOT_DIR) -C $(PWD)/apps
	@echo -e "\033[32mCompilation done SUCCESSFULLY\033[0m"

clean:
	make -C$(KERNELDIR)/ M=$(PWD) clean
	@find . -name 'receive' | xargs rm -rf
	@find . -name 'bt_util' | xargs rm -rf
	@find . -name 'onebox_util' | xargs rm -rf
	@find . -name 'transmit' | xargs rm -rf
	@find . -name 'transmit_packet' | xargs rm -rf
