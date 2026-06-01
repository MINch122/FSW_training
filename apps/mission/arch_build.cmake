###########################################################
#
# MISSION platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the MISSION configuration
set(MISSION_PLATFORM_CONFIG_FILE_LIST
  mission_internal_cfg.h
  mission_platform_cfg.h
  mission_perfids.h
  mission_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(MISSION_CFGFILE ${MISSION_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${MISSION_CFGFILE}" NAME_WE)
  if (DEFINED MISSION_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${MISSION_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${MISSION_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${MISSION_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
