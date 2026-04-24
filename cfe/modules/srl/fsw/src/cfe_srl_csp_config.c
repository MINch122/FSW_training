/* Auto-Generated file. Never change this code! */

#include "cfe_srl_csp.h"

int CFE_SRL_RtableCSP(csp_iface_t *Iface) {
	int Status;

	Status = csp_rtable_set(CSP_NODE_EPS_PMU, CSP_ID_HOST_SIZE, Iface, CSP_NO_VIA_ADDRESS);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_EPS_ACU1, CSP_ID_HOST_SIZE, Iface, CSP_NO_VIA_ADDRESS);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_EPS_ACU2, CSP_ID_HOST_SIZE, Iface, CSP_NO_VIA_ADDRESS);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_EPS_PDU, CSP_ID_HOST_SIZE, Iface, CSP_NO_VIA_ADDRESS);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_ADCS, CSP_ID_HOST_SIZE, Iface, CSP_NO_VIA_ADDRESS);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_PAYUEL_OBC, CSP_ID_HOST_SIZE, Iface, CSP_NO_VIA_ADDRESS);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_UTRX, CSP_ID_HOST_SIZE, Iface, CSP_NO_VIA_ADDRESS);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_PAYUEL_CAM, CSP_ID_HOST_SIZE, Iface, CSP_NO_VIA_ADDRESS);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_ROMA, CSP_ID_HOST_SIZE, Iface, CSP_NO_VIA_ADDRESS);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_BP8, CSP_ID_HOST_SIZE, Iface, CSP_NO_VIA_ADDRESS);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_GSTRX, CSP_ID_HOST_SIZE, Iface, CSP_NODE_UTRX);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_GS_KISS, CSP_ID_HOST_SIZE, Iface, CSP_NODE_UTRX);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	Status = csp_rtable_set(CSP_NODE_TEST, CSP_ID_HOST_SIZE, Iface, CSP_NO_VIA_ADDRESS);
	if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

	return CFE_SUCCESS;
}

int CFE_SRL_AllNodeConfigCSP(void) {

	CFE_SRL_NodeConfigCSP(CSP_NODE_EPS_PMU, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_NONE);
	CFE_SRL_NodeConfigCSP(CSP_NODE_EPS_ACU1, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_NONE);
	CFE_SRL_NodeConfigCSP(CSP_NODE_EPS_ACU2, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_NONE);
	CFE_SRL_NodeConfigCSP(CSP_NODE_EPS_PDU, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_NONE);
	CFE_SRL_NodeConfigCSP(CSP_NODE_ADCS, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_NONE);
	CFE_SRL_NodeConfigCSP(CSP_NODE_PAYUEL_OBC, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_NONE);
	CFE_SRL_NodeConfigCSP(CSP_NODE_UTRX, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_CRC32);
	CFE_SRL_NodeConfigCSP(CSP_NODE_PAYUEL_CAM, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_NONE);
	CFE_SRL_NodeConfigCSP(CSP_NODE_ROMA, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_NONE);
	CFE_SRL_NodeConfigCSP(CSP_NODE_BP8, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_NONE);
	CFE_SRL_NodeConfigCSP(CSP_NODE_GSTRX, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_CRC32);
	CFE_SRL_NodeConfigCSP(CSP_NODE_GS_KISS, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_CRC32);
	CFE_SRL_NodeConfigCSP(CSP_NODE_TEST, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_CRC32);

	return CFE_SUCCESS;
}

void CFE_SRL_ConfigHost(csp_conf_t *Conf) {

	Conf->address = CSP_NODE_OBC;
	Conf->hostname = "OBC";
	Conf->model = "BASE5TH";
	Conf->revision = "mozart";
	Conf->conn_max = 10;
	Conf->conn_queue_length = 10;
	Conf->fifo_length = 25;
	Conf->port_max_bind = 58;
	Conf->rdp_max_window = 20;
	Conf->buffers = 10;
	Conf->buffer_data_size = 512;
	Conf->conn_dfl_so = CSP_O_NONE;

	return;
}
