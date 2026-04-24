###########################################################
#
# UANT mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the UANT configuration
set(UANT_MISSION_CONFIG_FILE_LIST
  uant_fcncodes.h
  uant_interface_cfg.h
  uant_mission_cfg.h
  uant_perfids.h
  uant_msg.h
  uant_msgdefs.h
  uant_msgstruct.h
  uant_topicids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(UANT_CFGFILE ${UANT_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${UANT_CFGFILE}" NAME_WE)
  if (DEFINED UANT_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${UANT_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${UANT_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${UANT_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
