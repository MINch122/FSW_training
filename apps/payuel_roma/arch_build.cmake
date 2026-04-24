###########################################################
#
# PAYUEL_ROMA platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the PAYUEL_ROMA configuration
set(PAYUEL_ROMA_PLATFORM_CONFIG_FILE_LIST
  payuel_roma_internal_cfg.h
  payuel_roma_platform_cfg.h
  payuel_roma_perfids.h
  payuel_roma_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(PAYUEL_ROMA_CFGFILE ${PAYUEL_ROMA_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${PAYUEL_ROMA_CFGFILE}" NAME_WE)
  if (DEFINED PAYUEL_ROMA_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${PAYUEL_ROMA_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${PAYUEL_ROMA_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${PAYUEL_ROMA_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
