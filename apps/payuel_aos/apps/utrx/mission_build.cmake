###########################################################
#
# UTRX mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the UTRX configuration
set(UTRX_MISSION_CONFIG_FILE_LIST
  utrx_fcncodes.h
  utrx_interface_cfg.h
  utrx_mission_cfg.h
  utrx_perfids.h
  utrx_msg.h
  utrx_msgdefs.h
  utrx_msgstruct.h
  utrx_topicids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(UTRX_CFGFILE ${UTRX_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${UTRX_CFGFILE}" NAME_WE)
  if (DEFINED UTRX_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${UTRX_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${UTRX_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${UTRX_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
