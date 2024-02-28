#!/bin/bash
if [ "$1" == "STA" ] || [ "$1" == "AP" ] || [ "$1" == "BLE" ] || [ "$1" == "STA_BLE" ] || [ "$1" == "AP_STA" ] 
then
	echo "Mode Selected: $1"
	string=`echo $PATH`
	IFS=':' read -r -a array <<< "$string"
	supplicant=""
	host_apd=""
	if [ "$1" == "STA" ] || [ "$1" == "STA_BT" ] || [ "$1" == "STA_BLE" ] || [ "$1" == "AP_STA" ]
	then 
		dflag=0
		for element in "${array[@]}"
		do
     			str=`ls $element/ | grep wpa_supplicant`
     			if [ "$str" = '' ] 
     			then
     				dflag=0
     			else
     				dflag=1
     			fi
     			if [ $dflag == 1 ]
     			then
     				break
     			fi
		done
		pwd_name=`pwd`
		cflag=0
		st=`ls $pwd_name/ | grep wpa_supplicant`
		if [ "$st" = '' ]
		then
			cflag=0
		else
			cflag=1
		fi	
		if [ $dflag == 0 ] && [ $cflag == 0 ]
		then
			echo "Please ensure wpa_supplicant application is present in system"
			exit 0
		elif [ $dflag == 0 ] && [ $cflag == 1 ]
		then
			supplicant="./wpa_supplicant"
		elif [ $dflag == 1 ] && [ $cflag == 0 ]
		then
			supplicant="wpa_supplicant"
		elif [ $dflag == 1 ] && [ $cflag == 1 ]
		then
			supplicant="./wpa_supplicant"
		fi
	fi
	if [ "$1" == "AP" ] || [ "$1" == "AP_STA" ]
	then
		dflag=0
		for element in "${array[@]}"
		do
     			str=`ls $element/ | grep hostapd`
     			if [ "$str" = '' ] 
     			then
     				dflag=0
     			else
     				dflag=1
     			fi
     			if [ $dflag == 1 ]
     			then
     				break
     			fi
		done
		pwd_name=`pwd`
		cflag=0
		st=`ls $pwd_name/ | grep hostapd`
		if test -f $pwd_name/hostapd
		then
			cflag=1
		else
			clfag=0
		fi
		if [ $dflag == 0 ] && [ $cflag == 0 ]
		then
			echo "Please ensure hostapd application is present in system"
			exit 0
		elif [ $dflag == 0 ] && [ $cflag == 1 ]
		then
			host_apd="./hostapd"
		elif [ $dflag == 1 ] && [ $cflag == 0 ]
		then
			host_apd="hostapd"
		elif [ $dflag == 1 ] && [ $cflag == 1 ]
		then
			host_apd="./hostapd"
		fi
	fi
	sdio_ko=`lsmod | grep rsi_sdio`
	usb_ko=`lsmod | grep rsi_usb`
	rsi_ko=`lsmod | grep rsi_91x`
	v=0
	if [ "$usb_ko" == "" ]
	then
    		v=1
	else
    		echo "Unloading the existing rsi_usb"
		rmmod rsi_usb.ko
	fi
	if [ "$sdio_ko" == "" ]
	then
		v=1
	else
    		echo "Unloading the existing rsi_sdio"
		rmmod rsi_sdio.ko
	fi
	if [ "$rsi_ko" == "" ]
	then
    		v=1
	else
    		echo "Unloading the existing rsi_91x"
		rmmod rsi_91x.ko
	fi
	bti_count=0
	sleep 2
	wnp=`iw dev | grep Interface`
	wnpl=(${wnp//"Interface"/ })
	len=${#wnpl[@]}
	
	if [ "$1" == "STA" ]
	then
		sed -i 's/^DEV_OPER_MODE.*=.*/DEV_OPER_MODE=1/' osd_common_insert.sh
		v1=`head -n 17 ../Makefile | tail -1`
		v2=`head -n 20 ../Makefile | tail -1`
    		v3=`head -n 32 ../Makefile | tail -1`

		if [ "$v1" == "CONFIG_RSI_BT_ALONE=y" ] || [ "$v2" == "CONFIG_RSI_COEX_MODE=y" ] || [ "$v3" == "CONFIG_STA_PLUS_AP=y" ]
		then     
			sed -i '20s/.*/#CONFIG_RSI_COEX_MODE=y/' ../Makefile
            		sed -i '17s/.*/#CONFIG_RSI_BT_ALONE=y/' ../Makefile
                sed -i '32s/.*/#CONFIG_STA_PLUS_AP=y/' ../Makefile
            		echo "Driver not compiled in STA alone mode."
           		echo "************************** Compiling the driver **************************"
            		cd ../ && make
			cd release/
            		sleep 2 
		else
            		rsi=`ls | grep rsi_91x.ko`
			sdio=`ls | grep rsi_sdio.ko`
            		if [ "$rsi" == "" ] && [ "$sdio" == "" ]
            		then
		    		cd ../ && make
				cd release/
				sleep 2
            		fi
		fi
	elif [ "$1" == "AP" ]
	then	
		sed -i 's/^DEV_OPER_MODE.*=.*/DEV_OPER_MODE=1/' osd_common_insert.sh
    		v1=`head -n 17 ../Makefile | tail -1`
		v2=`head -n 20 ../Makefile | tail -1`
    		v3=`head -n 32 ../Makefile | tail -1`
		if [ "$v1" == "CONFIG_RSI_BT_ALONE=y" ] || [ "$v2" == "CONFIG_RSI_COEX_MODE=y" ] || [ "$v3" == "CONFIG_STA_PLUS_AP=y" ]
		then     
        		sed -i '20s/.*/#CONFIG_RSI_COEX_MODE=y/' ../Makefile
            		sed -i '17s/.*/#CONFIG_RSI_BT_ALONE=y/' ../Makefile
                	sed -i '32s/.*/#CONFIG_STA_PLUS_AP=y/' ../Makefile
            		echo "Driver not compiled in AP alone mode."
           		echo "************************** Compiling the driver **************************"
            		cd ../ && make
			cd release/
            		sleep 2 
		else
            		rsi=`ls | grep rsi_91x.ko`
			sdio=`ls | grep rsi_sdio.ko`
            		if [ "$rsi" == "" ] && [ "$sdio" == "" ] 
            		then
		    		cd ../ && make
				cd release/
				sleep 2
            		fi
		fi
	elif [ "$1" == "AP_STA" ] 
	then
		sed -i 's/^DEV_OPER_MODE.*=.*/DEV_OPER_MODE=1/' osd_common_insert.sh
    		v1=`head -n 17 ../Makefile | tail -1`
		v2=`head -n 20 ../Makefile | tail -1`
    		v3=`head -n 32 ../Makefile | tail -1`
		if [ "$v1" == "CONFIG_RSI_BT_ALONE=y" ] || [ "$v2" == "CONFIG_RSI_COEX_MODE=y" ] || [ "$v3" == "#CONFIG_STA_PLUS_AP=y" ]
		then     
        		    sed -i '20s/.*/#CONFIG_RSI_COEX_MODE=y/' ../Makefile
            		sed -i '17s/.*/#CONFIG_RSI_BT_ALONE=y/' ../Makefile
                sed -i '32s/.*/CONFIG_STA_PLUS_AP=y/' ../Makefile
            		echo "Driver not compiled in Concurrent(AP+STA) mode."
           		echo "************************** Compiling the driver **************************"
            	        cd ../ && make
			cd release/
            		sleep 2 
		else
            		rsi=`ls | grep rsi_91x.ko`
			sdio=`ls | grep rsi_sdio.ko`
            		if [ "$rsi" == "" ] && [ "$sdio" == "" ]
            		then
				cd ../ && make
				cd release/
				sleep 2
            		fi
		fi
	elif [ "$1" == "BT" ]
	then	
		sed -i 's/^DEV_OPER_MODE.*=.*/DEV_OPER_MODE=4/' osd_common_insert.sh
    		v1=`head -n 17 ../Makefile | tail -1`
		v2=`head -n 20 ../Makefile | tail -1`
    		v3=`head -n 32 ../Makefile | tail -1`
		if [ "$v1" == "#CONFIG_RSI_BT_ALONE=y" ] || [ "$v2" == "CONFIG_RSI_COEX_MODE=y" ] || [ "$v3" == "CONFIG_STA_PLUS_AP=y" ]
		then     
        		    sed -i '20s/.*/#CONFIG_RSI_COEX_MODE=y/' ../Makefile
            		sed -i '17s/.*/CONFIG_RSI_BT_ALONE=y/' ../Makefile
                sed -i '32s/.*/#CONFIG_STA_PLUS_AP=y/' ../Makefile
            		echo "Driver not compiled in BT mode."
           		echo "************************** Compiling the driver **************************"
            		make
            		sleep 2 
		else
			rsi=`ls | grep rsi_91x.ko`
			sdio=`ls | grep rsi_sdio.ko`
            		if [ "$rsi" == "" ] && [ "$sdio" == "" ]
            		then
		    		make
				    sleep 2
            		fi
		fi
		bti=`hciconfig | grep hci | cut -d ":" -f 1`
		declare -a btil
		btil=($bt)
		bti_count=${#btil[@]}
	elif [ "$1" == "BLE" ]
	then
		sed -i 's/^DEV_OPER_MODE.*=.*/DEV_OPER_MODE=8/' osd_common_insert.sh
    		v1=`head -n 17 ../Makefile | tail -1`
		v2=`head -n 20 ../Makefile | tail -1`
    		v3=`head -n 32 ../Makefile | tail -1`
		if [ "$v1" == "#CONFIG_RSI_BT_ALONE=y" ] || [ "$v2" == "CONFIG_RSI_COEX_MODE=y" ] || [ "$v3" == "CONFIG_STA_PLUS_AP=y" ]
		then     
        		    sed -i '20s/.*/#CONFIG_RSI_COEX_MODE=y/' ../Makefile
            		sed -i '17s/.*/CONFIG_RSI_BT_ALONE=y/' ../Makefile
                sed -i '32s/.*/#CONFIG_STA_PLUS_AP=y/' ../Makefile
            		echo "Driver not compiled in BLE alone mode."
           		echo "************************** Compiling the driver **************************"
            		cd ../ && make
			cd release/
            		sleep 2 
		else
            		rsi=`ls | grep rsi_91x.ko`
			sdio=`ls | grep rsi_sdio.ko`
            		if [ "$rsi" == "" ] && [ "$sdio" == "" ] 
            		then
				cd ../ && make
				sleep 2
            		fi
		fi
		bti=`hciconfig | grep hci | cut -d ":" -f 1`
		declare -a btil
		btil=($bt)
		bti_count=${#btil[@]}
	elif [ "$1" == "STA_BLE" ]
	then
		sed -i 's/^DEV_OPER_MODE.*=.*/DEV_OPER_MODE=9/' osd_common_insert.sh
    		v1=`head -n 17 ../Makefile | tail -1`
		v2=`head -n 20 ../Makefile | tail -1`
    		v3=`head -n 32 ../Makefile | tail -1`
		if [ "$v1" == "CONFIG_RSI_BT_ALONE=y" ] || [ "$v2" == "#CONFIG_RSI_COEX_MODE=y" ] || [ "$v3" == "CONFIG_STA_PLUS_AP=y" ]
		then     
			sed -i '20s/.*/CONFIG_RSI_COEX_MODE=y/' ../Makefile
            		sed -i '17s/.*/#CONFIG_RSI_BT_ALONE=y/' ../Makefile
			sed -i '32s/.*/#CONFIG_STA_PLUS_AP=y/' ../Makefile
            		echo "Driver not compiled in Coex mode."
           		echo "************************** Compiling the driver **************************"
            		cd ../ && make
			cd release/
            		sleep 2 
		else
            		rsi=`ls | grep rsi_91x.ko`
			sdio=`ls | grep rsi_sdio.ko`
            		if [ "$rsi" == "" ] && [ "$sdio" == "" ]
            		then
		    		cd ../ && make
				cd release/
				sleep 2
            		fi
		fi
		bti=`hciconfig | grep hci | cut -d ":" -f 1`
		declare -a btil
		btil=($bt)
		bti_count=${#btil[@]}
	elif [ "$1" == "AP_BLE" ]
	then
		sed -i 's/^DEV_OPER_MODE.*=.*/DEV_OPER_MODE=10/' osd_common_insert.sh
    		v1=`head -n 17 ../Makefile | tail -1`
		v2=`head -n 20 ../Makefile | tail -1`
    		v3=`head -n 32 ../Makefile | tail -1`
		if [ "$v1" == "CONFIG_RSI_BT_ALONE=y" ] || [ "$v2" == "#CONFIG_RSI_COEX_MODE=y" ] || [ "$v3" == "CONFIG_STA_PLUS_AP=y" ]
		then     
        		    sed -i '20s/.*/CONFIG_RSI_COEX_MODE=y/' ../Makefile
            		sed -i '17s/.*/#CONFIG_RSI_BT_ALONE=y/' ../Makefile
                sed -i '32s/.*/#CONFIG_STA_PLUS_AP=y/' ../Makefile
            		echo "Driver not compiled in coex mode."
           		echo "************************** Compiling the driver **************************"
            		make
            		sleep 2 
		else
            		rsi=`ls | grep rsi_91x.ko`
			sdio=`ls | grep rsi_sdio.ko`
            		if [ "$rsi" == "" ] && [ "$sdio" == "" ] 
            		then
		    		make
				    sleep 2
            		fi
		fi
		bti=`hciconfig | grep hci | cut -d ":" -f 1`
		declare -a btil
		btil=($bt)
		bti_count=${#btil[@]}
	fi

	
	var=`./osd_common_insert.sh`
	sleep 2
	
	if [ "$var" = 0 ]
	then
		echo "Error!!! No Module Connected"
		exit 0
	else
		echo "*********************************** Driver Loaded ***********************************"
		
		rfkill unblock all
	
		sleep 1

		service network-manager stop
	

		wn=`iw dev | grep Interface`
		wnl=(${wn//"Interface"/ })
		len=${#wnl[@]}

		tp_wval=''

		l1=${#wnpl[@]}
		l2=${#wnl[@]}
		flag=1
	


		if [ $l1 == 0 ]
		then
			tp_wval=${wnl[0]}
		else
			for (( i=0; i<$l2; i++))
			do
				for (( j=0; j<$l1; j++))
				do
					if [ ${wnl[$i]} != ${wnpl[$j]} ]
					then
    	    			flag=0
					else
    	    			flag=1
					fi
				done
				if [ $flag == 0 ]
				then
					tp_wval=${wnl[$i]}
				break
				fi
			done
		fi
		
		killall wpa_supplicant
		sleep 2
		killall hostapd
		sleep 2 
		
		echo "WIFI Interface is : $tp_wval"
		if [ "$1" == "STA" ] || [ "$1" == "STA_BLE" ] 
		then
			sec_mode=-1
			while [ true ];
			do
				read -p "In what security you want to connect the station .For open-security mode 1 OR for secured mode type 2: " sec_mode
				if [ $sec_mode == 1 ] || [ $sec_mode == 2 ]
				then
					break
				else
					echo "Enter a valid option"
				fi
			done
			if [ $sec_mode == 1 ]
			then
				read -p "SSID : " ssid
				echo ""
				sed -i '4s/.*/'network={'/' sta_settings.conf
				sed -i '5s/.*/'\ \ \ \ \ \ \ \ ssid=\"$ssid\"'/' sta_settings.conf
				sed -i '6s/.*/'\ \ \ \ \ \ \ \ key_mgmt=NONE'/' sta_settings.conf
				sed -i '7s/.*/'\ \ \ \ \ \ \ \ bgscan=\"simple:15:-45:20\"'/' sta_settings.conf
				sed -i '8s/.*/'\ \ \ \ \ \ \ \ priority=1'/' sta_settings.conf
				sed -i '9s/.*/'}'/' sta_settings.conf
        
				sed -i '12s/.*/'#network={'/' sta_settings.conf
				sed -i '13s/.*/'#\ \ \ \ \ \ \ \ ssid=\"$ssid\"'/' sta_settings.conf
				sed -i '14s/.*/'#\ \ \ \ \ \ \ \ pairwise=CCMP\ TKIP'/' sta_settings.conf
				sed -i '15s/.*/'#\ \ \ \ \ \ \ \ group=CCMP\ TKIP'/' sta_settings.conf
				sed -i '16s/.*/'#\ \ \ \ \ \ \ \ key_mgmt=WPA-PSK'/' sta_settings.conf
				sed -i '17s/.*/'#\ \ \ \ \ \ \ \ psk=\"$pass\"'/' sta_settings.conf
				sed -i '18s/.*/'#\ \ \ \ \ \ \ \ #bgscan=\"simple:15:-45:20\"'/' sta_settings.conf
				sed -i '19s/.*/'#\ \ \ \ \ \ \ \ proto=WPA2\ WPA'/' sta_settings.conf
				sed -i '20s/.*/'#}'/' sta_settings.conf
        
			elif [  $sec_mode == 2 ]
			then
				read -p "SSID : " ssid
				read -s -p "Please enter the password: " pass
				echo "" 
				sed -i '12s/.*/'network={'/' sta_settings.conf
				sed -i '13s/.*/'\ \ \ \ \ \ \ \ ssid=\"$ssid\"'/' sta_settings.conf
				sed -i '14s/.*/'\ \ \ \ \ \ \ \ pairwise=CCMP\ TKIP'/' sta_settings.conf
				sed -i '15s/.*/'\ \ \ \ \ \ \ \ group=CCMP\ TKIP'/' sta_settings.conf
				sed -i '16s/.*/'\ \ \ \ \ \ \ \ key_mgmt=WPA-PSK'/' sta_settings.conf
				sed -i '17s/.*/'\ \ \ \ \ \ \ \ psk=\"$pass\"'/' sta_settings.conf
				sed -i '18s/.*/'\ \ \ \ \ \ \ \ #bgscan=\"simple:15:-45:20\"'/' sta_settings.conf
				sed -i '19s/.*/'\ \ \ \ \ \ \ \ proto=WPA2\ WPA'/' sta_settings.conf
				sed -i '20s/.*/'}'/' sta_settings.conf
        
				sed -i '4s/.*/'#network={'/' sta_settings.conf
				sed -i '5s/.*/'#\ \ \ \ \ \ \ \ ssid=\"$ssid\"'/' sta_settings.conf
				sed -i '6s/.*/'#\ \ \ \ \ \ \ \ key_mgmt=NONE'/' sta_settings.conf
				sed -i '7s/.*/'#\ \ \ \ \ \ \ \ bgscan=\"simple:15:-45:20\"'/' sta_settings.conf
				sed -i '8s/.*/'#\ \ \ \ \ \ \ \ priority=1'/' sta_settings.conf
				sed -i '9s/.*/'#}'/' sta_settings.conf
			fi
			echo "Running Supplicant" 
			$supplicant -i $tp_wval -D nl80211 -c sta_settings.conf -dddt>log &
			sleep 2
			dhclient -r $tp_wval
			dhclient -v $tp_wval
			echo "Station with name : "$tp_wval " is up."
			bti=`hciconfig | grep hci | cut -d ":" -f 1`
			declare -a btil
			btil=($bt)
			tbti_count=${#btil[@]}
			if [ $tbti_count > $bti_count ] && [ "$1" == "STA_BLE" ]
			then
				echo "BLE Protocol Enabled"
			elif [  "$1" == "STA" ]
			then
				echo "Success"
			else
				echo "Could  Not Start the BLE Prtocol"
			fi

		elif [ "$1" == "AP" ] || [ "$1" == "AP_BLE" ]
		then	
			while [ 1 ];
			do	
				read -p "In what security you want to start the AP.For open-security mode 1 OR for secured mode type 2 : " ap_val	
				if [ $ap_val == '1' ]
				then
					echo "$(tail -n +2 ap_open.conf)" > ap_open.conf
					echo -e "interface=$tp_wval\n$(cat ap_open.conf)" > ap_open1conf
					$host_apd ap_open.conf -ddddt > log&
					break
				elif [ $ap_val == '2' ]
				then
					echo "$(tail -n +2 ap_wpa.conf)" > ap_wpa.conf
					echo -e "interface=$tp_wval\n$(cat ap_wpa.conf)" > ap_wpa.conf
					$host_apd ap_wpa.conf -ddddt > log&
					break
				else
					echo "Please!!!! Enter a correct option"
					continue
				fi
			done
			sleep 2
			echo "AP VAP Created"
			
			bti=`hciconfig | grep hci | cut -d ":" -f 1`
			declare -a btil
			btil=($bt)
			tbti_count=${#btil[@]}
			if [ $tbti_count > $bti_count ] && [ "$1" == "AP_BLE" ]
			then
				echo "BLE-Classic Protocol Enabled"
			elif [ "$1" == "AP" ]
			then
				echo "Success"
			else
				echo "Could  Not Start the BLE Prtocol"
			fi


		elif [ "$1" == "AP_STA" ] 
		then
			while [ true ];
			do
				read -p "In what security you want to connect the station .For open-security mode 1 OR for secured mode type 2: " sec_mode
				if [ $sec_mode == 1 ] || [ $sec_mode == 2 ]
				then
					break
				else
					echo "Enter a valid option"
				fi
			done
			if [ $sec_mode == 1 ]
			then
				read -p "SSID : " ssid
				echo ""
				sed -i '4s/.*/'network={'/' sta_settings.conf
				sed -i '5s/.*/'\ \ \ \ \ \ \ \ ssid=\"$ssid\"'/' sta_settings.conf
				sed -i '6s/.*/'\ \ \ \ \ \ \ \ key_mgmt=NONE'/' sta_settings.conf
				sed -i '7s/.*/'\ \ \ \ \ \ \ \ bgscan=\"simple:15:-45:20\"'/' sta_settings.conf
				sed -i '8s/.*/'\ \ \ \ \ \ \ \ priority=1'/' sta_settings.conf
				sed -i '9s/.*/'}'/' sta_settings.conf
        
				sed -i '12s/.*/'#network={'/' sta_settings.conf
				sed -i '13s/.*/'#\ \ \ \ \ \ \ \ ssid=\"$ssid\"'/' sta_settings.conf
				sed -i '14s/.*/'#\ \ \ \ \ \ \ \ pairwise=CCMP\ TKIP'/' sta_settings.conf
				sed -i '15s/.*/'#\ \ \ \ \ \ \ \ group=CCMP\ TKIP'/' sta_settings.conf
				sed -i '16s/.*/'#\ \ \ \ \ \ \ \ key_mgmt=WPA-PSK'/' sta_settings.conf
				sed -i '17s/.*/'#\ \ \ \ \ \ \ \ psk=\"$pass\"'/' sta_settings.conf
				sed -i '18s/.*/'#\ \ \ \ \ \ \ \ #bgscan=\"simple:15:-45:20\"'/' sta_settings.conf
				sed -i '19s/.*/'#\ \ \ \ \ \ \ \ proto=WPA2\ WPA'/' sta_settings.conf
				sed -i '20s/.*/'#}'/' sta_settings.conf
        
			elif [  $sec_mode == 2 ]
			then
				read -p "SSID : " ssid
				read -s -p "Please enter the password: " pass
				echo "" 
				sed -i '12s/.*/'network={'/' sta_settings.conf
				sed -i '13s/.*/'\ \ \ \ \ \ \ \ ssid=\"$ssid\"'/' sta_settings.conf
				sed -i '14s/.*/'\ \ \ \ \ \ \ \ pairwise=CCMP\ TKIP'/' sta_settings.conf
				sed -i '15s/.*/'\ \ \ \ \ \ \ \ group=CCMP\ TKIP'/' sta_settings.conf
				sed -i '16s/.*/'\ \ \ \ \ \ \ \ key_mgmt=WPA-PSK'/' sta_settings.conf
				sed -i '17s/.*/'\ \ \ \ \ \ \ \ psk=\"$pass\"'/' sta_settings.conf
				sed -i '18s/.*/'\ \ \ \ \ \ \ \ #bgscan=\"simple:15:-45:20\"'/' sta_settings.conf
				sed -i '19s/.*/'\ \ \ \ \ \ \ \ proto=WPA2\ WPA'/' sta_settings.conf
				sed -i '20s/.*/'}'/' sta_settings.conf
        
				sed -i '4s/.*/'#network={'/' sta_settings.conf
				sed -i '5s/.*/'#\ \ \ \ \ \ \ \ ssid=\"$ssid\"'/' sta_settings.conf
				sed -i '6s/.*/'#\ \ \ \ \ \ \ \ key_mgmt=NONE'/' sta_settings.conf
				sed -i '7s/.*/'#\ \ \ \ \ \ \ \ bgscan=\"simple:15:-45:20\"'/' sta_settings.conf
				sed -i '8s/.*/'#\ \ \ \ \ \ \ \ priority=1'/' sta_settings.conf
				sed -i '9s/.*/'#}'/' sta_settings.conf
			fi
			echo "Running Supplicant" 
			$supplicant -i $tp_wval -D nl80211 -c sta_settings.conf -dddt>log &
			sleep 2
			dhclient -r $tp_wval
			dhclient -v $tp_wval
			echo "Station with name : "$tp_wval " is up."
			
			iw dev $tp_wval interface add wlan1 type __ap
			
			sleep 2
			st=`wpa_cli -i wlan0 status | grep freq`

			#echo "Frequency String= $st"
			freq=${st:5:4}
			#echo "Frequency = $freq"
			x=$(($freq-2412))
			channel=$(($x/5))
			channel=$(($channel+1))
			#echo "Channel = $channel"
			wn=`iw dev | grep Interface`
			wnpl=(${wn//"Interface"/ })
			len=${#wnpl[@]}

			tp_wval1=''

			l1=${#wnpl1[@]}
			l2=${#wnpl[@]}
			flag=1
			
			if [ $l1 == 0 ]
			then
				tp_wval1=${wnpl[0]}
			else
				for (( i=0; i<$l2; i++))
				do
					for (( j=0; j<$l1; j++))
					do
						if [ ${wnpl[$i]} != ${wnpl1[$j]} ]
						then
    	    			   			flag=0
						else
    	    						flag=1
						fi
					done
					if [ $flag == 0 ]
					then
						tp_wval11=${wnpl[$i]}
					break
					fi
				done
			fi
			echo "AP Interface : $tp_wval1"
			while [ 1 ];
			do	
				read -p "In what security you want to start the AP.For open-security mode 1 OR for secured mode type 2: " ap_val	
				if [ $ap_val == '1' ]
				then
					echo "$(tail -n +2 ap_open.conf)" > ap_open.conf
					echo -e "interface=$tp_wval1\n$(cat ap_open.conf)" > ap_open.conf
					sleep 2
					sed -i '9s/.*/'channel="$channel"'/' ap_open.conf
					$host_apd ap_open.conf -dddt > log_ap_conc&
					
					break
				elif [ $ap_val == '2' ]
				then
					echo "$(tail -n +2 ap_wpa.conf)" > ap_wpa.conf
					echo -e "interface=$tp_wval1\n$(cat ap_wpa.conf)" > ap_wpa.conf
					sed -i '9s/.*/'channel="$channel"'/' ap_wpa.conf
					$host_apd ap_wpa.conf -ddddt > log_ap_conc&
					break
				else
					echo "Please!!!! Enter a correct option"
					continue

				fi
			done

			echo "AP VAP Created"
			echo "Success"
		elif [ "$1" == "BT" ]
		then	
			bti=`hciconfig | grep hci | cut -d ":" -f 1`
			declare -a btil
			btil=($bt)
			tbti_count=${#btil[@]}
			if [ $tbti_count > $bti_count ]
			then
				echo "BT-Classic Protocol Enabled"
			else
				echo "Could  Not Start the BT/BLE Prtocol"
			fi
		
		elif [ "$1" == "BLE" ]
		then
			bti=`hciconfig | grep hci | cut -d ":" -f 1`
			declare -a btil
			btil=($bt)
			tbti_count=${#btil[@]}
			if [ $tbti_count > $bti_count ]
			then
				echo "BLE Protocol Enabled"
			else
				echo "Could  Not Start the BT/BLE Prtocol"
			fi
		fi
	fi

	else
		echo "Wrong Arguments! Please specify a input for mode"
		echo "Available Options are : "
		echo "AP"	
		echo "STA"
		echo "AP_STA"
		echo "BLE"
		echo "STA_BLE"
fi

