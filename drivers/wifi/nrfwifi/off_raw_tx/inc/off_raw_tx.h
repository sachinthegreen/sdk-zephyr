/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief File containing internal structures for the offloaded raw TX feature in the driver.
 */
struct nrf_wifi_ctx_zep {
	void *drv_priv_zep;
	void *rpu_ctx;
};


struct nrf_wifi_off_raw_tx_drv_priv {
	struct nrf_wifi_fmac_priv *fmac_priv;
	/* TODO: Replace with a linked list to handle unlimited RPUs */
	struct nrf_wifi_ctx_zep rpu_ctx_zep;
};