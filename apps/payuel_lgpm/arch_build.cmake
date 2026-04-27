###########################################################
#
# PAYUEL_LGPM platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the PAYUEL_LGPM configuration
set(PAYUEL_LGPM_PLATFORM_CONFIG_FILE_LIST
  PAYUEL_LGPM_internal_cfg.h
  PAYUEL_LGPM_platform_cfg.h
  PAYUEL_LGPM_perfids.h
  PAYUEL_LGPM_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(PAYUEL_LGPM_CFGFILE ${PAYUEL_LGPM_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${PAYUEL_LGPM_CFGFILE}" NAME_WE)
  if (DEFINED PAYUEL_LGPM_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${PAYUEL_LGPM_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${PAYUEL_LGPM_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${PAYUEL_LGPM_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
