###########################################################
#
# LGBAT mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the LGBAT configuration
set(LGBAT_MISSION_CONFIG_FILE_LIST
  lgbat_fcncodes.h
  lgbat_interface_cfg.h
  lgbat_mission_cfg.h
  lgbat_perfids.h
  lgbat_msg.h
  lgbat_msgdefs.h
  lgbat_msgstruct.h
  lgbat_topicids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(LGBAT_CFGFILE ${LGBAT_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${LGBAT_CFGFILE}" NAME_WE)
  if (DEFINED LGBAT_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${LGBAT_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${LGBAT_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${LGBAT_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
