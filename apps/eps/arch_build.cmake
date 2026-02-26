###########################################################
#
# EPS platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the EPS configuration
set(EPS_PLATFORM_CONFIG_FILE_LIST
  eps_internal_cfg.h
  eps_platform_cfg.h
  eps_perfids.h
  eps_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(EPS_CFGFILE ${EPS_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${EPS_CFGFILE}" NAME_WE)
  if (DEFINED EPS_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${EPS_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${EPS_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${EPS_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
