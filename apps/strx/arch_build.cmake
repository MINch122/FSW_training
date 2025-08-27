###########################################################
#
# STRX platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the STRX configuration
set(STRX_PLATFORM_CONFIG_FILE_LIST
  strx_internal_cfg.h
  strx_platform_cfg.h
  strx_perfids.h
  strx_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(STRX_CFGFILE ${STRX_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${STRX_CFGFILE}" NAME_WE)
  if (DEFINED STRX_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${STRX_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${STRX_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${STRX_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
