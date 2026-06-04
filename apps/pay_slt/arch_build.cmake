###########################################################
#
# SLT_IFB_APP platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the SLT_IFB_APP configuration
set(SLT_IFB_PLATFORM_CONFIG_FILE_LIST
  SLT_IFB_internal_cfg.h
  SLT_IFB_platform_cfg.h
  SLT_IFB_perfids.h
  SLT_IFB_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(SLT_IFB_CFGFILE ${SLT_IFB_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${SLT_IFB_CFGFILE}" NAME_WE)
  if (DEFINED SLT_IFB_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${SLT_IFB_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${SLT_IFB_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${SLT_IFB_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()