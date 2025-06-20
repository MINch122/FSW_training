import sys
import json

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
    f.write(f"\tStatus = CFE_SRL_HandleInit(&Handles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER], \"{iface['name']}\", \"{iface['DevName']}\", SRL_DEVTYPE_{iface['type'].upper()}, CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER, 0, 0);\n")
    return

def Write_spi_handle_init(f, iface:dict):
    f.write(f"\t/* {iface['name']} Init */\n")
    f.write(f"\tStatus = CFE_SRL_HandleInit(&Handles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER], \"{iface['name']}\", \"{iface['DevName']}\", SRL_DEVTYPE_{iface['type'].upper()}, CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER, 0, 0);\n")
    return

def Write_uart_handle_init(f, iface:dict):
    f.write(f"\t/* {iface['name']} Init */\n")
    f.write(f"\tStatus = CFE_SRL_HandleInit(&Handles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER], \"{iface['name']}\", \"{iface['DevName']}\", SRL_DEVTYPE_{iface['type'].upper()}, CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER, {iface['baudrate']}, 0);\n")
    return

def Write_can_handle_init(f, iface:dict):
    f.write(f"\t/* {iface['name']} Init */\n")
    f.write(f"\tStatus = CFE_SRL_HandleInit(&Handles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER], \"{iface['name']}\", \"{iface['DevName']}\", SRL_DEVTYPE_{iface['type'].upper()}, CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER, 0, 0);\n")
    return

def Write_socat_handle_init(f, iface:dict):
    f.write(f"\t/* {iface['name']} Init */\n")
    f.write(f"\tStatus = CFE_SRL_HandleInit(&Handles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER], \"{iface['name']}\", \"{iface['DevName']}\", SRL_DEVTYPE_UART, CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER, 115200, 0);\n")

def Write_gpio_init(f, iface:dict):
    f.write(f"\t/* GPIO {iface['name']} Init */\n")
    f.write(f"\tStatus = CFE_SRL_GpioInit(&GPIO[CFE_SRL_{iface['name'].upper()}_GPIO_INDEXER], \"{iface['DevName']}\", {iface['line']}, \"{iface['name']}\", {iface['default']});\n")
    return

def Write_spi_handle_init(f, iface:dict):
    f.write(f"\t/* {iface['name']} Init */\n")
    f.write(f"\tStatus = CFE_SRL_HandleInit(&Handles[CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER], \"{iface['name']}\", \"{iface['DevName']}\", SRL_DEVTYPE_SPI, CFE_SRL_{iface['name'].upper()}_HANDLE_INDEXER, {iface['speed']}, {iface['mode']});\n")