###########################################################
#
# FTP_CLIENT mission build setup
#
###########################################################

set(FTP_CLIENT_MISSION_CONFIG_FILE_LIST
  ftp_client_fcncodes.h
  ftp_client_interface_cfg.h
  ftp_client_internal_cfg.h
  ftp_client_mission_cfg.h
  ftp_client_msg.h
  ftp_client_msgdefs.h
  ftp_client_msgids.h
  ftp_client_msgstruct.h
  ftp_client_perfids.h
  ftp_client_platform_cfg.h
  ftp_client_topicids.h
)

foreach(FTP_CLIENT_CFGFILE ${FTP_CLIENT_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${FTP_CLIENT_CFGFILE}" NAME_WE)
  if (DEFINED FTP_CLIENT_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${FTP_CLIENT_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${FTP_CLIENT_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME "${FTP_CLIENT_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
