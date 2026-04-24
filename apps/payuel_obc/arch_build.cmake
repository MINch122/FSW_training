###########################################################
#
# PAYUEL_OBC platform build setup
#
###########################################################

set(PAYUEL_OBC_PLATFORM_CONFIG_FILE_LIST
  payuel_obc_internal_cfg.h
  payuel_obc_platform_cfg.h
  payuel_obc_perfids.h
  payuel_obc_msgids.h
)

foreach(PAYUEL_OBC_CFGFILE ${PAYUEL_OBC_PLATFORM_CONFIG_FILE_LIST})
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
