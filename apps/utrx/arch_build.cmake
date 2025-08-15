###########################################################
#
# UTRX_APP platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the UTRX_APP configuration
set(UTRX_APP_PLATFORM_CONFIG_FILE_LIST
  utrx_app_internal_cfg.h
  utrx_app_platform_cfg.h
  utrx_app_perfids.h
  utrx_app_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(UTRX_APP_CFGFILE ${UTRX_APP_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${UTRX_APP_CFGFILE}" NAME_WE)
  if (DEFINED UTRX_APP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${UTRX_APP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${UTRX_APP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${UTRX_APP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
