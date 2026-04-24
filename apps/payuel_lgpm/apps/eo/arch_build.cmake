###########################################################
#
# EO platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the EO configuration
set(EO_PLATFORM_CONFIG_FILE_LIST
  eo_internal_cfg.h
  eo_platform_cfg.h
  eo_perfids.h
  eo_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(EO_CFGFILE ${EO_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${EO_CFGFILE}" NAME_WE)
  if (DEFINED EO_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${EO_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${EO_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${EO_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
