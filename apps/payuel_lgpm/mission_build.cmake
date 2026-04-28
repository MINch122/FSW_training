###########################################################
#
# PAYUEL_LGPM mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the PAYUEL_LGPM configuration
set(PAYUEL_LGPM_MISSION_CONFIG_FILE_LIST
  payuel_lgpm_fcncodes.h
  payuel_lgpm_interface_cfg.h
  payuel_lgpm_mission_cfg.h
  payuel_lgpm_perfids.h
  payuel_lgpm_msg.h
  payuel_lgpm_msgdefs.h
  payuel_lgpm_msgstruct.h
  payuel_lgpm_tbl.h
  payuel_lgpm_tbldefs.h
  payuel_lgpm_tblstruct.h
  payuel_lgpm_topicids.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(PAYUEL_LGPM_CFGFILE_SRC_payuel_lgpm_interface_cfg "payuel_lgpm_eds_designparameters.h")
  set(PAYUEL_LGPM_CFGFILE_SRC_payuel_lgpm_tbldefs       "payuel_lgpm_eds_typedefs.h")
  set(PAYUEL_LGPM_CFGFILE_SRC_payuel_lgpm_tblstruct     "payuel_lgpm_eds_typedefs.h")
  set(PAYUEL_LGPM_CFGFILE_SRC_payuel_lgpm_msgdefs       "payuel_lgpm_eds_typedefs.h")
  set(PAYUEL_LGPM_CFGFILE_SRC_payuel_lgpm_msgstruct     "payuel_lgpm_eds_typedefs.h")
  set(PAYUEL_LGPM_CFGFILE_SRC_payuel_lgpm_fcncodes      "payuel_lgpm_eds_cc.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(PAYUEL_LGPM_CFGFILE ${PAYUEL_LGPM_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${PAYUEL_LGPM_CFGFILE}" NAME_WE)
  if (DEFINED PAYUEL_LGPM_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${PAYUEL_LGPM_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${PAYUEL_LGPM_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${PAYUEL_LGPM_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
