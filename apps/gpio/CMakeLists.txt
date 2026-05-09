project(CFE_GPIO C)

set(APP_SRC_FILES
  fsw/src/gpio.c
  fsw/src/gpio_cmds.c
)

list(APPEND APP_SRC_FILES
    fsw/src/gpio_digpioatch.c
  )

# Create the app module
add_cfe_app(gpio ${APP_SRC_FILES})

target_include_directories(gpio PUBLIC fsw/inc)


