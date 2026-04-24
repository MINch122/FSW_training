###########################################################
#
# STX platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the STX configuration
set(STX_PLATFORM_CONFIG_FILE_LIST
  stx_internal_cfg.h
  stx_platform_cfg.h
  stx_perfids.h
  stx_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(STX_CFGFILE ${STX_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${STX_CFGFILE}" NAME_WE)
  if (DEFINED STX_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${STX_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${STX_CFGFILE}")
  endif()

  
  generate_config_includefile(
    FILE_NAME           "${STX_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
