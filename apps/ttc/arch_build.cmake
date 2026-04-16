###########################################################
#
# TTC platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the TTC configuration
set(TTC_PLATFORM_CONFIG_FILE_LIST
  ttc_internal_cfg_values.h
  ttc_platform_cfg.h
  ttc_perfids.h
  ttc_msgids.h
  ttc_msgid_values.h
)

# Create wrappers around all config header files so they can be
# individually overridden without modifying the distribution defaults.
foreach(TTC_CFGFILE ${TTC_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${TTC_CFGFILE}" NAME_WE)
  if (DEFINED TTC_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${TTC_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${TTC_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${TTC_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
