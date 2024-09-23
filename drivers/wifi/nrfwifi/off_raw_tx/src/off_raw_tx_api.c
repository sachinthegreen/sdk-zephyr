/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief File containing API definitions for the Offloaded raw TX feature.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>


#include <drivers/wifi/nrfwifi/off_raw_tx_api.h>
#include <off_raw_tx/inc/off_raw_tx.h>

#define DT_DRV_COMPAT nordic_wlan
LOG_MODULE_DECLARE(wifi_nrf, CONFIG_WIFI_NRF70_LOG_LEVEL);

struct nrf_wifi_off_raw_tx_drv_priv off_raw_tx_drv_priv;
extern const struct nrf_wifi_osal_ops nrf_wifi_os_zep_ops;

int nrf70_off_raw_tx_init(void)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct nrf_wifi_ctx_zep *rpu_ctx_zep = NULL;
	void *rpu_ctx = NULL;
	enum op_band op_band = CONFIG_NRF_WIFI_OP_BAND;
#ifdef CONFIG_NRF_WIFI_LOW_POWER
	int sleep_type = HW_SLEEP_ENABLE;
#endif /* CONFIG_NRF_WIFI_LOW_POWER */
	struct nrf_wifi_tx_pwr_ctrl_params tx_pwr_ctrl_params;
	struct nrf_wifi_tx_pwr_ceil_params tx_pwr_ceil_params;
	struct nrf_wifi_board_params board_params;
	unsigned int fw_ver = 0;

	/* The OSAL layer needs to be initialized before any other initialization
	 * so that other layers (like FW IF,HW IF etc) have access to OS ops
	 */
	nrf_wifi_osal_init(&nrf_wifi_os_zep_ops);

	off_raw_tx_drv_priv_zep.fmac_priv = nrf_wifi_fmac_init_offloaded_raw_tx();

	if (off_raw_tx_drv_priv_zep.fmac_priv == NULL) {
		LOG_ERR("%s: Failed to initialize nRF70 driver",
			__func__);
		goto err;
	}

	rpu_ctx_zep = &off_raw_tx_drv_priv.rpu_ctx_zep;

	rpu_ctx_zep->drv_priv_zep = &off_raw_tx_priv;

	rpu_ctx = nrf_wifi_fmac_dev_add(off_raw_tx_drv_priv.fmac_priv,
					rpu_ctx_zep);

	if (!rpu_ctx) {
		LOG_ERR("%s: Failed to add nRF70 device", __func__);
		rpu_ctx_zep = NULL;
		goto err;
	}

	rpu_ctx_zep->rpu_ctx = rpu_ctx;

	status = nrf_wifi_fw_load(rpu_ctx);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		LOG_ERR("%s: Failed to load the nRF70 firmware patch", __func__);
		goto err;
	}

	status = nrf_wifi_fmac_ver_get(rpu_ctx,
				       &fw_ver);

	if (status != NRF_WIFI_STATUS_SUCCESS) {
		LOG_ERR("%s: Failed to read the nRF70 firmware version", __func__);
		goto err;
	}

	LOG_DBG("nRF70 firmware (v%d.%d.%d.%d) booted successfully",
		NRF_WIFI_UMAC_VER(fw_ver),
		NRF_WIFI_UMAC_VER_MAJ(fw_ver),
		NRF_WIFI_UMAC_VER_MIN(fw_ver),
		NRF_WIFI_UMAC_VER_EXTRA(fw_ver));

	configure_tx_pwr_settings(&tx_pwr_ctrl_params,
				  &tx_pwr_ceil_params);

	configure_board_dep_params(&board_params);

	status = nrf_wifi_fmac_dev_init_offloaded_raw_tx(rpu_ctx_zep->rpu_ctx,
#ifdef CONFIG_NRF_WIFI_LOW_POWER
							 sleep_type,
#endif /* CONFIG_NRF_WIFI_LOW_POWER */
							 NRF_WIFI_DEF_PHY_CALIB,
							 op_band,
							 IS_ENABLED(CONFIG_NRF_WIFI_BEAMFORMING),
							 &tx_pwr_ctrl_params,
							 &tx_pwr_ceil_params,
							 &board_params);

	if (status != NRF_WIFI_STATUS_SUCCESS) {
		LOG_ERR("%s: nRF70 firmware initialization failed", __func__);
		goto err;
	}

	return status;
err:
	if (rpu_ctx) {
		nrf_wifi_fmac_dev_rem_offloaded_raw_tx(rpu_ctx);
		rpu_ctx_zep->rpu_ctx = NULL;
	}
	
	nrf70_off_raw_tx_deinit();
	return -1;
}


void nrf70_off_raw_tx_deinit(void)
{
	nrf_wifi_fmac_deinit_offloaded_raw_tx(rpu_drv_priv_zep.fmac_priv);
	nrf_wifi_osal_deinit();
}


int nrf70_off_raw_tx_conf(struct nrf_wifi_off_raw_tx_conf *conf)
{
	int ret = -1;
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	
	status = nrf_wifi_offloaded_raw_tx_conf(off_raw_tx_drv_priv.rpu_ctx_zep.rpu_ctx,
						conf);

	if (status != NRF_WIFI_STATUS_SUCCESS) {
		nrf_wifi_osal_log_err("%s: nRF70 offloaded raw TX configuration failed",
				      __func__);
		goto out;
	}

	ret = 0;
out:
	return ret;
}


int nrf70_off_raw_tx_start(void)
{
	int ret = -1;
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	
	status = nrf_wifi_offloaded_raw_tx_start(off_raw_tx_drv_priv.rpu_ctx_zep.rpu_ctx);

	if (status != NRF_WIFI_STATUS_SUCCESS) {
		nrf_wifi_osal_log_err("%s: nRF70 offloaded raw TX start failed",
				      __func__);
		goto out;
	}

	ret = 0;
out:
	return ret;
}


int nrf70_off_raw_tx_stop(void)
{
	int ret = -1;
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	
	status = nrf_wifi_offloaded_raw_tx_stop(off_raw_tx_drv_priv.rpu_ctx_zep.rpu_ctx);

	if (status != NRF_WIFI_STATUS_SUCCESS) {
		nrf_wifi_osal_log_err("%s: nRF70 offloaded raw TX stop failed",
				      __func__);
		goto out;
	}

	ret = 0;
out:
	return ret;
}