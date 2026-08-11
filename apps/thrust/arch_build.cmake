###########################################################
#
# THRUST platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the THRUST configuration
set(THRUST_PLATFORM_CONFIG_FILE_LIST
  thrust_internal_cfg.h
  thrust_platform_cfg.h
  thrust_perfids.h
  thrust_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(THRUST_CFGFILE ${THRUST_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${THRUST_CFGFILE}" NAME_WE)
  if (DEFINED THRUST_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${THRUST_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${THRUST_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${THRUST_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
