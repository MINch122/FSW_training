###########################################################
#
# EPS mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the EPS configuration
set(EPS_MISSION_CONFIG_FILE_LIST
  eps_fcncodes.h
  eps_interface_cfg.h
  eps_mission_cfg.h
  eps_perfids.h
  eps_msg.h
  eps_msgdefs.h
  eps_msgstruct.h
  eps_topicids.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(EPS_CFGFILE_SRC_eps_interface_cfg "eps_eds_designparameters.h")
  set(EPS_CFGFILE_SRC_eps_tbldefs       "eps_eds_typedefs.h")
  set(EPS_CFGFILE_SRC_eps_tblstruct     "eps_eds_typedefs.h")
  set(EPS_CFGFILE_SRC_eps_msgdefs       "eps_eds_typedefs.h")
  set(EPS_CFGFILE_SRC_eps_msgstruct     "eps_eds_typedefs.h")
  set(EPS_CFGFILE_SRC_eps_fcncodes      "eps_eds_cc.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(EPS_CFGFILE ${EPS_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${EPS_CFGFILE}" NAME_WE)
  if (DEFINED EPS_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${EPS_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${EPS_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${EPS_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
