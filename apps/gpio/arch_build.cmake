###########################################################
#
# GPIO platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the GPIO configuration
set(GPIO_PLATFORM_CONFIG_FILE_LIST
  gpio_internal_cfg.h
  gpio_platform_cfg.h
  gpio_perfids.h
  gpio_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(GPIO_CFGFILE ${GPIO_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${GPIO_CFGFILE}" NAME_WE)
  if (DEFINED GPIO_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${GPIO_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${GPIO_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${GPIO_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
