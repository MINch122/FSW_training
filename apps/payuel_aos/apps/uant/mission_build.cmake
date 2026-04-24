###########################################################
#
# UANT_APP mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the UANT_APP configuration
set(UANT_APP_MISSION_CONFIG_FILE_LIST
  uant_app_fcncodes.h
  uant_app_interface_cfg.h
  uant_app_mission_cfg.h
  uant_app_perfids.h
  uant_app_msg.h
  uant_app_msgdefs.h
  uant_app_msgstruct.h
  uant_app_tbl.h
  uant_app_tbldefs.h
  uant_app_tblstruct.h
  uant_app_topicids.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(UANT_APP_CFGFILE_SRC_uant_app_interface_cfg "uant_app_eds_designparameters.h")
  set(UANT_APP_CFGFILE_SRC_uant_app_tbldefs       "uant_app_eds_typedefs.h")
  set(UANT_APP_CFGFILE_SRC_uant_app_tblstruct     "uant_app_eds_typedefs.h")
  set(UANT_APP_CFGFILE_SRC_uant_app_msgdefs       "uant_app_eds_typedefs.h")
  set(UANT_APP_CFGFILE_SRC_uant_app_msgstruct     "uant_app_eds_typedefs.h")
  set(UANT_APP_CFGFILE_SRC_uant_app_fcncodes      "uant_app_eds_cc.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(UANT_APP_CFGFILE ${UANT_APP_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${UANT_APP_CFGFILE}" NAME_WE)
  if (DEFINED UANT_APP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${UANT_APP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${UANT_APP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${UANT_APP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
