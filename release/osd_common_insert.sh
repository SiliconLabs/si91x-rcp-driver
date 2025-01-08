
#!/bin/bash/

cat /dev/null > /var/log/messages


#Driver Mode 1 END-TO-END mode, 
#            2 RF Evaluation Mode

DRIVER_MODE=1

# DEV_OPER_MODE:                  
#							1    WLAN STATION /WIFI-Direct/WLAN PER/WLAN ACCESS POINT + STATION MODE

#							8    BT LE MODE /BT LE PER MODE
#							9    WLAN STATION + BT LE MODE

DEV_OPER_MODE=1



#Power Save Feature

#U-APSD Power Save                                  	
#0 - Disable U-APSD mode							
#1 - Enable U-APSD mode
ENABLED_UAPSD=0

#Rx data inactive interval for enhanced max psp
RX_DATA_INACTIVE_INTERVAL=0

#Max Service Period length
#0-All buffered packets will be delivered(Default).
#1-Two buffered packets will be delivered.
#2-Four buffered packets will be delivered.
#3-Six buffered packets will be delivered.
MAX_SP_LEN=0 				

#0 - No handshake Mode (Default)
#1 - GPIO Handshake Mode
LP_HANDSHAKE_MODE=0			

############################################################################################
#                                BLE RELATED MODULE PARAMS                              #
############################################################################################

#0 - EXTERNAL RF
#1 - INTERNAL RF (Default)
BT_RF_TYPE=1
				
#Range for the BLE Tx LP Chain Power, Index is 1 - 63 (0, 32 are invalid)
#Range for the BLE Tx HP Chain Power, Index is 64 - 76
#Default value = 30
BLE_TX_PWR_INX=30                              	

#BLE_DUTY_CYCLING   BIT(0)
#BLR_DUTY_CYCLING   BIT(1)
#BLE_PWR_SAVE_4X_MODE BIT(2)
BLE_POWER_SAVE_OPTIONS=2

#Ble Roles                       
#BIT[3:0] - Number of Central Roles Allowed
#BIT[7:4] - Number of Peripheral Roles Allowed				 							 
BLE_ROLES=19						

BLE_VENDOR_FEATURE_BITMAP=0x00  #BIT(0) SCAN_RESP_DISABLE

BT_RF_TX_POWER_MODE=0

BT_RF_RX_POWER_MODE=0

#BIT[0] - For Enabling role switch from host set this bit to 1
#BIT[1] - For Enabling sniff from host set this bit to 1
#BIT[5] - For Enabling BT Spoof MAC address i.e to use HARDCODE_MAC_ADDR in BT this bit should be set to #1
#BIT[7:3] - Reserved
BT_FEATURE_BITMAP=0 					



###################################################################################################
#                                   MISCELLANEOUS FEATURES                                        #
###################################################################################################

#SDIO Clock speed
#SDIO_CLOCK_SPEED=25
SDIO_CLOCK=50

#In GPIO handshake it is used to configure Host GPIO read pin. It will vary form platform to platform.Default value is 0xFF.
ULP_GPIO_READ=0xFFF					

#In GPIO handshake it is use to configure Host GPIO write pin. It will vary form platform to platform.Default value is 0xFF
ULP_GPIO_WRITE=0xFFF					

#Developer Mode Configuration Parameters

#Power Save options
#0 - Disable Duty Cycling & Undestined Packet Drop
#1 - Enable Duty Cycling
#2 - Enable Undestined Packet Drop
#3 - Enable Duty Cycling & Undestined Packet Drop (Default)
POWER_SAVE_OPT=3

#LP/HP Chain Selection in standby associated mode
#0 - HP Chain Enabled
#1 - LP Chain Enabled(Default)												
STANDBY_ASSOC_CHAIN_SEL=1      			

#9116 Feature Bitmap						
#BIT(0): Disable auto rate enhancements
#BIT(1): 1.8V enable
#BIT(2): Reserved
#BIT(3): Airplane mode				
FEATURE_BITMAP_9116=0										

#LMAC BEACON DROP Feature Options
#0 - Disable LMAC BEACON DROP Feature
#1 - Enable LMAC BEACON DROP Feature
LMAC_BCON_DROP=1   					

ANCHOR_POINT_GAP=1

#Sleep clock selection
#0 - Use RC clock as sleep clock
#1 - Use 32KHz clock from external XTAL OSCILLATOR
#2 - Use 32KHz bypass clock on UULP_GPIO_3
#3 - Use 32KHz bypass clock on UULP_GPIO_4
#Default value = 0
SLEEP_CLK_SOURCE_SEL=0      											

#sleep indication from device to host.
#0 - UULP_GPIO_3
#1 - UULP_GPIO_0
#Default value is 0.
SLEEP_IND_GPIO_SEL=1
							
#Host Interface on Demand Feature has the following possible values.
#0 - Disable Host Interface on Demand Feature
#1 - Enable Host Interface on Demand Feature
#Default value for Host Interface on Demand Feature Options is 0, which indicates that Host Interface on Demand Feature is disabled.					
HOST_INTF_ON_DEMAND=0					

#COUNTRY Selection
#0 - World Domain
#840 - US Domain Maps to US Region
#276 - Germany Maps to EU Region
#392 - Japan Maps to Japan Region
COUNTRY_CODE=0

#PTA Configuration
#0 - PTA_Disabled
#1 - PTA_CONFIG_1
#2 - PTA_CONFIG_2
#3 - PTA_CONFIG_3
PTA_CONFIG=0

#Extended Options.
#ext_opt value should be  0,1,2 or 3 as per front end switch selection.
#0 - Reserved
#1 - External Switch (ULP-GPIO 4, ULP-GPIO 5, ULP-GPIO 0)
#2 - Virtual/Internal Switch
#3 - Reserved
EXT_OPT=1

#Driver Logs Enable.
#BIT(0) - ERROR ZONE
#BIT(1) - INFO ZONE,  BIT(2) - INIT ZONE, BIT(3) - MGMT TX ZONE
#BIT(4) - MGMT RX ZONE, BIT(5) - DATA TX ZONE, BIT(6) - DATA RX ZONE
#BIT(7) - FSM ZONE, BIT(8) - ISR ZONE, BIT(9) - INT_MGMT_ZONE
#BIT(10) - MGMT_DEBUG_ZONE
RSI_ZONE_ENABLED=0x1


################################################################################
#                          917 SPECIFIC MODULE PARAMS                          #
################################################################################

#User based ng and cb enable
#'0' for disable and '1' for enable
ENABLE_NG_CB=0 

#ng (4bits MSB) and cb (4bits LSB)
#'0x00'  - ng is 0 and cb is 0
#'0x01'  - ng is 0 and cb is 1
#'0x10'  - ng is 1 and cb is 0
#'0x11'  - ng is 1 and cb is 1
VALUE_OF_NG_CB=0

#Trigger_Response_Indication
#'BIT(0)'  - Trigger Resoponse For BE
#'BIT(1)'  - Trigger Response For BK
#'BIT(2)'  - Trigger Response For VI
#'BIT(3)'  - Trigger Response For VO
TRIGGER_RESP_IND=0xF

#Gaurd interval
#'0' - 1x(3.2) HE_LTF + 0.8 GI
#'1' - 2x(6.4) HE_LTF + 0.8 GI
#'2' - 2x(6.4) HE_LTF + 1.6 GI
#'3' - 4x(12.8) HE_LTF + 3.2 GI
GUARD_INTERVAL=3

#Nominal Packet extention
#'0' for  0us
#'1' for 8us
#'2' for 16us
NOMINAL_PE=2

#DCM Enable
#'0' for Disabling DCM
#'1' for Enabling DCM
DCM_ENABLE=0

#LDPC Enable
#'0' for Disabling LDPC
#'1' for Enabling LDPC
LDPC_ENABLE=0

#ER_SU_CONFIG extended range single user configs
#'0' NO ER_SU support
#'1' Use ER_SU rates along with Non_ER_SU rates
#'2' Use ER_SU rates only
CONFIG_ER_SU=0

################################################################################
#                           END OF 917 SPECIFIC MODULE PARAMS                  #
################################################################################




PARAMS=" driver_mode_value=$DRIVER_MODE"
PARAMS=$PARAMS" dev_oper_mode=$DEV_OPER_MODE"
PARAMS=$PARAMS" rsi_zone_enabled=$RSI_ZONE_ENABLED"


#OSD Module Params
#PARAMS=$PARAMS" host_intf_on_demand=$HOST_INTF_ON_DEMAND"
#PARAMS=$PARAMS" ext_opt=$EXT_OPT"
#PARAMS=$PARAMS" country_code=$COUNTRY_CODE"
#PARAMS=$PARAMS" pta_config=$PTA_CONFIG"
#PARAMS=$PARAMS" sleep_clk_source_sel=$SLEEP_CLK_SOURCE_SEL"
#PARAMS=$PARAMS" sleep_ind_gpio_sel=$SLEEP_IND_GPIO_SEL"
#PARAMS=$PARAMS" anchor_point_gap=$ANCHOR_POINT_GAP"
#PARAMS=$PARAMS" lmac_bcon_drop=$LMAC_BCON_DROP"
#PARAMS=$PARAMS" feature_bitmap_9116=$FEATURE_BITMAP_9116"
#PARAMS=$PARAMS" standby_assoc_chain_sel=$STANDBY_ASSOC_CHAIN_SEL"
#PARAMS=$PARAMS" pwr_save_opt=$POWER_SAVE_OPT"
#PARAMS=$PARAMS" ulp_gpio_write=$ULP_GPIO_WRITE"
#PARAMS=$PARAMS" ulp_gpio_read=$ULP_GPIO_READ"
#PARAMS=$PARAMS" bt_feature_bitmap=$BT_FEATURE_BITMAP"
#PARAMS=$PARAMS" ble_vendor_feature_bitmap=$BLE_VENDOR_FEATURE_BITMAP"
#PARAMS=$PARAMS" bt_rf_rx_power_mode=$BT_RF_RX_POWER_MODE"
#PARAMS=$PARAMS" bt_rf_tx_power_mode=$BT_RF_TX_POWER_MODE"
#PARAMS=$PARAMS" ble_roles=$BLE_ROLES"
#PARAMS=$PARAMS" ble_pwr_save_options=$BLE_POWER_SAVE_OPTIONS"
#PARAMS=$PARAMS" ble_tx_pwr_inx=$BLE_TX_PWR_INX"
#PARAMS=$PARAMS" bt_rf_type=$BT_RF_TYPE"
#PARAMS=$PARAMS" lp_handshake_mode=$LP_HANDSHAKE_MODE"
#PARAMS=$PARAMS" max_sp_len=$MAX_SP_LEN"
#PARAMS=$PARAMS" rx_data_inactive_interval=$RX_DATA_INACTIVE_INTERVAL"
#PARAMS=$PARAMS" enabled_uapsd=$ENABLED_UAPSD"
#PARAMS=$PARAMS" enable_ng_cb=$ENABLE_NG_CB"
#PARAMS=$PARAMS" value_of_ng_cb=$VALUE_OF_NG_CB"
#PARAMS=$PARAMS" trigger_resp_ind=$TRIGGER_RESP_IND"
#PARAMS=$PARAMS" gaurd_interval=$GAURD_INTERVAL"
#PARAMS=$PARAMS" nominal_pe=$NOMINAL_PE"
#PARAMS=$PARAMS" dcm_enable=$DCM_ENABLE"
#PARAMS=$PARAMS" ldpc_enable=$LDPC_ENABLE"
#PARAMS=$PARAMS" config_er_su=$CONFIG_ER_SU"

modprobe mac80211
modprobe rfcomm

flag=0

sdio=`ls /sys/bus/sdio/devices | cut -d " " -f 2`
declare -a sdio_interfcaes
sdio_interfaces=($sdio)
len=${#sdio_interfaces[@]}

t_vid='xxxxxx'
for(( i=0; i<$len; i++))
do
  sdio_name=${sdio_interfaces[$i]}
  vendor_id=`cat /sys/bus/sdio/devices/$sdio_name/vendor`
  if [ $vendor_id == "0x041b" ]
  then
	  t_vid=$vendor_id
	  flag=2
	  break
  fi
done

if [ $flag == 2 ]
then
	yes | cp ../Firmware/* /lib/firmware/
	insmod rsi_91x.ko $PARAMS
	insmod rsi_sdio.ko sdio_clock=50
	sleep 2
else
	echo "0"
fi

sleep 1
