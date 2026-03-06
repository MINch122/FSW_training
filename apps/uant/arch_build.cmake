###########################################################
#
# UANT_APP platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the UANT_APP configuration
set(UANT_APP_PLATFORM_CONFIG_FILE_LIST
  uant_app_internal_cfg.h
  uant_app_platform_cfg.h
  uant_app_perfids.h
  uant_app_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(UANT_APP_CFGFILE ${UANT_APP_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${UANT_APP_CFGFILE}" NAME_WE)
  if (DEFINED UANT_APP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${UANT_APP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${UANT_APP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${UANT_APP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
