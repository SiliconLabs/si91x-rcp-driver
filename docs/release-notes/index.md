# **SiWT917 2.14.0.5 RCP Release Notes**
## **Release Details**

|**Item**|**Details**|
| :- | :- |
|Release date|28th March, 2025|
|Firmware Version|1711.2.14.1.0.0.5|
|Package Name|SiWT917.2.14.0.5|
|Linux Kernel Version support|From v3.18 to v6.6|
|Host interfaces supported|SDIO|
|Operating Modes Supported|Wi-Fi STA, Wi-Fi AP, Wi-Fi STA+BLE, Wi-Fi STA+AP, BLE|


- SiWT917 release consists of the following components 
  - Firmware - SiWx917 Firmware Binary
  - rsi - SiWT917 RCP driver source code
  - apps - contains driver tools source code 
  - release - contains kernel modules and script files
  - docs - contains release notes
- This software is meant only for use with designs based on SiWT917 Silicon
## **Supported Hardware OPNs**
|**Hardware**|**OPN (Ordering Part Number)**|
| :- | :- |
|OPNs|QFN OPN:   SIWT917M100XGTBA  </p><p> Module OPN: SIWT917Y100XGNBA Module with RF-PIN , SIWT917Y100XGABA Module with Antenna|
|Expansion kits:|<p>SiWx917-EB4346A (based on Radio board SiWx917-4346A + [8045A Co-Processor Adapter board] or [8045B Co-Processor Raspberry Pi Adapter board]) </p><p> Module Board: SiW917Y-RB4357A (SiWT917Y Module Wi-Fi 6 and Bluetooth LE Co-Processor Radio Board)|


## **Features Supported**
### **System**
- #### **Operating Modes :**
  - Wi-Fi STA (802.11ax, 802.11n),
  - Wi-Fi 802.11n AP,
  - Wi-Fi STA (802.11ax, 802.11n) + 802.11n AP,
  - Wi-Fi STA (802.11ax, 802.11n) + BLE,
  - BLE 
- #### **Host Interface :**
  - SDIO 2.0
- #### **Power save**
  - Deep Sleep
  - Connected Sleep
  - Host-based wake-up
### **Wi-Fi**
- #### **Wi-Fi protocols**
  - IEEE 802.11 b/g/n/ax (2.4GHz)
- #### **Wi-Fi Station mode**
  - #### **Scan** 
    - Selective Scan
    - Active/Passive Scanning
  - #### **Wi-Fi  Security**
    - Open Mode
    - WPA2 Personal, WPA2 Enhancements
    - WPA2 Enterprise
    - WPA3 Personal
    - Mixed Mode (WPA/WPA2)
    - WPA3 Personal Transition Mode (WPA2/WPA3)
  - #### **Wi-Fi STA Rejoin**
  - #### **Wi-Fi STA Roaming**
    - BG Scan
    - OKC (Opportunistic Key caching)
    - PMK (Pairwise Master Key) caching
    - Pre-Authentication
  - #### **Wi-Fi Protocol Power save**
    - Deep sleep (unconnected state)
    - Max PSP
    - Enhanced Max PSP
    - Fast PSP
    - TWT
  - #### **QoS**
    - WMM-QoS
  - #### **Wi-Fi 6 Feature**
    - MU-MIMO (DL)
    - OFDMA (UL/DL)
    - iTWT,TWT I-Frame & TWT Enhancements
    - BSS coloring
    - MBSSID
- #### **Access Point (AP) mode**
  - 8 Client Support
  - **Wi-Fi Security** 
    - Open Mode 
    - WPA2 Personal
    - WPA2 Enterprise
    - WPA3 Personal (H2E method only)
    - WPA Mixed mode (WPA/WPA2)
  - Hidden SSID Mode
  - Auto Channel Selection
- #### **WPA2 Enterprise security**
  - Method 
    - PEAP/TTLS/TLS/FAST
- #### **Wi-Fi Concurrency**
  - AP+STA (Same channel)
  - Scan in AP mode
- #### **Wi-Fi Band/Channels**
  - 2.4GHz CH1-11 (US)
  - 2.4GHz CH1-13 (Europe)
  - 2.4GHz CH1-14 (Japan)
- #### **Known Security vulnerabilities handled**
  - WPA2 KRACK Attacks
  - Fragment and Forge Vulnerability
### **BLE**
- Security
- Accept list
- LE PHY(1Mbps, 2Mbps) & Coded PHY(125Kbps, 500kbps)
- Simultaneous scanning on 1M and Coded PHY
- LE dual role topology
- LE data packet length extensions( DLE)
- Asymmetric PHYs
- LE channel selection algorithm 2 (CSA#2)
- Bluetooth 5.4 Qualified
### **Multi-protocol Mode**
- Wi-Fi STA + BLE
### **PTA Coexistence**
- 3 wire coex acting as Wi-Fi Station with external Bluetooth
- 3 wire coex acting as Wi-Fi Station with external Zigbee/OT
## **Changes and Fixes**
### **System**
- #### **Enhancements**
  - Updated Linux kernel version support upto version 6.6.
- #### **Fixed Issues**
  - None
### **Wi-Fi**
- #### **Enhancements**
  - None
- #### **Fixed Issues**
  - Resolved the ping issue between third-party stations in WPA3 AP mode.
  - Fixed the fragmented ICMP Ping request problem.
  - Addressed the Traffic Differentiation issue in Single BSS with 802.11n STA (WMM issue).
  - Corrected the A-MSDU Aggregation issue when the STA is the recipient.
  - Resolved the throughput inconsistency issue that occurs when more than 4 clients are connected to the DUT in AP mode.
  - Updated region gain tables for SiWT917Y modules.
### **BLE**
- #### **Enhancements**
  - Added separate country region command for BLE. Users have the flexibility to independently configure the BLE region, before establishing a connection to an Access Point (AP). In Coex mode, upon successful connection to an AP, the BLE region will be automatically updated to match the region supported by the connected AP.
- #### **Fixed issues**
  - None
### **Multi-protocol**
- #### **Enhancements**
  - None
- #### **Fixed issues**
  - None
## **Recommendations**
### **System**
- Set the recommended Power Save Profile (PSP) type to Enhanced Max PSP.
### **Wi-Fi**
- Disable power save for higher throughput values or use FAST PSP power save mode as per application requirement.
### **BLE**
- In BLE, the recommended range of Connection Interval in 
  - Power Save (BLE Only) - 100 ms to 1.28 s.
- In BLE, during Connection, the configuration of Scan Interval and Scan Window with the same value is not recommended. The suggested ratio of Scan Window to Scan Interval is 3:4.
- In BLE, if a device is acting as Central, the scan window must be less than the existing Connection Interval. The suggested ratio of Scan Window to Connection Interval is 2:3.
- In BLE mode, if scanning and advertising are in progress on the SiWx91x module and it subsequently gets connected and moves to the central role, scanning stops else if it moves to the peripheral role, advertising stops. To further establish a connection to another peripheral device or to a central device, the application should give a command for starting advertising and scanning again.
- Recommend using XTAL clock for BLE instead of RC clock.
### **Multi-protocol**
- For concurrent Wi-Fi + BLE, and while a Wi-Fi connection is active, we recommend setting the ratio of the BLE scan window to BLE scan interval to 1:3 or 1:4.
- Wi-Fi + BLE Advertising 
  - All standard advertising intervals are supported. As Wi-Fi throughput is increased, a slight difference in on-air advertisements compared to configured intervals may be seen.
  - BLE advertising is skipped if the advertising interval collides with Wi-Fi activity.
- Wi-Fi + BLE scanning 
  - All standard scan intervals are supported. For better scan results, we recommend setting the ratio of the BLE scan window to BLE scan interval to 1:3 or 1:4.
  - BLE scanning will be stopped for intervals that collide with Wi-Fi activity.
- Wi-Fi + BLE Central/Peripheral Connections 
  - All standard connection intervals are supported.
  - For a stable connection, use optimal connection intervals and max supervision timeout in the presence of Wi-Fi activity.
- Wi-Fi + BLE Central/Peripheral Data Transfer 
  - To achieve higher throughput for both Wi-Fi and BLE, use medium connection intervals, such as 45 to 80 ms with maximum supervision timeout.
  - Ensure Wi-Fi activity consumes lower intervals.
## **Known Issues**
### **Wi-Fi**
- The set rate cannot be used for setting non-MCS and non-basic rates in kernels above 4.13.16.
- Tx and Rx Rate information is not being updated in WLAN interface stats reported by the iwconfig tool.
- Auto Channel Selection in AP with WORLD region will not work.
- In Concurrent Mode, Wi-Fi performance may be reduced if more than four clients are connected to DUT-AP.
- Deep sleep is disabled by default. Users can enable it using the default_deep_sleep_enable module_param mentioned in the TRM.
- TWT device (STA) may not go to sleep in between TWT intervals in case of burst data traffic where data transfer happens over multiple TWT service periods.
- WoWLAN feature is not supported.
- In AP mode, performance is expected to degrade with the addition of the number of clients.
- Hardware Beacon Miss event observed using MBSSID mode.
- Around 38% of packet loss observed in RF chamber while running UDP-bidirectional traffic.
- Wi-Fi client receives deauthentication from the access point when trigger TWT with announced mode is enabled.
- Monitor/sniffer mode - observed 70% packet loss while capturing the on-air data.
- In AP mode, in congested environments where channel occupancy exceeds 85%, connected stations may experience disconnections due to high congestion.
- The current release does not meet the certification requirements of the WFA.
### **BLE**
- BLE disconnections are expected with the RC clock in power save.
- The current release does not meet the certification requirements of the WFA.
### **Multi-protocol**
- MU retries in DL MU-MIMO are more in CoEx
- Observed Wi-Fi throughput inconsistency (TPUT in Kbps) for STA(Wi-Fi Tx)+BLE Mode while doing BLE scan.
- Observed BLE disconnection while initiating connection with an access point configured in CHINA region.
### **System**
- None
## **Limitations and Unsupported Features**
### **System**
- None
### **Wi-Fi**
- Broadcast TWT is not supported.
- AMSDU TX is not supported.
- Fragmentation is not supported.
- AMSDUs within AMPDU is not supported.
- 802.11k is not supported
- 40 MHz bandwidth is not supported.
- Low Power mode is not supported.
- In iTWT mode, downlink data traffic should be limited. Disconnections may be observed if AP fails to send all the buffered data in the next TWT Service Period.
- The number of Non-Transmitting BSSIDs processed is limited by the beacon length that can be processed by the stack (which is 1024 bytes). Beacons greater than 1024 Bytes in length will not be processed.
- UL MU-MIMO is not supported.
- WPA3 AP supports only the H2E algorithm.
- PMKSA caching is not supported in WPA3 AP mode.
- Wi-Fi performance may be reduced in dense environments.
- 802.11j is not supported.
- Background scan(Scan after DUT-STA connection) and power-save features are not supported for the station mode vap in concurrent mode.
- In Wi-Fi concurrent mode, Both DUT AP and Testbed AP should be in the same channel. The Channel in the hostapd configuration file needs to be updated manually to the same channel as the Testbed AP.
- In Wi-Fi concurrent Mode, DUT-STA doesn't reconnect to testbed AP if the testbed AP changes the channel(Only if DUT-AP is up), also DUT-AP stays in the same channel and continues to beacon.
- WPS mode is not supported.
- GPIO-based power save is not supported.
- For SiWT917Y modules, Channels 12, 13 and 14 are not supported in AP mode.
- For SiWT917Y modules, Gain table update is not supported.
- For SiWT917Y modules, Country code configuration is not supported in AP mode.

### **BLE**
- For BLE, if the connection is established with a small connection interval (less than 15 ms), simultaneous roles (i.e., Central + Scanning and Peripheral + Advertising) are not supported.
- BLE maximum two concurrent connections are supported(1 Central connection and 1 Peripheral connection) or (2 Central connections) or (2 Peripheral connections).
- The advertising Extension feature is not supported.
- Isochronous channels feature is not supported.
- The connection subrating feature is not supported.
- LE power controller feature is not supported.
- The EATT feature is not supported.
- Periodic Advertising with response(PAwR) feature is not supported.
- BLE Audio is not supported.
- The dynamic adjustment of TX power while extended advertising is active is not supported.
- Peripheral Latency feature is not supported.
- Restricted the option for users to modify BLE Gain tables for the SiWT917Y module. 

### **Multi-protocol**
- BLE may disconnect with Wi-Fi + BLE configuration and Wi-Fi continuous data transfer when the low BLE supervision timeout is configured. When the supervision timeout is configured with the value of 16 seconds, no disconnections are observed.
- In Wi-Fi +BLE configuration with Wi-Fi disconnects, a BLE reconnection issue is observed (Refer the section '[Recommendations](#recommendations)'  for stable connections).

# **SiWT917 2.13.0.4 RCP Release Notes**
## **Release Details**

|**Item**|**Details**|
| :- | :- |
|Release date|8th January, 2025|
|Firmware Version|1711.2.13.1.0.0.4|
|Package Name|SiWT917.2.13.0.4|
|Linux Kernel Version support|From v3.18 to v6.1|
|Host interfaces supported|SDIO|
|Operating Modes Supported|Wi-Fi STA, Wi-Fi AP, Wi-Fi STA+BLE, Wi-Fi STA+AP, BLE|


- SiWT917 release consists of the following components 
  - Firmware - SiWx917 Firmware Binary
  - rsi - SiWT917 RCP driver source code
  - apps - contains driver tools source code 
  - release - contains kernel modules and script files
  - docs - contains release notes
- This software is meant only for use with designs based on SiWx917 Silicon
## **Supported Hardware OPNs**
|**Hardware**|**OPN (Ordering Part Number)**|
| :- | :- |
|IC OPN|QFN OPN:   SIWT917M100XGTBA |
|Expansion kits:|<p>SiWx917-EB4346A (based on Radio board SiWx917-4346A + [8045A Co-Processor Adapter board] or [8045B Co-Processor Raspberry Pi Adapter board])|



## **Features Supported**
### **System**
- #### **Operating Modes :**
  - Wi-Fi STA (802.11ax, 802.11n),
  - Wi-Fi 802.11n AP,
  - Wi-Fi STA (802.11ax, 802.11n) + 802.11n AP,
  - Wi-Fi STA (802.11ax, 802.11n) + BLE,
  - BLE 
- #### **Host Interface :**
  - SDIO 2.0
- #### **Power save**
  - Deep Sleep
  - Connected Sleep
  - Host-based wake-up
### **Wi-Fi**
- #### **Wi-Fi protocols**
  - IEEE 802.11 b/g/n/ax (2.4GHz)
- #### **Wi-Fi Station mode**
  - #### **Scan** 
    - Selective Scan
    - Active/Passive Scanning
  - #### **Wi-Fi  Security**
    - Open Mode
    - WPA2 Personal, WPA2 Enhancements
    - WPA2 Enterprise
    - WPA3 Personal
    - Mixed Mode (WPA/WPA2)
    - WPA3 Personal Transition Mode (WPA2/WPA3)
  - #### **Wi-Fi STA Rejoin**
  - #### **Wi-Fi STA Roaming**
    - BG Scan
    - OKC (Opportunistic Key caching)
    - PMK (Pairwise Master Key) caching
    - Pre-Authentication
  - #### **Wi-Fi Protocol Power save**
    - Deep sleep (unconnected state)
    - Max PSP
    - Enhanced Max PSP
    - Fast PSP
    - TWT
  - #### **QoS**
    - WMM-QoS
  - #### **Wi-Fi 6 Feature**
    - MU-MIMO (DL)
    - OFDMA (UL/DL)
    - iTWT,TWT I-Frame & TWT Enhancements
    - BSS coloring
    - MBSSID
- #### **Access Point (AP) mode**
  - 8 Client Support
  - **Wi-Fi Security** 
    - Open Mode 
    - WPA2 Personal
    - WPA2 Enterprise
    - WPA3 Personal (H2E method only)
    - WPA Mixed mode (WPA/WPA2)
  - Hidden SSID Mode
  - Auto Channel Selection
- #### **WPA2 Enterprise security**
  - Method 
    - PEAP/TTLS/TLS/FAST
- #### **Wi-Fi Concurrency**
  - AP+STA (Same channel)
  - Scan in AP mode
- #### **Wi-Fi Band/Channels**
  - 2.4GHz CH1-11
  - 2.4GHz CH1-13
  - 2.4GHz CH1-14 (Japan)
- #### **Known Security vulnerabilities handled**
  - WPA2 KRACK Attacks
  - Fragment and Forge Vulnerability
### **BLE**
- Security
- Accept list
- LE PHY(1Mbps, 2Mbps) & Coded PHY(125Kbps, 500kbps)
- Simultaneous scanning on 1M and Coded PHY
- LE dual role topology
- LE data packet length extensions( DLE)
- Asymmetric PHYs
- LE channel selection algorithm 2 (CSA#2)
- Bluetooth 5.4 Qualified
### **Multi-protocol Mode**
- Wi-Fi STA + BLE
### **PTA Coexistence**
- 3 wire coex acting as Wi-Fi Station with external Bluetooth
- 3 wire coex acting as Wi-Fi Station with external Zigbee/OT
## **Changes and Fixes**
### **System**
- #### **Enhancements**
  - Updated Linux kernel version support upto version 6.1.
- #### **Fixed Issues**
  - None
### **Wi-Fi**
- #### **Enhancements**
  - Added support for Customer MFG tool.
  - Enhancement on WLAN keepalive functionality
- #### **Fixed Issues**
  - None
### **BLE**
- #### **Enhancements**
  - Added support for reading BLE BBP register in PER mode.
- #### **Fixed issues**
  - Resolved memory corruption issue with BLE two connections.
  - Resolved BLE pairing failure issue. 
### **Multi-protocol**
- #### **Enhancements**
  - None
- #### **Fixed issues**
  - Fixed Packet loss issue in dense environments.
## **Recommendations**
### **System**
- Set the recommended Power Save Profile (PSP) type to Enhanced Max PSP.
### **Wi-Fi**
- Disable power save for higher throughput values or use FAST PSP power save mode as per application requirement.
### **BLE**
- In BLE, the recommended range of Connection Interval in 
  - Power Save (BLE Only) - 100 ms to 1.28 s.
- In BLE, during Connection, the configuration of Scan Interval and Scan Window with the same value is not recommended. The suggested ratio of Scan Window to Scan Interval is 3:4.
- In BLE, if a device is acting as Central, the scan window must be less than the existing Connection Interval. The suggested ratio of Scan Window to Connection Interval is 2:3.
- In BLE mode, if scanning and advertising are in progress on the SiWx91x module and it subsequently gets connected and moves to the central role, scanning stops else if it moves to the peripheral role, advertising stops. To further establish a connection to another peripheral device or to a central device, the application should give a command for starting advertising and scanning again.
- Recommend using XTAL clock for BLE instead of RC clock.
### **Multi-protocol**
- For concurrent Wi-Fi + BLE, and while a Wi-Fi connection is active, we recommend setting the ratio of the BLE scan window to BLE scan interval to 1:3 or 1:4.
- Wi-Fi + BLE Advertising 
  - All standard advertising intervals are supported. As Wi-Fi throughput is increased, a slight difference in on-air advertisements compared to configured intervals may be seen.
  - BLE advertising is skipped if the advertising interval collides with Wi-Fi activity.
- Wi-Fi + BLE scanning 
  - All standard scan intervals are supported. For better scan results, we recommend setting the ratio of the BLE scan window to BLE scan interval to 1:3 or 1:4.
  - BLE scanning will be stopped for intervals that collide with Wi-Fi activity.
- Wi-Fi + BLE Central/Peripheral Connections 
  - All standard connection intervals are supported.
  - For a stable connection, use optimal connection intervals and max supervision timeout in the presence of Wi-Fi activity.
- Wi-Fi + BLE Central/Peripheral Data Transfer 
  - To achieve higher throughput for both Wi-Fi and BLE, use medium connection intervals, such as 45 to 80 ms with maximum supervision timeout.
  - Ensure Wi-Fi activity consumes lower intervals.
## **Known Issues**
### **Wi-Fi**
- The set rate cannot be used for setting non-MCS and non-basic rates in kernels above 4.13.16.
- Enterprise security with WPA3 is not supported.
- Tx and Rx Rate information is not being updated in WLAN interface stats reported by the iwconfig tool.
- Auto Channel Selection in AP with WORLD region will not work.
- In Concurrent Mode, Wi-Fi performance may be reduced if more than four clients are connected to DUT-AP.
- Deep sleep is disabled by default. Users can enable it using the default_deep_sleep_enable module_param mentioned in the TRM.
- Observed inconsistency with MQTT keep alive when TWT is enabled
- TWT device (STA) may not go to sleep in between TWT intervals in case of burst data traffic where data transfer happens over multiple TWT service periods.
- Observed interop issue (connectivity failure) with few APs (EERO 6+, EERO PRO 6E, Cisco Catalyst 9120AXID) when device reset/reboot happens on active connection.
- Disconnections were observed with Netgear RAX120 AP in WPA3 security at expected intervals with power save enabled.
- WoWLAN feature is not supported.
- In AP mode, performance is expected to degrade with the addition of the number of clients.
- WFA cases- a few of PMF/AMB cases not passing.
- Hardware Beacon Miss event observed using MBSSID mode.
- Throughput inconsistency(TPUT in Kbps) seen when > 4 clients are connected to DUT as AP mode.
- Around 38% of packet loss observed in RF chamber while running UDP-bidirectional traffic.
- Wi-Fi Client received de-auth from AP when enabled trigger TWT with announced.
- Wi-Fi Client is failed to connect with EAP TLS1.0 version.
- Monitor/sniffer mode - observed 70% packet loss while capturing the on-air data.
- AP Mode - WPA3-PSK - Data transfers failed between the Wi-Fi Clients connected to 917 AP, i.e. ping.
- It is recommended to compile the driver locally to support Wi-Fi STA + AP mode.
### **BLE**
- The maximum power that BLE can support is 16 dBm. It is not permitted for users to program more than 16dBm. 127 power_index is not supported in this release.
- BLE disconnections are expected with the RC clock in power save. 
### **Multi-protocol**
- MU retries in DL MU-MIMO are more in CoEx
- Observed Wi-Fi throughput inconsistency (TPUT in Kbps) for STA(Wi-Fi Tx)+BLE Mode while doing BLE scan.
- BLE disconnections observed after two hours, when STA + BLE running simultaneous traffic. 
### **System**
- None
## **Limitations and Unsupported Features**
### **System**
- None
### **Wi-Fi**
- Broadcast TWT is not supported.
- AMSDU TX is not supported.
- Fragmentation is not supported.
- AMSDUs within AMPDU is not supported.
- 802.11k is not supported
- 40 MHz bandwidth is not supported.
- Low Power mode is not supported.
- In iTWT mode, downlink data traffic should be limited. Disconnections may be observed if AP fails to send all the buffered data in the next TWT Service Period.
- The number of Non-Transmitting BSSIDs processed is limited by the beacon length that can be processed by the stack (which is 1024 bytes). Beacons greater than 1024 Bytes in length will not be processed.
- UL MU-MIMO is not supported.
- WPA3 AP supports only the H2E algorithm.
- PMKSA caching is not supported in WPA3 AP mode.
- Wi-Fi performance may be reduced in dense environments.
- 802.11j is not supported.
- Background scan(Scan after DUT-STA connection) and power-save features are not supported for the station mode vap in concurrent mode.
- In Wi-Fi concurrent mode, Both DUT AP and Testbed AP should be in the same channel. The Channel in the hostapd configuration file needs to be updated manually to the same channel as the Testbed AP.
- In Wi-Fi concurrent Mode, DUT-STA doesn't reconnect to testbed AP if the testbed AP changes the channel(Only if DUT-AP is up), also DUT-AP stays in the same channel and continues to beacon.
- WPS mode is not supported.
- GPIO-based power save is not supported.

### **BLE**
- For BLE, if the connection is established with a small connection interval (less than 15 ms), simultaneous roles (i.e., Central + Scanning and Peripheral + Advertising) are not supported.
- BLE maximum two concurrent connections are supported(1 Central connection and 1 Peripheral connection) or (2 Central connections) or (2 Peripheral connections).
- The advertising Extension feature is not supported.
- Isochronous channels feature is not supported.
- The connection subrating feature is not supported.
- LE power controller feature is not supported.
- The EATT feature is not supported.
- Periodic Advertising with response(PAwR) feature is not supported.
- BLE Audio is not supported.
- The dynamic adjustment of TX power while extended advertising is active is not supported.
- Peripheral Latency feature is not supported.

### **Multi-protocol**
- BLE may disconnect with Wi-Fi + BLE configuration and Wi-Fi continuous data transfer when the low BLE supervision timeout is configured. When the supervision timeout is configured with the value of 16 seconds, no disconnections are observed.
- In Wi-Fi +BLE configuration with Wi-Fi disconnects, a BLE reconnection issue is observed (Refer the section '[Recommendations](#recommendations)'  for stable connections).

# **SiWT917 2.10.0.5 RCP Release Notes**
## **Release Details**

|**Item**|**Details**|
| :- | :- |
|Release date|28th February, 2024|
|Firmware Version|1711.2.10.1.0.0.5|
|Package Name|SiWT917.2.10.0.5|
|Linux Kernel Version support|From v3.18 to v5.19|
|Host interfaces supported|SDIO|
|Operating Modes Supported|Wi-Fi STA, Wi-Fi AP, Wi-Fi STA+BLE, Wi-Fi STA+AP, BLE|
|Build Quality|GA|

- SiWT917 release consists of the following components 
  - Firmware - SiWx917 Firmware Binary
  - rsi - SiWT917 RCP driver source code
  - apps - contains driver tools source code 
  - release - contains kernel modules and script files
- This software is meant only for use with designs based on SiWx917 Silicon
## **Supported hardware/device setups**
- **SiWx917-EB4346A** - SiWx917 Wi-Fi 6 and Bluetooth LE Co-Processor EXP Expansion Kit
- **SiWx917-EB4346B** - SiWx917 Wi-Fi 6 and Bluetooth LE Co-Processor Raspberry Pi Expansion Kit
## **Features Supported**
### **System**
- #### **Operating Modes :**
  - Wi-Fi STA (802.11ax, 802.11n),
  - Wi-Fi 802.11n AP,
  - Wi-Fi STA (802.11ax, 802.11n) + 802.11n AP,
  - Wi-Fi STA (802.11ax, 802.11n) + BLE,
  - BLE 
- #### **Host Interface :**
  - SDIO 2.0
- #### **Power save**
  - Deep Sleep
  - Connected Sleep
  - Host-based wake-up
### **Wi-Fi**
- #### **Wi-Fi protocols**
  - IEEE 802.11 b/g/n/ax (2.4GHz)
- #### **Wi-Fi Station mode**
  - #### **Scan** 
    - Selective Scan
    - Active/Passive Scanning
  - #### **Wi-Fi  Security**
    - Open Mode
    - WPA2 Personal, WPA2 Enhancements
    - WPA3 Personal
    - Mixed Mode (WPA/WPA2)
    - WPA3 Personal Transition Mode (WPA2/WPA3)
  - #### **Wi-Fi STA Rejoin**
  - #### **Wi-Fi STA Roaming**
    - BG Scan
    - OKC (Opportunistic Key caching)
    - PMK (Pairwise Master Key) caching
    - Pre-Authentication
  - #### **Wi-Fi Protocol Power save**
    - Deep sleep (unconnected state)
    - Max PSP
    - Enhanced Max PSP
    - Fast PSP
    - TWT
  - #### **QoS**
    - WMM-QoS
  - #### **Wi-Fi 6 Feature**
    - MUMIMO (DL)
    - OFDMA (UL/DL)
    - iTWT,** TWT I-Frame & TWT Enhancements
    - BSS coloring
    - MBSSID
- #### **Access Point (AP) mode**
  - 8 Client Support
  - **Wi-Fi Security** 
    - Open Mode 
    - WPA2 Personal
    - WPA3 Personal (H2E method only)
    - WPA Mixed mode (WPA/WPA2)
  - Hidden SSID Mode
  - Auto Channel Selection
- #### **WPA2 Enterprise security**
  - Method 
    - PEAP/TTLS/TLS/FAST
- #### **Wi-Fi Concurrency**
  - AP+STA (Same channel)
  - Scan in AP mode
- #### **Wi-Fi Band/Channels**
  - 2.4GHz CH1-11
  - 2.4GHz CH1-13
  - 2.4GHz CH1-14 (Japan)
- #### **Known Security vulnerabilities handled**
  - WPA2 KRACK Attacks
  - Fragment and Forge Vulnerability
### **BLE**
- Security
- Accept list
- LE PHY(1Mbps, 2Mbps) & Coded PHY(125Kbps, 500kbps)
- Simultaneous scanning on 1M and Coded PHY
- LE dual role topology
- LE data packet length extensions( DLE)
- Asymmetric PHYs
- LE channel selection algorithm 2 (CSA#2)
- Bluetooth 5.4 Qualified
### **Multi-protocol Mode**
- Wi-Fi STA + BLE
### **PTA Coexistence**
- 3 wire coex acting as Wi-Fi Station with external Bluetooth
- 3 wire coex acting as Wi-Fi Station with external Zigbee/OT
## **Changes and Fixes**
### **Wi-Fi**
- #### **Enhancements**
  - WPA2/WPA3 Personal, WPA2-EAP Support for Station Mode.
  - Wi-Fi STA(802.11n/802.11ax) + 802.11n AP Support
  - HE (802.11ax) Support
  - Individual TWT Feature Support
  - Multiple BSSID feature support as an STA
  - Encap offload support
  - Added changes to support EDCA suspension.
  - Added user configurable option for 11ax rate, beacon miss threshold, keep alive period and get rssi.
  - Added ER_SU support in 11ax mode.
  - Added GI_LTF reconfigurability option from the user.
  - Added support for WPA3 personal security (with H2E method only) in AP mode.
- #### **Fixed Issues**
  - None
### **BLE**
- #### **Enhancements**
  - None
- #### **Fixed issues**
  - Resolved issue where DUT continues transmitting the BLE scan response packets with random address even after changing from random static address to public address.
  - The issue with BLE advertise reports not appearing in the scan results of the remote device (mobile) when the address was changed from random to public is resolved.
### **Multi-protocol**
- #### **Enhancements**
  - Increased priority of Wi-Fi handling of data transmission in coex mode.
- #### **Fixed issues**
  - Handling of PLL value in coex mode.
  - Resolved DUT hang issue when performing WLAN ping traffic, BLE advertising enabled and disabled continuously with two peripheral connections. 
## **Recommendations**
### **System**
- Set the recommended Power Save Profile (PSP) type to Enhanced Max PSP.
### **Wi-Fi**
- Disable power save for higher throughput values or use FAST PSP power save mode as per application requirement.
### **BLE**
- In BLE, the recommended range of Connection Interval in 
  - Power Save (BLE Only) - 100 ms to 1.28 s.
- In BLE, during Connection, the configuration of Scan Interval and Scan Window with the same value is not recommended. The suggested ratio of Scan Window to Scan Interval is 3:4.
- In BLE, if a device is acting as Central, the scan window must be less than the existing Connection Interval. The suggested ratio of Scan Window to Connection Interval is 2:3.
- In BLE mode, if scanning and advertising are in progress on the SiWx91x module and it subsequently gets connected and moves to the central role, scanning stops else if it moves to the peripheral role, advertising stops. To further establish a connection to another peripheral device or to a central device, the application should give a command for starting advertising and scanning again.
- Recommend using XTAL clock for BLE instead of RC clock.
### **Multi-protocol**
- For concurrent Wi-Fi + BLE, and while a Wi-Fi connection is active, we recommend setting the ratio of the BLE scan window to BLE scan interval to 1:3 or 1:4.
- Wi-Fi + BLE Advertising 
  - All standard advertising intervals are supported. As Wi-Fi throughput is increased, a slight difference in on-air advertisements compared to configured intervals may be seen.
  - BLE advertising is skipped if the advertising interval collides with Wi-Fi activity.
- Wi-Fi + BLE scanning 
  - All standard scan intervals are supported. For better scan results, we recommend setting the ratio of the BLE scan window to BLE scan interval to 1:3 or 1:4.
  - BLE scanning will be stopped for intervals that collide with Wi-Fi activity.
- Wi-Fi + BLE Central/Peripheral Connections 
  - All standard connection intervals are supported.
  - For a stable connection, use optimal connection intervals and max supervision timeout in the presence of Wi-Fi activity.
- Wi-Fi + BLE Central/Peripheral Data Transfer 
  - To achieve higher throughput for both Wi-Fi and BLE, use medium connection intervals, such as 45 to 80 ms with maximum supervision timeout.
  - Ensure Wi-Fi activity consumes lower intervals.
## **Known Issues**
### **Wi-Fi**
- The set rate cannot be used for setting non-MCS and non-basic rates in kernels above 4.13.16.
- Enterprise security with WPA3 is not supported.
- Tx and Rx Rate information is not being updated in WLAN interface stats reported by the iwconfig tool.
- Auto Channel Selection in AP with WORLD region will not work.
- In Concurrent Mode, Wi-Fi performance may be reduced if more than four clients are connected to DUT-AP.
- Deep sleep is disabled by default. Users can enable it using the default_deep_sleep_enable module_param mentioned in the TRM.
- Observed inconsistency with MQTT keep alive when TWT is enabled
- TWT device (STA) may not go to sleep in between TWT intervals in case of burst data traffic where data transfer happens over multiple TWT service periods.
- Observed interop issue (connectivity failure) with few APs (EERO 6+, EERO PRO 6E, Cisco Catalyst 9120AXID) when device reset/reboot happens on active connection.
- Disconnections were observed with Netgear RAX120 AP in WPA3 security at expected intervals with power save enabled.
- WoWLAN feature is not supported.
- In AP mode, performance is expected to degrade with the addition of the number of clients.
- WFA cases- a few of PMF/AMB cases not passing.
- Hardware Beacon Miss event observed using MBSSID mode.
- Throughput inconsistency(TPUT in Kbps) seen when > 4 clients are connected to DUT as AP mode.
- Around 38% of packet loss observed in RF chamber while running UDP-bidirectional traffic.
- Wi-Fi Client received de-auth from AP when enabled trigger TWT with announced.
- Wi-Fi Client is failed to connect with EAP TLS1.0 version.
- Monitor/sniffer mode - observed 70% packet loss while capturing the on-air data.
- AP Mode - WPA3-PSK - Data transfers failed between the Wi-Fi Clients connected to 917 AP, i.e. ping.
### **BLE**
- The maximum power that BLE can support is 16 dBm. It is not permitted for users to program more than 16dBm. 127 power_index is not supported in this release.
- BLE disconnections are expected with the RC clock in power save. 
### **Multi-protocol**
- MU retries in DL MUMIMO are more in CoEx
- Observed Wi-Fi throughput inconsistency (TPUT in Kbps) for STA(Wi-Fi Tx)+BLE Mode while doing BLE scan.
- BLE disconnections observed after two hours, when STA + BLE running simultaneous traffic. 
### **System**
- None
## **Limitations and Unsupported Features**
### **System**
- Current software is not validated on [SiWT917M100XGT](https://www.silabs.com/wireless/wi-fi/siwx917-wireless-socs/device.siwt917m100xgt?tab=specs).
### **Wi-Fi**
- Broadcast TWT is not supported.
- AMSDU TX is not supported.
- Fragmentation is not supported.
- AMSDUs within AMPDU is not supported.
- 802.11k is not supported
- 40 MHz bandwidth for the 2.4 GHz band is not supported.
- Low Power mode is not supported.
- In iTWT mode, downlink data traffic should be limited. Disconnections may be observed if AP fails to send all the buffered data in the next TWT Service Period.
- The number of Non-Transmitting BSSIDs processed is limited by the beacon length that can be processed by the stack (which is 1024 bytes). Beacons greater than 1024 Bytes in length will not be processed.
- UL MUMIMO is not supported.
- WPA3 AP supports only the H2E algorithm.
- PMKSA caching is not supported in WPA3 AP mode.
- Wi-Fi performance may be reduced in dense environments.
- 802.11j is not supported.
- Background scan(Scan after DUT-STA connection) and power-save features are not supported for the station mode vap in concurrent mode.
- In Wi-Fi concurrent mode, Both DUT AP and Testbed AP should be in the same channel. The Channel in the hostapd configuration file needs to be updated manually to the same channel as the Testbed AP.
- In Wi-Fi concurrent Mode, DUT-STA doesn't reconnect to testbed AP if the testbed AP changes the channel(Only if DUT-AP is up), also DUT-AP stays in the same channel and continues to beacon.
- WPS mode is not supported.
- GPIO-based power save is not supported.
### **BLE**
- For BLE, if the connection is established with a small connection interval (less than 15 ms), simultaneous roles (i.e., Central + Scanning and Peripheral + Advertising) are not supported.
- BLE maximum two concurrent connections are supported(1 Central connection and 1 Peripheral connection) or (2 Central connections) or (2 Peripheral connections).
- BLE Slave latency value is valid up to 32 only.
- The advertising Extension feature is not supported.
- Isochronous channels feature is not supported.
- The connection subrating feature is not supported.
- LE power controller feature is not supported.
- The EATT feature is not supported.
- Periodic Advertising with response(PAwR) feature is not supported.
- BLE Audio is not supported.
- The feature of dynamically changing the TX power when extended advertising is active is not supported.
### **Multi-protocol**
- BLE may disconnect with Wi-Fi + BLE configuration and Wi-Fi continuous data transfer when the low BLE supervision timeout is configured. When the supervision timeout is configured with the value of 16 seconds, no disconnections are observed.
- In Wi-Fi +BLE configuration with Wi-Fi disconnects, a BLE reconnection issue is observed (Refer the section '[Recommendations](#recommendations)'  for stable connections).

