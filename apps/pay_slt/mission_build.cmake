###########################################################
#
# SLT_IFB mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the SAMPLE_APP configuration
set(SLT_IFB_MISSION_CONFIG_FILE_LIST
  SLT_IFB_fcncodes.h
  SLT_IFB_interface_cfg.h
  SLT_IFB_mission_cfg.h
  SLT_IFB_perfids.h
  SLT_IFB_msg.h
  SLT_IFB_msgdefs.h
  SLT_IFB_msgstruct.h
  SLT_IFB_tbl.h
  SLT_IFB_tbldefs.h
  SLT_IFB_tblstruct.h
  SLT_IFB_topicids.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(SLT_IFB_CFGFILE_SRC_SLT_IFB_interface_cfg "SLT_IFB_eds_designparameters.h")
  set(SLT_IFB_CFGFILE_SRC_SLT_IFB_tbldefs       "SLT_IFB_eds_typedefs.h")
  set(SLT_IFB_CFGFILE_SRC_SLT_IFB_tblstruct     "SLT_IFB_eds_typedefs.h")
  set(SLT_IFB_CFGFILE_SRC_SLT_IFB_msgdefs       "SLT_IFB_eds_typedefs.h")
  set(SLT_IFB_CFGFILE_SRC_SLT_IFB_msgstruct     "SLT_IFB_eds_typedefs.h")
  set(SLT_IFB_CFGFILE_SRC_SLT_IFB_fcncodes      "SLT_IFB_eds_cc.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(SLT_IFB_CFGFILE ${SLT_IFB_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${SLT_IFB_CFGFILE}" NAME_WE)
  if (DEFINED SLT_IFB_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${SLT_IFB_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${SLT_IFB_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${SLT_IFB_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()