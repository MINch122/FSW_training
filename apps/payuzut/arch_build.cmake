###########################################################
#
# PAYUZUT platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the PAYUZUT configuration
set(PAYUZUT_PLATFORM_CONFIG_FILE_LIST
  payuzut_internal_cfg.h
  payuzut_platform_cfg.h
  payuzut_perfids.h
  payuzut_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(PAYUZUT_CFGFILE ${PAYUZUT_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${PAYUZUT_CFGFILE}" NAME_WE)
  if (DEFINED PAYUZUT_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${PAYUZUT_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${PAYUZUT_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${PAYUZUT_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
