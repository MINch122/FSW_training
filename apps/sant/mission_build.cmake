###########################################################
#
# SANT_APP mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the SANT_APP configuration
set(SANT_APP_MISSION_CONFIG_FILE_LIST
  sant_app_fcncodes.h
  sant_app_interface_cfg.h
  sant_app_mission_cfg.h
  sant_app_perfids.h
  sant_app_msg.h
  sant_app_msgdefs.h
  sant_app_msgstruct.h
  sant_app_topicids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(SANT_APP_CFGFILE ${SANT_APP_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${SANT_APP_CFGFILE}" NAME_WE)
  if (DEFINED SANT_APP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${SANT_APP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${SANT_APP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${SANT_APP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
