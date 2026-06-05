###########################################################
#
# LTRX platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the LTRX configuration
set(LTRX_PLATFORM_CONFIG_FILE_LIST
  ltrx_internal_cfg.h
  ltrx_platform_cfg.h
  ltrx_perfids.h
  ltrx_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(LTRX_CFGFILE ${LTRX_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${LTRX_CFGFILE}" NAME_WE)
  if (DEFINED LTRX_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${LTRX_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${LTRX_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${LTRX_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
