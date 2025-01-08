cpy=cp -rf

#__9117_CODE_START
#CFLAGS += -DCONFIG_TWT_SUPPORT
CFLAGS += -DCONFIG_917

EXTRA_CFLAGS += diagnostic_report.c
#__9117_CODE_END

all: 
	@echo -e "\033[32mCompiling Application...\033[0m"
	$(CC) -o receive per_receive.c per_sock.c $(CFLAGS)
	$(CC) -o bt_util bt_util.c per_sock.c $(CFLAGS)
	$(CC) -o onebox_util update_wlan_gain_table.c onebox_util.c per_sock.c $(CFLAGS) $(EXTRA_CFLAGS)
	$(CC) -o bt_ble_gain_table_update update_bt_ble_gain_table.c per_sock.c $(CFLAGS)
	$(CC) -o transmit transmit.c per_sock.c $(CFLAGS)
	$(CC) -o ble_transmit ble_transmit.c per_sock.c $(CFLAGS)
	$(CC) -o ble_receive ble_receive.c per_sock.c $(CFLAGS)
	$(CC) -o transmit_packet transmit_packet.c per_sock.c $(CFLAGS)
	$(CC) -o bbp_util matlab_utils.c per_sock.c -lm -lpthread $(CFLAGS)
	#Uncomment the -D DEBUG flags in the below statement to increase log level for cmfg
	$(CC) -o mfg cmfg/*.c -Icmfg/ #-D DEBUG

	@echo -e "Copying to release folder...."
	$(cpy) receive ../release/
	$(cpy) bt_util ../release/
	$(cpy) onebox_util ../release/
	$(cpy) transmit ../release/
	$(cpy) ble_transmit ../release/
	$(cpy) ble_receive ../release/
	$(cpy) transmit_packet ../release/
	$(cpy) bt_ble_gain_table_update ../release/
	$(cpy) bbp_util ../release/
	$(cpy) mfg ../release/
