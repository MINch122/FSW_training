###########################################################
#
# SP mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the SP configuration
set(SP_MISSION_CONFIG_FILE_LIST
  sp_fcncodes.h
  sp_interface_cfg.h
  sp_mission_cfg.h
  sp_perfids.h
  sp_msg.h
  sp_msgdefs.h
  sp_msgstruct.h
  sp_topicids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(SP_CFGFILE ${SP_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${SP_CFGFILE}" NAME_WE)
  if (DEFINED SP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${SP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${SP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${SP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
