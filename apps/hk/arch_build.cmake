###########################################################
#
# HK App platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the app configuration
set(HK_PLATFORM_CONFIG_FILE_LIST
  # hk_internal_cfg.h
  hk_msgids.h
  hk_platform_cfg.h
  hk_perfids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(HK_CFGFILE ${HK_PLATFORM_CONFIG_FILE_LIST})
   get_filename_component(CFGKEY "${HK_CFGFILE}" NAME_WE) 
   if (DEFINED HK_CFGFILE_SRC_${CFGKEY}) 
     set(DEFAULT_SOURCE GENERATED_FILE "${HK_CFGFILE_SRC_${CFGKEY}}")
   else() 
     set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${HK_CFGFILE}")
   endif()

  generate_config_includefile(
    FILE_NAME           "${HK_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
