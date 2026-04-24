###########################################################
#
# PAYUEL_OBC mission build setup
#
###########################################################

set(PAYUEL_OBC_MISSION_CONFIG_FILE_LIST
  payuel_obc_fcncodes.h
  payuel_obc_interface_cfg.h
  payuel_obc_mission_cfg.h
  payuel_obc_perfids.h
  payuel_obc_msg.h
  payuel_obc_msgdefs.h
  payuel_obc_msgstruct.h
  payuel_obc_tbl.h
  payuel_obc_tbldefs.h
  payuel_obc_tblstruct.h
  payuel_obc_topicids.h
)

foreach(PAYUEL_OBC_CFGFILE ${PAYUEL_OBC_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${PAYUEL_OBC_CFGFILE}" NAME_WE)
  if (DEFINED PAYUEL_OBC_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${PAYUEL_OBC_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${PAYUEL_OBC_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${PAYUEL_OBC_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
