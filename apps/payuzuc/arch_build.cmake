###########################################################
#
# PAYUZUC platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the PAYUZUC configuration
set(PAYUZUC_PLATFORM_CONFIG_FILE_LIST
  payuzuc_internal_cfg.h
  payuzuc_platform_cfg.h
  payuzuc_perfids.h
  payuzuc_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(PAYUZUC_CFGFILE ${PAYUZUC_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${PAYUZUC_CFGFILE}" NAME_WE)
  if (DEFINED PAYUZUC_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${PAYUZUC_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${PAYUZUC_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${PAYUZUC_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
