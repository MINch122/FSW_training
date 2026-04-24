###########################################################
#
# RF Core Module mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the RF configuration
set(RF_MISSION_CONFIG_FILE_LIST
  cfe_rf_extern_typedefs.h
  cfe_rf_interface_cfg.h
  cfe_rf_mission_cfg.h
  cfe_rf_msgids.h
  cfe_rf_topicids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(RF_CFGFILE ${RF_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${RF_CFGFILE}" NAME_WE)
  if (DEFINED RF_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${RF_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${RF_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${RF_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()