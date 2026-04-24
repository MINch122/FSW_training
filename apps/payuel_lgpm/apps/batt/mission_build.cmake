###########################################################
#
# BATT mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the BATT configuration
set(BATT_MISSION_CONFIG_FILE_LIST
  batt_fcncodes.h
  batt_interface_cfg.h
  batt_mission_cfg.h
  batt_perfids.h
  batt_msg.h
  batt_msgdefs.h
  batt_msgstruct.h
  batt_topicids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(BATT_CFGFILE ${BATT_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${BATT_CFGFILE}" NAME_WE)
  if (DEFINED BATT_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${BATT_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${BATT_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${BATT_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
