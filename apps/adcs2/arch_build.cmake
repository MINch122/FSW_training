###########################################################
#
# ADCS2_APP platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the ADCS2_APP configuration
set(ADCS2_APP_PLATFORM_CONFIG_FILE_LIST
  adcs2_internal_cfg.h
  adcs2_platform_cfg.h
  adcs2_perfids.h
  adcs2_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(ADCS2_APP_CFGFILE ${ADCS2_APP_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${ADCS2_APP_CFGFILE}" NAME_WE)
  if (DEFINED ADCS2_APP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${ADCS2_APP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${ADCS2_APP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${ADCS2_APP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
