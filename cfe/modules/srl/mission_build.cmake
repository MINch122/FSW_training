###########################################################
#
# SRL Core Module mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################
message(STATUS "srl mission_build start")
# The list of header files that control the SRL configuration
set(SRL_MISSION_CONFIG_FILE_LIST
  cfe_srl_mission_cfg.h
  cfe_srl_interface_cfg.h
  cfe_srl_extern_typedefs.h
  cfe_srl_fcncodes.h
  cfe_srl_msgdefs.h
  cfe_srl_msg.h
  cfe_srl_msgstruct.h
  cfe_srl_topicids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(SRL_CFGFILE ${SRL_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${SRL_CFGFILE}" NAME_WE)
  if (DEFINED SRL_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${SRL_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${SRL_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${SRL_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()

message(STATUS "srl mission_build end")