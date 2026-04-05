###########################################################
#
# STX mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the STX configuration
set(STX_MISSION_CONFIG_FILE_LIST
  stx_fcncodes.h
  stx_interface_cfg.h
  stx_mission_cfg.h
  stx_perfids.h
  stx_msg.h
  stx_msgdefs.h
  stx_msgstruct.h
  stx_tbl.h
  stx_tbldefs.h
  stx_tblstruct.h
  stx_topicids.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(STX_CFGFILE_SRC_stx_interface_cfg "stx_eds_designparameters.h")
  set(STX_CFGFILE_SRC_stx_tbldefs       "stx_eds_typedefs.h")
  set(STX_CFGFILE_SRC_stx_tblstruct     "stx_eds_typedefs.h")
  set(STX_CFGFILE_SRC_stx_msgdefs       "stx_eds_typedefs.h")
  set(STX_CFGFILE_SRC_stx_msgstruct     "stx_eds_typedefs.h")
  set(STX_CFGFILE_SRC_stx_fcncodes      "stx_eds_cc.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(STX_CFGFILE ${STX_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${STX_CFGFILE}" NAME_WE)
  if (DEFINED STX_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${STX_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${STX_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${STX_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
