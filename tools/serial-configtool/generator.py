import sys
import json
from genfunction import *


cfg = Get_Serial_module_cfg("../../cfe/modules/srl/config/default_cfe_srl_interface_cfg.h")
max_handle = int(cfg["CFE_SRL_GLOBAL_HANDLE_NUM"])
max_csp = int(cfg["CFE_SRL_CSP_MAX_DEVICE_NUM"])

with open('../../sample_defs/Interface_config.json') as fp:
    config = json.load(fp)
interfaces = config['interfaces']    
namearr = Get_general_srl_namearr(interfaces)
if (len(namearr) > max_handle):
    raise Exception(f"General Device is too many. Max: {max_handle} || Input: {len(namearr)}.\nEnlarge the Maximum number in the srl interface config header.")

with open("../../sample_defs/csp_config.json") as fp:
    cspcfg = json.load(fp)
csp_host = cspcfg['host'] # host config -> dict
csp_node = cspcfg['external'] # extgernal gomspace config -> 'list' of dict
if (len(csp_node)+1 > max_csp):
    raise Exception((f"CSP Device is too many. Max: {max_csp} || Input: {len(csp_node)+1}.\nReduce CSP device."))

# Generate cfe_srl_mission_cfg.h
with open('../../cfe/modules/srl/config/default_cfe_srl_mission_cfg.h', 'w') as f:
    f.write("/* Auto-Generated file. Never change this code! */\n")
    f.write("#ifndef CFE_SRL_MISSION_CFG_H\n")
    f.write("#define CFE_SRL_MISSION_CFG_H\n")
    f.write("\n")
    f.write('#include "cfe_srl_interface_cfg.h"\n\n')
    if namearr:
        f.write("typedef enum {\n")
        for name in namearr:
            f.write(f"\tCFE_SRL_{name}_HANDLE_INDEXER,\n")
        f.write("} CFE_SRL_Handle_Indexer_t;\n\n")

        f.write("/* \# of General serial device */\n")
        f.write("#define CFE_SRL_GNRL_DEVICE_NUM\t\t")
        f.write(f"(CFE_SRL_{namearr[-1]}_HANDLE_INDEXER + 1)\n\n")
    else:
        f.write("typedef enum {\n")
        f.write("\tNOTHING1\n")
        f.write("} CFE_SRL_Handle_Indexer_t;\n\n")
        f.write("/* Every interfaces are not ready yet */\n")
        f.write("#define CFE_SRL_GNRL_DEVICE_NUM\t\t0\n\n")
    
    gpio_num = Get_gpio_num(config['interfaces'])
    if (gpio_num) :
        f.write("/* \# of used gpio pin */\n")
        f.write(f"#define CFE_SRL_TOT_GPIO_NUM\t\t{gpio_num}\n\n")
        f.write("typedef enum {\n")
        for iface in interfaces:
            if iface['type'] == 'gpio' and iface['ready']:
                f.write(f"\tCFE_SRL_{iface['name'].upper()}_GPIO_INDEXER,\n")
        f.write("} CFE_SRL_GPIO_Indexer_t;\n\n")
    else :
        f.write("/* \# of used gpio pin */\n")
        f.write(f"#define CFE_SRL_TOT_GPIO_NUM\t\t{gpio_num}\n\n")
        f.write("typedef enum {\n")
        f.write("\tNOTHING\n")
        f.write("} CFE_SRL_GPIO_Indexer_t;\n\n\n")



# Generate CSP Node configuration
    f.write("/* CSP Node  Configuration */\n")
    f.write("typedef enum {\n")
    f.write(f"\tCSP_NODE_{csp_host['hostname']} = {csp_host['address']},\n")
    for iface in csp_node:
        f.write(f"\tCSP_NODE_{iface['name']} = {iface['node']},\n")
    f.write("} CFE_SRL_CSP_Node_t;\n\n")
    f.write("#endif /* CFE_SRL_MISSION_CFG_H */")

# Generate CSP Init
with open('../../cfe/modules/srl/fsw/src/cfe_srl_csp_config.c', 'w') as f:
    f.write("/* Auto-Generated file. Never change this code! */\n\n")
    f.write('#include "cfe_srl_csp.h"\n\n')
    f.write("int CFE_SRL_RtableCSP(csp_iface_t *Iface) {\n")
    f.write("\tint Status;\n\n")
    for node in csp_node:
        via = "CSP_NO_VIA_ADDRESS" if node['via'] == None else f"CSP_NODE_{node['via']}"
        f.write(f"\tStatus = csp_rtable_set(CSP_NODE_{node['name']}, CSP_ID_HOST_SIZE, Iface, {via});\n")
        f.write("\tif (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;\n\n")
    f.write("\treturn CFE_SUCCESS;\n")
    f.write("}\n\n")


    f.write("int CFE_SRL_AllNodeConfigCSP(void) {\n\n")
    for node in csp_node:
        priority = 'CSP_PRIO_NORM'
        if node['priority'] == 0: priority = 'CSP_PRIO_CRITICAL'
        elif node['priority'] == 1: priority = 'CSP_PRIO_HIGH'
        elif node['priority'] == 2: priority = 'CSP_PRIO_NORM'
        elif node['priority'] == 3: priority = 'CSP_PRIO_LOW'
        f.write(f"\tCFE_SRL_NodeConfigCSP(CSP_NODE_{node['name']}, {priority}, CSP_TIMEOUT({node['timeout']}), {node['option']});\n")
    f.write("\n\treturn CFE_SUCCESS;\n")
    f.write("}\n\n")

    f.write("void CFE_SRL_ConfigHost(csp_conf_t *Conf) {\n\n")
    f.write(f"\tConf->address = CSP_NODE_{csp_host['hostname']};\n")
    f.write(f"\tConf->hostname = \"{csp_host['hostname']}\";\n")
    f.write(f"\tConf->model = \"{csp_host['model']}\";\n")
    f.write(f"\tConf->revision = \"{csp_host['revision']}\";\n")
    f.write(f"\tConf->conn_max = {csp_host['conn_max']};\n")
    f.write(f"\tConf->conn_queue_length = {csp_host['conn_queue_length']};\n")
    f.write(f"\tConf->fifo_length = {csp_host['fifo_length']};\n")
    f.write(f"\tConf->port_max_bind = {csp_host['port_max_bind']};\n")
    f.write(f"\tConf->rdp_max_window = {csp_host['rdp_max_window']};\n")
    f.write(f"\tConf->buffers = {csp_host['buffers']};\n")
    f.write(f"\tConf->buffer_data_size = {csp_host['buffer_data_size']};\n")
    f.write(f"\tConf->conn_dfl_so = {csp_host['conn_dfl_so']};\n")
    f.write("\n\treturn;\n")
    f.write("}\n")

# Generate cfe_srl_error.h
with open('../../cfe/modules/core_api/fsw/inc/cfe_srl_error.h', 'w') as f:
    offset = 90
    f.write("/* Auto-Generated file. Never change this code! */\n")
    f.write("#ifndef CFE_SRL_ERROR_H\n")
    f.write("#define CFE_SRL_ERROR_H\n\n")
    f.write('#include "cfe_usr_error_macro.h"\n\n')
    f.write("/**\n")
    f.write(" * Early Init error define\n")
    f.write(" */\n")
    for iface in interfaces:
        if iface['ready']:
            f.write(f"#define CFE_SRL_{iface['name'].upper()}_INIT_ERR\t\tCFE_SERIAL_ERROR({offset})\n")
            offset+=1
    f.write("\n\n")
    f.write("#endif /* CFE_SRL_ERROR_H */")

# Generate EarlyInit function
with open('../../cfe/modules/srl/fsw/src/cfe_srl_init.c', 'w') as f:
    f.write("/* Auto-Generated file. Never change this code! */\n")
    f.write("/**\n * Required header files\n*/\n")
    f.write('#include "cfe_srl_module_all.h"\n\n')
    f.write("/**\n * Global data\n*/\n")
    f.write('CFE_SRL_IO_Handle_t *Handles[CFE_SRL_GNRL_DEVICE_NUM];\n')
    f.write("/**************************************************\n * Index of Each device\n")
    for i, name in enumerate(namearr):
        f.write(f" * {i} : {name} Handle\n")
    if not namearr:
        f.write(f" * 0 : Not initializaed padding Handle\n")
    f.write(" **************************************************/\n\n")
    gpio_num = Get_gpio_num(config['interfaces'])
    f.write("CFE_SRL_GPIO_Handle_t GPIO[CFE_SRL_TOT_GPIO_NUM];\n\n\n")

    f.write("/************************************************************************\n")
    f.write(" * Early Initialization function executed at cFE ES\n")
    f.write(" * Append object to `cfe_es_objtab.c`\n")
    f.write(" * Declaration is located at\n")
    f.write(" * `cfe/modules/core_private/fsw/inc/cfe_srl_core_internal.h`\n")
    f.write(" ************************************************************************/\n")
    f.write('int32 CFE_SRL_EarlyInit(void) {\n')
    f.write('\tint32 Status;\n\n')
    f.write('\tStatus = CFE_SRL_PriorInit();\n')
    f.write('\tif(Status != CFE_SUCCESS) return Status;\n')
    f.write('\tCFE_ES_WriteToSysLog("%s: Prior Initialized.", __func__);\n')
    f.write("\t/**************************************************\n")
    f.write("\t * Serial Comm. Init\n \t * Only `ready == true` interface is initialized\n")
    f.write("\t **************************************************/\n")

    for iface in interfaces:
        if not iface['ready']:
            continue
        if iface['type']=="i2c":
            Write_i2c_handle_init(f, iface)
        elif iface['type']=='spi':
            Write_spi_handle_init(f, iface)
        elif iface['type'] in ['uart', 'rs232', 'rs422', 'rs485']:
            Write_uart_handle_init(f, iface)
        elif iface['type'] == 'can':
            Write_can_handle_init(f, iface)
        elif iface['type'] == 'socat':
            Write_socat_handle_init(f, iface)

        if iface['type'] == 'gpio':
            Write_gpio_init(f, iface)
            f.write("\tif (Status != CFE_SUCCESS) {\n")
            f.write(f"\t\tCFE_ES_WriteToSysLog(\"%s: GPIO {iface['name']} Initialization failed! RC=%d\\n\", __func__, Status);\n")
            f.write(f"\t\treturn CFE_SRL_{iface['name'].upper()}_INIT_ERR;\n")
            f.write("\t}\n\n")
        else:
            # f.write(f"\tHandles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER]->Func.TxFunc = CFE_SRL_WriteGeneric{iface['type'].upper()};\n")
            # f.write(f"\tHandles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER]->Func.RxFunc = CFE_SRL_ReadGeneric{iface['type'].upper()};\n")
            f.write("\tif (Status != CFE_SUCCESS) {\n")
            f.write(f"\t\tCFE_ES_WriteToSysLog(\"%s: {iface['name']} Initialization failed! RC=%d\\n\", __func__, Status);\n")
            f.write(f"\t\treturn CFE_SRL_{iface['name'].upper()}_INIT_ERR;\n")
            f.write("\t}\n")
            f.write(f"\tCFE_ES_WriteToSysLog(\"%s: {iface['name']} Initialized. FD=%d || DevName=%s\\n\", __func__, Handles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER])->DevName);\n\n")
    # Total CSP Init
    if (csp_host['ready']):
        f.write("\tStatus = CFE_SRL_InitCSP();\n")
        f.write("\tif (Status != CFE_SUCCESS) {\n")
        f.write(f"\t\tCFE_ES_WriteToSysLog(\"%s: CSP Initialization failed! RC=%d\\n\", __func__, Status);\n")
        f.write(f"\t\treturn CFE_SRL_CSP_INIT_ERR;\n")
        f.write("\t}\n")
        f.write(f"\tCFE_ES_WriteToSysLog(\"%s: CSP Successfully Initialized.\\n\", __func__);\n\n")
    f.write("return CFE_SUCCESS;\n")
    f.write("}\n")

print("Code generation Successfully done.")

