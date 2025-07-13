/* Auto-Generated file. Never change this code! */

#include "cfe_srl_csp.h"

int CFE_SRL_RtableCSP(csp_iface_t *Iface) {
	int Status;

	Status = csp_rtable_set(CSP_NODE_EPS, CSP_ID_HOST_SIZE, Iface, CSP_NO_VIA_ADDRESS);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_UTRX, CSP_ID_HOST_SIZE, Iface, CSP_NO_VIA_ADDRESS);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_STRX, CSP_ID_HOST_SIZE, Iface, CSP_NO_VIA_ADDRESS);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_GS_KISS, CSP_ID_HOST_SIZE, Iface, CSP_NODE_UTRX);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_GSTRX, CSP_ID_HOST_SIZE, Iface, CSP_NODE_UTRX);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	return CFE_SUCCESS;
}

int CFE_SRL_AllNodeConfigCSP(void) {

	CFE_SRL_NodeConfigCSP(CSP_NODE_EPS, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_CRC32);
	CFE_SRL_NodeConfigCSP(CSP_NODE_UTRX, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_CRC32);
	CFE_SRL_NodeConfigCSP(CSP_NODE_STRX, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_CRC32);
	CFE_SRL_NodeConfigCSP(CSP_NODE_GS_KISS, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_CRC32);
	CFE_SRL_NodeConfigCSP(CSP_NODE_GSTRX, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_CRC32);

	return CFE_SUCCESS;
}

void CFE_SRL_ConfigHost(csp_conf_t *Conf) {

	Conf->address = CSP_NODE_POBC;
	Conf->hostname = "POBC";
	Conf->model = "COSMIC";
	Conf->revision = "mozart";
	Conf->conn_max = 10;
	Conf->conn_queue_length = 10;
	Conf->fifo_length = 25;
	Conf->port_max_bind = 32;
	Conf->rdp_max_window = 20;
	Conf->buffers = 10;
	Conf->buffer_data_size = 256;
	Conf->conn_dfl_so = CSP_O_NONE;

	return;
}
