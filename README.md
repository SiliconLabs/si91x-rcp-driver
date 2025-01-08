# SiWT917 Open Source Driver
The SiWT917 RCP Driver is a SoftMAC driver that interacts with the Linux wireless MAC layer, MAC80211. It consists of simple and efficient kernel modules that currently support SiWT917 RCP chipsets and can be ported to any embedded platform, in addition to the X-86 platform.

- [Release Notes](#release-notes)
- [Licensing](#licensing)
- [Documentation](#documentation)
  - [Software Reference](#software-reference)
  - [Hardware Reference](#hardware-reference)
- [Package structure](#Package-structure)



## Release Notes

- See [here](https://github.com/SiliconLabs/si91x-rcp-driver/blob/master/docs/release-notes/index.md) for the latest RCP release notes.

## Licensing

The default license for SiWT917 RCP Driver is [GNU General Public License v2.0](https://github.com/SiliconLabs/si91x-rcp-driver/blob/master/LICENSE).

## Documentation

The online RCP documentation is available [here](https://docs.silabs.com/wifi91xrcp/latest/wifi91xrcp-developing-in-rcp-mode/).

### Software Reference

  - See the [Getting Started Guide](https://docs.silabs.com/wifi91xrcp/latest/wifi91xrcp-getting-started/) section for instructions on how to set up the SiWT917 Linux driver in RCP mode.
  - [SiWT917 Software Reference Manual](https://docs.silabs.com/wifi91xrcp/latest/wifi91xrcp-developers-guide-overview/)

### Hardware Reference

  - [SiWT917 RCP Datasheet](https://www.silabs.com/documents/public/data-sheets/siwt917-rcp-datasheet.pdf)


## Package Structure

This software package contains following folders/files :

SiWT917.X.X.X.X

        ├── Firmware  
        ├── apps   
        ├── release   
        ├── rsi 
        └── docs

### Content Description

 - Firmware : This folder contains firmware binary for SiWT917.

 - apps : This folder contains driver tools source code.

 - release : This folder contains kernel modules, configuration files of
supplicant, hostapd and script files.

 - rsi : This folder contains SiWT917 RCP driver source code.

 - docs : This folder contains release notes.
 
