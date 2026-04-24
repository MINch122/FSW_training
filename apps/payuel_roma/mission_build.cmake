###########################################################
#
# PAYUEL_ROMA mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the PAYUEL_ROMA configuration
set(PAYUEL_ROMA_MISSION_CONFIG_FILE_LIST
  payuel_roma_fcncodes.h
  payuel_roma_interface_cfg.h
  payuel_roma_mission_cfg.h
  payuel_roma_perfids.h
  payuel_roma_msg.h
  payuel_roma_msgdefs.h
  payuel_roma_msgstruct.h
  payuel_roma_tbl.h
  payuel_roma_tbldefs.h
  payuel_roma_tblstruct.h
  payuel_roma_topicids.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(PAYUEL_ROMA_CFGFILE_SRC_PAYUEL_ROMA_interface_cfg "PAYUEL_ROMA_eds_designparameters.h")
  set(PAYUEL_ROMA_CFGFILE_SRC_PAYUEL_ROMA_tbldefs       "PAYUEL_ROMA_eds_typedefs.h")
  set(PAYUEL_ROMA_CFGFILE_SRC_PAYUEL_ROMA_tblstruct     "PAYUEL_ROMA_eds_typedefs.h")
  set(PAYUEL_ROMA_CFGFILE_SRC_PAYUEL_ROMA_msgdefs       "PAYUEL_ROMA_eds_typedefs.h")
  set(PAYUEL_ROMA_CFGFILE_SRC_PAYUEL_ROMA_msgstruct     "PAYUEL_ROMA_eds_typedefs.h")
  set(PAYUEL_ROMA_CFGFILE_SRC_PAYUEL_ROMA_fcncodes      "PAYUEL_ROMA_eds_cc.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(PAYUEL_ROMA_CFGFILE ${PAYUEL_ROMA_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${PAYUEL_ROMA_CFGFILE}" NAME_WE)
  if (DEFINED PAYUEL_ROMA_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${PAYUEL_ROMA_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${PAYUEL_ROMA_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${PAYUEL_ROMA_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
