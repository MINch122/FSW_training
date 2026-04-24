import sys
import json

import re

# general serial protocol type name
namearr = ['i2c', 'spi', 'uart', 'rs232', 'rs422', 'rs485', 'can', 'socat']
def Get_general_srl_namearr(interfaces: list):
    HandleNameArr = []
    for iface in interfaces:
        if (iface['type'] in namearr):
            if iface['ready']:
                HandleNameArr.append(iface['name'].upper())
    return HandleNameArr

def Get_gpio_num(interfaces: list):
    num = 0
    for iface in interfaces:
        if (iface['type']== 'gpio' and iface['ready']):
            num+=1
    return num

def Write_i2c_handle_init(f, iface:dict):
    f.write(f"\t/* {iface['name']} Init */\n")
    f.write(f"\tConfig.cfg.i2c = (CFE_PSP_I2C_cfg_t) {{.tenbit = {str(iface['tenbit']).lower()},\n\
            \t\t\t\t\t\t\t  .pec_en = {str(iface['pec_en']).lower()},\n\
            \t\t\t\t\t\t\t  .retries = {iface['retries']}}};\n")
    f.write(f"\tStatus = CFE_SRL_HandleInit(&Handles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER], \"{iface['name']}\", \"{iface['DevName']}\", SRL_DEVTYPE_{iface['type'].upper()}, CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER, &Config);\n")
    return

def Write_spi_handle_init(f, iface:dict):
    f.write(f"\t/* {iface['name']} Init */\n")
    f.write(f"\tConfig.cfg.spi = (CFE_PSP_SPI_cfg_t) {{.mode = {iface['mode']},\n\
            \t\t\t\t\t\t\t  .bpw = {iface['bitperword']},\n\
            \t\t\t\t\t\t\t  .speed = {iface['speed']}}};\n")
    f.write(f"\tStatus = CFE_SRL_HandleInit(&Handles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER], \"{iface['name']}\", \"{iface['DevName']}\", SRL_DEVTYPE_{iface['type'].upper()}, CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER, &Config);\n")
    return

def Write_uart_handle_init(f, iface:dict):
    f.write(f"\t/* {iface['name']} Init */\n")
    f.write(f"\tConfig.cfg.uart = (CFE_PSP_UART_cfg_t) {{.baud = {iface['baudrate']},\n\
            \t\t\t\t\t\t\t    .databits = {iface['databits']},\n\
            \t\t\t\t\t\t\t    .parity = {iface['parity']},\n\
            \t\t\t\t\t\t\t    .stopbits = {iface['stopbits']}}};\n")
    f.write(f"\tStatus = CFE_SRL_HandleInit(&Handles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER], \"{iface['name']}\", \"{iface['DevName']}\", SRL_DEVTYPE_{iface['type'].upper()}, CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER, &Config);\n")
    return

def Write_can_handle_init(f, iface:dict):
    f.write(f"\t/* {iface['name']} Init */\n")
    f.write(f"\tCFE_PSP_CAN_filter_t filters[{iface['filter_num']}];\n")

    can_filter = iface['filter']
    for i,v in enumerate(can_filter):
        if (i >= iface['filter_num']):
            break
        f.write(f"\tfilters[{i}] = (CFE_PSP_CAN_filter_t) {{ .id = {v['id']},\n\
            \t\t\t\t\t\t\t  .mask = {v['mask']},\n\
            \t\t\t\t\t\t\t  .is_ext = {str(v['is_ext']).lower()}}};\n")
        
    f.write(f"\tConfig.cfg.can = (CFE_PSP_CAN_cfg_t) {{.filter = filters,\n\
            \t\t\t\t\t\t\t  .filter_num = {iface['filter_num']},\n\
            \t\t\t\t\t\t\t  .loopback_en = {str(iface['loopback_en']).lower()},\n\
            \t\t\t\t\t\t\t  .recv_own_msgs = {str(iface['recv_own_msgs']).lower()},\n\
            \t\t\t\t\t\t\t  .recv_err_frame = {str(iface['recv_err_frame']).lower()}}};\n")
    f.write(f"\tStatus = CFE_SRL_HandleInit(&Handles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER], \"{iface['name']}\", \"{iface['DevName']}\", SRL_DEVTYPE_{iface['type'].upper()}, CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER, &Config);\n")
    return

#deprecated
def Write_socat_handle_init(f, iface:dict):
    f.write(f"\t/* {iface['name']} Init */\n")
    f.write(f"\tStatus = CFE_SRL_HandleInit(&Handles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER], \"{iface['name']}\", \"{iface['DevName']}\", SRL_DEVTYPE_UART, CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER, 115200, 0);\n")

def Write_gpio_init(f, iface:dict):
    f.write(f"\t/* GPIO {iface['name']} Init */\n")
    if iface['direction'] == 'in':
        f.write(f"\tStatus = CFE_SRL_GpioInit(&GPIO[CFE_SRL_{iface['name'].upper()}_GPIO_INDEXER], \"{iface['DevName']}\", {iface['line']}, \"{iface['name']}\", {iface['default']}, false);\n")
    elif iface['direction'] == 'out':
        f.write(f"\tStatus = CFE_SRL_GpioInit(&GPIO[CFE_SRL_{iface['name'].upper()}_GPIO_INDEXER], \"{iface['DevName']}\", {iface['line']}, \"{iface['name']}\", {iface['default']}, true);\n")
    else:
        raise Exception("GPIO direction is not proper. Check again the setting. It should be \"in\" or \"out\"")
    return

def Get_Serial_module_cfg(path:str) -> dict:
    with open(path, "r") as f:
        content = f.read(-1)
    pattern = re.compile(r"#define\s+(CFE_SRL_GLOBAL_HANDLE_NUM|CFE_SRL_CSP_MAX_DEVICE_NUM)\s+(\d+)")

    value = {}

    for match in pattern.finditer(content):
        macro_name = match.group(1)
        macro_value = match.group(2)
        value[macro_name] = macro_value

    return value