###########################################################
#
# SLT FTP mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the FTP configuration
set(FTP_MISSION_CONFIG_FILE_LIST
  ftp_fcncodes.h
  ftp_interface_cfg.h
  ftp_mission_cfg.h
  ftp_perfids.h
  ftp_msg.h
  ftp_msgdefs.h
  ftp_msgstruct.h
  ftp_tbl.h
  ftp_tbldefs.h
  ftp_tblstruct.h
  ftp_topicids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(FTP_CFGFILE ${FTP_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${FTP_CFGFILE}" NAME_WE)
  if (DEFINED FTP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${FTP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${FTP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${FTP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
