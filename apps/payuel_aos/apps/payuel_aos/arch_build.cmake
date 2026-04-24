###########################################################
#
# PAYUEL_AOS platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the PAYUEL_AOS configuration
set(PAYUEL_AOS_PLATFORM_CONFIG_FILE_LIST
  payuel_aos_internal_cfg.h
  payuel_aos_platform_cfg.h
  payuel_aos_perfids.h
  payuel_aos_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(PAYUEL_AOS_CFGFILE ${PAYUEL_AOS_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${PAYUEL_AOS_CFGFILE}" NAME_WE)
  if (DEFINED PAYUEL_AOS_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${PAYUEL_AOS_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${PAYUEL_AOS_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${PAYUEL_AOS_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
