###########################################################
#
# FM mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the FM configuration
set(FM_MISSION_CONFIG_FILE_LIST
  fm_extern_typedefs.h
  fm_mission_cfg.h
  fm_fcncodes.h
  fm_perfids.h
  fm_msg.h
  fm_msgdefs.h
  fm_msgstruct.h
  fm_topicids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(FM_CFGFILE ${FM_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${FM_CFGFILE}" NAME_WE)
  if (DEFINED FM_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${FM_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${FM_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${FM_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
