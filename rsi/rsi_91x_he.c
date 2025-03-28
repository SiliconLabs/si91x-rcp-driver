// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

//__9117_CODE_START
#include "rsi_mgmt.h"
#include "rsi_he.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
static struct ieee80211_sband_iftype_data rsi_he_cap = {
	.types_mask = BIT(NL80211_IFTYPE_STATION),
	.he_cap = {
		.has_he = true,
		.he_cap_elem = {
#ifdef CONFIG_TWT_SUPPORT
			.mac_cap_info[0] =
				IEEE80211_HE_MAC_CAP0_TWT_REQ,
#else

			.mac_cap_info[0] = 0,
#endif
			.mac_cap_info[1] =
				IEEE80211_HE_MAC_CAP1_TF_MAC_PAD_DUR_16US,
			//	IEEE80211_HE_MAC_CAP1_MULTI_TID_AGG_RX_QOS_8,
			.mac_cap_info[2] =
				IEEE80211_HE_MAC_CAP2_32BIT_BA_BITMAP |
				IEEE80211_HE_MAC_CAP2_ALL_ACK |
				IEEE80211_HE_MAC_CAP2_BSR,
			.mac_cap_info[3] =
				IEEE80211_HE_MAC_CAP3_RX_CTRL_FRAME_TO_MULTIBSS|
//	IEEE80211_HE_MAC_CAP3_OMI_CONTROL |
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 13, 0)
				IEEE80211_HE_MAC_CAP3_MAX_AMPDU_LEN_EXP_VHT_2,
#else
				IEEE80211_HE_MAC_CAP3_MAX_AMPDU_LEN_EXP_EXT_2,
#endif
			.mac_cap_info[4] = 0,
			//	IEEE80211_HE_MAC_CAP4_AMDSU_IN_AMPDU |
			//	IEEE80211_HE_MAC_CAP4_MULTI_TID_AGG_TX_QOS_B39,
			.mac_cap_info[5] =
			//	IEEE80211_HE_MAC_CAP5_MULTI_TID_AGG_TX_QOS_B40 |
			//	IEEE80211_HE_MAC_CAP5_MULTI_TID_AGG_TX_QOS_B41 |
			//	IEEE80211_HE_MAC_CAP5_UL_2x996_TONE_RU |
			//	IEEE80211_HE_MAC_CAP5_HE_DYNAMIC_SM_PS |
				IEEE80211_HE_MAC_CAP5_HT_VHT_TRIG_FRAME_RX,
			.phy_cap_info[0] =
				IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_RU_MAPPING_IN_2G,
			//	IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_RU_MAPPING_IN_5G,
			.phy_cap_info[1] =
				IEEE80211_HE_PHY_CAP1_DEVICE_CLASS_A,
			//	IEEE80211_HE_PHY_CAP1_LDPC_CODING_IN_PAYLOAD |
			//	IEEE80211_HE_PHY_CAP1_HE_LTF_AND_GI_FOR_HE_PPDUS_0_8US,
			.phy_cap_info[2] =
				IEEE80211_HE_PHY_CAP2_NDP_4x_LTF_AND_3_2US |
			//	IEEE80211_HE_PHY_CAP2_STBC_RX_UNDER_80MHZ |
				IEEE80211_HE_PHY_CAP2_UL_MU_FULL_MU_MIMO |
				IEEE80211_HE_PHY_CAP2_UL_MU_PARTIAL_MU_MIMO,
			.phy_cap_info[3] =
				IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_16_QAM |
				IEEE80211_HE_PHY_CAP3_DCM_MAX_TX_NSS_1 |
				IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_16_QAM	|
				IEEE80211_HE_PHY_CAP3_DCM_MAX_RX_NSS_1,
			//	IEEE80211_HE_PHY_CAP3_RX_HE_MU_PPDU_FROM_NON_AP_STA,
			.phy_cap_info[4] =
				IEEE80211_HE_PHY_CAP4_SU_BEAMFORMEE |
				IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_4,
			//	IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_8,
			.phy_cap_info[5] =
				IEEE80211_HE_PHY_CAP5_NG16_SU_FEEDBACK |
				IEEE80211_HE_PHY_CAP5_NG16_MU_FEEDBACK,
			.phy_cap_info[6] =
				IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_42_SU |
				IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_75_MU |
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 13, 0)
				IEEE80211_HE_PHY_CAP6_TRIG_SU_BEAMFORMER_FB |
				IEEE80211_HE_PHY_CAP6_TRIG_MU_BEAMFORMER_FB	|
#else
				IEEE80211_HE_PHY_CAP6_TRIG_SU_BEAMFORMING_FB |
				IEEE80211_HE_PHY_CAP6_TRIG_MU_BEAMFORMING_PARTIAL_BW_FB |
#endif
				IEEE80211_HE_PHY_CAP6_TRIG_CQI_FB	|
				IEEE80211_HE_PHY_CAP6_PARTIAL_BW_EXT_RANGE |
				IEEE80211_HE_PHY_CAP6_PARTIAL_BANDWIDTH_DL_MUMIMO,
			//	IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT,
			.phy_cap_info[7] =
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 13, 0)
				IEEE80211_HE_PHY_CAP7_POWER_BOOST_FACTOR_AR |
#else
				IEEE80211_HE_PHY_CAP7_POWER_BOOST_FACTOR_SUPP |
#endif
				IEEE80211_HE_PHY_CAP7_HE_SU_MU_PPDU_4XLTF_AND_08_US_GI |
				IEEE80211_HE_PHY_CAP7_MAX_NC_1,
			.phy_cap_info[8] =
				IEEE80211_HE_PHY_CAP8_HE_ER_SU_PPDU_4XLTF_AND_08_US_GI |
				IEEE80211_HE_PHY_CAP8_HE_ER_SU_1XLTF_AND_08_US_GI |
				IEEE80211_HE_PHY_CAP8_DCM_MAX_RU_242,
			.phy_cap_info[9] =
				IEEE80211_HE_PHY_CAP9_NON_TRIGGERED_CQI_FEEDBACK |
				IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_COMP_SIGB |
				IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_NON_COMP_SIGB |
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
				IEEE80211_HE_PHY_CAP9_NOMIMAL_PKT_PADDING_16US,
#else
				(IEEE80211_HE_PHY_CAP9_NOMINAL_PKT_PADDING_16US << 6),
#endif
		},
		/*
		 * Set default Tx/Rx HE MCS NSS Support field.
		 * Indicate support for up to 2 spatial streams and all
		 * MCS, without any special cases
		 */
		.he_mcs_nss_supp = {
			.rx_mcs_80 = cpu_to_le16(0xfffc),
			.tx_mcs_80 = cpu_to_le16(0xfffc),
			.rx_mcs_160 = cpu_to_le16(0xffff),
			.tx_mcs_160 = cpu_to_le16(0xffff),
			.rx_mcs_80p80 = cpu_to_le16(0xffff),
			.tx_mcs_80p80 = cpu_to_le16(0xffff),
		},
		/*
		 * Set default PPE thresholds, with PPET16 set to 0,
		 * PPET8 set to 7
		 */
		.ppe_thres = {0x00, 0x00, 0x00, 0x00},
	},
};
void rsi_update_ax_capabilities(struct rsi_hw *adapter, struct ieee80211_supported_band *sbands)
{
  if (adapter->device_model >= RSI_DEV_9117) {
    sbands->vht_cap.vht_supported = true;
    sbands->vht_cap.cap = IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454 | IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ
                          | IEEE80211_VHT_CAP_RXLDPC | IEEE80211_VHT_CAP_SHORT_GI_80 | IEEE80211_VHT_CAP_SHORT_GI_160
                          | IEEE80211_VHT_CAP_TXSTBC | IEEE80211_VHT_CAP_RXSTBC_4
                          | IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK;
    sbands->vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(
      IEEE80211_VHT_MCS_SUPPORT_0_7 << 0 | IEEE80211_VHT_MCS_SUPPORT_0_7 << 2 | IEEE80211_VHT_MCS_SUPPORT_0_7 << 4
      | IEEE80211_VHT_MCS_SUPPORT_0_7 << 6 | IEEE80211_VHT_MCS_SUPPORT_0_7 << 8 | IEEE80211_VHT_MCS_SUPPORT_0_7 << 10
      | IEEE80211_VHT_MCS_SUPPORT_0_7 << 12 | IEEE80211_VHT_MCS_SUPPORT_0_7 << 14);
    sbands->vht_cap.vht_mcs.tx_mcs_map = sbands->vht_cap.vht_mcs.rx_mcs_map;

    sbands->iftype_data        = &rsi_he_cap;
    sbands->n_iftype_data      = 1; //No of vertual interfaces supporting he caps
    adapter->ax_params.ax_rate = _11AX_MCS0;
  }
}
#endif

int rsi_gi_ltf_read(struct seq_file *seq, void *data)
{
  struct rsi_common *common = seq->private;
  struct rsi_hw *adapter    = common->priv;
  rsi_dbg(FSM_ZONE, "GI_LTF value is :%x\n", adapter->ax_params.GI_LTF);
  seq_printf(seq, "Current GI_LTF value is %#x\n", adapter->ax_params.GI_LTF);
  return 0;
}

int rsi_get_gi_ltf(struct inode *inode, struct file *file)
{
  return single_open(file, rsi_gi_ltf_read, inode->i_private);
}

ssize_t rsi_set_gi_ltf(struct file *filp, const char __user *buff, size_t len, loff_t *data)
{
  struct rsi_common *common = filp->f_inode->i_private;
  struct rsi_hw *adapter    = common->priv;
  unsigned long gi_ltf;
  int ret;

  if (!len)
    return 0;

  ret = kstrtoul_from_user(buff, len, 16, &gi_ltf);

  if (ret)
    return ret;

  if (gi_ltf < 0 && gi_ltf > 3) {
    rsi_dbg(ERR_ZONE, "Invalid value for GI_LTF\n");
    return -EINVAL;
  }
  adapter->ax_params.GI_LTF  = gi_ltf;
  adapter->rate_field_params = cpu_to_le16(
    (adapter->ax_params.nominal_pe << RATE_OFFSET_NOMINAL_PE) | (adapter->ax_params.GI_LTF << RATE_OFFSET_GI_LTF)
    | (adapter->ax_params.DCM << RATE_OFFSET_DCM) | (adapter->ax_params.coding_type << RATE_OFFSET_CODING_TYPE));
  common->fixed_rate = (adapter->ax_params.ax_rate | adapter->rate_field_params);
  return len;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
/**
 * rsi_ax_rate_read() - This function display the currently
 *			used 11ax rat.
 * @seq: Pointer to the sequence file structure.
 * @data: Pointer to the data.
 *
 * Return: 0 on success, -1 on failure.
 */
int rsi_ax_rate_read(struct seq_file *seq, void *data)
{
  struct rsi_common *common = seq->private;
  struct rsi_hw *adapter    = common->priv;
  rsi_dbg(FSM_ZONE, "%x: rsi_ax_rate_read", adapter->ax_params.ax_rate);
  seq_printf(seq, "Current 11ax rate is %#x\n", (adapter->ax_params.ax_rate & AX_RATE_MASK));
  return 0;
}

/**
 * rsi_get_ax_rate() - This function calls single open function of seq_file to
 *		      open file and read contents from it.
 * @inode: Pointer to the inode structure.
 * @file: Pointer to the file structure.
 *
 * Return: Pointer to the opened file status: 0 on success, ENOMEM on failure.
 */
int rsi_get_ax_rate(struct inode *inode, struct file *file)
{
  return single_open(file, rsi_ax_rate_read, inode->i_private);
}

/**
 * rsi_set_ax_rate() - This function writes into hal variable as per user
 *			    requirement.
 * @filp: Pointer to the file structure.
 * @buff: Pointer to the character buffer.
 * @len: Length of the data to be written into buffer.
 * @data: Pointer to the data.
 *
 * Return: len: Number of bytes read.
 */
ssize_t rsi_set_ax_rate(struct file *filp, const char __user *buff, size_t len, loff_t *data)
{
  struct rsi_common *common = filp->f_inode->i_private;
  struct rsi_hw *adapter    = common->priv;
  unsigned long ax_rate;
  int ret;

  if (!len)
    return 0;

  ret = kstrtoul_from_user(buff, len, 16, &ax_rate);

  if (ret)
    return ret;

  if (!adapter->ax_params._11ax_enabled) {
    rsi_dbg(ERR_ZONE, "Can't set 11ax rate in non-11ax Mode");
    return -EINVAL;
  }

  mutex_lock(&common->mutex);
  if (ax_rate < 0 || ax_rate > 7) {
    rsi_dbg(ERR_ZONE, "User given invalid 11ax rate, Configuring defult as MCS0");
    ax_rate = 0;
  }
  adapter->ax_params.ax_rate = (ax_rate & AX_RATE_MASK) | AX_RATE_VALID; //  Converting into rate code
  common->fixed_rate_en      = true;
  if (common->enable_er_su && (common->config_er_su == ER_SU_ONLY)
      && (ax_rate > 2)) {                    /* Config ER_SU_ONLY and 11ax rate greater than MCS2 */
    adapter->ax_params.ax_rate = _11AX_MCS2; /* In ER_SU_ONLY hardcode 11ax rate >= MCS3 to MCS2 */
    common->fixed_rate         = (adapter->ax_params.ax_rate | adapter->rate_field_params);
  } else {
    common->fixed_rate = (adapter->ax_params.ax_rate | adapter->rate_field_params);
  }
  rsi_send_vap_dynamic_update(common);
  mutex_unlock(&common->mutex);
  return len;
}
#endif

/**
 * rsi_read_edca_susp() - This function display the currently
 *			used edca_suspension value..
 * @seq: Pointer to the sequence file structure.
 * @data: Pointer to the data.
 *
 * Return: 0 on success, -1 on failure.
 */
int rsi_read_edca_susp(struct seq_file *seq, void *data)
{
  struct rsi_common *common = seq->private;
  struct rsi_hw *adapter    = common->priv;
  rsi_dbg(INFO_ZONE, "edca_susp value is %x", adapter->ax_params.edca_susp);
  seq_printf(seq, "edca_susp value %#x\n", adapter->ax_params.edca_susp);
  return 0;
}

/**
 * rsi_get_edca_susp() - This function calls single open function of seq_file to
 *		      open file and read contents from it.
 * @inode: Pointer to the inode structure.
 * @file: Pointer to the file structure.
 *
 * Return: Pointer to the opened file status: 0 on success, ENOMEM on failure.
 */
int rsi_get_edca_susp(struct inode *inode, struct file *file)
{
  return single_open(file, rsi_read_edca_susp, inode->i_private);
}

/**
 * rsi_set_edca_susp() - This function writes into hal variable as per user
 *			    requirement.
 * @filp: Pointer to the file structure.
 * @buff: Pointer to the character buffer.
 * @len: Length of the data to be written into buffer.
 * @data: Pointer to the data.
 *
 * Return: len: Number of bytes read.
 */
ssize_t rsi_set_edca_susp(struct file *filp, const char __user *buff, size_t len, loff_t *data)
{
  struct rsi_common *common = filp->f_inode->i_private;
  struct rsi_hw *adapter    = common->priv;
  unsigned long edca_susp   = 0;
  int ret;

  if (!len)
    return 0;

  ret = kstrtoul_from_user(buff, len, 16, &edca_susp);
  if (ret)
    return ret;

  adapter->ax_params.edca_susp = edca_susp;
  rsi_dbg(INFO_ZONE, "Send Radio Cap frame for EDCA Suspension\n");
  rsi_load_radio_caps(common);
  return len;
}

/**
 * rsi_read_config_er_su() - This function display the currently
 *			used config_er_su value..
 * @seq: Pointer to the sequence file structure.
 * @data: Pointer to the data.
 *
 * Return: 0 on success, -1 on failure.
 */
int rsi_read_config_er_su(struct seq_file *seq, void *data)
{
  struct rsi_common *common = seq->private;
  rsi_dbg(INFO_ZONE, "config_er_su value is %x", common->config_er_su);
  seq_printf(seq, "config_er_su value %#x\n", common->config_er_su);
  return 0;
}

/**
 * rsi_get_config_er_su() - This function calls single open function of seq_file to
 *		      open file and read contents from it.
 * @inode: Pointer to the inode structure.
 * @file: Pointer to the file structure.
 *
 * Return: Pointer to the opened file status: 0 on success, ENOMEM on failure.
 */
int rsi_get_config_er_su(struct inode *inode, struct file *file)
{
  return single_open(file, rsi_read_config_er_su, inode->i_private);
}

/**
 * rsi_set_config_er_su() - This function writes into hal variable as per user
 *			    requirement.
 * @filp: Pointer to the file structure.
 * @buff: Pointer to the character buffer.
 * @len: Length of the data to be written into buffer.
 * @data: Pointer to the data.
 *
 * Return: len: Number of bytes read.
 */
ssize_t rsi_set_config_er_su(struct file *filp, const char __user *buff, size_t len, loff_t *data)
{
  struct rsi_common *common  = filp->f_inode->i_private;
  unsigned long config_er_su = 0;
  int ret;
#ifdef CONFIG_STA_PLUS_AP
  struct ieee80211_vif *vif = NULL;
  struct rsi_hw *adapter    = common->priv;
  vif                       = rsi_get_sta_vif(adapter);
#endif

  if (!len)
    return 0;

  ret = kstrtoul_from_user(buff, len, 16, &config_er_su);
  if (ret)
    return ret;

  common->config_er_su = config_er_su;
  rsi_dbg(INFO_ZONE, "Send AUTO-RATE Request ER_SU Config\n");
#ifndef CONFIG_STA_PLUS_AP
  rsi_send_auto_rate_request(common, NULL, 0);
#else
  rsi_send_auto_rate_request(common, NULL, 0, vif);
#endif
  return len;
}

//__9117_CODE_END
