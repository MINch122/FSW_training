###########################################################
#
# PAYUEL_CAM platform build setup
#
###########################################################

set(PAYUEL_CAM_PLATFORM_CONFIG_FILE_LIST
  payuel_cam_internal_cfg.h
  payuel_cam_platform_cfg.h
  payuel_cam_perfids.h
  payuel_cam_msgids.h
)

foreach(PAYUEL_CAM_CFGFILE ${PAYUEL_CAM_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${PAYUEL_CAM_CFGFILE}" NAME_WE)
  if (DEFINED PAYUEL_CAM_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${PAYUEL_CAM_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${PAYUEL_CAM_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${PAYUEL_CAM_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
