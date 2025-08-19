###########################################################
#
# SANT mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the SANT configuration
set(SANT_MISSION_CONFIG_FILE_LIST
  sant_fcncodes.h
  sant_interface_cfg.h
  sant_mission_cfg.h
  sant_perfids.h
  sant_msg.h
  sant_msgdefs.h
  sant_msgstruct.h
  sant_topicids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(SANT_CFGFILE ${SANT_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${SANT_CFGFILE}" NAME_WE)
  if (DEFINED SANT_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${SANT_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${SANT_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${SANT_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
