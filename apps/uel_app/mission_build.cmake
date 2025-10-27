###########################################################
#
# UEL_APP mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the UEL_APP configuration
set(UEL_APP_MISSION_CONFIG_FILE_LIST
  uel_app_fcncodes.h
  uel_app_interface_cfg.h
  uel_app_mission_cfg.h
  uel_app_perfids.h
  uel_app_msg.h
  uel_app_msgdefs.h
  uel_app_msgstruct.h
  uel_app_tbl.h
  uel_app_tbldefs.h
  uel_app_tblstruct.h
  uel_app_topicids.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(UEL_APP_CFGFILE_SRC_uel_app_interface_cfg "uel_app_eds_designparameters.h")
  set(UEL_APP_CFGFILE_SRC_uel_app_tbldefs       "uel_app_eds_typedefs.h")
  set(UEL_APP_CFGFILE_SRC_uel_app_tblstruct     "uel_app_eds_typedefs.h")
  set(UEL_APP_CFGFILE_SRC_uel_app_msgdefs       "uel_app_eds_typedefs.h")
  set(UEL_APP_CFGFILE_SRC_uel_app_msgstruct     "uel_app_eds_typedefs.h")
  set(UEL_APP_CFGFILE_SRC_uel_app_fcncodes      "uel_app_eds_cc.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(UEL_APP_CFGFILE ${UEL_APP_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${UEL_APP_CFGFILE}" NAME_WE)
  if (DEFINED UEL_APP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${UEL_APP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${UEL_APP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${UEL_APP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
