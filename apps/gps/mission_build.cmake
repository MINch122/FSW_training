###########################################################
#
# GPS mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the GPS configuration
set(GPS_MISSION_CONFIG_FILE_LIST
  gps_fcncodes.h
  gps_interface_cfg.h
  gps_mission_cfg.h
  gps_perfids.h
  gps_msg.h
  gps_msgdefs.h
  gps_msgstruct.h
  gps_topicids.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(GPS_CFGFILE_SRC_gps_interface_cfg "gps_eds_designparameters.h")
  set(GPS_CFGFILE_SRC_gps_tbldefs       "gps_eds_typedefs.h")
  set(GPS_CFGFILE_SRC_gps_tblstruct     "gps_eds_typedefs.h")
  set(GPS_CFGFILE_SRC_gps_msgdefs       "gps_eds_typedefs.h")
  set(GPS_CFGFILE_SRC_gps_msgstruct     "gps_eds_typedefs.h")
  set(GPS_CFGFILE_SRC_gps_fcncodes      "gps_eds_cc.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(GPS_CFGFILE ${GPS_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${GPS_CFGFILE}" NAME_WE)
  if (DEFINED GPS_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${GPS_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${GPS_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${GPS_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
