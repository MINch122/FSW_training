# About **Serial Config tool**
- Author: Kweon Hyeok-jin

# Overview
- This tool is a user-friendly configuration tool for mission-specific serial communication interfaces
- This tool automatically generated some source code
    - `default_cfe_srl_mission_cfg.h` in `/cfe/modules/srl/config/`
    - `cfe_srl_init.c` in `/cfe/modules/srl/fsw/src/`
    - `cfe_srl_error.h` in `cfe/modules/core_api/fsw/inc`
    - `cfe_srl_csp_config.c` in `/cfe/modules/srl/fsw/src/`
- This tool reads `Interface_config.json` & `csp_config.json` which determines the serial interface configuration
    - `Interface_config.json` is in `cfe/modules/srl/`
    
# How to use
- End user just revises `Interface_config.json` & `csp_config.json` only
    - The **former is for Non-CSP** interface, the **latter is for CSP interface**
- After revision, manually execute `generator.py` in `tools/serial-configtool/`
- Or, just run the build script from `make prep`

# Edit `.json` file
## Non-CSP interface
- Every interface are single element of **one list which is the value of key "interfaces"**
- So, the end user just revises each element like
```json
{
    "type": "i2c",
    "ready": false,
    "name": "I2C0",
    "DevName": "/dev/i2c-0",
    "MutexID": 0
}
```
### Common element
- Every interface (i.e. protocol) has common keys
    1. type : protocol type. MUST type in **lower case**
    2. ready : readiness flag. MUST type in **bool**
    3. name : Nickname of each interface. MUST type **device file name in upper case**
    4. DevName : Device file path. Must type the exact same file path
    5. MutexID : **Deprecated**
- Some interfaces have unique keys
### UART series
1. baudrate
### GPIO
1. line : the line number of each gpiochip
2. default : default value of gpio. Must type `1` or `0`
3. direction (NOT supported now): default gpio direction Must type `in` or `out`

## CSP interface
- This json has two kinds of elements
    - One is for the "host" (mostly OBC), the other is for the "external" CSP devices communicate with "host" via CSP
### Host
- There is many elements in the "host"
    1. "address": Node value of the "host". MUST type in **int**
    2. "hostname": MUST type in **string**
    2. "model": MUST type in **string**
    2. "revision": MUST type in **string**
    2. "conn_dfl_so": MUST type in **string** and should be matched CSP source code format
        - Default value is `"CSP_O_NONE"`
    - For more details, please refer the `libgscsp` source code
### External
- There is 7 elements in the "external"
    1. "name": CSP device name. MUST type in **string** and **upper case**
    2. "node": CSP Node number. MUST type in **int**
    3. "interface": CSP comm. interface. This can be `"CAN"` or `"I2C"`. **`"I2C"` is not supported.**
    4. "via": via address. If there is no via, type `null`
    5. "priority": Communication priority. MUST type in **0, 1, 2, 3**. Lower value is higher priority
    6. "timeout": Communication Timeout. MUST type in **int** in **sec**
    7. "option": MUST type in **string** and should be matched CSP source code format
    - For more details, please refer the `libgscsp` source code